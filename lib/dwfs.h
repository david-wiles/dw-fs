#ifndef DW_FS_H
#define DW_FS_H

#include <stdlib.h>
#include <string.h>
#include "dir.h"
#include "err.h"
#include "conf.h"


// typedef
typedef struct dw_file dw_file;
typedef struct dwfs dwfs;

// An opened file object.
struct dw_file
{
  char *name;
  fp_node *fp; // Each block of data is stored in a linked list
};

struct dwfs
{
  unsigned char *blocks; // Allocated memory
  unsigned int n_blocks; // Total number of blocks
  unsigned int n_free;   // Number of free blocks

  dw_dir *dir; // Directory data node. New files are pushed to the front of the stack
};

/**
 * Initialize a new file system with the specified size
 */
dwfs *dwfs_init(unsigned int num_blocks);

/**
 * Free all resources related to the filesystem
 */
void dwfs_free(dwfs *self);

// Creates a new file in the file system with the given name
// The name must be unique, or the error ERR_NON_UNIQUE_NAME will be returned
// through the *err variable.
dw_file dwfs_create(
        dwfs *self,
        char *name,   // Unique identifier for the file
        int *err // Any errors that occur are returned in this variable
                   );

// Opens the specified file in the file system. If the file does not
// exist, the error ERR_NOT_EXIST will be returned
dw_file dwfs_open(
        dwfs *self,
        char *name,   // Name of the file to open
        int *err // Any errors that occur are returned in this variable
                  );

// Close an opened file object
void dwfs_close(
        dwfs *self,
        dw_file *file, // Reference to the file to close
        int *err  // Any errors that occur are returned in this variable
               );

// Reads specified number of bytes from the file into a char array
unsigned char *dwfs_read(
        dwfs *self,
        dw_file *file,  // Reference to an open file
        unsigned int n, // Number of bytes to read
        int *err   // Any errors that occur are returned in this variable
                        );

// Writes specified number of bytes to an open file
void dwfs_write(
        dwfs *self,
        dw_file *file, // The open file to write bytes to
        const char *bytes,   // The bytes to write to the file
        int n,         // Number of bytes to write. Should NOT exceed length of bytes array
        int *err  // Any errors that occur are returned in this variable
               );

// Reads all files in the directory and returns a list of their names
char **dwfs_dir(
        dwfs *self,
        int *err
               );

// Deletes the specified file from the directory
void dwfs_delete(
        dwfs *self,
        char *name,   // Name of the file to remove
        int *err // Any errors that occur are returned in this variable
                );

#endif // DW_FS_H
