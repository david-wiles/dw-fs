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
#include <printf.h>
#include <time.h>
#include <assert.h>

#include "dwfs.h"
#include "conf.h"
#include "htable.h"


dwfs *test_init_fs()
{
  // Initialize fs with four files. 8 blocks should still remain
  int err = 0;
  dwfs *instance = dwfs_init(12);
  dw_file f1 = dwfs_create(instance, "file 1", &err);
  dw_file f2 = dwfs_create(instance, "file 2", &err);
  dw_file f3 = dwfs_create(instance, "file 3", &err);
  dw_file f4 = dwfs_create(instance, "file 4", &err);

  dwfs_close(instance, &f1, &err);
  dwfs_close(instance, &f2, &err);
  dwfs_close(instance, &f3, &err);
  dwfs_close(instance, &f4, &err);

  if (err != 0) {
    printf("Could not initialize fs, %i", err);
  }

  return instance;
}

char *read_n(const unsigned char *block, int n)
{
  char *s = calloc(n + 1, sizeof(char));
  int i = 0;
  for (; i < n; i++) {
    s[i] = (char) block[i];
  }

  s[i] = '\0';

  return s;
}

// Read all text from a file
//
// Filename specifies the file to read, and text is the
// char array return value. The file will be opened using 'r' mode.
// The function returns the size of the array returned
long read_file(char *filename, char **text)
{
  FILE *fp = NULL;

  if ((fp = fopen(filename, "r")) == NULL) {
    return 0;
  }

  fseek(fp, 0L, SEEK_END);
  long size = ftell(fp);
  rewind(fp);

  if ((*text = malloc(size + 1)) == NULL) {
    return 0;
  }

  fread(*text, size, 1, fp);
  fclose(fp);

  (*text)[size] = '\0';

  return size;
}

void test_mem_create()
{
  dw_mem *mem = dw_mem_allocate(1);
  assert(mem != NULL);
  assert(mem->bitset != NULL);
  assert(mem->n_blocks == 1);
  assert(mem->n_free == 1);
  dw_mem_deallocate(mem);
}

void test_mem_get_block()
{
  dw_mem *mem = dw_mem_allocate(1);
  assert(mem != NULL);

  void *block = dw_mem_malloc(mem);
  assert(block != NULL);
  assert(mem->n_free == 0);
  assert(mem->blocks == block);

  dw_mem_deallocate(mem);

  mem = dw_mem_allocate(12);
  assert(mem != NULL);

  block = dw_mem_malloc(mem);
  assert(block != NULL);
  assert(mem->n_free == 11);
  assert(mem->blocks == block);

  block = dw_mem_malloc(mem);
  assert(block != NULL);
  assert(mem->n_free == 10);
  assert(mem->blocks + (BLOCK_SIZE) == block);

  block = dw_mem_malloc(mem);
  assert(block != NULL);
  assert(mem->n_free == 9);
  assert(mem->blocks + (BLOCK_SIZE * 2) == block);

  block = dw_mem_malloc(mem);
  assert(block != NULL);
  assert(mem->n_free == 8);
  assert(mem->blocks + (BLOCK_SIZE * 3) == block);

  block = dw_mem_malloc(mem);
  assert(block != NULL);
  assert(mem->n_free == 7);
  assert(mem->blocks + (BLOCK_SIZE * 4) == block);

  dw_mem_deallocate(mem);

  mem = dw_mem_allocate(4);
  assert(mem != NULL);

  block = dw_mem_malloc(mem);
  assert(block != NULL);
  assert(mem->n_free == 3);
  assert(mem->blocks + (BLOCK_SIZE * 0) == block);

  block = dw_mem_malloc(mem);
  assert(block != NULL);
  assert(mem->n_free == 2);
  assert(mem->blocks + (BLOCK_SIZE * 1) == block);

  block = dw_mem_malloc(mem);
  assert(block != NULL);
  assert(mem->n_free == 1);
  assert(mem->blocks + (BLOCK_SIZE * 2) == block);

  block = dw_mem_malloc(mem);
  assert(block != NULL);
  assert(mem->n_free == 0);
  assert(mem->blocks + (BLOCK_SIZE * 3) == block);

  // Make sure get_block returns null when there are no free blocks
  block = dw_mem_malloc(mem);
  assert(block == NULL);
  assert(mem->n_free == 0);

  dw_mem_deallocate(mem);
}

