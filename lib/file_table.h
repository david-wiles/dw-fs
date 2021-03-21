#ifndef DW_FS_FILE_TABLE_H
#define DW_FS_FILE_TABLE_H

#include <pthread.h>
#include <semaphore.h>
#include "search_hsearch_r.h"
#include "dir.h"


typedef struct hsearch_data file_table;
typedef struct ft_entry ft_entry;

struct ft_entry
{
  pthread_mutex_t entry_mu;
  pthread_mutex_t write_mu; // Lock write/read of files

  int read_mu;  // Synchronize count of files reading
  int open_cnt; // Number of threads using this file
  fp_node *fp;  // File pointer
};

ft_entry *ft_entry_init(fp_node * fp, int count);

file_table *ft_init(unsigned int size);
void ft_free(file_table *self);

int ft_is_open(file_table *self, const char *name, int *err);

void ft_open_file(file_table *self, fp_node *fp, int *err);
void ft_close_file(file_table *self, const char *name, int *err);

void ft_read_lock(file_table *self, const char *name, int *err);
void ft_read_unlock(file_table *self, const char *name, int *err);

void ft_write_lock(file_table *self, const char *name, int *err);
void ft_write_unlock(file_table *self, const char *name, int *err);

#endif //DW_FS_FILE_TABLE_H
