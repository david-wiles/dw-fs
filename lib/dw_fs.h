#ifndef DW_FS_H
#define DW_FS_H

#include <stdlib.h>
#include <string.h>


// typedef
typedef int dw_error;
typedef struct dw_file dw_file;
typedef struct fp_node fp_node;
typedef struct data_node data_node;
typedef struct dw_fs dw_fs;

#define BLOCK_SIZE 512
#define MAX_DATA_SIZE (BLOCK_SIZE - sizeof(data_node *) - sizeof(int))
#define MAX_FILENAME_LENGTH (BLOCK_SIZE - sizeof(fp_node *) - sizeof(data_node *))

#define ERR_NON_UNIQUE_NAME 111
#define ERR_NOT_EXISTS 112
#define ERR_OOM 113

#define ITER_DIR(fp) for ( (fp) = _dw_fs.dir; (fp) != 0; (fp) = (fp)->next)

dw_fs _dw_fs;

/**
 * Initialize a new file system with the specified size
 */
void dw_fs_init(unsigned int num_blocks);

/**
 * Free all resources related to the filesystem
 */
void dw_fs_free();

// An opened file object.
struct dw_file
{
  char *name;
  fp_node *fp; // Each block of data is stored in a linked list
};

struct fp_node
{
  char name[MAX_FILENAME_LENGTH]; // Name of the file
  fp_node *next;   // Link to the next file node, for directory structure
  data_node *data; // Link to the actual data contained in this file
};

struct data_node
{
  data_node *next; // Link to next data node if there is more data in the file
  int bytes;       // Number of bytes stored in the data. Used to determine EOF
  char data[MAX_DATA_SIZE];     // File data contained on this block
};

struct dw_fs
{
  // Memory metadata
  char *blocks;
  unsigned int n_blocks;

  fp_node *dir; // Directory data node. New files are pushed to the front of the stack
  int n_files;  // Number of files currently in the directory
};

// Searches for the file with the name
int file_exists(const char *name);

// Searches for a file with the given name. If the file is found, the FCB for the
// node is returned. If it doesn't exist, the error value is set and null is returned
fp_node *search_file(
        const char *name,
        dw_error *err
                    );

// Creates a new file in the file system with the given name
// The name must be unique, or the error ERR_NON_UNIQUE_NAME will be returned
// through the *err variable.
dw_file create(
        char *name,   // Unique identifier for the file
        dw_error *err // Any errors that occur are returned in this variable
              );

// Opens the specified file in the file system. If the file does not
// exist, the error ERR_NOT_EXIST will be returned
dw_file *open(
        char *name,   // Name of the file to open
        dw_error *err // Any errors that occur are returned in this variable
             );

// Close an opened file object
void close(
        dw_file *file, // Reference to the file to close
        dw_error *err  // Any errors that occur are returned in this variable
          );

// Reads specified number of bytes from the file into a char array
char *read(
        dw_file *file,  // Reference to an open file
        unsigned int n, // Number of bytes to read
        dw_error *err   // Any errors that occur are returned in this variable
          );

// Writes specified number of bytes to an open file
void write(
        dw_file *file, // The open file to write bytes to
        const char *bytes,   // The bytes to write to the file
        int n,         // Number of bytes to write. Should NOT exceed length of bytes array
        dw_error *err  // Any errors that occur are returned in this variable
          );

// Reads all files in the directory and returns a list of their names
char **dir(dw_error *err);

// Deletes the specified file from the directory
void delete(
        char *name,   // Name of the file to remove
        dw_error *err // Any errors that occur are returned in this variable
           );

#endif // DW_FS_H
