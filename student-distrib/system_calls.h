#ifndef SYSTEM_CALLS_H
#define SYSTEM_CALLS_H

#include "types.h"

#define FILE_NUM              8
#define MAX_NUM_FILE          6
#define MIN_FILE_IDX          2
#define PCB_MASK              0xFFFFE000      //mask lower 13 bits
#define _8MB                  0x800000
#define _4MB                  0x400000
#define _8KB                  0x2000
#define _4KB                  0x1000
#define PROG_IMAGE_ADDR       0x08048000
#define PI_OFFSET             0x48000
#define KEYBOARD_BUFFER_SIZE  128
#define FOUR_BYTES            4
#define SHIFT_24_BITS         24
#define SHIFT_16_BITS         16
#define SHIFT_8_BITS          8
#define NUM_FUNC              4
#define FILE_TYPE_RTC         0
#define FILE_TYPE_DIR         1
#define FILE_TYPE_FILE        2
#define VM_START_ADDR         0x8000000  // 128 MB
#define VM_END_ADDR           0x8400000  // 132 MB
#define HALT_BY_EXCEP         256
#define MAGIC_BUF_0           0x7f
#define MAGIC_BUF_1           0x45
#define MAGIC_BUF_2           0x4c
#define MAGIC_BUF_3           0x46



// 6 processes max
extern int process_flag[MAX_NUM_FILE];

typedef struct{
  int32_t (*read)(int32_t fd, const void* buf, int32_t nbytes);
  int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
  int32_t (*open)(const uint8_t* filename);
  int32_t (*close)(int32_t fd);
} file_op_table_t;

/* file descriptor structure */
typedef struct {
    // int32_t* file_op_ptr;
    file_op_table_t file_op_ptr;
    int32_t inode;
    int32_t file_pos;
    int32_t in_use_flag;
} file_descriptor_t;

/* pcb structure */
typedef struct {
    uint32_t parent_ebp;
    file_descriptor_t file_des[FILE_NUM];
    uint8_t arg_buf[KEYBOARD_BUFFER_SIZE];
    uint32_t terminal_id;
    uint32_t pid;
    uint32_t parent_pid;
    uint32_t child_pid;
    uint32_t file_type;
    uint32_t my_ebp;
    uint8_t status_excep;
} __attribute__((packed)) pcb_t;

/* open the file */
int32_t open(const uint8_t* filename);

/* close the file */
int32_t close(int32_t fd);

/* read the file */
int32_t read(int32_t fd, const void* buf, int32_t nbytes);

/* write to the file */
int32_t write(int32_t fd, const void* buf, int32_t nbytes);

/* excute the command */
int32_t execute(const uint8_t* command);

/* halt the status */
int32_t halt(uint8_t status);

/* return -1 if invalid */
int32_t invalid_return();

/* get current pcb */
pcb_t* get_pcb();

/* parse argument and command */
int32_t parse_arg(const uint8_t* command, uint8_t* cmd_buf, uint8_t* arg_buf);



/* get arguments */
int32_t getargs(uint8_t* buf, int32_t nbytes);

/* map text-mode video memory to user space */
int32_t vidmap(uint8_t** screen_start);

/* signal handling */
int32_t set_handler(int32_t signum, void* handler_address);

/* signal return */
int32_t sigreturn(void);


#endif