void test_mem_free_block_single()
{
  dw_mem *mem = dw_mem_allocate(4);
  assert(mem != NULL);

  void *block = dw_mem_malloc(mem);
  assert(block != NULL);
  assert(mem->n_free == 3);
  assert(mem->blocks + (BLOCK_SIZE * 0) == block);

  int err = 0;
  dw_mem_free(mem, block, &err);
  assert(err == 0);
  assert(bitset_get(mem->bitset, 0) == 0);

  dw_mem_free(mem, block, &err);
  assert(err == ERR_PTR_NOT_ALLOCATED);

  dw_mem_deallocate(mem);
}

void test_mem_free_block_multiple()
{
  dw_mem *mem = dw_mem_allocate(4);
  assert(mem != NULL);

  int err = 0;

  void *block1 = dw_mem_malloc(mem);
  assert(block1 != NULL);
  assert(mem->n_free == 3);
  assert(mem->blocks + (BLOCK_SIZE * 0) == block1);

  void *block2 = dw_mem_malloc(mem);
  assert(block2 != NULL);
  assert(mem->n_free == 2);
  assert(mem->blocks + (BLOCK_SIZE * 1) == block2);

  void *block3 = dw_mem_malloc(mem);
  assert(block3 != NULL);
  assert(mem->n_free == 1);
  assert(mem->blocks + (BLOCK_SIZE * 2) == block3);

  void *block4 = dw_mem_malloc(mem);
  assert(block4 != NULL);
  assert(mem->n_free == 0);
  assert(mem->blocks + (BLOCK_SIZE * 3) == block4);

  dw_mem_free(mem, block1, &err);
  assert(err == 0);
  assert(bitset_get(mem->bitset, 0) == 0);
  assert(mem->n_free == 1);

  dw_mem_free(mem, block4, &err);
  assert(err == 0);
  assert(bitset_get(mem->bitset, 3) == 0);
  assert(mem->n_free == 2);

  dw_mem_free(mem, block3, &err);
  assert(err == 0);
  assert(bitset_get(mem->bitset, 2) == 0);
  assert(mem->n_free == 3);

  dw_mem_free(mem, block2, &err);
  assert(err == 0);
  assert(bitset_get(mem->bitset, 1) == 0);
  assert(mem->n_free == 4);

  dw_mem_deallocate(mem);
}

void test_mem_repeated_ops()
{
  dw_mem *mem = dw_mem_allocate(4);
  assert(mem != NULL);
  int err = 0;

  void *block1 = dw_mem_malloc(mem);
  assert(block1 != NULL);
  assert(mem->n_free == 3);
  assert(mem->blocks + (BLOCK_SIZE * 0) == block1);

  void *block2 = dw_mem_malloc(mem);
  assert(block2 != NULL);
  assert(mem->n_free == 2);
  assert(mem->blocks + (BLOCK_SIZE * 1) == block2);

  void *block3 = dw_mem_malloc(mem);
  assert(block3 != NULL);
  assert(mem->n_free == 1);
  assert(mem->blocks + (BLOCK_SIZE * 2) == block3);

  void *block4 = dw_mem_malloc(mem);
  assert(block4 != NULL);
  assert(mem->n_free == 0);
  assert(mem->blocks + (BLOCK_SIZE * 3) == block4);

  void *none = dw_mem_malloc(mem);
  assert(none == NULL);
  assert(mem->n_free == 0);

  dw_mem_free(mem, block1, &err);
  assert(err == 0);
  assert(bitset_get(mem->bitset, 0) == 0);
  assert(mem->n_free == 1);

  void *block1_2 = dw_mem_malloc(mem);
  assert(block1_2 != NULL);
  assert(mem->n_free == 0);
  assert(mem->blocks + (BLOCK_SIZE * 0) == block1);

  dw_mem_free(mem, block2, &err);
  assert(err == 0);
  assert(bitset_get(mem->bitset, 1) == 0);
  assert(mem->n_free == 1);

  dw_mem_free(mem, block4, &err);
  assert(err == 0);
  assert(bitset_get(mem->bitset, 3) == 0);
  assert(mem->n_free == 2);

  void *block2_2 = dw_mem_malloc(mem);
  assert(block2_2 != NULL);
  assert(mem->n_free == 1);
  assert(mem->blocks + (BLOCK_SIZE * 1) == block2_2);

  void *block4_2 = dw_mem_malloc(mem);
  assert(block4_2 != NULL);
  assert(mem->n_free == 0);
  assert(mem->blocks + (BLOCK_SIZE * 3) == block4_2);

  dw_mem_free(mem, block2_2, &err);
  assert(err == 0);
  assert(bitset_get(mem->bitset, 1) == 0);
  assert(mem->n_free == 1);

  dw_mem_deallocate(mem);
}

