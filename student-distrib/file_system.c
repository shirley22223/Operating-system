#include "file_system.h"
#include "system_calls.h"
#include "lib.h"

/* file_sys_init
 *
 * Description: initialize file system
 * Inputs: mod_start -- starting address of the boot block
 * Outputs: None
 * Return Value: None
 * Side Effects: initialize boot_block, index_node and data_blk_start
 */
void file_sys_init(boot_block_t* mod_start){
  // address of boot_block is same as mod_start
 boot_block = (boot_block_t*)mod_start;

  //starting address of index_node is one block after boot_block
  index_node = (inode_t*)((uint32_t)(mod_start) + BLK_SIZE);
  //starting address of data block is mod_start added with one block of boot_block and all inode blocks
  data_blk_start = (uint32_t)(mod_start) + BLK_SIZE + ((boot_block->inode_count)*BLK_SIZE);
}

/* read_dentry_by_name
 *
 * Description: search the dentry by name
 * Inputs: fname -- the name of the file need searched
 *         dentry -- the destination structure needed to copy to
 * Outputs: None
 * Return Value: 0 for success and -1 for fail
 * Side Effects: copy the structure found to the input dentry
 */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
  int i;
  //get the length of the file name
  uint32_t string_length = strlen((int8_t*)fname);
  if (fname == NULL || dentry == NULL || string_length > MAX_FILENAME_LEN)
    return -1;

  //go through every directory
  for (i = 0; i < boot_block->dir_count; i++) {
    //get current file name
    int8_t* compare = (int8_t*)boot_block->direntries[i].filename;
    //compare input filename and current filename
    int diff = strncmp(compare, (int8_t*)fname, MAX_FILENAME_LEN);
    //if filenames are same
    if (diff == 0) {
      //copy filename, filetype, and inode_num to dest dentry
      return read_dentry_by_index(i, dentry);
    }
  }
  //if not found, fail
  return -1;
}

/* read_dentry_by_index
 *
 * Description: search the dentry by index
 * Inputs: index -- the index of the directory need searched
 *         dentry -- the destination structure needed to copy to
 * Outputs: None
 * Return Value: 0 for success and -1 for fail
 * Side Effects: copy the structure found to the input dentry
 */
int32_t read_dentry_by_index (int32_t index, dentry_t* dentry){
  //if index is invalid, fail
  if (dentry == NULL || index < 0 || index >= boot_block->dir_count) {
    return -1;
  }
  //copy filename, filetype, and inode_num of specified directory to dest dentry
  strncpy((int8_t*)dentry->filename, (int8_t*)boot_block->direntries[index].filename, MAX_FILENAME_LEN);
  dentry->filetype = boot_block->direntries[index].filetype;
  dentry->inode_num = boot_block->direntries[index].inode_num;
  return 0;
}

/* read_data
 *
 * Description: store data into buf starting from offset and read length bytes
 * Inputs: inode -- the index node of the file needed to read
 *         offset -- the starting position to read
 *         buf -- the destination buffer needed to copy to
 *         length -- number of bytes needed to read
 * Outputs: None
 * Return Value: number of bytes copied
 * Side Effects: copy data found to the input buffer
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
  //if inode is invalid or length is invalid or offset is invalid, fail
  if (inode < 0 || inode >= boot_block->inode_count || length < 0 || offset < 0) {
    return -1;
  }
  //get the current index node needed
  inode_t* cur_inode = (inode_t*)((uint32_t)index_node + inode*BLK_SIZE);
  //if length is zero or offset is out of bound, fail
  if (length == 0 || offset >= cur_inode->blk_length) {
    return 0;
  }
  //calculate which block it is
  uint32_t blk_idx = offset / BLK_SIZE;
  //get the starting address of data block
  uint32_t cur_data_blk = data_blk_start + cur_inode->data_block_num[blk_idx] * BLK_SIZE;
  cur_data_blk = cur_data_blk + (offset%BLK_SIZE);

  int32_t count = length;
  //if number of bytes needed to read is larger than remaining bytes
  if (count > cur_inode->blk_length - offset) {
    //just set number of bytes needed to read as the remaining bytes
    count = cur_inode->blk_length - offset;
  }
  //if number of bytes needed to read is within the current block
  if (count <= BLK_SIZE - (offset%BLK_SIZE)) {
    //copy data in this block and return
    memcpy(buf,(uint8_t*)cur_data_blk, count);
    return count;
  }
  //store the number of bytes needed to copy which should be the return value
  int32_t read_count = count;
  //copy the first block
  memcpy(buf,(void*)cur_data_blk, BLK_SIZE - (offset%BLK_SIZE));
  //update count, index, and all pointers
  count = count - (BLK_SIZE - (offset%BLK_SIZE));
  buf = buf + BLK_SIZE - (offset%BLK_SIZE);
  blk_idx++;
  cur_data_blk = data_blk_start + cur_inode->data_block_num[blk_idx] * BLK_SIZE;

  //when number of bytes needed to copy is positive
  while (count > 0) {
    //if remaining number of bytes is less than one block, just copy and return
    if (count <= BLK_SIZE) {
      memcpy(buf,(void*)cur_data_blk, count);
      return read_count;
    }
    //otherwise, copy the whole block
    memcpy(buf,(void*)cur_data_blk, BLK_SIZE);
    //update count, index and all pointers
    count = count - BLK_SIZE;
    buf = buf + BLK_SIZE;
    blk_idx++;
    //get the next starting address of data block
    cur_data_blk = data_blk_start + cur_inode->data_block_num[blk_idx] * BLK_SIZE;
  }
  return read_count;
}

/* open_file
 *
 * Description: open the file
 * Inputs: filename -- name of the file to open
 * Outputs: None
 * Return Value: zero
 * Side Effects: None
 */
