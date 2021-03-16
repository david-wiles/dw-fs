#pragma once

#include <stdint.h>
#include <stdbool.h>


#define CLIB_WORD_BIT (int)(sizeof(int)*8)

typedef struct clib_bitmap
{
  int *map;
  uint64_t size;
} clib_bitmap;

clib_bitmap *clib_bitmap_init(uint64_t size);
void clib_bitmap_free(clib_bitmap *bitmap);

bool bitmap_get(clib_bitmap *, int);
void bitmap_set(clib_bitmap *, int);
void bitmap_reset(clib_bitmap *, int);
