# dw-fs 
## (David Wiles FileSystem)

This is an in-memory filesystem with a flat directory structure. It is loosely based on the FAT filesystem and uses
links between data blocks for all data structures.

## Installation

run ```make``` and move the generated file ```dwfs.o``` wherever you want.  

## API

The API is based around the dwfs struct. To use the filesystem, you must first initialize the file system with a fixed
size, in number of blocks. This memory is used to store the directory structures and file data. 

After initialization, the dwfs instance should be used as an opaque object, and simply passed to functions.

All methods are reentrant.

The details can be found in the code, but all functions are exposed to the user through the dwfs.h header file
```c


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
// The file must be opened before it can be written to
void dwfs_create(
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

```