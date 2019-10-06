#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEM_H

#include "types.h"

#define BLK_SIZE            4096
#define MAX_FILENAME_LEN    32    // maximum length of a filename
#define NUM_DENTRIES        63    // maximum number of dentries in a boot block
#define BOOT_BLK_RESERVED   52    // number of bytes reserved in a boot block
#define DENTRY_RESERVED     24    // number of bytes reserved in dentry
#define MAX_DATA            1023  // maximum number of data blocks in a file


/* struct for dentry */
typedef struct {
  uint8_t filename[MAX_FILENAME_LEN];
  uint32_t filetype;
  uint32_t inode_num;
  uint8_t reserved[DENTRY_RESERVED];   //24 is the number of bytes reserved
} __attribute__((packed)) dentry_t;

/* struct for boot block */
typedef struct {
  uint32_t dir_count;
  uint32_t inode_count;
  uint32_t data_count;
  uint8_t reserved[BOOT_BLK_RESERVED];   //52 is the number of bytes reserved
  dentry_t direntries[NUM_DENTRIES];    //63 is the maximum number of dentries in one boot block
} __attribute__((packed)) boot_block_t;

/* struct for inode */
typedef struct {
  uint32_t blk_length;
  uint32_t data_block_num[MAX_DATA];    //1023 is maximum number of data blocks in one file
} __attribute__((packed)) inode_t;

/* initialize the file system */
void file_sys_init(boot_block_t* mod_start);

/* search the dentry by name */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);

/* search the dentry by index */
int32_t read_dentry_by_index (int32_t index, dentry_t* dentry);

/* store data into buf starting from offset and read length bytes */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* open the file */
int32_t open_file(const uint8_t* filename);

/* read the file by name starting from offset and read nbytes */
int32_t read_file(int32_t fd, void* buf, int32_t nbytes);

/* write the file */
int32_t write_file(int32_t fd, const void* buf, int32_t nbytes);

/* close the file */
int32_t close_file(int32_t fd);

/* open the directory */
int32_t open_dir(const uint8_t* dirname);

/* read the directory */
int32_t read_dir(int32_t fd, void* buf, int32_t nbytes);

/* write the directory */
int32_t write_dir(int32_t fd, const void* buf, int32_t nbytes);

/* close the directory */
int32_t close_dir(int32_t fd);

/* global variable, pointer to current boot block */
boot_block_t* boot_block;

/* global variable, pointer to current index node */
inode_t* index_node;

/* global variable, the address of the start of data block */
uint32_t data_blk_start;

#endif
