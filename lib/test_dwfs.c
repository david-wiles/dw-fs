#include <check.h>
#include <printf.h>
#include "dwfs.h"


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

START_TEST(test_dw_fs_init)
{
  dwfs *instance = dwfs_init(1);

  ck_assert_ptr_nonnull(instance->blocks);
  ck_assert_int_eq(instance->n_blocks, 1);

  // Test size of allocated blocks
}

START_TEST(test_dw_fs_free)
{
  dwfs *instance = dwfs_init(1);

  ck_assert_ptr_nonnull(instance->blocks);
  ck_assert_int_eq(instance->n_blocks, 1);

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
  test_init_fs();
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
  ck_assert_str_eq((char *) fp->data->data, "asdfasdf");

  dwfs_free(instance);
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
  Suite *file, *fs;
  SRunner *runner;

  file = file_suite();
  fs = fs_suite();

  runner = srunner_create(file);
  srunner_add_suite(runner, fs);

  srunner_run_all(runner, CK_NORMAL);
  number_failed = srunner_ntests_failed(runner);
  srunner_free(runner);

  return (number_failed == 0) ? 0 : 1;
}

