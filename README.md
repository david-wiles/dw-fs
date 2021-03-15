# dw-fs 
## (David Wiles FileSystem)

This is an in-memory filesystem with a flat directory structure. It is loosely based on the FAT filesystem and uses
links between data blocks for all data structures.

Since the filesystem is implemented on a single level, the directory entries are identical to the file entries. These
will use an entire block for storage of metadata and pointers to other blocks.

The data in the files is also stored in a linked list. The file entry has a pointer to the first data block, which 
will then each contain their own pointers to more data blocks. This makes allocation very simple.

