#ifndef DW_FS_MEM_H
#define DW_FS_MEM_H

#include <stdlib.h>


void *allocate(size_t size);

void deallocate(void *ptr);

// Get a block of specified size from the pre-allocated memory
void *get_block();

void free_block(void *ptr);

#endif //DW_FS_MEM_H
