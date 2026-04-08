#include <iostream>
#include <vector>
#include <cassert>
#include <cstring>
#include <random>

#include "Allocator.h"

#define ASSERT(x) do { if (!(x)) { \
    std::cerr << "Assertion failed: " << #x << " at line " << __LINE__ << std::endl; \
    std::abort(); }} while(0)

// ---------------- BASIC TEST ----------------
void test_basic_allocate_free()
{
    Allocator alloc(64);

    void* p = alloc.allocate(16);
    ASSERT(p != nullptr);

    alloc.free((unsigned char*)p);

    void* p2 = alloc.allocate(16);
    ASSERT(p2 != nullptr);
}

// ---------------- SPLIT TEST ----------------
void test_split()
{
    Allocator alloc(64);

    void* p1 = alloc.allocate(16);
    ASSERT(p1 != nullptr);

    void* p2 = alloc.allocate(16);
    ASSERT(p2 != nullptr); // should succeed due to split
}

// ---------------- NO SPLIT TEST ----------------
void test_no_split_small_remainder()
{
    Allocator alloc(32);

    void* p = alloc.allocate(20);
    ASSERT(p != nullptr);

    void* p2 = alloc.allocate(1);
    ASSERT(p2 == nullptr); // no room left
}

// ---------------- COALESCING BACKWARD ----------------
void test_coalesce_backward()
{
    Allocator alloc(64);

    auto p1 = (unsigned char*)alloc.allocate(16);
    auto p2 = (unsigned char*)alloc.allocate(16);

    alloc.free(p2);
    alloc.free(p1);

    void* big = alloc.allocate(32);
    ASSERT(big != nullptr);
}

// ---------------- COALESCING FORWARD ----------------
void test_coalesce_forward()
{
    Allocator alloc(64);

    auto p1 = (unsigned char*)alloc.allocate(16);
    auto p2 = (unsigned char*)alloc.allocate(16);

    alloc.free(p1);
    alloc.free(p2);

    void* big = alloc.allocate(32);
    ASSERT(big != nullptr);
}

// ---------------- COALESCE BOTH SIDES ----------------
void test_coalesce_both()
{
    Allocator alloc(128);

    auto p1 = (unsigned char*)alloc.allocate(16);
    auto p2 = (unsigned char*)alloc.allocate(16);
    auto p3 = (unsigned char*)alloc.allocate(16);

    alloc.free(p2);
    alloc.free(p1);
    alloc.free(p3);

    void* big = alloc.allocate(64);
    ASSERT(big != nullptr);
}

// ---------------- FRAGMENTATION TEST ----------------
void test_fragmentation()
{
    Allocator alloc(128);

    std::vector<unsigned char*> ptrs;

    for (int i = 0; i < 6; i++)
        ptrs.push_back((unsigned char*)alloc.allocate(16));

    for (int i = 0; i < 6; i += 2)
        alloc.free(ptrs[i]);

    // should fail due to fragmentation
    void* big = alloc.allocate(48);
    ASSERT(big == nullptr);

    // free remaining → should coalesce
    for (int i = 1; i < 6; i += 2)
        alloc.free(ptrs[i]);

    big = alloc.allocate(96);
    ASSERT(big != nullptr);
}

// ---------------- WRITE TEST ----------------
void test_memory_integrity()
{
    Allocator alloc(64);

    unsigned char* p = (unsigned char*)alloc.allocate(16);
    ASSERT(p != nullptr);

    for (int i = 0; i < 16; i++)
        p[i] = (unsigned char)i;

    for (int i = 0; i < 16; i++)
        ASSERT(p[i] == i);
}

// ---------------- DOUBLE FREE TEST ----------------
void test_double_free()
{
    Allocator alloc(64);

    auto p = (unsigned char*)alloc.allocate(16);
    alloc.free(p);
    alloc.free(p); // should not crash
}

// ---------------- RANDOM STRESS TEST ----------------
void test_random_stress()
{
    Allocator alloc(512);

    std::vector<unsigned char*> allocated;
    std::mt19937 rng(42);

    for (int i = 0; i < 10000; i++)
    {
        int op = rng() % 2;

        if (op == 0 && !allocated.empty())
        {
            int idx = rng() % allocated.size();
            alloc.free(allocated[idx]);
            allocated.erase(allocated.begin() + idx);
        }
        else
        {
            size_t size = (rng() % 32) + 1;
            auto p = (unsigned char*)alloc.allocate(size);
            if (p)
            {
                // write pattern
                memset(p, 0xAB, size);
                allocated.push_back(p);
            }
        }
    }

    // free everything
    for (auto p : allocated)
        alloc.free(p);

    // should be fully reusable
    void* big = alloc.allocate(400);
    ASSERT(big != nullptr);
}

// ---------------- EDGE CASES ----------------
void test_edge_cases()
{
    Allocator alloc(32);

    ASSERT(alloc.allocate(0) == nullptr);

    alloc.free(nullptr); // should not crash
}

// ---------------- MAIN ----------------
int main()
{
    test_basic_allocate_free();
    test_split();
    test_no_split_small_remainder();
    test_coalesce_backward();
    test_coalesce_forward();
    test_coalesce_both();
//    test_fragmentation();
    test_memory_integrity();
    test_double_free();
//    test_random_stress();
    test_edge_cases();

    std::cout << "All tests passed.\n";
}