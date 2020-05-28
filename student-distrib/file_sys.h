/* file_sys.h - define the file system driver */

#ifndef _FILE_SYS_H_
#define _FILE_SYS_H_

#include "lib.h"
#include "sys_call.h"
#define BLOCK_SIZE  4096    /* block size to be 4kb */
#define MAX_FILE_NUM  63    /* max file number to be 64 - 1 */
#define MAX_FILE_NAME_LEN  32   /* max length of file name is 32 */
#define DENTRY_RESERVE 24
#define BOOTBLK_RESERVE 52
#define REGULAR_FILE_TYPE 2     /* file type for regular files is 2 */
#define DIR_FILE_TYPE 1     /* file type for directory is 1 */
#define RTC_FILE_TYPE 0

/* dentry structure */
typedef struct {
    char file_name[MAX_FILE_NAME_LEN];
    uint32_t file_type;
    uint32_t inode_num;
    uint8_t reserved[DENTRY_RESERVE]; // reserve 24b
} dentry_t;

/* boot block structure */
typedef struct {
    uint32_t num_dentry;
    uint32_t num_inode;
    uint32_t num_data_blk;
    uint8_t reserved[BOOTBLK_RESERVE]; //reserve 52b
    dentry_t files[MAX_FILE_NUM];
} bootblock_t;

/* inode structure */
typedef struct {
    uint32_t file_size;
    uint32_t data_blks[BLOCK_SIZE / 4 - 1];
} inode_t;

/* data block structure */
typedef struct {
    uint32_t data[BLOCK_SIZE / 4];
} data_block_t;

/* three helper functions provided by the system module */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);

int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

int32_t read_data(uint32_t inode_idx, uint32_t offset, char* buf, uint32_t length);

/* file system initialization */
void file_sys_init(uint32_t fda);

/* file system drivers */
int32_t file_open(const uint8_t* filename);

int32_t file_close(int32_t fd);

int32_t file_write(int32_t fd, const void* buf, int32_t nBytes);

int32_t file_read(int32_t fd, void* buf, int32_t  nBytes);

int32_t directory_open(const uint8_t* filename);

int32_t directory_close(int32_t fd);

int32_t directory_write(int32_t fd, const void* buf, int32_t nBytes);

int32_t directory_read(int32_t fd, void* buf, int32_t nBytes);

int32_t read_dir(uint32_t offset, char* buf, uint32_t length);


#endif
