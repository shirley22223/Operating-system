#include "system_calls.h"
#include "terminal.h"
#include "keyboard.h"
#include "rtc.h"
#include "file_system.h"
#include "x86_desc.h"
#include "paging.h"
#include "lib.h"


#define IN_USE  1
#define NOT_IN_USE 0

int process_flag[MAX_NUM_FILE] = {NOT_IN_USE, NOT_IN_USE, NOT_IN_USE,
  NOT_IN_USE, NOT_IN_USE, NOT_IN_USE};

// function pointer arrays
file_op_table_t stdin_func = {(void*)tread, (void*)invalid_return, (void*)topen, (void*)tclose};
file_op_table_t stdout_func = {(void*)invalid_return, (void*)twrite, (void*)topen, (void*)tclose};
file_op_table_t rtc_func = {(void*)rtc_read, (void*)rtc_write, (void*)rtc_open, (void*)rtc_close};
file_op_table_t dir_func = {(void*)read_dir, (void*)write_dir, (void*)open_dir, (void*)close_dir};
file_op_table_t file_func = {(void*)read_file, (void*)write_file, (void*)open_file, (void*)close_file};

/*
 * int32_t open(const uint8_t* filename);
 * Inputs: const uint8_t* filename -- name of file to open
 * Return Value: int32_t -- return file index or -1 for failure
 * Function: Open the file and set its file operation pointer
 */
int32_t open(const uint8_t* filename){
    dentry_t dentry;
    int i;
    pcb_t* cur_pcb = get_pcb();
    // check if the file exists
    if(read_dentry_by_name(filename, &dentry) == -1)
      // return -1 if doesn't exist
      return -1;
    else {
      // set file operation pointer for stdin and stdout
      cur_pcb->file_des[0].file_op_ptr = stdin_func;
      cur_pcb->file_des[1].file_op_ptr = stdout_func;
      cur_pcb->file_des[0].in_use_flag = IN_USE;
      cur_pcb->file_des[1].in_use_flag = IN_USE;

      // find first available index
      i = MIN_FILE_IDX;
      for (; i < FILE_NUM; i++) {
        if (cur_pcb->file_des[i].in_use_flag != IN_USE) {
          cur_pcb->file_des[i].in_use_flag = IN_USE;
          break;
        }
      }
      // if index out of range
      if (i == FILE_NUM) {
        return -1;
      }
      // check the file type of each entry
      if (dentry.filetype == FILE_TYPE_RTC){
        cur_pcb->file_des[i].file_op_ptr = rtc_func;
        // set as default values
        cur_pcb->file_des[i].inode = 0;
        cur_pcb->file_des[i].file_pos = 0;
      }
      else if (dentry.filetype == FILE_TYPE_DIR) {
        cur_pcb->file_des[i].file_op_ptr = dir_func;
        // set as default values
        cur_pcb->file_des[i].inode = 0;
        cur_pcb->file_des[i].file_pos = 0;
      }
      else if (dentry.filetype == FILE_TYPE_FILE) {
        cur_pcb->file_des[i].file_op_ptr = file_func;
        // set as default values
        cur_pcb->file_des[i].inode = dentry.inode_num;
        cur_pcb->file_des[i].file_pos = 0;
      }
      else{
        cur_pcb->file_des[i].in_use_flag = NOT_IN_USE;
        // error
        return -1;
      }
    }
    int32_t ret_val = cur_pcb->file_des[i].file_op_ptr.open(filename);
    // return file index
    if (ret_val != 0)
        return -1;
    return i;
}

/*
 * int32_t close(int32_t fd)
 * Inputs: int32_t fd -- index for file descriptor
 * Return Value: int32_t -- -1 for error, 0 for success
 * Function: close the file
 */
int32_t close(int32_t fd){
    if (fd < MIN_FILE_IDX || fd >= FILE_NUM)
        return -1;
    pcb_t* cur_pcb = get_pcb();
    // if file is already closed
    if (cur_pcb->file_des[fd].in_use_flag == NOT_IN_USE)
        return -1;
    cur_pcb->file_des[fd].in_use_flag = NOT_IN_USE;
    // check if device closes successfully
    // set function pointer to corresponding close function in file operation pointer
    int32_t ret_val = cur_pcb->file_des[fd].file_op_ptr.close(fd);
    // check if it closes successfully
    if (ret_val != 0)
        return -1;
    return 0;
}

/*
 * int32_t read(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: int32_t fd -- file descriptor number
 *         void* buf -- buffer to store file data
 *         int32_t nbytes -- number of bytes read
 * Return Value: int32_t -- -1 for error or number of bytes read
 * Function: read the file
 */
