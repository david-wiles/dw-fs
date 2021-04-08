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
  dw_dir *dir = NULL;

  if ((dir = calloc(1, sizeof(dw_dir))) == NULL) {
    return NULL;
  }

  pthread_rwlock_init(&dir->mu, NULL);

  dir->head = NULL;
  return dir;
}

bool dw_dir_file_exists(
        dw_dir *self,
        const char *filename
                       )
{
  int err = 0;
  return dw_dir_search_file(self, filename, &err) != NULL;
}

fp_node *dw_dir_search_file(
        dw_dir *self,
        const char *filename,
        int *err
                           )
{
  pthread_rwlock_rdlock(&self->mu);

  fp_node *fp = NULL;
  for (fp = self->head; fp != NULL; fp = fp->next) {
    if (strcmp(fp->name, filename) == 0) {
      break;
    }
  }

  pthread_rwlock_unlock(&self->mu);

  if (fp == NULL) {
    *err = ERR_NOT_EXISTS;
  }

  return fp;
}

fp_node *dw_dir_add(
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

  time_t now = time(NULL);

  pthread_rwlock_wrlock(&self->mu);

  fp->next = self->head;
  fp->data = NULL;
  fp->create_time = now;
  fp->mod_time = now;

  self->head = fp;
  self->n_files++;

  pthread_rwlock_unlock(&self->mu);

  return fp;
}

void dw_dir_remove(
        dw_dir *self,
        const char *filename,
        int *err
                  )
{
  fp_node *last = NULL,
          *fp = NULL;

  pthread_rwlock_wrlock(&self->mu);

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

  pthread_rwlock_unlock(&self->mu);
}

fp_node **dw_dir_gather_entries(
        dw_dir *self,
        int *len,
        int *err
                               )
{
  fp_node **entries = NULL;

  pthread_rwlock_rdlock(&self->mu);

  if (self->n_files == 0) {
    *err = ERR_NOT_EXISTS;
  } else {
    entries = calloc(self->n_files, sizeof(fp_node *));

    int i = 0;
    for (fp_node *fp = self->head; fp != NULL; fp = fp->next) {
      entries[i++] = fp;
    }

    *len = self->n_files;
  }

  pthread_rwlock_unlock(&self->mu);

  return entries;
}

// Ensure that all nodes have been freed first
void dw_dir_free(dw_dir *self)
{
  free(self);
}
