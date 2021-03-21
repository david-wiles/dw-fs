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
// This implementation of a file table uses the GNU hash table implementation. Each open file
// is placed into the hash table using the filename as a key and the open file entry as the value
#ifndef DW_FS_FILE_TABLE_H
#define DW_FS_FILE_TABLE_H

#include <pthread.h>
#include "search_hsearch_r.h"
#include "dir.h"


typedef struct hsearch_data file_table;
typedef struct ft_entry ft_entry;

struct ft_entry
{
  pthread_mutex_t entry_mu; // Lock modification of entry
  pthread_mutex_t write_mu; // Lock write/read of files

  int read_mu;  // Synchronize count of files reading
  int open_cnt; // Number of threads using this file
  fp_node *fp;  // File pointer to this file
};

// Initialize a new entry for the file table
ft_entry *ft_entry_init(fp_node *fp, int count);

// Initialize the open file table. Since the file_table type is an alias for hsearch_data,
// this will call hcreate_r using the number of blocks as the size, since the maximum
// number of files will be the number of blocks
file_table *ft_init(unsigned int size);

// Free all resources associated with the table (hdestroy_r)
void ft_free(file_table *self);

// Checks whether the file is already open
int ft_is_open(file_table *self, const char *name, int *err);

// Open or close file entries. If the last open file is closed for a filename, the file will
// be deleted from the file table. Although the maximum number of files is the number of blocks,
// the number of entries in the file table could exceed this number through creation/deletion
// of files if the files are not deleted.
void ft_open_file(file_table *self, fp_node *fp, int *err);
void ft_close_file(file_table *self, const char *name, int *err);

// Lock a file for reading.
//
// An unlimited number of threads may read from the same file, but no thread may write while
// another is reading the file. Once the last thread reading the file unlocks the read lock,
// the write lock is lifted and another file may write to it.
//
// There is no advanced scheduling algorithm here, so it is possible that threads may be starved.
// For example, a process may read from a file in a loop, never lifting the read lock, so
// another would constantly wait to write. In practice, it seems that this would not happen very
// often since the lock is lifted as soon as the bytes have been read from the data blocks
void ft_read_lock(file_table *self, const char *name, int *err);
void ft_read_unlock(file_table *self, const char *name, int *err);

// Lock a file for writing
//
// Only a single thread may write to a file at a time. Once the file is done writing, the lock is
// lifted and another thread may read from the file or write to it
void ft_write_lock(file_table *self, const char *name, int *err);
void ft_write_unlock(file_table *self, const char *name, int *err);

#endif //DW_FS_FILE_TABLE_H
