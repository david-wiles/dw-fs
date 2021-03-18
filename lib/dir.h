#ifndef DW_FS_DIR_H
#define DW_FS_DIR_H

#include "conf.h"


typedef struct dw_dir dw_dir;
typedef struct fp_node fp_node;
typedef struct data_node data_node;

struct dw_dir
{
  fp_node *head;
  int n_files;  // Number of files currently in the directory
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
  unsigned char data[MAX_DATA_SIZE];     // File data contained on this block
};

dw_dir *dw_dir_init();

// Searches for the file with the name
int file_exists(
        dw_dir *,
        const char *
               );

// Searches for a file with the given name. If the file is found, the FCB for the
// node is returned. If it doesn't exist, the error value is set and null is returned
fp_node *search_file(
        dw_dir *,
        const char *,
        int *
                    );

// Add directory entry
fp_node *add_entry(
        dw_dir *,
        void *,
        const char *,
        int *
                  );

// Remove directory entry
void remove_entry(
        dw_dir *,
        const char *,
        int *
                 );

fp_node **gather_entries(dw_dir *, int *, int *);

void dw_dir_free(dw_dir *);

#endif //DW_FS_DIR_H
