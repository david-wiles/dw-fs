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
#include "hash_table.h"


hash_table *hinit(size_t size)
{
  hash_table *tab = 0;

  if ((tab = calloc(1, sizeof(hash_table))) == 0) {
    return 0;
  }

  hcreate_r(size, tab);

  return tab;
}

void hfree(hash_table *tab)
{
  hdestroy_r(tab);
  free(tab);
}

void hadd(struct hsearch_data *tab, char *key, void *value)
{
  ENTRY item = {key, value};
  ENTRY *pitem = &item;

  if (hsearch_r(item, ENTER, &pitem, tab)) {
    pitem->data = value;
  }
}

void hdelete(struct hsearch_data *tab, char *key)
{
  ENTRY item = {key};
  ENTRY *pitem = &item;

  if (hsearch_r(item, FIND, &pitem, tab)) {
    pitem->data = (void *) 0;
  }
}

void * hfind(struct hsearch_data *tab, char *key)
{
  ENTRY item = {key};
  ENTRY *pitem = &item;

  if (hsearch_r(item, FIND, &pitem, tab)) {
    return (void *) pitem->data;
  }
  return 0;
}