int32_t read(int32_t fd, const void* buf, int32_t nbytes){
    // check invalid conditions
    if (fd < 0 || fd >= FILE_NUM)
        return -1;
    if (buf == NULL) return -1;
    pcb_t* cur_pcb = get_pcb();
    // if file is already closed
    if (cur_pcb->file_des[fd].in_use_flag == NOT_IN_USE)
        return -1;
    // set function pointer to corresponding read function in file operation pointer
    int32_t ret_val = cur_pcb->file_des[fd].file_op_ptr.read(fd, buf, nbytes);

    return ret_val;
}

/*
 * int32_t write(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: int32_t fd -- file descriptor number
 *         void* buf -- buffer that stores file data
 *         int32_t nbytes -- number of bytes written
 * Return Value: int32_t -- -1 for error or number of bytes written
 * Function: write to the file
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes){
    // check invalid conditions
    if (fd < 0 || fd >= FILE_NUM)
        return -1;
    if (buf == NULL) return -1;
    pcb_t* cur_pcb = get_pcb();
    // if file is already closed
    if (cur_pcb->file_des[fd].in_use_flag == NOT_IN_USE)
        return -1;
    // set function pointer to corresponding write function in file operation pointer
    int32_t ret_val = cur_pcb->file_des[fd].file_op_ptr.write(fd, buf, nbytes);
    return ret_val;
}

/*
 * pcb_t* get_pcb()
 * Inputs: None
 * Return Value: pcb_t* cur_pcb -- current pcb
 * Function: get current pcb
 */
pcb_t* get_pcb(){
  pcb_t* cur_pcb;
  uint32_t addr_pcb;
  asm volatile (
      "movl %%esp, %0;"
      : "=r"(addr_pcb) //output
      :
      : "%eax"
  );
  cur_pcb = (pcb_t*) (addr_pcb & PCB_MASK);
  return cur_pcb;
}

/* int32_t invalid_return()
 * Inputs: None
 * Return Value: -1
 * Function: return -1 if the invalid conditions are met
 */
int32_t invalid_return(){
  return -1;
}

/*
 * int32_t execute(const uint8_t* command);
 * Inputs: const uint8_t* command -- command to execute
 * Return Value: int32_t -- 0 for success, -1 for error
 * Function: Execute corresponding command
 */
