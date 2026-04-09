#include "Allocator.h"

std::mutex Allocator::allocator_mutex;

void* Allocator::allocate(size_t numOfBytes)
{
    if (numOfBytes == 0) return nullptr;

    std::lock_guard<std::mutex> guard { allocator_mutex };
    for(byte* i = m_allocatedMemory.data(); 
        i < m_allocatedMemory.data() + m_allocatedMemory.size();
        i += HEADER + memoryBlockSize(i) + FOOTER)
    {
        // If not dirty, AND block size is atleast numOfBytes, then allocate.
        if(!isDirty(i) && memoryBlockSize(i) >= numOfBytes)
        {
            // Set dirty bit to 1 and size to numOfBytes in both header and footer.
            size_t originalSize = memoryBlockSize(i);
            /* CASE 1: Split block */
            if (originalSize >= numOfBytes + HEADER + FOOTER + 1)
            {
                // allocated block
                setSize(i, numOfBytes);
                setDirty(i);
                byte* allocatedFooter = i + HEADER + numOfBytes;
                setSize(allocatedFooter, numOfBytes);
                setDirty(allocatedFooter);
                // remaining free block
                byte* nextHeader = allocatedFooter + FOOTER;
                size_t nextSize = originalSize - numOfBytes - HEADER - FOOTER;
                setSize(nextHeader, nextSize);
                setClean(nextHeader);
                byte* nextFooter = nextHeader + HEADER + nextSize;
                setSize(nextFooter, nextSize);
                setClean(nextFooter);
            }
            /* CASE 2: take the entire block */
            else
            {
                setDirty(i);
                byte* footer = i + HEADER + originalSize;
                setDirty(footer);
            }

            return i + HEADER;
        }
    }
    // if we get here, there is no memory available to allocate.
    return nullptr;
}

void Allocator::free(byte* ptr)
{
    std::lock_guard<std::mutex> guard { allocator_mutex };

    if (!ptr) return;
    byte* header = ptr - HEADER;
    size_t size = memoryBlockSize(header);
    byte* footer = header + HEADER + size;
    if (!isDirty(header)) return;

    setClean(header);
    setClean(footer);

    // ---- backward coalescing ----
    if (header != m_allocatedMemory.data())
    {
        byte* prevFooter = header - FOOTER;

        if (!isDirty(prevFooter))
        {
            byte* prevHeader = prevFooter - memoryBlockSize(prevFooter);
            mergeBlocks(prevHeader, header);
            header = prevHeader;            // set current header to previous header.
        }
    }

    // ---- forward coalescing ----
    byte* nextHeader = header + HEADER + memoryBlockSize(header) + FOOTER;

    if (nextHeader < m_allocatedMemory.data() + m_allocatedMemory.size())
    {
        if (!isDirty(nextHeader))
        {
            mergeBlocks(header, nextHeader);
        }
    }
}


void Allocator::mergeBlocks(byte* leftHeader, byte* rightHeader)
{
    size_t leftSize  = memoryBlockSize(leftHeader);
    size_t rightSize = memoryBlockSize(rightHeader);

    size_t newSize = leftSize + HEADER + FOOTER + rightSize;

    byte* newFooter = rightHeader + HEADER + rightSize;

    setSize(leftHeader, newSize);
    setSize(newFooter, newSize);

    setClean(leftHeader);
    setClean(newFooter);
}