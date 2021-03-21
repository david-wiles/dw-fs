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