int32_t execute(const uint8_t* command){
  cli();
  int pid;
  // check availability
  for (pid = 0; pid < MAX_NUM_FILE; pid++){
    if (process_flag[pid] == NOT_IN_USE) break;
  }
  // if no file blocks are available
  if (pid == MAX_NUM_FILE) return -1;
  // else put it in use
  else process_flag[pid] = IN_USE;

  // parse arguments
  uint8_t cmd_buf[KEYBOARD_BUFFER_SIZE], arg_buf[KEYBOARD_BUFFER_SIZE];
  dentry_t dentry;
  // check if command and arguments are vaild
  if (parse_arg(command, cmd_buf, arg_buf) == -1) {
    process_flag[pid] = NOT_IN_USE;
    sti();
    return -1;
  }

  // check file validity
  if(read_dentry_by_name(cmd_buf, &dentry) == -1) {
    process_flag[pid] = NOT_IN_USE;
    sti();
    return -1;
  }
  // check if the file is vaild using empty magic buf
  uint8_t magic_buf[FOUR_BYTES] = {0, 0, 0, 0};
  read_data(dentry.inode_num, 0, magic_buf, FOUR_BYTES);
  // first four bytes of the file are "magic numbers"
  if (magic_buf[0] != MAGIC_BUF_0 || magic_buf[1] != MAGIC_BUF_1
    ||  magic_buf[2] != MAGIC_BUF_2 ||  magic_buf[3] != MAGIC_BUF_3) {
      process_flag[pid] = NOT_IN_USE;
      sti();
      return -1;
    }

  // set up paging
  map_4MB_page(pid);

  // load file into memory
  if (read_data(dentry.inode_num, 0, (uint8_t*)PROG_IMAGE_ADDR, (_4MB-PI_OFFSET)) == -1){
    process_flag[pid] = NOT_IN_USE;
    sti();
    return -1;
  }

  // find entry point into the program
  uint32_t entry_pt = 0x0;
  uint8_t entry_pt_buf[FOUR_BYTES];
  read_data(dentry.inode_num, SHIFT_24_BITS, entry_pt_buf, FOUR_BYTES);
  // little endian so shift byte 24-27 (right to left)
  entry_pt |= entry_pt_buf[3] << SHIFT_24_BITS; // set byte 27
  entry_pt |= entry_pt_buf[2] << SHIFT_16_BITS; // set byte 26
  entry_pt |= entry_pt_buf[1] << SHIFT_8_BITS; // set byte 25
  entry_pt |= entry_pt_buf[0]; // set byte 24

  // create PCB
  pcb_t * child_pcb = (pcb_t*) (_8MB - (pid + 1) * _8KB);  // calculate new address for child pcb
  child_pcb->pid = pid;
  child_pcb->status_excep = 0;
  uint32_t len_arg_buf = strlen((int8_t*)arg_buf);
  memcpy((int8_t*)child_pcb->arg_buf, (int8_t*)arg_buf, len_arg_buf);
  child_pcb->arg_buf[len_arg_buf] = '\0';
  terminal[cur_terminal].active_process = pid;
  child_pcb->terminal_id = cur_terminal;

  // if the child pcb is the first one
  if (pid == 0 || pid == 1 || pid == 2) {
      //child_pcb->terminal_id = pid;
      child_pcb->parent_pid = pid;
  }
  else {
      child_pcb->parent_pid = get_pcb()->pid;   // set parent pid to previous pid

  }

  child_pcb->file_des[0].file_op_ptr = stdin_func;  // set file operation pointer to stdin
  child_pcb->file_des[1].file_op_ptr = stdout_func; // set next file operation pointer to stdout
  child_pcb->file_des[0].in_use_flag = IN_USE;     // set as in use
  child_pcb->file_des[1].in_use_flag = IN_USE;     // set as in use
  int i;
  // set other variables as default in child PCB
  for (i = 2; i < FILE_NUM; i++) {
      child_pcb->file_des[i].inode = 0;
      child_pcb->file_des[i].in_use_flag = NOT_IN_USE;
      child_pcb->file_des[i].file_pos = 0;
  }

  // prepare for context switch
  tss.ss0 = KERNEL_DS;
  tss.esp0 = _8MB - pid * _8KB;
  uint32_t temp;

  // save parent ebp
  asm volatile("movl %%ebp, %0" : "=r"(temp));
  child_pcb->parent_ebp = temp;

  // push iret context onto stack
  asm volatile (
      "pushl $0x002B;"        // push user DS

      "movl $0x83FFFFC, %%eax;"     // push esp
      "pushl %%eax;"

      "pushfl;"         // push EFLAG
      "popl %%eax;"
      "orl $0x4200, %%eax;"       // set interrupt bits in EFLAG to enable interrupts
      "pushl %%eax;"

      "pushl $0x0023;"    // push User CS

      "pushl %0;"         // push EIP
      "iret;"

      "FROM_HALT:;"       //LABEL for halt to jump back
      "leave;"
      "ret;"
      :
      :"r" (entry_pt) //input
      :"%eax"
  );

  return 0;
}


/*
 * parse_arg(const uint8_t* command, uint8_t* cmd_buf, uint8_t* arg_buf)
 * Inputs: const uint8_t* command -- command to execute
 *         uint8_t* cmd_buf -- buffer that stores command
 *         uint8_t* arg_buf -- buffer that stores argument
 * Return Value: int32_t -- 0 for success, -1 for error
 *  Function: Execute corresponding command
 */
int32_t parse_arg(const uint8_t* command, uint8_t* cmd_buf, uint8_t* arg_buf){
    // invalid command cases
    if (strlen((int8_t*)command) > KEYBOARD_BUFFER_SIZE) return -1;
    if (command[0] == '\0') return -1;

    int i, j, start_idx;
    // strip leading spaces
    for (i = 0; i < KEYBOARD_BUFFER_SIZE; i++){
      if (command[i] != ' ' && command[i] != '\0') break;
    }
    // if command contains only spaces or '\0'
    if (i == KEYBOARD_BUFFER_SIZE-1) return -1;
    // store command starting index
    start_idx = i;
    // find separating space
    for (; i < KEYBOARD_BUFFER_SIZE; i++){
      if (command[i] == ' ' || command[i] == '\0') break;
    }
    // store command(what's before space) to cmd_buf
    for (j = 0; j < i - start_idx; j++){
      cmd_buf[j] = command[j + start_idx];
    }
    // set last element in cmd_buf to be terminator
    cmd_buf[j] = '\0';

    // if no arguments at all, set it with a terminator
    if (i == KEYBOARD_BUFFER_SIZE-1 || command[i] == '\0') {
      arg_buf[0] = '\0';
      return 0;
    }
    else {
      // compute argument starting index
      start_idx = i+1;

      for (i = start_idx; i < KEYBOARD_BUFFER_SIZE; i++){
        if (command[i] != ' ' && command[i] != '\0') break;
      }
      // if command contains only spaces or '\0'
      if (i == KEYBOARD_BUFFER_SIZE-1){
        arg_buf[0] = '\0';
        return -1;
      }
      start_idx = i;
      for ( i = start_idx; i < KEYBOARD_BUFFER_SIZE; i++){
        if (command[i] == '\0') break;
      }
      // store arguments to buffer (what's after space)
      for (j = 0; j < i - start_idx; j ++){
        arg_buf[j] = command[j + start_idx];
      }
      // set last element in arg_buf with terminator
      arg_buf[j] = '\0';
    }

    return 0;
}