// Repeated operations to ensure longevity
void test_mem_longevity()
{
  dw_mem *mem = dw_mem_allocate(4);
  void *block = 0;
  int err = 0;

  assert(mem != NULL);

  srand(time(0));

  for (int i = 0; i < 100000; i++) {
    if (rand() & 1)
      block = dw_mem_malloc(mem);
    else
      dw_mem_free(mem, block, &err);
  }

  dw_mem_deallocate(mem);
}


void test_dw_fs_init()
{
  dwfs *instance = dwfs_init(1);

  assert(instance->blocks != NULL);
  assert(instance->blocks->n_blocks == 1);

  // Test size of allocated blocks
}

void test_dw_fs_free()
{
  dwfs *instance = dwfs_init(1);

  assert(instance->blocks != NULL);
  assert(instance->blocks->n_blocks == 1);

  dwfs_free(instance);
}

void test_dw_fs_search_file()
{
  dwfs *instance = dwfs_init(12);

  int err = 0;
  dwfs_create(instance, "file 1", &err);
  assert(err == 0);

  // Search can find file
  fp_node *f = dw_dir_search_file(instance->dir, "file 1", &err);
  assert(err == 0);
  assert(f != NULL);
  assert(strcmp(f->name, "file 1") == 0);

  // Search for non-existent file returns null pointer
  f = dw_dir_search_file(instance->dir, "file 0", &err);
  assert(err == ERR_NOT_EXISTS);
  assert(f == NULL);

  // Test finds many files
  dwfs_create(instance, "file 2", &err);
  dwfs_create(instance, "file 3", &err);
  dwfs_create(instance, "file 4", &err);
  dwfs_create(instance, "file 5", &err);
  dwfs_create(instance, "file 6", &err);
  dwfs_create(instance, "file 7", &err);

  err = 0;
  f = dw_dir_search_file(instance->dir, "file 7", &err);
  assert(err == 0);
  assert(f != NULL);
  assert(strcmp(f->name, "file 7") == 0);

  f = dw_dir_search_file(instance->dir, "file 6", &err);
  assert(err == 0);
  assert(f != NULL);
  assert(strcmp(f->name, "file 6") == 0);

  f = dw_dir_search_file(instance->dir, "file 5", &err);
  assert(err == 0);
  assert(f != NULL);
  assert(strcmp(f->name, "file 5") == 0);
}


void test_ft_init()
{
  file_table *ft = ft_init(1);
  assert(ft != NULL);

  file_table *ft2 = ft_init(2048);
  assert(ft2 != NULL);
}

void test_ft_free()
{
  file_table *ft = ft_init(1);
  assert(ft != NULL);
  ft_free(ft);

  file_table *ft2 = ft_init(2048);
  assert(ft2 != NULL);
  ft_free(ft2);
}

void test_ft_entry_init()
{
  ft_entry *entry = ft_entry_init(malloc(1), 1);
  assert(entry != NULL);
  assert(entry->open_cnt == 1);
}

void test_ft_is_open()
{
  file_table *ft = ft_init(4);
  assert(ft != NULL);

  int err = 0;
  bool is_open = ft_is_open(ft, "filename", &err);
  assert(err == 0);
  assert(is_open == 0);
}

