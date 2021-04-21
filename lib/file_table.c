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
#include <stdlib.h>

#include "htable.h"
#include "file_table.h"
#include "err.h"


ft_entry *ft_entry_init(fp_node *fp, int count)
{
  ft_entry *entry = 0;
  if ((entry = calloc(1, sizeof(ft_entry))) == NULL) {
    return NULL;
  }

  pthread_rwlock_init(&entry->mu, NULL);
  entry->fp = fp;
  entry->open_cnt = 1;

  return entry;
}

file_table *ft_init(unsigned int size)
{
  return htable_create(size);
}

void ft_free(file_table *self)
{
  // Free all file entries since the table allocated them
  htable_itr itr = htable_iterator_mut(self);
  htable_entry *entry = NULL;
  while ((entry = htable_iterator_next(&itr)) != NULL) {
    free(entry->val);
  }

  htable_iterator_destroy(&itr);
  htable_destroy(self);
}

bool ft_is_open(file_table *self, const char *name, int *err)
{
  ft_entry *entry = 0;
  bool is_open = false;

  if ((entry = (ft_entry *) htable_get(self, name)) == NULL) {
    return is_open;
  }

  pthread_rwlock_rdlock(&entry->mu);
  is_open = entry->open_cnt != 0;
  pthread_rwlock_unlock(&entry->mu);

  return is_open;
}

void ft_open_file(file_table *self, fp_node *fp, int *err)
{
  ft_entry *entry = 0;
  if ((entry = (ft_entry *) htable_get(self, fp->name)) == NULL) {
    entry = ft_entry_init(fp, 1);
    htable_set(self, fp->name, entry);
    return;
  }

  pthread_rwlock_rdlock(&entry->mu);
  entry->open_cnt++;
  pthread_rwlock_unlock(&entry->mu);
}

void ft_close_file(file_table *self, const char *name, int *err)
{
  ft_entry *entry = 0;
  if ((entry = (ft_entry *) htable_get(self, name)) == NULL) {
    *err = ERR_FILE_NOT_OPEN;
    return;
  }

  pthread_rwlock_rdlock(&entry->mu);
  entry->open_cnt--;
  if (entry->open_cnt == 0) {
    entry = htable_remove(self, name);
    free(entry);
    return;
  }
  pthread_rwlock_unlock(&entry->mu);
}

void ft_read_lock(file_table *self, const char *name, int *err)
{
  ft_entry *entry = 0;

  if ((entry = (ft_entry *) htable_get(self, name)) == NULL) {
    *err = ERR_FILE_NOT_OPEN;
    return;
  }

  pthread_rwlock_rdlock(&entry->mu);
}

void ft_read_unlock(file_table *self, const char *name, int *err)
{
  ft_entry *entry = 0;
  if ((entry = (ft_entry *) htable_get(self, name)) == NULL) {
    *err = ERR_FILE_NOT_OPEN;
    return;
  }

  pthread_rwlock_unlock(&entry->mu);
}

void ft_write_lock(file_table *self, const char *name, int *err)
{
  ft_entry *entry = 0;
  if ((entry = (ft_entry *) htable_get(self, name)) == NULL) {
    *err = ERR_FILE_NOT_OPEN;
    return;
  }

  // Block until we can acquire the write lock
  pthread_rwlock_wrlock(&entry->mu);
}

void ft_write_unlock(file_table *self, const char *name, int *err)
{
  ft_entry *entry = 0;
  if ((entry = (ft_entry *) htable_get(self, name)) == NULL) {
    *err = ERR_FILE_NOT_OPEN;
    return;
  }

  pthread_rwlock_unlock(&entry->mu);
}