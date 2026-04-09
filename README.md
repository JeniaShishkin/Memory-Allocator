# Custom Memory Allocator

This repository contains a simple custom memory allocator implementation in C++.

## Overview

The allocator manages a contiguous block of memory stored in a `std::vector<unsigned char>`.
Each allocated chunk is tracked with an 8-byte header and an 8-byte footer.
The header/footer store the block size and a dirty bit that indicates whether the block is currently allocated.

## Files

- `Allocator.h` - Declares the `Allocator` class and helper methods.
- `Allocator.cpp` - Implements allocation, deallocation, block splitting, and coalescing logic.

## Features

- First-fit allocation strategy
- Block splitting when a large free block is used for a smaller request
- Coalescing free blocks during deallocation to reduce fragmentation
- Thread-safe operations using a static `std::mutex`

## How It Works

Memory blocks are laid out as follows:

```
+----------------+---------------------------+----------------+
|  8-byte header |        user data         |  8-byte footer |
+----------------+---------------------------+----------------+
```

- The header and footer store a block size with a dirty bit.
- When a block is allocated, the dirty bit is set on both header and footer.
- When a block is freed, adjacent free blocks are merged.

## Usage

Create an `Allocator` instance and use `allocate()` to reserve memory and `free()` to release it.

Example:

```cpp
Allocator allocator(1024);
unsigned char* data = static_cast<unsigned char*>(allocator.allocate(128));
if (data) {
    // use the memory
    allocator.free(data);
}
```

## Build Instructions

### Windows

If you have Visual Studio installed, open a "Developer Command Prompt for VS" and run:

```powershell
cl /EHsc Allocator.cpp main.cpp
```

If you have MinGW or another GCC toolchain installed, use:

```powershell
g++ -std=c++17 -O2 Allocator.cpp main.cpp -o allocator.exe
```

### Linux

Use GCC or Clang from a terminal:

```bash
g++ -std=c++17 -O2 Allocator.cpp main.cpp -o allocator
```

Then run the program with:

```bash
./allocator
```

## Notes

- The `Allocator` class is not copyable.
- The allocation API returns `nullptr` when there is not enough free memory.
- This implementation is intended for learning and demonstration purposes.
