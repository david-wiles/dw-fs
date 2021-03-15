#include <check.h>
#include <printf.h>
#include "dw_fs.h"


void test_init_fs()
{
  // Initialize fs with four files. 8 blocks should still remain
  dw_error err = 0;
  dw_fs_init(12);
  create("file 1", &err);
  create("file 2", &err);
  create("file 3", &err);
  create("file 4", &err);

  if (err != 0) {
    printf("Could not initialize fs, %i", err);
  }
}

START_TEST(test_dw_fs_init)
{
  dw_fs_init(1);

  ck_assert_ptr_nonnull(_dw_fs.blocks);
  ck_assert_int_eq(_dw_fs.n_blocks, 1);

  // Test size of allocated blocks
}

START_TEST(test_dw_fs_free)
{
  dw_fs_init(1);

  ck_assert_ptr_nonnull(_dw_fs.blocks);
  ck_assert_int_eq(_dw_fs.n_blocks, 1);

  dw_fs_free();

  ck_assert_ptr_null(_dw_fs.blocks);
  ck_assert_int_eq(_dw_fs.n_blocks, 0);

}

START_TEST(test_dw_fs_search_file)
{
  dw_fs_init(12);

  dw_error err = 0;
  create("file 1", &err);
  ck_assert_int_eq(err, 0);

  // Search can find file
  fp_node *f = search_file("file 1", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_ptr_nonnull(f);
  ck_assert_str_eq(f->name, "file 1");

  // Search for non-existent file returns null pointer
  f = search_file("file 0", &err);
  ck_assert_int_eq(err, ERR_NOT_EXISTS);
  ck_assert_ptr_null(f);

  // Test finds many files
  create("file 2", &err);
  create("file 3", &err);
  create("file 4", &err);
  create("file 5", &err);
  create("file 6", &err);
  create("file 7", &err);

  err = 0;
  f = search_file("file 7", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_ptr_nonnull(f);
  ck_assert_str_eq(f->name, "file 7");

  f = search_file("file 6", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_ptr_nonnull(f);
  ck_assert_str_eq(f->name, "file 6");

  f = search_file("file 5", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_ptr_nonnull(f);
  ck_assert_str_eq(f->name, "file 5");
}

START_TEST(test_file_create)
{
  dw_fs_init(12);

  dw_error err = 0;
  dw_file f = create("name", &err);

  ck_assert_int_eq(err, 0);
  ck_assert_str_eq(f.name, "name");
  ck_assert_ptr_nonnull(f.fp);
  ck_assert_int_eq(_dw_fs.n_files, 1);

  // Creating a file with the same name should fail
  err = 0;
  f = create("name", &err);

  ck_assert_int_eq(err, ERR_NON_UNIQUE_NAME);
  ck_assert_ptr_null(f.fp);
  ck_assert_int_eq(_dw_fs.n_files, 1);

  err = 0;
  f = create("name2", &err);

  ck_assert_int_eq(err, 0);
  ck_assert_str_eq(f.name, "name2");
  ck_assert_ptr_nonnull(f.fp);
  ck_assert_int_eq(_dw_fs.n_files, 2);

  fp_node *fp = _dw_fs.dir;

  ck_assert_str_eq(fp->name, "name2");

  fp = fp->next;

  ck_assert_str_eq(fp->name, "name");

  fp = fp->next;

  ck_assert_ptr_null(fp);

  dw_fs_free();

  // Test create 4 files
  test_init_fs();
  ck_assert_int_eq(_dw_fs.n_files, 4);
  fp = _dw_fs.dir;
  ck_assert_str_eq(fp->name, "file 4");
  fp = fp->next;
  ck_assert_str_eq(fp->name, "file 3");
  fp = fp->next;
  ck_assert_str_eq(fp->name, "file 2");
  fp = fp->next;
  ck_assert_str_eq(fp->name, "file 1");
  ck_assert_ptr_null(fp->next);
  dw_fs_free();
}

START_TEST(test_file_delete)
{
  test_init_fs();
  dw_error err = 0;

  // Delete file that does not exist
  delete("file 5", &err);
  ck_assert_int_eq(err, ERR_NOT_EXISTS);
  ck_assert_int_eq(_dw_fs.n_files, 4);

  err = 0;

  // Delete files in directory order
  delete("file 4", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(_dw_fs.n_files, 3);
  ck_assert_str_eq(_dw_fs.dir->name, "file 3");

  delete("file 3", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(_dw_fs.n_files, 2);
  ck_assert_str_eq(_dw_fs.dir->name, "file 2");

  delete("file 2", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(_dw_fs.n_files, 1);
  ck_assert_str_eq(_dw_fs.dir->name, "file 1");

  delete("file 1", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(_dw_fs.n_files, 0);
  ck_assert_ptr_null(_dw_fs.dir);

  dw_fs_free();

  test_init_fs();

  // Delete files in reverse order
  delete("file 1", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(_dw_fs.n_files, 3);
  ck_assert_str_eq(_dw_fs.dir->name, "file 4");

  delete("file 2", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(_dw_fs.n_files, 2);
  ck_assert_str_eq(_dw_fs.dir->name, "file 4");

  delete("file 3", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(_dw_fs.n_files, 1);
  ck_assert_str_eq(_dw_fs.dir->name, "file 4");

  delete("file 4", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(_dw_fs.n_files, 0);
  ck_assert_ptr_null(_dw_fs.dir);

  dw_fs_free();

  test_init_fs();

  // Delete files in arbitrary order

  delete("file 3", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(_dw_fs.n_files, 3);
  ck_assert_str_eq(_dw_fs.dir->name, "file 4");

  delete("file 4", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(_dw_fs.n_files, 2);
  ck_assert_str_eq(_dw_fs.dir->name, "file 2");

  delete("file 2", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(_dw_fs.n_files, 1);
  ck_assert_str_eq(_dw_fs.dir->name, "file 1");

  delete("file 1", &err);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(_dw_fs.n_files, 0);
  ck_assert_ptr_null(_dw_fs.dir);

  dw_fs_free();
}

START_TEST(test_file_write)
{
  dw_error err = 0;
  dw_fs_init(12);

  create("file 1", &err);
  ck_assert_int_eq(err, 0);

  fp_node *fp = _dw_fs.dir;
  dw_file f = (dw_file) {
          "file 1",
          fp
  };

  write(&f, "asdfasdf", 8, &err);
  ck_assert_int_eq(err, 0);
  ck_assert_ptr_nonnull(fp->data);
  ck_assert_str_eq(fp->data->data, "asdfasdf");

  dw_fs_free();
}


Suite *file_suite()
{
  Suite *suite;
  TCase *t_case;
  suite = suite_create("create and destroy");
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
  suite = suite_create("create and destroy");
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

