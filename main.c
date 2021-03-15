#include <printf.h>
#include "lib/dw_fs.h"


int main()
{

  // Initialize fs with four files. 8 blocks should still remain
  dw_error err = 0;
  dw_fs_init(12);
  create("file 1", &err);

  fp_node *fp = _dw_fs.dir;
  dw_file f = (dw_file) {
          "file 1",
          fp
  };

  write(&f, "asdfasdf", 8, &err);

  return 0;
}