int32_t open_file(const uint8_t* filename){
  // check if filename is invalid
  if (filename == NULL) return -1;
  return 0;

}

/* read_file
 *
 * Description: read the file by name starting from offset and read nbytes
 * Inputs: fd -- the file
 *         buf -- the destination buffer needed to copy to
 *         nbytes -- number of bytes needed to read
 *         name -- name of the file needed to read
 *         offset -- the starting position to read
 * Outputs: None
 * Return Value: number of bytes copied
 * Side Effects: copy data found to the input buffer
 */
int32_t read_file(int32_t fd, void* buf, int32_t nbytes){
  pcb_t* cur_pcb = get_pcb();
  //call read_data to get the data and return number of bytes copied
  int32_t ret_val = read_data(cur_pcb->file_des[fd].inode, cur_pcb->file_des[fd].file_pos, (uint8_t*)buf, nbytes);
  if (ret_val != -1) {
    cur_pcb->file_des[fd].file_pos += ret_val;    //update offset
  }
  return ret_val;
}

/* write_file
 *
 * Description: write the file
 * Inputs: fd -- the file
 *         buf -- the destination buffer needed to copy to
 *         nbytes -- number of bytes needed to write
 * Outputs: None
 * Return Value: -1
 * Side Effects: None
 */
int32_t write_file(int32_t fd, const void* buf, int32_t nbytes){
  return -1;    //do nothing
}

/* close_file
 *
 * Description: close the file
 * Inputs: fd -- the file
 * Outputs: None
 * Return Value: zero
 * Side Effects: None
 */
int32_t close_file(int32_t fd){
  return 0;   //do nothing
}

/* open_dir
 *
 * Description: open the directory
 * Inputs: dirname -- name of the directory needed to open
 * Outputs: None
 * Return Value: zero
 * Side Effects: None
 */
int32_t open_dir(const uint8_t* dirname){
  return 0;   //do nothing
}

/* read_dir
 *
 * Description: read the directory by index
 * Inputs: fd -- the file
 *         buf -- the destination buffer needed to copy to
 *         nbytes -- number of bytes needed to read
 *         index -- index of the directory needed to read
 * Outputs: None
 * Return Value: number of bytes read
 * Side Effects: copy filename found to the input buffer
 */
int32_t read_dir(int32_t fd, void* buf, int32_t nbytes){
  if (buf == NULL) return -1;
  //if index or nbytes is invalid, fail
  pcb_t* cur_pcb = get_pcb();
  int32_t index = cur_pcb->file_des[fd].file_pos;
  if (index < 0 || nbytes < 0) {
    return -1;
  }
  //if index is larger than maximum index of directory, reach the end, return 0
  if (index >= boot_block->dir_count) {
    return 0;
  }
  //get the copied dentry by calling read_dentry_by_index with input index
  dentry_t temp_dentry;
  //int32_t returned_val = read_dentry_by_index(index, &temp_dentry);
  //if return value is -1, read failed
  if (read_dentry_by_index(index, &temp_dentry) == -1)
    return -1;
  //get the length of the filename
  int i = 0;
  while (temp_dentry.filename[i] != NULL) {
    i++;
  }
  //if length is greater than MAX_FILENAME_LEN, just use MAX_FILENAME_LEN
  if (i > MAX_FILENAME_LEN) {
    i = MAX_FILENAME_LEN;
  }
  //copy the filename into dest buf
  strncpy((int8_t*)buf, (int8_t*)temp_dentry.filename, i);
  //update offset
  cur_pcb->file_des[fd].file_pos++;
  //return number of bytes copied
  return i;
}

/* write_dir
 *
 * Description: write the directory
 * Inputs: fd -- the file
 *         buf -- the destination buffer needed to copy to
 *         nbytes -- number of bytes needed to read
 * Outputs: None
 * Return Value: -1
 * Side Effects: None
 */
int32_t write_dir(int32_t fd, const void* buf, int32_t nbytes){
  return -1;    //do nothing
}

/* close_dir
 *
 * Description: close the directory
 * Inputs: fd -- the file
 * Outputs: None
 * Return Value: zero
 * Side Effects: None
 */
int32_t close_dir(int32_t fd){
  return 0;   //do nothing
}
