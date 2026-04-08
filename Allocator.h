#include <vector>



// Allocator contains bytes structured as follows:
// +----------------+---------------------------+----------------+
// |  8-byte header |        user data         |  8-byte footer |
// +----------------+---------------------------+----------------+


using byte = unsigned char;
class Allocator
{
public:
    explicit Allocator(size_t numOfBytes = 0) : m_allocatedMemory(HEADER + numOfBytes + FOOTER, 0)
    {
        setSize(m_allocatedMemory.data(), numOfBytes);
        setSize(m_allocatedMemory.data() + HEADER + numOfBytes, numOfBytes);
    }

    void* allocate(size_t numOfBytes)
    {
        for(byte* i = m_allocatedMemory.data(); i < m_allocatedMemory.data() + m_allocatedMemory.size();)
        {
            // If not dirty, AND block size is atleast numOfBytes, then allocate.
            if(!isDirty(i) && memoryBlockSize(i) >= numOfBytes)
            {
                // Set dirty bit to 1.
                setDirty(i);                            // Dirty bit of header.
                setDirty(i + HEADER + numOfBytes);      // DIrty bit of footer.
                byte* nextBlockAddress = i + HEADER + numOfBytes + FOOTER;

                // setting next block size and dirty bit, if it exists.
                if (nextBlockAddress < m_allocatedMemory.data() + m_allocatedMemory.size())
                {
                    // Set information of head and footer of next block. 
                    size_t nextBlockSize = memoryBlockSize(i) - numOfBytes - HEADER - FOOTER;

                    // Set head information.
                    setSize(nextBlockAddress, nextBlockSize);    
                    setClean(nextBlockAddress);         // Set dirty bit to 0.

                    // Set footer information.
                    setSize(nextBlockAddress + HEADER + nextBlockSize, nextBlockSize);  
                    *(reinterpret_cast<size_t*>(nextBlockAddress + HEADER + nextBlockSize)) = nextBlockSize;     
                    setClean(nextBlockAddress + HEADER + nextBlockSize); // Set dirty bit to 0.
                }
                
                return i + HEADER;                      // return pointer to block without the metadata section.
            }
            i +=  HEADER + memoryBlockSize(i) + FOOTER; // increment pointer to next memory block.
        }
        // if we get here, there is no memory available to allocate.
        return nullptr;
    }

    void free(byte* ptr)
    {
        byte* headerPtr = ptr - HEADER;
        byte* footerPtr = ptr + memoryBlockSize(headerPtr);
        byte* prevFooterPtr = headerPtr - FOOTER;
        byte* prevHeaderPtr = prevFooterPtr - memoryBlockSize(prevFooterPtr);

        // Set both header and footer of current memory block to clean.
        setClean(ptr);                                      // Header starting address.
        setClean(footerPtr);                                // Footer starting address.

        // Check if there is a memory block before current one. If there is, check if it's free. If it is, combine both free blocks into one block.
        if(headerPtr != m_allocatedMemory.data() && !isDirty(prevFooterPtr))        
        {

            // Set size of header of current memory block and size of footer of previous memory block to zero, since we don't need this metadata anymore.
            setSize(headerPtr, 0); 
            setSize(prevFooterPtr, 0); 
            setClean(prevFooterPtr);

            // Set size of marginal headers and footers to combined blocks size, including the footer and header of the separate blocks.
            size_t combinedSize = memoryBlockSize(prevFooterPtr) + FOOTER + HEADER + memoryBlockSize(headerPtr);
            setSize(footerPtr, combinedSize);
            setSize(prevHeaderPtr, combinedSize);
        }
    }


    /* Extra functions */
    void setSize(byte* metadataPtr, size_t blockSize) { *(reinterpret_cast<size_t*>(metadataPtr)) = blockSize; }
    void setClean(byte *metadataPtr) { *(reinterpret_cast<size_t*>(metadataPtr)) &= ~DIRTY_MASK; }
    void setDirty(byte* metadataPtr ) { *reinterpret_cast<size_t*>(metadataPtr) |= DIRTY_MASK;  }
    bool isDirty(byte* metadataPtr) { return *(reinterpret_cast<size_t*>(metadataPtr)) & DIRTY_MASK; }
    size_t memoryBlockSize(byte* metadataPtr) { return *(reinterpret_cast<size_t*>(metadataPtr)) & SIZE_MASK; }


    ~Allocator() { }  


private:

    // Static constants used by the allocator.
    static constexpr byte HEADER = 8;
    static constexpr byte FOOTER = 8;
	static constexpr unsigned long long DIRTY_MASK = 0x8000000000000000; // 1000 0000
	static constexpr unsigned long long SIZE_MASK = 0x7FFFFFFFFFFFFFFF; // 0111 1111

    // Allocated bytes.
    std::vector<byte> m_allocatedMemory;
};