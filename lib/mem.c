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
#include <stdlib.h>

#include "mem.h"
#include "err.h"
#include "conf.h"


// Size is the number of blocks
dw_mem *dw_mem_allocate(unsigned int n)
{
  dw_mem *mem = 0;
  if ((mem = (dw_mem *) calloc(1, sizeof(dw_mem))) == 0) {
    return 0;
  }

  if ((mem->blocks = malloc(n * BLOCK_SIZE)) == 0) {
    free(mem);
    return 0;
  }

  if ((mem->bitset = bitset_create_with_capacity(n)) == 0) {
    free(mem->blocks);
    free(mem);
    return 0;
  };

  mem->n_free = n;
  mem->n_blocks = n;

  return mem;
}

void dw_mem_deallocate(dw_mem *self)
{
//  bitset_free(mem->bitset); // why does this cause seg faults...
  free(self->blocks);
  free(self);
}

// Get a block of specified size from the pre-allocated memory
void *dw_mem_malloc(dw_mem *self)
{
  if (self->n_free == 0) {
    return 0;
  }

  for (int i = 0, end = (int) self->n_blocks; i < end; i++) {
    if (bitset_get(self->bitset, i) == 0) {

      bitset_set(self->bitset, i);
      self->n_free--;

      // Return the address of the block at i
      return self->blocks + i * BLOCK_SIZE;
    }
  }

  // Return null if there is no available memory
  return 0;
}

void dw_mem_free(
        dw_mem *self,
        void *block,
        int *err
                )
{
  // Determine where the ptr is relative to the block start
  uintptr_t diff = (uintptr_t) block - (uintptr_t) self->blocks;
  int idx = (int) diff / BLOCK_SIZE;

  if (bitset_get(self->bitset, idx) == 1) {
    // Reset pointer
    bitset_set_to_value(self->bitset, idx, 0);
    self->n_free++;
  } else {
    *err = ERR_PTR_NOT_ALLOCATED;
  }
}


