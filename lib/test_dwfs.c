#include <stdlib.h>
#include <check.h>
#include <printf.h>
#include <time.h>
#include "dwfs.h"
#include "conf.h"


dwfs *test_init_fs()
{
  // Initialize fs with four files. 8 blocks should still remain
  int err = 0;
  dwfs *instance = dwfs_init(12);
  dwfs_create(instance, "file 1", &err);
  dwfs_create(instance, "file 2", &err);
  dwfs_create(instance, "file 3", &err);
  dwfs_create(instance, "file 4", &err);

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

START_TEST(test_mem_create)
{
  dw_mem *mem = allocate(1);
  ck_assert_ptr_nonnull(mem);
  ck_assert_ptr_nonnull(mem->bitset);
  ck_assert_int_eq(mem->n_blocks, 1);
  ck_assert_int_eq(mem->n_free, 1);
  deallocate(mem);
}

START_TEST(test_mem_get_block)
{
  dw_mem *mem = allocate(1);
  ck_assert_ptr_nonnull(mem);

  void *block = get_block(mem);
  ck_assert_ptr_nonnull(block);
  ck_assert_int_eq(mem->n_free, 0);
  ck_assert_ptr_eq(mem->blocks, block);

  deallocate(mem);

  mem = allocate(12);
  ck_assert_ptr_nonnull(mem);

  block = get_block(mem);
  ck_assert_ptr_nonnull(block);
  ck_assert_int_eq(mem->n_free, 11);
  ck_assert_ptr_eq(mem->blocks, block);

  block = get_block(mem);
  ck_assert_ptr_nonnull(block);
  ck_assert_int_eq(mem->n_free, 10);
  ck_assert_ptr_eq(mem->blocks + (BLOCK_SIZE), block);

  block = get_block(mem);
  ck_assert_ptr_nonnull(block);
  ck_assert_int_eq(mem->n_free, 9);
  ck_assert_ptr_eq(mem->blocks + (BLOCK_SIZE * 2), block);

  block = get_block(mem);
  ck_assert_ptr_nonnull(block);
  ck_assert_int_eq(mem->n_free, 8);
  ck_assert_ptr_eq(mem->blocks + (BLOCK_SIZE * 3), block);

  block = get_block(mem);
  ck_assert_ptr_nonnull(block);
  ck_assert_int_eq(mem->n_free, 7);
  ck_assert_ptr_eq(mem->blocks + (BLOCK_SIZE * 4), block);

  deallocate(mem);

  mem = allocate(4);
  ck_assert_ptr_nonnull(mem);

  block = get_block(mem);
  ck_assert_ptr_nonnull(block);
  ck_assert_int_eq(mem->n_free, 3);
  ck_assert_ptr_eq(mem->blocks + (BLOCK_SIZE * 0), block);

  block = get_block(mem);
  ck_assert_ptr_nonnull(block);
  ck_assert_int_eq(mem->n_free, 2);
  ck_assert_ptr_eq(mem->blocks + (BLOCK_SIZE * 1), block);

  block = get_block(mem);
  ck_assert_ptr_nonnull(block);
  ck_assert_int_eq(mem->n_free, 1);
  ck_assert_ptr_eq(mem->blocks + (BLOCK_SIZE * 2), block);

  block = get_block(mem);
  ck_assert_ptr_nonnull(block);
  ck_assert_int_eq(mem->n_free, 0);
  ck_assert_ptr_eq(mem->blocks + (BLOCK_SIZE * 3), block);

  // Make sure get_block returns null when there are no free blocks
  block = get_block(mem);
  ck_assert_ptr_null(block);
  ck_assert_int_eq(mem->n_free, 0);

  deallocate(mem);
}

START_TEST(test_mem_free_block_single)
{
  dw_mem *mem = allocate(4);
  ck_assert_ptr_nonnull(mem);

  void *block = get_block(mem);
  ck_assert_ptr_nonnull(block);
  ck_assert_int_eq(mem->n_free, 3);
  ck_assert_ptr_eq(mem->blocks + (BLOCK_SIZE * 0), block);

  int err = 0;
  free_block(mem, block, &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(bitset_get(mem->bitset, 0), 0);

  free_block(mem, block, &err);
  ck_assert_int_eq(err, ERR_PTR_NOT_ALLOCATED);

  deallocate(mem);
}

START_TEST(test_mem_free_block_multiple)
{
  dw_mem *mem = allocate(4);
  ck_assert_ptr_nonnull(mem);

  int err = 0;

  void *block1 = get_block(mem);
  ck_assert_ptr_nonnull(block1);
  ck_assert_int_eq(mem->n_free, 3);
  ck_assert_ptr_eq(mem->blocks + (BLOCK_SIZE * 0), block1);

  void *block2 = get_block(mem);
  ck_assert_ptr_nonnull(block2);
  ck_assert_int_eq(mem->n_free, 2);
  ck_assert_ptr_eq(mem->blocks + (BLOCK_SIZE * 1), block2);

  void *block3 = get_block(mem);
  ck_assert_ptr_nonnull(block3);
  ck_assert_int_eq(mem->n_free, 1);
  ck_assert_ptr_eq(mem->blocks + (BLOCK_SIZE * 2), block3);

  void *block4 = get_block(mem);
  ck_assert_ptr_nonnull(block4);
  ck_assert_int_eq(mem->n_free, 0);
  ck_assert_ptr_eq(mem->blocks + (BLOCK_SIZE * 3), block4);

  free_block(mem, block1, &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(bitset_get(mem->bitset, 0), 0);
  ck_assert_int_eq(mem->n_free, 1);

  free_block(mem, block4, &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(bitset_get(mem->bitset, 3), 0);
  ck_assert_int_eq(mem->n_free, 2);

  free_block(mem, block3, &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(bitset_get(mem->bitset, 2), 0);
  ck_assert_int_eq(mem->n_free, 3);

  free_block(mem, block2, &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(bitset_get(mem->bitset, 1), 0);
  ck_assert_int_eq(mem->n_free, 4);

  deallocate(mem);
}

START_TEST(test_mem_repeated_ops)
{
  dw_mem *mem = allocate(4);
  ck_assert_ptr_nonnull(mem);
  int err = 0;

  void *block1 = get_block(mem);
  ck_assert_ptr_nonnull(block1);
  ck_assert_int_eq(mem->n_free, 3);
  ck_assert_ptr_eq(mem->blocks + (BLOCK_SIZE * 0), block1);

  void *block2 = get_block(mem);
  ck_assert_ptr_nonnull(block2);
  ck_assert_int_eq(mem->n_free, 2);
  ck_assert_ptr_eq(mem->blocks + (BLOCK_SIZE * 1), block2);

  void *block3 = get_block(mem);
  ck_assert_ptr_nonnull(block3);
  ck_assert_int_eq(mem->n_free, 1);
  ck_assert_ptr_eq(mem->blocks + (BLOCK_SIZE * 2), block3);

  void *block4 = get_block(mem);
  ck_assert_ptr_nonnull(block4);
  ck_assert_int_eq(mem->n_free, 0);
  ck_assert_ptr_eq(mem->blocks + (BLOCK_SIZE * 3), block4);

  void *none = get_block(mem);
  ck_assert_ptr_null(none);
  ck_assert_int_eq(mem->n_free, 0);

  free_block(mem, block1, &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(bitset_get(mem->bitset, 0), 0);
  ck_assert_int_eq(mem->n_free, 1);

  void *block1_2 = get_block(mem);
  ck_assert_ptr_nonnull(block1_2);
  ck_assert_int_eq(mem->n_free, 0);
  ck_assert_ptr_eq(mem->blocks + (BLOCK_SIZE * 0), block1);

  free_block(mem, block2, &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(bitset_get(mem->bitset, 1), 0);
  ck_assert_int_eq(mem->n_free, 1);

  free_block(mem, block4, &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(bitset_get(mem->bitset, 3), 0);
  ck_assert_int_eq(mem->n_free, 2);

  void *block2_2 = get_block(mem);
  ck_assert_ptr_nonnull(block2_2);
  ck_assert_int_eq(mem->n_free, 1);
  ck_assert_ptr_eq(mem->blocks + (BLOCK_SIZE * 1), block2_2);

  void *block4_2 = get_block(mem);
  ck_assert_ptr_nonnull(block4_2);
  ck_assert_int_eq(mem->n_free, 0);
  ck_assert_ptr_eq(mem->blocks + (BLOCK_SIZE * 3), block4_2);

  free_block(mem, block2_2, &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(bitset_get(mem->bitset, 1), 0);
  ck_assert_int_eq(mem->n_free, 1);

  deallocate(mem);
}

// Repeated operations to ensure longevity
START_TEST(test_mem_longevity)
{
  dw_mem *mem = allocate(4);
  void *block = 0;
  int err = 0;

  ck_assert_ptr_nonnull(mem);

  srand(time(0));

  for (int i = 0; i < 100000; i++) {
    if (rand() & 1)
      block = get_block(mem);
    else
      free_block(mem, block, &err);
  }

  deallocate(mem);
}


START_TEST(test_dw_fs_init)
{
  dwfs *instance = dwfs_init(1);

  ck_assert_ptr_nonnull(instance->blocks);
  ck_assert_int_eq(instance->blocks->n_blocks, 1);

  // Test size of allocated blocks
}

START_TEST(test_dw_fs_free)
{
  dwfs *instance = dwfs_init(1);

  ck_assert_ptr_nonnull(instance->blocks);
  ck_assert_int_eq(instance->blocks->n_blocks, 1);

  dwfs_free(instance);
}

START_TEST(test_dw_fs_search_file)
{
  dwfs *instance = dwfs_init(12);

  int err = 0;
  dwfs_create(instance, "file 1", &err);
  ck_assert_int_eq(err, 0);

  // Search can find file
  fp_node *f = search_file(instance->dir, "file 1", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_ptr_nonnull(f);
  ck_assert_str_eq(f->name, "file 1");

  // Search for non-existent file returns null pointer
  f = search_file(instance->dir, "file 0", &err);
  ck_assert_int_eq(err, ERR_NOT_EXISTS);
  ck_assert_ptr_null(f);

  // Test finds many files
  dwfs_create(instance, "file 2", &err);
  dwfs_create(instance, "file 3", &err);
  dwfs_create(instance, "file 4", &err);
  dwfs_create(instance, "file 5", &err);
  dwfs_create(instance, "file 6", &err);
  dwfs_create(instance, "file 7", &err);

  err = 0;
  f = search_file(instance->dir, "file 7", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_ptr_nonnull(f);
  ck_assert_str_eq(f->name, "file 7");

  f = search_file(instance->dir, "file 6", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_ptr_nonnull(f);
  ck_assert_str_eq(f->name, "file 6");

  f = search_file(instance->dir, "file 5", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_ptr_nonnull(f);
  ck_assert_str_eq(f->name, "file 5");
}


START_TEST(test_file_create)
{
  dwfs *instance = dwfs_init(12);

  int err = 0;
  dw_file f = dwfs_create(instance, "name", &err);

  ck_assert_int_eq(err, 0);
  ck_assert_str_eq(f.name, "name");
  ck_assert_ptr_nonnull(f.fp);
  ck_assert_int_eq(instance->dir->n_files, 1);

  // Creating a file with the same name should fail
  err = 0;
  f = dwfs_create(instance, "name", &err);

  ck_assert_int_eq(err, ERR_NON_UNIQUE_NAME);
  ck_assert_ptr_null(f.fp);
  ck_assert_int_eq(instance->dir->n_files, 1);

  err = 0;
  f = dwfs_create(instance, "name2", &err);

  ck_assert_int_eq(err, 0);
  ck_assert_str_eq(f.name, "name2");
  ck_assert_ptr_nonnull(f.fp);
  ck_assert_int_eq(instance->dir->n_files, 2);

  fp_node *fp = instance->dir->head;

  ck_assert_str_eq(fp->name, "name2");

  fp = fp->next;

  ck_assert_str_eq(fp->name, "name");

  fp = fp->next;

  ck_assert_ptr_null(fp);

  dwfs_free(instance);

  // Test create 4 files
  instance = test_init_fs();
  ck_assert_int_eq(instance->dir->n_files, 4);
  fp = instance->dir->head;
  ck_assert_str_eq(fp->name, "file 4");
  fp = fp->next;
  ck_assert_str_eq(fp->name, "file 3");
  fp = fp->next;
  ck_assert_str_eq(fp->name, "file 2");
  fp = fp->next;
  ck_assert_str_eq(fp->name, "file 1");
  ck_assert_ptr_null(fp->next);
  dwfs_free(instance);
}

START_TEST(test_file_delete)
{
  dwfs *instance = test_init_fs();
  int err = 0;

  // Delete file that does not exist
  dwfs_delete(instance, "file 5", &err);
  ck_assert_int_eq(err, ERR_NOT_EXISTS);
  ck_assert_int_eq(instance->dir->n_files, 4);

  err = 0;

  // Delete files in directory order
  dwfs_delete(instance, "file 4", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(instance->dir->n_files, 3);
  ck_assert_str_eq(instance->dir->head->name, "file 3");

  dwfs_delete(instance, "file 3", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(instance->dir->n_files, 2);
  ck_assert_str_eq(instance->dir->head->name, "file 2");

  dwfs_delete(instance, "file 2", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(instance->dir->n_files, 1);
  ck_assert_str_eq(instance->dir->head->name, "file 1");

  dwfs_delete(instance, "file 1", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(instance->dir->n_files, 0);
  ck_assert_ptr_null(instance->dir->head);

  dwfs_free(instance);

  instance = test_init_fs();

  // Delete files in reverse order
  dwfs_delete(instance, "file 1", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(instance->dir->n_files, 3);
  ck_assert_str_eq(instance->dir->head->name, "file 4");

  dwfs_delete(instance, "file 2", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(instance->dir->n_files, 2);
  ck_assert_str_eq(instance->dir->head->name, "file 4");

  dwfs_delete(instance, "file 3", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(instance->dir->n_files, 1);
  ck_assert_str_eq(instance->dir->head->name, "file 4");

  dwfs_delete(instance, "file 4", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(instance->dir->n_files, 0);
  ck_assert_ptr_null(instance->dir->head);

  dwfs_free(instance);

  instance = test_init_fs();

  // Delete files in arbitrary order

  dwfs_delete(instance, "file 3", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(instance->dir->n_files, 3);
  ck_assert_str_eq(instance->dir->head->name, "file 4");

  dwfs_delete(instance, "file 4", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(instance->dir->n_files, 2);
  ck_assert_str_eq(instance->dir->head->name, "file 2");

  dwfs_delete(instance, "file 2", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(instance->dir->n_files, 1);
  ck_assert_str_eq(instance->dir->head->name, "file 1");

  dwfs_delete(instance, "file 1", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(instance->dir->n_files, 0);
  ck_assert_ptr_null(instance->dir->head);

  dwfs_free(instance);
}

START_TEST(test_file_write)
{
  int err = 0;
  dwfs *instance = dwfs_init(12);

  dwfs_create(instance, "file 1", &err);
  ck_assert_int_eq(err, 0);

  fp_node *fp = instance->dir->head;
  dw_file f = (dw_file) {
          "file 1",
          fp
  };

  dwfs_write(instance, &f, "asdfasdf", 8, &err);
  ck_assert_int_eq(err, 0);
  ck_assert_ptr_nonnull(fp->data);
  ck_assert_str_eq(read_n(fp->data->data, 8), "asdfasdf");

  dwfs_write(instance, &f, "asdfasdf", 8, &err);
  ck_assert_int_eq(err, 0);
  ck_assert_ptr_nonnull(fp->data);
  ck_assert_str_eq(read_n(fp->data->data, 16), "asdfasdfasdfasdf");

  dwfs_create(instance, "file 2", &err);
  ck_assert_int_eq(err, 0);

  fp = instance->dir->head;

  ck_assert_str_eq(fp->name, "file 2");

  f = (dw_file) {
          "file 2",
          fp
  };

  dwfs_write(instance, &f,
             "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
             1000, &err);
  ck_assert_int_eq(err, 0);
  ck_assert_ptr_nonnull(fp->data);

  // On my development machine, the MAX_DATA_SIZE macro expands to 500
  ck_assert_int_eq(fp->data->bytes, 500);
  ck_assert_str_eq(read_n(fp->data->data, 500),
                   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
  ck_assert_ptr_nonnull(fp->data->next);
  ck_assert_int_eq(fp->data->next->bytes, 500);
  ck_assert_str_eq(read_n(fp->data->next->data, 500),
                   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

  dwfs_free(instance);
}

START_TEST(test_file_read)
{
  int err = 0;
  dwfs *instance = dwfs_init(12);

  dwfs_create(instance, "file 1", &err);
  ck_assert_int_eq(err, 0);

  fp_node *fp = instance->dir->head;
  dw_file f = (dw_file) {
          "file 1",
          fp
  };

  dwfs_write(instance, &f, "asdfasdf", 8, &err);
  ck_assert_int_eq(err, 0);
  ck_assert_ptr_nonnull(fp->data);

  unsigned char *read_bytes = dwfs_read(instance, &f, 8, &err);
  ck_assert_ptr_nonnull(read_bytes);
  ck_assert_str_eq(read_n(read_bytes, 8), "asdfasdf");

  dwfs_write(instance, &f, "asdfasdfasdfasdf", 8, &err);
  ck_assert_int_eq(err, 0);
  ck_assert_ptr_nonnull(fp->data);

  read_bytes = dwfs_read(instance, &f, 16, &err);
  ck_assert_ptr_nonnull(read_bytes);
  ck_assert_str_eq(read_n(read_bytes, 16), "asdfasdfasdfasdf");

  dwfs_create(instance, "file 2", &err);
  ck_assert_int_eq(err, 0);

  fp = instance->dir->head;

  ck_assert_str_eq(fp->name, "file 2");

  f = (dw_file) {
          "file 2",
          fp
  };

  dwfs_write(instance, &f,
             "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
             1000, &err);
  ck_assert_int_eq(err, 0);
  ck_assert_ptr_nonnull(fp->data);

  read_bytes = dwfs_read(instance, &f, 1000, &err);
  ck_assert_ptr_nonnull(read_bytes);
  ck_assert_str_eq(read_n(read_bytes, 1000), "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

  dwfs_free(instance);
}


Suite *mem_suite()
{
  Suite *suite;
  TCase *t_case;
  suite = suite_create("dwfs_create and destroy");
  t_case = tcase_create("Core");
  tcase_add_test(t_case, test_mem_create);
  tcase_add_test(t_case, test_mem_get_block);
  tcase_add_test(t_case, test_mem_free_block_single);
  tcase_add_test(t_case, test_mem_free_block_multiple);
  tcase_add_test(t_case, test_mem_repeated_ops);
  tcase_add_test(t_case, test_mem_longevity);
  suite_add_tcase(suite, t_case);
  return suite;
}

Suite *file_suite()
{
  Suite *suite;
  TCase *t_case;
  suite = suite_create("dwfs_create and destroy");
  t_case = tcase_create("Core");
  tcase_add_test(t_case, test_file_create);
  tcase_add_test(t_case, test_file_delete);
  tcase_add_test(t_case, test_file_write);
  tcase_add_test(t_case, test_file_read);
  suite_add_tcase(suite, t_case);
  return suite;
}

Suite *fs_suite()
{
  Suite *suite;
  TCase *t_case;
  suite = suite_create("dwfs_create and destroy");
  t_case = tcase_create("Core");
  tcase_add_test(t_case, test_dw_fs_init);
  tcase_add_test(t_case, test_dw_fs_free);
  tcase_add_test(t_case, test_dw_fs_search_file);
  suite_add_tcase(suite, t_case);
  return suite;
}

int main()
{
  int number_failed;
  Suite *file, *fs, *mem;
  SRunner *runner;

  mem = mem_suite();
  file = file_suite();
  fs = fs_suite();

  runner = srunner_create(mem);
  srunner_add_suite(runner, file);
  srunner_add_suite(runner, fs);

  srunner_run_all(runner, CK_NORMAL);
  number_failed = srunner_ntests_failed(runner);
  srunner_free(runner);

  return (number_failed == 0) ? 0 : 1;
}

