#include <stdlib.h>

#include "mem.h"
#include "conf.h"


// Size is the number of blocks
dw_mem *allocate(unsigned int size)
{
  dw_mem *mem = calloc(1, sizeof(dw_mem *));
  mem->blocks = malloc(size * BLOCK_SIZE);

  mem->n_free = size;
  mem->n_blocks = size;
  mem->bitset = bitset_create();

  return mem;
}

void deallocate(dw_mem *mem)
{
  bitset_free(mem->bitset);
  free(mem->blocks);
  free(mem);
}

// Get a block of specified size from the pre-allocated memory
void *get_block(dw_mem *self)
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

void free_block(dw_mem *self, void *ptr)
{
  // Determine where the ptr is relative to the block start
  uintptr_t diff = (uintptr_t) ptr - (uintptr_t) self->blocks;
  int idx = (int) diff / BLOCK_SIZE;

  // Reset pointer
  bitset_set_to_value(self->bitset, idx, 0);
  self->n_free++;
}


