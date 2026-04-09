#include <vector>
#include <thread>
#include <mutex>

// Allocator contains bytes structured as follows:
// +----------------+---------------------------+----------------+
// |  8-byte header |        user data         |  8-byte footer |
// +----------------+---------------------------+----------------+


using byte = unsigned char;

class Allocator 
{
public:
    explicit Allocator(size_t numOfBytes = 1) : m_allocatedMemory(HEADER + numOfBytes + FOOTER, 0)
    {
        setSize(m_allocatedMemory.data(), numOfBytes);
        setSize(m_allocatedMemory.data() + HEADER + numOfBytes, numOfBytes);
    }
    Allocator(const Allocator &other) = delete;
    Allocator &operator=(const Allocator &other) = delete;

    /* Allocator functions */
    void* allocate(size_t numOfBytes);
    void free(byte* ptr);

    /* Helper functions */
    void mergeBlocks(byte* leftHeader, byte* rightHeader);
    void setSize(byte* metadataPtr, size_t blockSize) { *(reinterpret_cast<size_t*>(metadataPtr)) = blockSize; }
    void setClean(byte *metadataPtr) { *(reinterpret_cast<size_t*>(metadataPtr)) &= ~DIRTY_MASK; }
    void setDirty(byte* metadataPtr ) { *reinterpret_cast<size_t*>(metadataPtr) |= DIRTY_MASK;  }
    bool isDirty(byte* metadataPtr) { return *(reinterpret_cast<size_t*>(metadataPtr)) & DIRTY_MASK; }
    size_t memoryBlockSize(byte* metadataPtr) { return *(reinterpret_cast<size_t*>(metadataPtr)) & SIZE_MASK; }

    ~Allocator() = default; 


private:

    // Static constants used by the allocator.
    static constexpr byte HEADER = 8;
    static constexpr byte FOOTER = 8;
	static constexpr size_t DIRTY_MASK = 0x8000000000000000; // 1000 0000
	static constexpr size_t SIZE_MASK = 0x7FFFFFFFFFFFFFFF; // 0111 1111

    // Allocated bytes.
    std::vector<byte> m_allocatedMemory;

    static std::mutex allocator_mutex;
};