#ifndef DW_FS_MEM_H
#define DW_FS_MEM_H

#include <stdlib.h>


void *allocate(size_t size)
{
  return malloc(size);
}

void deallocate(void *ptr)
{
  free(ptr);
}

// Get a block of specified size from the pre-allocated memory
void *get_block()
{
  return calloc(BLOCK_SIZE, 1);
}

void free_block(void *ptr)
{
  free(ptr);
}

#endif //DW_FS_MEM_H
