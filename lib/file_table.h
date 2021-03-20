#ifndef DW_FS_FILE_TABLE_H
#define DW_FS_FILE_TABLE_H

#include <pthread.h>
#include <search.h>
#include "dir.h"


typedef struct file_table file_table;

file_table *ft_init(unsigned int size);
void ft_free(file_table *self);

void ft_open_file(file_table *self, fp_node *fp, int *err);
int ft_is_open(file_table *self, fp_node *fp, int *err);
void ft_close_file(file_table *self, fp_node *fp, int *err);

void ft_read_lock(file_table *self, fp_node *fp, int *err);
void ft_read_unlock(file_table *self, fp_node *fp, int *err);

void ft_write_lock(file_table *self, fp_node *fp, int *err);
void ft_write_unlock(file_table *self, fp_node *fp, int *err);

#endif //DW_FS_FILE_TABLE_H
