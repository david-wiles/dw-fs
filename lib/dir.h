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
//
// This file contains a linked-list implementation of a single-level directory. Each file node
// is added to the top of the stack, and no reordering is done when files are removed or searched.
//
// Each file node occupies a block with the name, modification time metadata, pointer to next block in
// directory, and pointer to first data block. Each data block contains the data as an array of bytes,
// the number of bytes containing data, and a pointer to the next data block for the file.
#ifndef DW_FS_DIR_H
#define DW_FS_DIR_H

#include <stdbool.h>
#include "conf.h"


typedef struct dw_dir dw_dir;
typedef struct fp_node fp_node;
typedef struct data_node data_node;

// Directory head node.
//
// This just contains a link to the first file in the directory and the total number of
// files in the directory
struct dw_dir
{
  fp_node *head; // Pointer to current head of directory
  int n_files;   // Number of files currently in the directory
};

// File pointer node. Contains metadata and pointers to other blocks in the file system
struct fp_node
{
  char name[MAX_FILENAME_LENGTH]; // Name of the file
  fp_node *next;                  // Link to the next file node, for directory structure
  data_node *data;                // Link to the actual data contained in this file
};

struct data_node
{
  data_node *next;                   // Link to next data node if there is more data in the file
  int bytes;                         // Number of bytes stored in the data. Used to determine EOF
  unsigned char data[MAX_DATA_SIZE]; // File data contained on this block
};

// Initializes the directory structure
dw_dir *dw_dir_init();

// Frees resources associated with the directory
void dw_dir_free(dw_dir *);

// Searches for the file with the filename. Returns a boolean indicating whether the file exists
bool file_exists(
        dw_dir *self,        // Directory instance
        const char *filename // Filename to search for
                );

// Searches for a file with the given name. If the file is found, the FCB for the
// node is returned. If it doesn't exist, the error value is set and null is returned
fp_node *search_file(
        dw_dir *self,         // Directory instance
        const char *filename, // Filename to search for
        int *err              // Error return value
                    );

// Add directory entry.
//
// The caller should allocate a block of memory and pass it to this function through the block
// parameter. Due to this, the function can't fail due to out-of-memory conditions.
//
// If the entry is added successfully, the new node is returned
fp_node *add_entry(
        dw_dir *self,         // Directory entry
        void *block,          // Pre-allocated memory
        const char *filename, // Filename of the new file to add
        int *err              // Error return value
                  );

// Remove directory entry with the given name
//
// Since filenames must be unique, this will always delete the correct file.
void remove_entry(
        dw_dir *self,         // Directory entry
        const char *filename, // Filename of the file entry to delete
        int *err              // error return value
                 );

// Create an array of all directory entry nodes and return to the caller
// this really isn't a good idea...
fp_node **gather_entries(
        dw_dir *self,
        int *len,
        int *err
                        );

#endif //DW_FS_DIR_H
