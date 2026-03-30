#include <vector>

class Allocator
{
public:
    Allocator() { }
    ~Allocator() { }  


private:
    std::vector<unsigned long long> m_allocatedMemory{};
};