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
#ifndef DW_FS_H
#define DW_FS_H

#include "dir.h"
#include "err.h"
#include "conf.h"
#include "mem.h"
#include "file_table.h"


typedef struct dwfs dwfs;
typedef struct dw_file dw_file;


// File system instance struct. This must be initialized before use of any file operations
// Each dwfs struct contains a pre-allocated block of memory for the files, a reference
// to the directory data structure, and a reference to the instance's open file table.
//
// This struct should be used as a opaque structure, none of the internal data structures
// should be used directly. Only the dwfs instance and dw_file structs should be used
struct dwfs
{
  dw_mem *blocks;  // Memory manager and allocated blocks
  dw_dir *dir;     // Directory manager
  file_table *tab; // Open file table
};

// The user's file object
struct dw_file
{
  char *name;
  fp_node *fp;
};

// Initialize a new file system instance with the specified number of blocks
// The size of each block must be set during compilation, in conf.h.
//
// By default, the block size is 512 bytes.
dwfs *dwfs_init(unsigned int block_size);

// Free all resources associated with the specific file system
void dwfs_free(dwfs *self);

// Creates a new file in the file system with the given name
//
// The filename must be unique. If it is not, then the operation will fail and an error
// will be returned through err.
//
// If the file is created, the user's file object will be returned. The file will be
// empty, but will be placed in the directory and will allocate a single block for the
// directory node.
//
// Creating a file opens it, so the file should be closed after is it created
dw_file dwfs_create(
        dwfs *self,     // dwfs instance
        char *filename, // Unique identifier for the file
        int *err        // Any errors that occur are returned in this variable
                );

// Opens the specified file in the file system.
//
// If the file does not exist, the error ERR_NOT_EXIST will be returned through err
dw_file dwfs_open(
        dwfs *self,     // dwfs instance
        char *filename, // Name of the file to open
        int *err        // Any errors that occur are returned in this variable
                 );

// Close an opened file object
//
// The file must be open before it can be closed. If it is not currently open, ERR_FILE_NOT_OPEN
// will be returned through err
void dwfs_close(
        dwfs *self,
        dw_file *file, // Reference to the file to close
        int *err  // Any errors that occur are returned in this variable
               );

// Reads specified number of bytes from the file into an unsigned char array
//
// The file must be opened before it can be read. Any errors are returned through err
//
// The returned array is not null-terminated. However, the length of the returned array is given
// by n. This may be less than the number of bytes requested, but will not be greater
unsigned char *dwfs_read(
        dwfs *self,           // The file system instance
        dw_file *file,        // Reference to an open file
        unsigned int n,       // Number of bytes to read
        unsigned int *n_read, // Number of bytes actually read
        int *err              // Any errors that occur are returned in this variable
                        );

// Writes an array of bytes to an opened file
//
// The file must be opened before it can be written. Any errors are returned through err
void dwfs_write(
        dwfs *self,                 // The file system instance
        dw_file *file,              // The open file to write bytes to
        const unsigned char *bytes, // The bytes to write to the file
        int n,                      // Number of bytes to write. Should NOT exceed length of bytes array
        int *err                    // Any errors that occur are returned in this variable
               );

// Reads all files in the directory and returns a list of their names
char **dwfs_dir(
        dwfs *self, // File system instance
        int *len,   // Length of the directory list
        int *err    // Errors
               );

// Deletes the specified file from the directory
//
// There should be no opened files when the file is deleted. If the file is opened, the operation
// will fail and an error will be returned through err
void dwfs_delete(
        dwfs *self,     // File system instane
        char *filename, // Name of the file to remove
        int *err        // Any errors that occur are returned in this variable
                );

#endif // DW_FS_H
