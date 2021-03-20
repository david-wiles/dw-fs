#include <string.h>
#include <stdlib.h>
#include "dwfs.h"
#include "dir.h"
#include "mem.h"


dwfs *dwfs_init(unsigned int num_blocks)
{
  dwfs *self = calloc(1, sizeof(dwfs));
  self->blocks = allocate(num_blocks);
  self->dir = dw_dir_init();
  return self;
}

void dwfs_free(dwfs *self)
{
  deallocate(self->blocks);
  dw_dir_free(self->dir);
  free(self);
}

dw_file dwfs_create(
        dwfs *self,
        char *name,
        int *err
                   )
{
  // Check whether file exists
  if (file_exists(self->dir, name) != 0) {
    *err = ERR_NON_UNIQUE_NAME;
    return (dw_file) {};
  }

  fp_node *fp = add_entry(self->dir, get_block(self->blocks), name, err);
  if (*err != 0) return (dw_file) {};

//  ft_open_file(self->tab, fp, err);
  if (*err != 0) return (dw_file) {};

  // fp should NEVER be null for a file
  return (dw_file) {name, fp};
}

dw_file dwfs_open(
        dwfs *self,
        char *name,   // Name of the file to open
        int *err // Any errors that occur are returned in this variable
                 )
{
  fp_node *fp = search_file(self->dir, name, err);
  if (*err != 0) return (dw_file) {};

//  ft_open_file(self->tab, fp, err);
  if (*err != 0) return (dw_file) {};

  return (dw_file) {fp->name, fp};
}

// Close an opened file object
void dwfs_close(
        dwfs *self,
        dw_file *file, // Reference to the file to close
        int *err  // Any errors that occur are returned in this variable
               )
{
//  ft_close_file(self->tab, file->fp, err);
}

// Reads specified number of bytes from the file into a char array
unsigned char *dwfs_read(
        dwfs *self,
        dw_file *file,  // Reference to an open file
        unsigned int n, // Number of bytes to read
        int *err   // Any errors that occur are returned in this variable
                        )
{
  // Check if file is open, if not return error
//  if (ft_is_open(self->tab, file->fp, err) == 0) {
//    *err = ERR_FILE_NOT_OPEN;
//    return 0;
//  }

  // Copy data from blocks to buffer
  unsigned char *data = calloc(n, sizeof(unsigned char));
  unsigned int n_read = 0;

//  ft_read_lock(self->tab, file->fp, err);
//  if (*err != 0) return 0;

  // Iterate data blocks
  for (data_node *d = file->fp->data; d != 0 && n_read < n; d = d->next) {
    // Iterate each byte in the block
    for (int i = 0; i < d->bytes; i++) {
      data[n_read++] = d->data[i];
    }
  }

//  ft_read_unlock(self->tab, file->fp, err);

  return data;
}


// Writes specified number of bytes to an open file
void dwfs_write(
        dwfs *self,
        dw_file *file, // The open file to write bytes to
        const char *bytes,   // The bytes to write to the file
        int n,         // Number of bytes to write
        int *err  // Any errors that occur are returned in this variable
               )
{
  // Check if file is open, if not return error
//  if (ft_is_open(self->tab, file->fp, err) == 0) {
//    *err = ERR_FILE_NOT_OPEN;
//    return;
//  }

  // Copy data from bytes to file blocks
  int max_data_size = BLOCK_SIZE - (sizeof(data_node *) + sizeof(int)),
          b = 0,
          i = 0,
          to_write = 0,
          written = 0;
  data_node *d = file->fp->data;

//  ft_write_lock(self->tab, file->fp, err);
  if (*err != 0) return;

  // Find last data node in file
  if (d != 0) {
    for (; d->next != 0; d = d->next);
  } else {
    file->fp->data = get_block(self->blocks);
    file->fp->data->bytes = 0;
    file->fp->data->next = 0;
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
      d->next = get_block(self->blocks);

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

//  ft_write_unlock(self->tab, file->fp, err);
}

char **dwfs_dir(
        dwfs *self,
        int *err
               )
{
  int n_files = 0;
  fp_node **entries = gather_entries(self->dir, &n_files, err);
  if (err != 0) {
    return 0;
  }

  char **filenames = calloc(n_files, sizeof(char *));
  for (int i = 0; i < n_files; i++) {
    filenames[i] = calloc(strlen(entries[i]->name), sizeof(char));
    strcpy(filenames[i], entries[i]->name);
  }

  free(entries);
  return filenames;
}

void dwfs_delete(
        dwfs *self,
        char *name,   // Name of the file to remove
        int *err // Any errors that occur are returned in this variable
                )
{
  fp_node *fp = search_file(self->dir, name, err);
  if (*err != 0) return;

//  if (ft_is_open(self->tab, fp, err) != 0) return;

  remove_entry(self->dir, name, err);
  if (err != 0) return;

  if (fp->data == 0) free_block(self->blocks, fp->data, err);
  if (err != 0) return;

  if (fp != 0) free_block(self->blocks, fp, err);
  if (err != 0) return;


}

