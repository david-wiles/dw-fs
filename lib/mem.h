// Copyright 2021 David Wiles <david@wiles.fyi>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
// associated documentation files (the "Software"), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial
// portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
// LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Implementation of a custom allocator for the file system.
//
// One of the requirements of the file system is that it should be stored in a contiguous block of bytes
// in main memory. This is achieved by requesting the bytes from the OS using malloc, and then using the
// bytes as needed using pointer arithmetic.
//
// When a block of memory is requested, the allocator look for a free block of memory and return it to the
// calling function as (void *).
//
// A bitset is used to store the positions of free blocks
#ifndef DW_FS_MEM_H
#define DW_FS_MEM_H

#include "bitset.h"


typedef struct dw_mem dw_mem;

struct dw_mem
{
  void *blocks;          // Allocated memory
  unsigned int n_blocks; // Total number of blocks
  unsigned int n_free;   // Number of free blocks
  bitset_t *bitset;
};

// Allocate a new block of memory with n blocks
dw_mem *dw_mem_allocate(unsigned int n);

// Release all memory back to the OS
void dw_mem_deallocate(dw_mem *self);

void *dw_mem_malloc(dw_mem *self);
void dw_mem_free(
        dw_mem *self,
        void *block,  // Block of memory to return
        int *err      // Error return value
                );

#endif //DW_FS_MEM_H
