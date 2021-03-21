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
// Convenience methods for modifying a hash table, adapted from an answer on stack overflow concerning
// how to use GNU search.h. This implementation uses an adaptation of the original hsearch_r so that it
// can be used on macOS and other platforms, in addition to unix based OS
#ifndef DW_FS_HASH_TABLE_H
#define DW_FS_HASH_TABLE_H


#include "search_hsearch_r.h"


typedef struct hsearch_data hash_table;

hash_table *hinit(size_t size);
void hfree(hash_table *tab);

void hadd(struct hsearch_data *tab, char *key, void *value);
void hdelete(struct hsearch_data *tab, char *key);
void *hfind(struct hsearch_data *tab, char *key);

#endif //DW_FS_HASH_TABLE_H
