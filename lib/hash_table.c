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

