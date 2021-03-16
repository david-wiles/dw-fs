#ifndef DW_FS_CONF_H
#define DW_FS_CONF_H

#define BLOCK_SIZE 512
#define MAX_DATA_SIZE (BLOCK_SIZE - sizeof(data_node *) - sizeof(int))
#define MAX_FILENAME_LENGTH (BLOCK_SIZE - sizeof(fp_node *) - sizeof(data_node *))

#endif //DW_FS_CONF_H
