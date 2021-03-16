#include <string.h>
#include <stdlib.h>
#include "dir.h"
#include "err.h"
#include "mem.h"


dw_dir *dw_dir_init()
{
  dw_dir *dir = calloc(1, sizeof(dw_dir *));
  dir->head = 0;
  dir->n_files = 0;
  return dir;
}

int file_exists(
        dw_dir *dir,
        const char *name
               )
{
  int err = 0;
  return search_file(dir, name, &err) != 0;
}

fp_node *search_file(
        dw_dir *dir,
        const char *name,
        int *err
                    )
{
  // Loop through each file to determine if file exists and find node.
  // Checking the cache first would be unnecessary since that would require
  // strcmp() for each file name anyway
  fp_node *fp;
  for ((fp) = dir->head; (fp) != 0; (fp) = (fp)->next) {
    if (strcmp(fp->name, name) == 0) {
      return fp;
    }
  }

  *err = ERR_NOT_EXISTS;
  return 0;
}

fp_node *add_entry(
        dw_dir *self,
        const char *name,
        int *err
                  )
{
  // Allocate space for a new file
  fp_node *fp = get_block();
  int i = 0;
  // Copy name to fp
  for (i = 0; name[i] != '\0' && i < MAX_FILENAME_LENGTH - 1; i++) {
    fp->name[i] = name[i];
  }

  fp->name[i] = '\0';

  fp->next = self->head;
  fp->data = 0;

  self->head = fp;
  self->n_files++;
  return fp;
}

void remove_entry(
        dw_dir *self,
        const char *name,
        int *err
                 )
{
  fp_node *last = 0,
          *fp = 0;
  for (fp = self->head; fp != 0; fp = fp->next) {
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
    self->head = fp->next;
  } else {
    last->next = fp->next;
  }

  self->n_files--;
  free(fp->data);
  free(fp);
}

fp_node **gather_entries(
        dw_dir *self,
        int *len,
        int *err
                        )
{
  if (self->n_files == 0) {
    *err = ERR_NOT_EXISTS;
    return 0;
  } else {
    fp_node **entries = calloc(self->n_files, sizeof(fp_node *));

    int i = 0;
    for (fp_node *fp = self->head; fp != 0; fp = fp->next) {
      entries[i++] = fp;
    }

    *len = self->n_files;
    return entries;
  }
}

// Ensure that all nodes have been freed first
void dw_dir_free(dw_dir *self)
{
  free(self);
}