void test_ft_open_file()
{
  file_table *ft = ft_init(4);
  assert(ft != NULL);

  int err = 0;
  fp_node fp1 = (fp_node) {"file 1"};
  ft_open_file(ft, &fp1, &err);
  assert(err == 0);

  ft_entry *entry = (ft_entry *) htable_get(ft, fp1.name);
  assert(entry != NULL);
  assert(entry->open_cnt == 1);

  fp_node fp2 = (fp_node) {"file 2"};
  ft_open_file(ft, &fp2, &err);
  assert(err == 0);

  ft_entry *entry2 = (ft_entry *) htable_get(ft, fp2.name);
  assert(entry2 != NULL);
  assert(entry2->open_cnt == 1);

  ft_open_file(ft, &fp2, &err);
  assert(err == 0);

  ft_entry *entry3 = (ft_entry *) htable_get(ft, fp2.name);
  assert(entry3 != NULL);
  assert(entry3->open_cnt == 2);
}

void test_ft_close_file()
{
  file_table *ft = ft_init(4);
  assert(ft != NULL);

  int err = 0;
  fp_node fp1 = (fp_node) {"file 1"};
  ft_open_file(ft, &fp1, &err);
  assert(err == 0);

  ft_entry *entry = (ft_entry *) htable_get(ft, fp1.name);
  assert(entry != NULL);
  assert(entry->open_cnt == 1);

  ft_close_file(ft, fp1.name, &err);
  assert(err == 0);

  ft_entry *entry2 = (ft_entry *) htable_get(ft, fp1.name);
  assert(entry2 == NULL);

  assert(ft_is_open(ft, "file 1", &err) == 0);

  fp_node fp2 = (fp_node) {"file 2"};
  ft_open_file(ft, &fp2, &err);
  assert(err == 0);

  ft_open_file(ft, &fp2, &err);
  assert(err == 0);

  ft_close_file(ft, fp2.name, &err);
  assert(err == 0);

  ft_entry *entry3 = (ft_entry *) htable_get(ft, fp2.name);
  assert(entry3 != NULL);
  assert(entry3->open_cnt == 1);

  ft_close_file(ft, fp2.name, &err);
  assert(err == 0);

  ft_entry *entry4 = (ft_entry *) htable_get(ft, fp2.name);
  assert(entry4 == NULL);

  ft_close_file(ft, fp2.name, &err);
  assert(err == ERR_FILE_NOT_OPEN);

  fp_node fp3 = (fp_node) {"third file"};
  ft_close_file(ft, fp3.name, &err);
  assert(err == ERR_FILE_NOT_OPEN);
}

void test_file_create()
{
  dwfs *instance = dwfs_init(12);

  int err = 0;
  dw_file f = dwfs_create(instance, "name", &err);

  assert(err == 0);
  assert(strcmp(f.name, "name") == 0);
  assert(f.fp != NULL);
  assert(instance->dir->n_files == 1);

  // Creating a file with the same name should fail
  err = 0;
  dwfs_create(instance, "name", &err);
  assert(err == ERR_NON_UNIQUE_NAME);
  assert(instance->dir->n_files == 1);

  err = 0;
  f = dwfs_create(instance, "name2", &err);
  assert(err == 0);
  assert(strcmp(f.name, "name2") == 0);
  assert(f.fp != NULL);
  assert(instance->dir->n_files == 2);

  fp_node *fp = instance->dir->head;

  assert(strcmp(fp->name, "name2") == 0);

  fp = fp->next;

  assert(strcmp(fp->name, "name") == 0);

  fp = fp->next;

  assert(fp == NULL);

  dwfs_free(instance);

  // Test create 4 files
  instance = test_init_fs();
  assert(instance->dir->n_files == 4);
  fp = instance->dir->head;
  assert(strcmp(fp->name, "file 4") == 0);
  fp = fp->next;
  assert(strcmp(fp->name, "file 3") == 0);
  fp = fp->next;
  assert(strcmp(fp->name, "file 2") == 0);
  fp = fp->next;
  assert(strcmp(fp->name, "file 1") == 0);
  assert(fp->next == NULL);
  dwfs_free(instance);
}

