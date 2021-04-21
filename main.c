#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "dwfs.h"


// Global instance of the file system. Just makes things easier
dwfs *fs;

void *create_lorum_ipsums(void *unused)
{
  int err = 0;

  // Thread 1 work: create and write two files.
  // The files are opened automatically when they are created

  dw_file first_file = dwfs_create(fs, "file 1", &err);
  if (err != 0) return (void *) err;

  // Here, I will write a 5-paragraph lorum ipsum to the file
  dwfs_write(fs,
             &first_file,
             (const unsigned char *)
                     "This is file 1. \n\n Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras eget "
                     "molestie tortor. Nam viverra ipsum in elit dictum dapibus. Ut mollis massa sed ipsum "
                     "feugiat rhoncus. Suspendisse gravida sapien in arcu varius tristique. Nam luctus iaculis "
                     "blandit. Vestibulum odio est, dapibus in velit tempor, tincidunt vehicula justo. Vivamus "
                     "non vehicula nunc. Sed commodo purus sed consectetur mattis. Proin tempor dolor nisl, at "
                     "consectetur ipsum condimentum sit amet. Vestibulum ante ipsum primis in faucibus orci "
                     "luctus et ultrices posuere cubilia curae; Suspendisse convallis bibendum nibh, id "
                     "aliquet felis auctor at. Duis elementum consequat ultricies.\n\nNullam faucibus nisi "
                     "vitae quam egestas iaculis. Integer pretium pharetra orci, nec bibendum quam posuere "
                     "eget. Duis nibh nulla, cursus eu lorem a, faucibus venenatis neque. In elementum mattis "
                     "lacus, sit amet bibendum tellus sagittis accumsan. Fusce ultrices, leo a ultricies "
                     "dignissim, velit elit molestie leo, at gravida lacus lacus ac augue. Cras auctor libero "
                     "malesuada dolor iaculis, consequat finibus orci blandit. Morbi convallis felis a ante "
                     "faucibus vestibulum. In justo mi, scelerisque non mauris id, blandit mattis lorem. "
                     "Morbi at fringilla enim, in auctor eros. Praesent accumsan urna eget ex venenatis "
                     "elementum. Nullam tortor quam, ultrices nec felis at, consequat blandit nisi. Nullam "
                     "magna risus, rhoncus in laoreet id, auctor sit amet neque.\n\nProin elementum nec dolor "
                     "a dignissim. Vestibulum scelerisque eget ligula nec placerat. Vivamus tristique risus "
                     "eu ex venenatis molestie. Integer sit amet interdum odio. Donec vitae enim non est "
                     "vehicula viverra in quis lorem. Nulla dignissim ut neque eu faucibus. Phasellus "
                     "dignissim ipsum eu ante commodo blandit. Morbi porta lectus vitae consectetur "
                     "elementum.\n\nSed tristique placerat tellus, ut tempor felis commodo at. Donec "
                     "malesuada, lacus volutpat pharetra fringilla, erat nisi euismod augue, eget accumsan "
                     "tellus nibh ac diam. Vivamus vehicula facilisis egestas. Aenean ut tincidunt erat. "
                     "Fusce sapien velit, placerat quis ullamcorper vitae, lacinia a sapien. Suspendisse "
                     "laoreet augue non interdum tempor. Cras interdum risus sit amet tellus pulvinar, vel "
                     "suscipit risus scelerisque. Nullam mi ex, dictum sed pharetra lacinia, venenatis quis "
                     "mi. Nam porta, lectus vel sollicitudin scelerisque, nibh metus mattis nibh, vitae "
                     "pretium lacus nisi in nunc. Integer eget congue lorem. In posuere sollicitudin nulla, "
                     "in congue nunc bibendum et. In diam velit, varius et fringilla vitae, porttitor in "
                     "sapien. Duis sed felis nibh.\n\nDuis viverra urna sed lectus congue lobortis. Etiam "
                     "metus enim, egestas ac hendrerit id, auctor ut erat. Nam dictum, felis non vehicula "
                     "iaculis, lectus sem accumsan tortor, sed vestibulum mauris orci vel lorem. Mauris "
                     "tincidunt sollicitudin ultricies. Donec fringilla, lorem id hendrerit mollis, sapien "
                     "tortor dignissim neque, in gravida lorem nunc nec ante. Morbi sit amet quam lacinia, "
                     "fringilla lectus ac, ornare libero. Vestibulum vitae rhoncus mi. Morbi ut euismod odio."
                     " Morbi venenatis eleifend libero. Donec commodo mollis metus, et mattis neque luctus nec.",
             3127,
             &err);

  dwfs_close(fs, &first_file, &err);
  if (err != 0) return (void *) err;

  dw_file second_file = dwfs_create(fs, "file 2", &err);
  if (err != 0) return (void *) err;

  // Write the same 5-paragraph lorum ipsum, but change the text at the beginning to identify the file
  dwfs_write(fs,
             &second_file,
             (const unsigned char *)
                     "This is file 2. \n\n Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras eget "
                     "molestie tortor. Nam viverra ipsum in elit dictum dapibus. Ut mollis massa sed ipsum "
                     "feugiat rhoncus. Suspendisse gravida sapien in arcu varius tristique. Nam luctus iaculis "
                     "blandit. Vestibulum odio est, dapibus in velit tempor, tincidunt vehicula justo. Vivamus "
                     "non vehicula nunc. Sed commodo purus sed consectetur mattis. Proin tempor dolor nisl, at "
                     "consectetur ipsum condimentum sit amet. Vestibulum ante ipsum primis in faucibus orci "
                     "luctus et ultrices posuere cubilia curae; Suspendisse convallis bibendum nibh, id "
                     "aliquet felis auctor at. Duis elementum consequat ultricies.\n\nNullam faucibus nisi "
                     "vitae quam egestas iaculis. Integer pretium pharetra orci, nec bibendum quam posuere "
                     "eget. Duis nibh nulla, cursus eu lorem a, faucibus venenatis neque. In elementum mattis "
                     "lacus, sit amet bibendum tellus sagittis accumsan. Fusce ultrices, leo a ultricies "
                     "dignissim, velit elit molestie leo, at gravida lacus lacus ac augue. Cras auctor libero "
                     "malesuada dolor iaculis, consequat finibus orci blandit. Morbi convallis felis a ante "
                     "faucibus vestibulum. In justo mi, scelerisque non mauris id, blandit mattis lorem. "
                     "Morbi at fringilla enim, in auctor eros. Praesent accumsan urna eget ex venenatis "
                     "elementum. Nullam tortor quam, ultrices nec felis at, consequat blandit nisi. Nullam "
                     "magna risus, rhoncus in laoreet id, auctor sit amet neque.\n\nProin elementum nec dolor "
                     "a dignissim. Vestibulum scelerisque eget ligula nec placerat. Vivamus tristique risus "
                     "eu ex venenatis molestie. Integer sit amet interdum odio. Donec vitae enim non est "
                     "vehicula viverra in quis lorem. Nulla dignissim ut neque eu faucibus. Phasellus "
                     "dignissim ipsum eu ante commodo blandit. Morbi porta lectus vitae consectetur "
                     "elementum.\n\nSed tristique placerat tellus, ut tempor felis commodo at. Donec "
                     "malesuada, lacus volutpat pharetra fringilla, erat nisi euismod augue, eget accumsan "
                     "tellus nibh ac diam. Vivamus vehicula facilisis egestas. Aenean ut tincidunt erat. "
                     "Fusce sapien velit, placerat quis ullamcorper vitae, lacinia a sapien. Suspendisse "
                     "laoreet augue non interdum tempor. Cras interdum risus sit amet tellus pulvinar, vel "
                     "suscipit risus scelerisque. Nullam mi ex, dictum sed pharetra lacinia, venenatis quis "
                     "mi. Nam porta, lectus vel sollicitudin scelerisque, nibh metus mattis nibh, vitae "
                     "pretium lacus nisi in nunc. Integer eget congue lorem. In posuere sollicitudin nulla, "
                     "in congue nunc bibendum et. In diam velit, varius et fringilla vitae, porttitor in "
                     "sapien. Duis sed felis nibh.\n\nDuis viverra urna sed lectus congue lobortis. Etiam "
                     "metus enim, egestas ac hendrerit id, auctor ut erat. Nam dictum, felis non vehicula "
                     "iaculis, lectus sem accumsan tortor, sed vestibulum mauris orci vel lorem. Mauris "
                     "tincidunt sollicitudin ultricies. Donec fringilla, lorem id hendrerit mollis, sapien "
                     "tortor dignissim neque, in gravida lorem nunc nec ante. Morbi sit amet quam lacinia, "
                     "fringilla lectus ac, ornare libero. Vestibulum vitae rhoncus mi. Morbi ut euismod odio."
                     " Morbi venenatis eleifend libero. Donec commodo mollis metus, et mattis neque luctus nec.",
             3127,
             &err);
  if (err != 0) return (void *) err;

  dwfs_close(fs, &second_file, &err);
  if (err != 0) return (void *) err;

  return (void *) 0;
}

