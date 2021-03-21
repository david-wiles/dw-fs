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
#include "hash_table.h"
#include "file_table.h"
#include "err.h"


ft_entry *ft_entry_init(fp_node *fp, int count)
{
  ft_entry *entry = 0;
  if ((entry = calloc(1, sizeof(ft_entry))) == NULL) {
    return NULL;
  }

  pthread_mutex_init(&entry->entry_mu, 0);
  pthread_mutex_init(&entry->write_mu, 0);

  entry->read_mu = 0;
  entry->open_cnt = count;
  entry->fp = fp;

  return entry;
}

file_table *ft_init(unsigned int size)
{
  return hinit(size);
}

void ft_free(file_table *self)
{
  hfree(self);
}

int ft_is_open(file_table *self, const char *name, int *err)
{
  ft_entry *entry = 0;
  int is_open = 0;

  if ((entry = hfind(self, name)) == 0) {
    return is_open;
  }

  pthread_mutex_lock(&entry->entry_mu);
  is_open = entry->open_cnt;
  pthread_mutex_unlock(&entry->entry_mu);

  return is_open;
}

void ft_open_file(file_table *self, fp_node *fp, int *err)
{
  ft_entry *entry = 0;
  if ((entry = hfind(self, fp->name)) == 0) {
    entry = ft_entry_init(fp, 1);
    hadd(self, fp->name, entry);
    return;
  }

  pthread_mutex_lock(&entry->entry_mu);
  entry->open_cnt++;
  pthread_mutex_unlock(&entry->entry_mu);
}

void ft_close_file(file_table *self, const char *name, int *err)
{
  ft_entry *entry = 0;
  if ((entry = hfind(self, name)) == 0) {
    *err = ERR_FILE_NOT_OPEN;
    return;
  }

  pthread_mutex_lock(&entry->entry_mu);
  entry->open_cnt--;
  if (entry->open_cnt == 0) {
    hdelete(self, name);
    return;
  }
  pthread_mutex_unlock(&entry->entry_mu);
}

void ft_read_lock(file_table *self, const char *name, int *err)
{
  ft_entry *entry = 0;
  int lock_write = 0;

  if ((entry = hfind(self, name)) == 0) {
    *err = ERR_FILE_NOT_OPEN;
    return;
  }

  pthread_mutex_lock(&entry->entry_mu);

  if (entry->read_mu == 0) {
    // If this is the first thread to acquire a read lock, we should also
    // lock the file for writing. This should block until the writing
    // process unlocks the resource
    //
    // If this is not the first thread to unlock for writing, then the
    // write mutex should already be locked
    lock_write = 1;
  }

  entry->read_mu++;
  pthread_mutex_unlock(&entry->entry_mu);

  if (lock_write != 0) {
    pthread_mutex_lock(&entry->write_mu);
  }

}

void ft_read_unlock(file_table *self, const char *name, int *err)
{
  ft_entry *entry = 0;
  int unlock_write = 0;
  if ((entry = hfind(self, name)) == 0) {
    *err = ERR_FILE_NOT_OPEN;
    return;
  }

  pthread_mutex_lock(&entry->entry_mu);
  entry->read_mu--;

  // If this is the last thread to unlock the read lock, also unlock for writing
  if (entry->read_mu == 0) {
    unlock_write = 1;
  }
  pthread_mutex_unlock(&entry->entry_mu);

  if (unlock_write != 0) {
    pthread_mutex_unlock(&entry->write_mu);
  }
}

void ft_write_lock(file_table *self, const char *name, int *err)
{
  ft_entry *entry = 0;
  if ((entry = hfind(self, name)) == 0) {
    *err = ERR_FILE_NOT_OPEN;
    return;
  }

  // Block until we can acquire the write lock
  pthread_mutex_lock(&entry->write_mu);
}

void ft_write_unlock(file_table *self, const char *name, int *err)
{
  ft_entry *entry = 0;
  if ((entry = hfind(self, name)) == 0) {
    *err = ERR_FILE_NOT_OPEN;
    return;
  }

  pthread_mutex_unlock(&entry->write_mu);
}