/*
 * int32_t halt(uint8_t status);
 * Inputs: uint8_t status -- status from execute
 * Return Value: int32_t -- return -1 for error,
 *                          otherwise return status to execute
 * Function: Halt current process and return to execute
 */
int32_t halt(uint8_t status){
  // clears keyboard buffer
    int j = 0;
    for (j = 0; j < key_buffer_index[cur_terminal]; j++) {
      keyboard_buffer[cur_terminal][j] = NULL;
    }
    key_buffer_index[cur_terminal] = 0;

    for (j = 0; j < term_buffer_index[cur_terminal]; j++) {
      terminal_buffer[cur_terminal][j] = NULL;
    }
    term_buffer_index[cur_terminal] = 0;

    pcb_t * cur_pcb = get_pcb();
    if (terminal[cur_pcb->terminal_id].fish_check != 0) {
      terminal[cur_pcb->terminal_id].fish_check--;
    }
    // if current shell is the last shell, start a new shell
    if (cur_pcb->pid == 0 || cur_pcb->pid == 1 || cur_pcb->pid == 2) {
        process_flag[cur_pcb->pid] = NOT_IN_USE;
        // first command is "shell", length = 5
        uint8_t command_str[5] = "shell";
        execute((uint8_t*)command_str);
    }
    else{
      process_flag[cur_pcb->pid] = NOT_IN_USE;         // set process as not in use

      // close ALL relevant FDs
      int i;
      for (i = MIN_FILE_IDX; i < FILE_NUM; i++) {
          close(i);
      }

      //restore parent paging
      map_4MB_page(cur_pcb->parent_pid);

      terminal[cur_terminal].active_process = cur_pcb->parent_pid;

      //restore stack pointer
      tss.esp0 = _8MB - cur_pcb->parent_pid * _8KB;

    }
    uint32_t status_check = (uint32_t)status;
    if (cur_pcb->status_excep){
      status_check = HALT_BY_EXCEP;
    }
    // jump to execute return
    asm volatile(
        "movl %0, %%eax;"
        "movl %1, %%ebp;"   // restore parent ebp
        "jmp FROM_HALT;"
        :
        :"r"(status_check), "r"(cur_pcb->parent_ebp)
        :"%eax"
    );
      // return -1 if didn't return properly to execute
      return -1;
}

/*
 * int32_t getargs(uint8_t* buf, int32_t nbytes)
 * Inputs: uint8_t* buf -- buf that stores the argument
 *         int32_t nbytes -- number of bytes in the argument
 * Return Value: -1 if failed, 0 if success
 * Function: store argument passed in the buf
 *
 */
/* get arguments (not implemented for check pt 3)*/
int32_t getargs(uint8_t* buf, int32_t nbytes){

  pcb_t* pcb = get_pcb();
  int32_t arg_length = strlen((int8_t*) pcb->arg_buf);
  // check invalid conditions
  if (nbytes < arg_length || arg_length <= 0 || buf == NULL) return -1;
  // copy the argument into the buffer
	memcpy((int8_t*)buf, (int8_t*)pcb->arg_buf, nbytes);
  // add a null terminator in the end
  ((uint8_t*)buf)[arg_length] = '\0';

  return 0;
}


/*
 * int32_t vidmap(uint8_t** screen_start)
 * Inputs: uint8_t** screen_start -- start of vidmap screen
 * Return Value: -1 if failed, or virtual address of
 *               the start of the screen should point to
 * Function: map text-mode video memory into user space
 *           at a pre-set virtual address
 */
/* map text-mode video memory to user space (not implemented for check pt 3) */
int32_t vidmap(uint8_t** screen_start){
  // check if screen_start is valid and within the range
  if (screen_start == NULL) return -1;
  if ((uint32_t)screen_start < VM_START_ADDR || (uint32_t)screen_start >= VM_END_ADDR)
    return -1;
  // arbitrarily assign video memory virtual address
  uint32_t virtual_addr = VM_END_ADDR + (screen_terminal)*_4KB;
  map_video_mem(virtual_addr);
  // make the start of the screen points to the virtual address assigned
  *screen_start = (uint8_t*) virtual_addr;
  terminal[screen_terminal].fish_check++;
  return virtual_addr;
}


/* signal handling (not implemented for check pt 3) */
int32_t set_handler(int32_t signum, void* handler_address){
  return -1;
}

/* signal return (not implemented for check pt 3) */
int32_t sigreturn(void){
  return -1;
}
