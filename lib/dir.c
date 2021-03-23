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

  pthread_mutex_init(&dir->mu, 0);
  pthread_mutex_init(&dir->w_mu, 0);

  dir->head = NULL;
  dir->n_files = 0;
  dir->read_cnt = 0;
  return dir;
}

static void dw_dir_read_lock(dw_dir *self)
{
  bool lock_write = false;
  pthread_mutex_lock(&self->mu);

  if (self->read_cnt == 0) {
    lock_write = true;
  }

  self->read_cnt++;
  pthread_mutex_unlock(&self->mu);

  if (lock_write != false) {
    pthread_mutex_lock(&self->w_mu);
  }
}

static void dw_dir_read_unlock(dw_dir *self)
{
  bool unlock_write = false;
  pthread_mutex_lock(&self->mu);
  self->read_cnt--;

  // If this is the last thread to unlock the read lock, also unlock for writing
  if (self->read_cnt == 0) {
    unlock_write = true;
  }
  pthread_mutex_unlock(&self->mu);

  if (unlock_write != false) {
    pthread_mutex_unlock(&self->w_mu);
  }
}

static void dw_dir_write_lock(dw_dir *self)
{
  pthread_mutex_lock(&self->w_mu);
}

static void dw_dir_write_unlock(dw_dir *self)
{
  pthread_mutex_unlock(&self->w_mu);
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
  dw_dir_read_lock(self);

  fp_node *fp = NULL;
  for (fp = self->head; fp != NULL; fp = fp->next) {
    if (strcmp(fp->name, filename) == 0) {
      break;
    }
  }

  dw_dir_read_unlock(self);

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

  dw_dir_write_lock(self);

  fp->next = self->head;
  fp->data = NULL;
  fp->create_time = now;
  fp->mod_time = now;

  self->head = fp;
  self->n_files++;

  dw_dir_write_unlock(self);

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

  dw_dir_write_lock(self);

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

  dw_dir_write_unlock(self);
}

fp_node **dw_dir_gather_entries(
        dw_dir *self,
        int *len,
        int *err
                               )
{
  fp_node **entries = NULL;

  dw_dir_read_lock(self);

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

  dw_dir_read_unlock(self);

  return entries;
}

// Ensure that all nodes have been freed first
void dw_dir_free(dw_dir *self)
{
  free(self);
}
