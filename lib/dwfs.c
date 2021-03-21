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
#include <string.h>
#include <stdlib.h>
#include "dwfs.h"
#include "dir.h"
#include "mem.h"


dwfs *dwfs_init(unsigned int num_blocks)
{
  dwfs *self = calloc(1, sizeof(dwfs));
  self->blocks = dw_mem_allocate(num_blocks);
  self->dir = dw_dir_init();
  self->tab = ft_init(num_blocks);
  return self;
}

void dwfs_free(dwfs *self)
{
  dw_mem_deallocate(self->blocks);
  dw_dir_free(self->dir);
  ft_free(self->tab);
  free(self);
}

void dwfs_create(
        dwfs *self,
        char *filename,
        int *err
                )
{
  // Check whether file exists
  if (dw_dir_file_exists(self->dir, filename) != false) {
    *err = ERR_NON_UNIQUE_NAME;
    return;
  }

  dw_dir_add(self->dir, dw_mem_malloc(self->blocks), filename, err);
}

dw_file dwfs_open(
        dwfs *self,
        char *filename,
        int *err
                 )
{
  fp_node *fp = dw_dir_search_file(self->dir, filename, err);
  if (*err != 0) return (dw_file) {};

  ft_open_file(self->tab, fp, err);
  if (*err != 0) return (dw_file) {};

  return (dw_file) {fp->name, fp};
}

void dwfs_close(
        dwfs *self,
        dw_file *file,
        int *err
               )
{
  ft_close_file(self->tab, file->name, err);
}

// Reads specified number of bytes from the file into a char array
unsigned char *dwfs_read(
        dwfs *self,
        dw_file *file,
        unsigned int n,
        unsigned int *n_read,
        int *err
                        )
{
  unsigned int read = 0;
  // Check if file is open, if not return error
  if (ft_is_open(self->tab, file->name, err) == false) {
    *err = ERR_FILE_NOT_OPEN;
    return NULL;
  }

  // Copy data from blocks to buffer
  unsigned char *data = calloc(n, sizeof(unsigned char));

  ft_read_lock(self->tab, file->name, err);
  if (*err != 0) return NULL;

  // Iterate data blocks
  for (data_node *d = file->fp->data; d != NULL && read < n; d = d->next) {
    // Iterate each byte in the block
    for (int i = 0; i < d->bytes; i++) {
      data[read++] = d->data[i];
    }
  }

  ft_read_unlock(self->tab, file->name, err);

  *n_read = read;
  return data;
}


// Writes specified number of bytes to an open file
void dwfs_write(
        dwfs *self,
        dw_file *file,
        const unsigned char *bytes,
        int n,
        int *err
               )
{
  // Check if file is open, if not return error
  if (ft_is_open(self->tab, file->name, err) == false) {
    *err = ERR_FILE_NOT_OPEN;
    return;
  }

  // Copy data from bytes to file blocks
  int max_data_size = BLOCK_SIZE - (sizeof(data_node *) + sizeof(int)),
          b = 0,
          i = 0,
          to_write = 0,
          written = 0;
  data_node *d = file->fp->data;

  ft_write_lock(self->tab, file->name, err);
  if (*err != 0) return;

  // Find last data node in file
  if (d != NULL) {
    for (; d->next != NULL; d = d->next);
  } else {
    file->fp->data = dw_mem_malloc(self->blocks);
    file->fp->data->bytes = 0;
    file->fp->data->next = NULL;
    d = file->fp->data;
  }

  while (written < n) {
    // Write data from buffer to node
    b = d->bytes;
    for (i = 0, to_write = max_data_size - b; i < to_write && written < n; i++) {
      d->data[b + i] = bytes[written++];
    }
    d->bytes += i;

    // Create new data nodes as needed
    if (written < n) {
      d->next = dw_mem_malloc(self->blocks);

      // Check that the allocation was successful
      if (d->next == 0) {
        *err = ERR_OOM;
        return;
      }

      d = d->next;
      d->bytes = 0;
      d->next = 0;
    }
  }

  file->fp->mod_time = time(NULL);
  ft_write_unlock(self->tab, file->name, err);
}

char **dwfs_dir(
        dwfs *self,
        int *err
               )
{
  int n_files = 0;
  fp_node **entries = dw_dir_gather_entries(self->dir, &n_files, err);
  if (*err != 0) {
    return NULL;
  }

  char **filenames = calloc(n_files, sizeof(char *));
  for (int i = 0; i < n_files; i++) {
    int len = strlen(entries[i]->name);
    filenames[i] = calloc(len + 1, sizeof(char));

    // Copy entry's filename to the array. We can't use strcpy here
    for (int str_i = 0; str_i < len; str_i++) {
      filenames[i][str_i] = entries[i]->name[str_i];
    }
    filenames[i][len] = '\0';
  }

  free(entries);
  return filenames;
}

void dwfs_delete(
        dwfs *self,
        char *name,
        int *err
                )
{
  fp_node *fp = dw_dir_search_file(self->dir, name, err);
  if (*err != 0) return;

  if (ft_is_open(self->tab, fp->name, err) != false) return;

  dw_dir_remove(self->dir, name, err);
  if (err != 0) return;

  if (fp->data == NULL) dw_mem_free(self->blocks, fp->data, err);
  if (err != 0) return;

  if (fp != NULL) dw_mem_free(self->blocks, fp, err);
  if (err != 0) return;
}