void test_file_delete()
{
  dwfs *instance = test_init_fs();
  int err = 0;

  // Delete file that does not exist
  dwfs_delete(instance, "file 5", &err);
  assert(err == ERR_NOT_EXISTS);
  assert(instance->dir->n_files == 4);

  err = 0;

  // Delete files in directory order
  dwfs_delete(instance, "file 4", &err);
  assert(err == 0);
  assert(instance->dir->n_files == 3);
  assert(strcmp(instance->dir->head->name, "file 3") == 0);

  dwfs_delete(instance, "file 3", &err);
  assert(err == 0);
  assert(instance->dir->n_files == 2);
  assert(strcmp(instance->dir->head->name, "file 2") == 0);

  dwfs_delete(instance, "file 2", &err);
  assert(err == 0);
  assert(instance->dir->n_files == 1);
  assert(strcmp(instance->dir->head->name, "file 1") == 0);

  dwfs_delete(instance, "file 1", &err);
  assert(err == 0);
  assert(instance->dir->n_files == 0);
  assert(instance->dir->head == NULL);

  dwfs_free(instance);

  instance = test_init_fs();

  // Delete files in reverse order
  dwfs_delete(instance, "file 1", &err);
  assert(err == 0);
  assert(instance->dir->n_files == 3);
  assert(strcmp(instance->dir->head->name, "file 4") == 0);

  dwfs_delete(instance, "file 2", &err);
  assert(err == 0);
  assert(instance->dir->n_files == 2);
  assert(strcmp(instance->dir->head->name, "file 4") == 0);

  dwfs_delete(instance, "file 3", &err);
  assert(err == 0);
  assert(instance->dir->n_files == 1);
  assert(strcmp(instance->dir->head->name, "file 4") == 0);

  dwfs_delete(instance, "file 4", &err);
  assert(err == 0);
  assert(instance->dir->n_files == 0);
  assert(instance->dir->head == NULL);

  dwfs_free(instance);

  instance = test_init_fs();

  // Delete files in arbitrary order

  dwfs_delete(instance, "file 3", &err);
  assert(err == 0);
  assert(instance->dir->n_files == 3);
  assert(strcmp(instance->dir->head->name, "file 4") == 0);

  dwfs_delete(instance, "file 4", &err);
  assert(err == 0);
  assert(instance->dir->n_files == 2);
  assert(strcmp(instance->dir->head->name, "file 2") == 0);

  dwfs_delete(instance, "file 2", &err);
  assert(err == 0);
  assert(instance->dir->n_files == 1);
  assert(strcmp(instance->dir->head->name, "file 1") == 0);

  dwfs_delete(instance, "file 1", &err);
  assert(err == 0);
  assert(instance->dir->n_files == 0);
  assert(instance->dir->head == NULL);

  dwfs_free(instance);

  instance = dwfs_init(1024);

  // Write large file
  char *f_text = NULL;
  int f_size = (int) read_file("./asset/hamlet.txt", &f_text);
  assert(f_size > 0);
  assert(f_text != NULL);

  dw_file f = dwfs_create(instance, "hamlet.txt", &err);
  assert(err == 0);

  dwfs_write(instance, &f, (unsigned char *) f_text, f_size, &err);
  assert(err == 0);

  dwfs_close(instance, &f, &err);
  assert(err == 0);

  dwfs_delete(instance, "hamlet.txt", &err);
  assert(err == 0);
  assert(instance->dir->n_files == 0);
  assert(instance->dir->head == NULL);
}

