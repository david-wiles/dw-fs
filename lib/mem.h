#ifndef DW_FS_MEM_H
#define DW_FS_MEM_H

#include "bitset.h"


typedef struct dw_mem dw_mem;

struct dw_mem
{
  void *blocks; // Allocated memory
  unsigned int n_blocks; // Total number of blocks
  unsigned int n_free;   // Number of free blocks

  bitset_t *bitset;
};


dw_mem *allocate(unsigned int);

void deallocate(dw_mem *);

// Get a block of specified size from the pre-allocated memory
void *get_block(dw_mem *);

void free_block(dw_mem *, void *);

#endif //DW_FS_MEM_H
