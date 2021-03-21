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
#include "dir.h"
#include "err.h"


dw_dir *dw_dir_init()
{
  dw_dir *dir = calloc(1, sizeof(dw_dir));
  dir->head = NULL;
  dir->n_files = 0;
  return dir;
}

bool file_exists(
        dw_dir *dir,
        const char *filename
                )
{
  int err = 0;
  return search_file(dir, filename, &err) != NULL;
}

fp_node *search_file(
        dw_dir *dir,
        const char *filename,
        int *err
                    )
{
  // Loop through each file to determine if file exists and find node.
  // Checking the cache first would be unnecessary since that would require
  // strcmp() for each file name anyway
  fp_node *fp;
  for (fp = dir->head; fp != NULL; fp = fp->next) {
    if (strcmp(fp->name, filename) == false) {
      return fp;
    }
  }

  *err = ERR_NOT_EXISTS;
  return NULL;
}

fp_node *add_entry(
        dw_dir *self,
        void *block,
        const char *filename,
        int *err
                  )
{
  // Allocate space for a new file
  fp_node *fp = block;
  int i = 0;

  if (strlen(filename) > MAX_FILENAME_LENGTH - 1) {
    *err = ERR_NAME_LENGTH_EXCEEDED;
    return NULL;
  }

  // Copy name to fp
  for (i = 0; filename[i] != '\0' && i < (int) MAX_FILENAME_LENGTH - 1; i++) {
    fp->name[i] = filename[i];
  }

  fp->name[i] = '\0';

  fp->next = self->head;
  fp->data = NULL;

  self->head = fp;
  self->n_files++;
  return fp;
}

void remove_entry(
        dw_dir *self,
        const char *filename,
        int *err
                 )
{
  fp_node *last = NULL,
          *fp = NULL;
  for (fp = self->head; fp != NULL; fp = fp->next) {
    if (strcmp(fp->name, filename) == false) {
      break;
    }
    last = fp;
  }

  if (fp == NULL) {
    *err = ERR_NOT_EXISTS;
    return;
  }

  // Now, fp is the node to delete and last is the node linking to fp
  if (last == NULL) {
    self->head = fp->next;
  } else {
    last->next = fp->next;
  }

  self->n_files--;
}

fp_node **gather_entries(
        dw_dir *self,
        int *len,
        int *err
                        )
{
  if (self->n_files == 0) {
    *err = ERR_NOT_EXISTS;
    return NULL;
  } else {
    fp_node **entries = calloc(self->n_files, sizeof(fp_node *));

    int i = 0;
    for (fp_node *fp = self->head; fp != NULL; fp = fp->next) {
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