void *print_file(void *filename)
{
  int err = 0;
  dw_file f = dwfs_open(fs, (char *) filename, &err);
  if (err != 0) return (void *) err;

  unsigned int len = 0;
  unsigned char *bytes = dwfs_read(fs, &f, 3127, &len, &err);
  if (err != 0) return (void *) err;

  // Write the contents of the file to stdout
  printf("\nFile length: \t%i\nFile contents: \n%s\n", len, (const char *) bytes);

  // dwfs_read returns a copy of the bytes in the file, we must free it ourselves
  free(bytes);

  dwfs_close(fs, &f, &err);
  if (err != 0) return (void *) err;

  return (void *) 0;
}

int main()
{

  // Initial setup: create the in-memory file system with 512 blocks with default size 2048 bytes
  fs = dwfs_init(512);
  int err = 0;

  // Show directory is empty

  int len;
  char **files = dwfs_dir(fs, &len, &err);
  if (err != 0) return err;

  printf("\nDirectory size: %i\n", len);
  free(files);

  pthread_t t1, t2, t3;

  pthread_create(&t1, NULL, create_lorum_ipsums, NULL);
  pthread_join(t1, (void *) &err);
  if (err != 0) return err;

  printf("\nReading from files...\n");

  // Now that the two files have been written to and closed, we will start concurrently reading the files
  pthread_create(&t2, NULL, print_file, (void *) "file 1");
  pthread_create(&t3, NULL, print_file, (void *) "file 2");

  pthread_join(t2, (void *) &err);
  if (err != 0) return err;

  pthread_join(t3, (void *) &err);
  if (err != 0) return err;

  // Directory and delete operations
  // Print all file names

  files = dwfs_dir(fs, &len, &err);
  if (err != 0) return err;

  printf("\nFinished reading files.\n");
  printf("\nDirectory size: %i\n", len);

  printf("\nReading directory...\n");
  for (int i = 0; i < len; i++) {
    printf("%s\n", files[i]);
    free(files[i]);
  }
  free(files);

  // Delete files
  dwfs_delete(fs, "file 1", &err);
  if (err != 0) return err;

  dwfs_delete(fs, "file 2", &err);
  if (err != 0) return err;

  files = dwfs_dir(fs, &len, &err);
  if (err != 0) return err;

  printf("\nFinished deleting files.\n");
  printf("\nDirectory size: %i\n", len);

  free(files);
  dwfs_free(fs);

  return 0;
}

