#include <vector>



// Allocator contains bytes structured as follows:
// +----------------+---------------------------+----------------+
// |  8-byte header |        user data         |  8-byte footer |
// +----------------+---------------------------+----------------+


using byte = unsigned char;
class Allocator
{
public:
    explicit Allocator(size_t numOfBytes = HEADER + FOOTER) : m_allocatedMemory(numOfBytes, 0)
    {
        *reinterpret_cast<size_t*>(m_allocatedMemory.data()) = numOfBytes;
        *reinterpret_cast<size_t*>(m_allocatedMemory.data() + HEADER + numOfBytes) = numOfBytes;
    }
    void* allocate(size_t numOfBytes)
    {
        for(byte* i = m_allocatedMemory.data(); i < m_allocatedMemory.data() + m_allocatedMemory.size();)
        {
            // If not dirty, AND block size is atleast numOfBytes, then allocate.
            if((*(reinterpret_cast<size_t*>(i)) & DIRTY_MASK) == 0 && (*(reinterpret_cast<size_t*>(i)) & SIZE_MASK) >= numOfBytes)
            {
                // Set dirty bit to 1.
                *reinterpret_cast<size_t*>(i) |= DIRTY_MASK; 
                *reinterpret_cast<size_t*>(i + HEADER + numOfBytes) |= DIRTY_MASK;
                
                // setting next block size and dirty bit, if it exists.
                if (i + numOfBytes + HEADER + FOOTER + HEADER + FOOTER < m_allocatedMemory.data() + m_allocatedMemory.size())
                {
                    // Set information of head and footer of next block. 
                    byte* nextBlock = i + HEADER + numOfBytes + FOOTER;
                    size_t remainingSize = (*(reinterpret_cast<size_t*>(i)) & SIZE_MASK) - numOfBytes - HEADER - FOOTER;

                    // Set head information.
                    *(reinterpret_cast<size_t*>(nextBlock)) = remainingSize;     
                    *(reinterpret_cast<size_t*>(nextBlock)) &= ~DIRTY_MASK; // Set dirty bit to 0.

                    // Set footer information.
                    *(reinterpret_cast<size_t*>(nextBlock + HEADER + remainingSize)) = remainingSize;     
                    *(reinterpret_cast<size_t*>(nextBlock + HEADER + remainingSize)) &= ~DIRTY_MASK; // Set dirty bit to 0.
                }
                
                return i + HEADER; // return pointer to block without the metadata section.
            }
            i += (*reinterpret_cast<size_t*>(i) & SIZE_MASK) + HEADER + FOOTER;
        }
        // if we reach here, there is no memory available to allocate.
        return nullptr;
    }


    void free(char* ptrToAllocatedBlock)
    {


    }
    ~Allocator() { }  


private:
    static constexpr unsigned long long HEADER = 8;
    static constexpr unsigned long long FOOTER = 8;
	static constexpr unsigned long long DIRTY_MASK = 0x8000000000000000; // 1000 0000
	static constexpr unsigned long long SIZE_MASK = 0x7FFFFFFFFFFFFFFF; // 0111 1111
    std::vector<byte> m_allocatedMemory;
};