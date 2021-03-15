#include <stdlib.h>
#include <string.h>
#include "dw_fs.h"
#include "mem.h"


void dw_fs_init(unsigned int num_blocks)
{
  _dw_fs.blocks = allocate(BLOCK_SIZE * num_blocks);
  _dw_fs.n_blocks = num_blocks;

  _dw_fs.dir = 0;
  _dw_fs.n_files = 0;
}

void dw_fs_free()
{
  deallocate(_dw_fs.blocks);
  _dw_fs.blocks = 0;
  _dw_fs.n_blocks = 0;
}

int file_exists(const char *name)
{
  int err = 0;
  return search_file(name, &err) != 0;
}

fp_node *search_file(
        const char *name,
        dw_error *err
                    )
{
  // Loop through each file to determine if file exists and find node.
  // Checking the cache first would be unnecessary since that would require
  // strcmp() for each file name anyway
  fp_node *fp;
  ITER_DIR(fp) {
    if (strcmp(fp->name, name) == 0) {
      return fp;
    }
  }

  *err = ERR_NOT_EXISTS;
  return 0;
}

char **dir(dw_error *err)
{
  char **files = 0;
  // If the first directory block is null, there are no files and we should set the file list to be null
  if (_dw_fs.dir == 0) {
    return 0;
  } else {
    // Otherwise, loop through the files and build a new list
    int f_size = 0, i = 0;

    // Loop once to find number of files
    fp_node *fp;
    ITER_DIR(fp) {
      f_size++;
    }

    files = calloc(f_size, sizeof(char *));

    // Add each file name to the list
    ITER_DIR(fp) {
      files[i] = fp->name;
    }
  }

  return files;
}

void delete(
        char *name,   // Name of the file to remove
        dw_error *err // Any errors that occur are returned in this variable
           )
{
  fp_node *last = 0,
          *fp = 0;
  ITER_DIR(fp) {
    if (strcmp(fp->name, name) == 0) {
      break;
    }
    last = fp;
  }

  if (fp == 0) {
    *err = ERR_NOT_EXISTS;
    return;
  }

  // Now, fp is the node to delete and last is the node linking to fp
  if (last == 0) {
    _dw_fs.dir = fp->next;
  } else {
    last->next = fp->next;
  }

  _dw_fs.n_files--;
  free(fp->data);
  free(fp);
}


dw_file create(
        char *name,
        dw_error *err
              )
{
  // Check whether file exists
  if (file_exists(name) != 0) {
    *err = ERR_NON_UNIQUE_NAME;
    return (dw_file) {};
  }

  // Allocate space for a new file
  fp_node *fp = get_block();
  int i = 0;
  // Copy name to fp
  for (i = 0; name[i] != '\0' && i < MAX_FILENAME_LENGTH - 1; i++) {
    fp->name[i] = name[i];
  }

  fp->name[i] = '\0';

  fp->next = _dw_fs.dir;
  fp->data = 0;

  // Push node to top of dir stack
  _dw_fs.dir = fp;
  _dw_fs.n_files++;

  *err = 0;
  // fp should NEVER be null for a file
  return (dw_file) {name, fp};
}

dw_file *open(
        char *name,   // Name of the file to open
        dw_error *err // Any errors that occur are returned in this variable
             )
{
  fp_node *fp = search_file(name, err);
  if (err != 0) {
    return 0;
  }

  dw_file *f = malloc(sizeof(dw_file));
  f->name = fp->name;
  f->fp = fp;

  // update file table

  return f;
}

// Close an opened file object
void close(
        dw_file *file, // Reference to the file to close
        dw_error *err  // Any errors that occur are returned in this variable
          )
{
  // Update file table
}

// Reads specified number of bytes from the file into a char array
char *read(
        dw_file *file,  // Reference to an open file
        unsigned int n, // Number of bytes to read
        dw_error *err   // Any errors that occur are returned in this variable
          )
{
  // Check if file is open, if not return error

  // Copy data from blocks to buffer

  char *data = malloc(n);
  int n_read = 0;

  // Iterate data blocks
  for (data_node *d = file->fp->data; d != 0 && n_read < n; d = d->next) {
    // Iterate each byte in the block
    for (int i = 0; i < d->bytes; i++) {
      data[n_read++] = d->data[i];
    }
  }

  return data;
}


// Writes specified number of bytes to an open file
void write(
        dw_file *file, // The open file to write bytes to
        const char *bytes,   // The bytes to write to the file
        int n,         // Number of bytes to write
        dw_error *err  // Any errors that occur are returned in this variable
          )
{
  // Check if file is open, if not return error
  // Copy data from bytes to file blocks

  int max_data_size = BLOCK_SIZE - (sizeof(data_node *) + sizeof(int)),
          b = 0,
          i = 0,
          to_write = 0,
          written = 0;
  data_node *d = file->fp->data;

  // Find last data node in file
  if (d != 0) {
    for (; d->next != 0; d = d->next);
  } else {
    file->fp->data = get_block();
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
      d->next = get_block();

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
}