void test_file_write()
{
  int err = 0;
  dwfs *instance = dwfs_init(12);

  dw_file f = dwfs_create(instance, "file 1", &err);
  assert(err == 0);

  fp_node *fp = f.fp;

  dwfs_open(instance, "file 1", &err);
  assert(err == 0);

  dwfs_write(instance, &f, (unsigned char *) "asdfasdf", 8, &err);
  assert(err == 0);
  assert(fp->data != NULL);
  assert(strcmp(read_n(fp->data->data, 8), "asdfasdf") == 0);

  dwfs_write(instance, &f, (unsigned char *) "asdfasdf", 8, &err);
  assert(err == 0);
  assert(fp->data != NULL);
  assert(strcmp(read_n(fp->data->data, 16), "asdfasdfasdfasdf") == 0);

  f = dwfs_create(instance, "file 2", &err);
  assert(err == 0);

  fp = f.fp;

  dwfs_write(instance, &f,
             (unsigned char *) "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
             1000, &err);
  assert(err == 0);
  assert(fp->data != NULL);

  // On my development machine, the MAX_DATA_SIZE macro expands to 500
  assert(fp->data->bytes == MAX_DATA_SIZE > 1000 ? 1000 : MAX_DATA_SIZE);
  assert(strcmp(read_n(fp->data->data, 500),
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa") ==
         0);

  dwfs_free(instance);
}

void test_file_read()
{
  int err = 0;
  dwfs *instance = dwfs_init(1024);

  dwfs_create(instance, "file 1", &err);
  assert(err == 0);

  fp_node *fp = instance->dir->head;
  dw_file f = (dw_file) {
          "file 1",
          fp
  };

  dwfs_write(instance, &f, (unsigned char *) "asdfasdf", 8, &err);
  assert(err == 0);
  assert(fp->data != NULL);

  unsigned int n_read = 0;
  unsigned char *read_bytes = dwfs_read(instance, &f, 8, &n_read, &err);
  assert(read_bytes != NULL);
  assert(strcmp(read_n(read_bytes, 8), "asdfasdf") == 0);

  dwfs_write(instance, &f, (unsigned char *) "asdfasdfasdfasdf", 8, &err);
  assert(err == 0);
  assert(fp->data != NULL);

  n_read = 0;
  read_bytes = dwfs_read(instance, &f, 16, &n_read, &err);
  assert(read_bytes != NULL);
  assert(strcmp(read_n(read_bytes, 16), "asdfasdfasdfasdf") == 0);

  dwfs_create(instance, "file 2", &err);
  assert(err == 0);

  fp = instance->dir->head;

  assert(strcmp(fp->name, "file 2") == 0);

  f = (dw_file) {
          "file 2",
          fp
  };

  dwfs_write(instance, &f,
             (unsigned char *) "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
             1000, &err);
  assert(err == 0);
  assert(fp->data != NULL);

  n_read = 0;
  read_bytes = dwfs_read(instance, &f, 1000, &n_read, &err);
  assert(read_bytes != NULL);
  assert(strcmp(read_n(read_bytes, 1000),
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa") ==
         0);

  // Write large file
  char *f_text = NULL;
  int f_size = (int) read_file("./asset/hamlet.txt", &f_text);
  assert(f_size > 0);
  assert(f_text != NULL);

  f = dwfs_create(instance, "hamlet.txt", &err);
  assert(err == 0);

  dwfs_write(instance, &f, (unsigned char *) f_text, f_size, &err);
  assert(err == 0);

  read_bytes = dwfs_read(instance, &f, f_size, &n_read, &err);
  assert(err == 0);
  assert(read_bytes != NULL);
  assert(strcmp(read_n(read_bytes, f_size), f_text) == 0);

  dwfs_free(instance);
}

void test_dir()
{
  dwfs *instance = dwfs_init(256);
  assert(instance != NULL);

  int err = 0;
  dwfs_create(instance, "file 1", &err);
  assert(err == 0);

  int len = 0;
  char **files = dwfs_dir(instance, &len, &err);
  assert(len == 1);
  assert(err == 0);
  assert(strcmp(files[0], "file 1") == 0);

  dwfs_create(instance, "2nd file", &err);
  assert(err == 0);
  dwfs_create(instance, "another filename", &err);
  assert(err == 0);
  dwfs_create(instance, "file 5", &err);
  assert(err == 0);
  dwfs_create(instance, "file 6", &err);
  assert(err == 0);
  dwfs_create(instance, "f", &err);
  assert(err == 0);

  files = dwfs_dir(instance, &len, &err);
  assert(len == 6);
  assert(err == 0);
  assert(strcmp(files[0], "f") == 0);
  assert(strcmp(files[1], "file 6") == 0);
  assert(strcmp(files[2], "file 5") == 0);
  assert(strcmp(files[3], "another filename") == 0);
  assert(strcmp(files[4], "2nd file") == 0);
  assert(strcmp(files[5], "file 1") == 0);
}

int main()
{
  test_mem_create();
  test_mem_get_block();
  test_mem_free_block_single();
  test_mem_free_block_multiple();
  test_mem_repeated_ops();
  test_mem_longevity();
  test_dw_fs_init();
  test_dw_fs_free();
  test_dw_fs_search_file();
  test_ft_init();
  test_ft_free();
  test_ft_entry_init();
  test_ft_is_open();
  test_ft_open_file();
  test_ft_close_file();
  test_file_create();
  test_file_delete();
  test_file_write();
  test_file_read();
  test_dir();

  printf("All tests passed successfully\n");

  return 0;
}

