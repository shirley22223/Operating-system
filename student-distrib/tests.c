#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "idt_handler.h"
#include "idt_linkage.h"
#include "idt_init.h"
#include "paging.h"
#include "rtc.h"
#include "file_system.h"
#include "types.h"
#include "keyboard.h"
#include "terminal.h"


#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
		 asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// added more tests

/* IRQ Test
 *
 * Asserts that IRQ_numbers sent are valid
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: enable/disable IRQ, PIC initialization
 * Files: i8259.c/h
 */
int irq_test(){
	TEST_HEADER;

	int result = PASS;
	// valid master irq number
	enable_irq(4);
	disable_irq(4);

	// valid slave irq number
	enable_irq(10);
	disable_irq(10);

	// multiple disable ira
	disable_irq(9);
	disable_irq(9);

	// multiple enable irq
	enable_irq(3);
	enable_irq(3);

	// invalid irq number
	enable_irq(25);
	disable_irq(25);

	return result;
}

/* Paging Test
 *
 * Asserts that dereference NULL raises exception
 * Inputs: None
 * Outputs: FAIL/Page Fault Exception
 * Side Effects: None
 * Coverage: paging initilization, IDT handler
 * Files: paging.c/h, idt_handler.c/h
 */
int paging_test(){
		TEST_HEADER;
		// assigns a pointer to NULL
		int* a = 0;
		// assign a random value to test dereferencing NULL pointer
		*a = 2;

		return FAIL;
}

/* Exception Test
 *
 * Asserts that Divide error exception arises when dividing by zero
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: paging initilization, IDT handler
 * Files: idt_handler.c/h
 */
int exception_test(){
		TEST_HEADER;

		cli();
		int result = PASS;
		// test dividing by zero
		int a = 0;
		int b = 4 / a;
		if (b == 0) {
			assertion_failure();
			result = FAIL;
		}
		assertion_failure();
		result = FAIL;

		return result;
}

/* System Interrupts Test
 *
 * Asserts that system interrupt is set up correctly
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: enable/disable IRQ, PIC initialization
 * Files: i8259.c/h
 */
int sys_interrupt_test(){
		TEST_HEADER;
		// System call vector
		asm volatile("int $0x80");
		return PASS;
}


/* Checkpoint 2 tests */
/* Terminal read write Test
 *
 * Asserts that the terminal reads and writes successfully
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Terminal Read/Write
 * Files: terminal.c/h
 */
int terminal_read_write_test() {
		TEST_HEADER;
		// random integer as the first argument to the tread/twrite function
		int32_t fd = 2;
		int32_t a, b;
		uint8_t buf[KEYBOARD_BUFFER_SIZE];
		// loop forever
		while(1) {
				a = tread(fd, buf, KEYBOARD_BUFFER_SIZE);
				b = twrite(fd, buf, a);
		}
		assertion_failure();
		return FAIL;
}

/* Terminal write Test
 *
 * Asserts that the terminal writes contents fewer than buffer size successfully
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Terminal Write
 * Files: terminal.c/h
 */
int terminal_write_test_short() {// shorter than buffer size
		TEST_HEADER;
		// random integer as the first argument to the tread/twrite function
		int32_t fd = 2;
		// initialize a buffer of 30 characters (shorter than buffer size 128)
		uint8_t file[30] = {'S', 'h', 'e', 'r', 'r', 'y', ' ', 'W', 'u', ' ', 'i', 's',
		' ', 'a', ' ', 'C', 'u', 't', 'i', 'e', '.', '\n'};
		// tell terminal to write 30 bytes/characters
		twrite(fd, file, 30);
		return PASS;
}

/* Terminal write Test
 *
 * Asserts that the terminal writes contents larger than buffer size successfully
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Terminal Write
 * Files: terminal.c/h
 */
int terminal_write_test_long() {// shorter than buffer size
		TEST_HEADER;
		// random integer as the first argument to the tread/twrite function
		int32_t fd = 2;
		// initialize a buffer of 300 characters (larger than buffer size 128)
		uint8_t file[300] = {'S', 'h', 'e', 'r', 'r', 'y', ' ', 'W', 'u', ' ', 'i', 's',
		' ', 'a', ' ', 'C', 'u', 't', 'i', 'e', '.', '\n','S', 'h', 'e', 'r', 'r', 'y', ' ', 'W', 'u', ' ', 'i', 's',
		' ', 'a', ' ', 'C', 'u', 't', 'i', 'e', '.', '\n','S', 'h', 'e', 'r', 'r', 'y', ' ', 'W', 'u', ' ', 'i', 's',
		' ', 'a', ' ', 'C', 'u', 't', 'i', 'e', '.', '\n','S', 'h', 'e', 'r', 'r', 'y', ' ', 'W', 'u', ' ', 'i', 's',
		' ', 'a', ' ', 'C', 'u', 't', 'i', 'e', '.', '\n','S', 'h', 'e', 'r', 'r', 'y', ' ', 'W', 'u', ' ', 'i', 's',
		' ', 'a', ' ', 'C', 'u', 't', 'i', 'e', '.', '\n','S', 'h', 'e', 'r', 'r', 'y', ' ', 'W', 'u', ' ', 'i', 's',
		' ', 'a', ' ', 'C', 'u', 't', 'i', 'e', '.', '\n','S', 'h', 'e', 'r', 'r', 'y', ' ', 'W', 'u', ' ', 'i', 's',
		' ', 'a', ' ', 'C', 'u', 't', 'i', 'e', '.', '\n','S', 'h', 'e', 'r', 'r', 'y', ' ', 'W', 'u', ' ', 'i', 's',
		' ', 'a', ' ', 'C', 'u', 't', 'i', 'e', '.', '\n','S', 'h', 'e', 'r', 'r', 'y', ' ', 'W', 'u', ' ', 'i', 's',
		' ', 'a', ' ', 'C', 'u', 't', 'i', 'e', '.', '\n','S', 'h', 'e', 'r', 'r', 'y', ' ', 'W', 'u', ' ', 'i', 's',
		' ', 'a', ' ', 'C', 'u', 't', 'i', 'e', '.', '\n','S', 'h', 'e', 'r', 'r', 'y', ' ', 'W', 'u', ' ', 'i', 's',
		' ', 'a', ' ', 'C', 'u', 't', 'i', 'e', '.', '\n','S', 'h', 'e', 'r', 'r', 'y', ' ', 'W', 'u', ' ', 'i', 's',
		' ', 'a', ' ', 'C', 'u', 't', 'i', 'e', '.', '\n','S', 'h', 'e', 'r', 'r', 'y', ' ', 'W', 'u', ' ', 'i', 's',
		' ', 'a', ' ', 'C', 'u', 't', 'i', 'e', '.', '\n'};
		// tell terminal to write 300 bytes/characters
		int a = twrite(fd, file, 300);
		if(a > KEYBOARD_BUFFER_SIZE){
			assertion_failure();
			return FAIL;
		}
		else return PASS;
}



// /* RTC Open Test
//  *
//  * Asserts that RTC is opened correctly
//  * Inputs: None
//  * Outputs: PASS/FAIL
//  * Side Effects: None
//  * Coverage: RTC open
//  * Files: rtc.c/h
//  */
// int rtc_open_test(){
// 	int8_t file;
// 	TEST_HEADER;
// 	if (rtc_open(file) != 0){
// 		assertion_failure();
// 		return FAIL;
// 	}
// 	// repeat calls
// 	if (rtc_open(file) != 0){
// 		assertion_failure();
// 		return FAIL;
// 	}
// 	if (rtc_open(file) != 0){
// 		assertion_failure();
// 		return FAIL;
// 	}
//
// 	return PASS;
// }
//
//
// /* RTC Read Write Test
//  *
//  * Asserts that RTC is written and read correctly, and
//  * the freuqncy of RTC can be changed correctly
//  * Inputs: None
//  * Outputs: PASS/FAIL
//  * Side Effects: None
//  * Coverage: RTC read/write and set freuqncy
//  * Files: rtc.c/h
//  */
// int rtc_read_write_test(){
// 	TEST_HEADER;
// 	// counter used to show changed freuqncy interval
// 	int i;
// 	int32_t fd;
//
// 	// invalid test cases
// 	int32_t rate = 3;
// 	if (rtc_write(fd, &rate, 4) != -1){
// 		assertion_failure();
// 		return FAIL;
// 	}
//
// 	rate = 0;
// 	if (rtc_write(fd, &rate, 4) != -1){
// 		assertion_failure();
// 		return FAIL;
// 	}
//
// 	rate = 4;
// 	if (rtc_write(fd, &rate, 2) != -1){
// 		assertion_failure();
// 		return FAIL;
// 	}
//
// 	rate = 2046;
// 	if (rtc_write(fd, &rate, 4) != -1){
// 		assertion_failure();
// 		return FAIL;
// 	}
//
// 	//valid test cases
//
// 	// random valid rtc rate
// 	rate = 16;
// 	rtc_write(fd, &rate, 4);
// 	if (rtc_write(fd, &rate, 4) != 4){
// 		assertion_failure();
// 		return FAIL;
// 	}
// 	for (i = 0; i < 20; i++){
// 		rtc_read(fd, &rate, 4);
// 		putkey('1');
// 	}
// 	putkey('\n');
//
// 	// max rtc rate
// 	rate = 1024;
// 	rtc_write(fd, &rate, 4);
// 	if (rtc_write(fd, &rate, 4) != 4){
// 		assertion_failure();
// 		return FAIL;
// 	}
// 	for (i = 0; i < 100; i++){
// 		rtc_read(fd, &rate, 4);
// 		putkey('1');
// 	}
// 	putkey('\n');
//
//
// 	// set rate to default freq
// 	rate = 2;
// 	rtc_write(fd, &rate, 4);
// 	if (rtc_write(fd, &rate, 4) != 4){
// 		assertion_failure();
// 		return FAIL;
// 	}
// 	for (i = 0; i < 20; i++){
// 		rtc_read(fd, &rate, 4);
// 		putkey('1');
// 	}
// 	putkey('\n');
//
// 	return PASS;
// }


/* filesys read directory Test
 *
 * Asserts that all directories can be read
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: read_dir, read_dentry_by_index
 * Files: file_system.c/h
 */
// int filesys_read_dir_test(){
// 	TEST_HEADER;
// 	int32_t fd = 0;	//initialize fd
// 	int i = 0;
// 	clear();	//clear the screen
// 	set_text_pos(0, 0);
// 	uint8_t fname_buf[MAX_FILENAME_LEN + 1];
// 	//test read_dir when index is -1, which is invalid
// 	if (read_dir(fd, fname_buf, MAX_FILENAME_LEN, -1) != -1) {
// 		printf("test index -1: %d", read_dir(fd, fname_buf, MAX_FILENAME_LEN, -1));
// 		return FAIL;
// 	}
// 	else printf("test index -1: pass\n");
// 	//go through for loop and read all directories
// 	for (i = 0; i < boot_block->dir_count; i++) {
// 		uint8_t fname_buf[MAX_FILENAME_LEN + 1]; // filename buffer
// 		int32_t retVal = read_dir(fd, fname_buf, MAX_FILENAME_LEN, i);
// 		if (retVal == 0)
// 			break; // EOF
// 		else if (retVal < 0)		//number of characters read is less than zero
// 		{
// 			printf("Read Failed");
// 			break;
// 		}
// 		fname_buf[retVal] = '\0';		//add a null terminator
// 		uint32_t idx_inode = boot_block->direntries[i].inode_num;
// 		inode_t* cur_idx_inode = (inode_t*)((uint32_t)index_node + idx_inode*BLK_SIZE);
// 		//print current filename, filetype, data size
// 		printf("%s filetype: %d  file size: %d \n", fname_buf, boot_block->direntries[i].filetype, cur_idx_inode->blk_length);
// 	}
// 	//test if reaches the end of directories
// 	if (i == boot_block->dir_count){
// 		printf("reach the last directory\n");
// 		//test the next directory which is out of bound and should be invalid
// 		if (read_dir(fd, fname_buf, MAX_FILENAME_LEN, i) == 0) {
// 			return PASS;
// 		}
// 		else{		//if the next directory can still be read, the test failed
// 			printf("read out of bound\n");
// 			return FAIL;
// 		}
// 	}
// 	else{		//if not reaches the end of directories, the test failed
// 		printf("not reach the end of directory\n");
// 		return FAIL;
// 	}
//  }

 /* filesys read file Test
  *
  * Asserts that all files can be read correctly
  * Inputs: None
  * Outputs: PASS/FAIL
  * Side Effects: None
  * Coverage: read_file, read_dentry_by_name, read_data
  * Files: file_system.c/h
  */
// int filesys_read_file_test(){
//  	TEST_HEADER;
//  	int32_t fd = 0;	//initialize fd
// 	//use all filenames to test
// 	uint8_t file_name[1] = ".";
// 	//uint8_t file_name[8] = "sigtest\0";
// 	//uint8_t file_name[6] = "shell\0";
// 	//uint8_t file_name[5] = "grep\0";
// 	//uint8_t file_name[7] = "syserr\0";
// 	//uint8_t file_name[4] = "rtc\0";
// 	//uint8_t file_name[5] = "fish\0";
// 	//uint8_t file_name[8] = "counter\0";
// 	//uint8_t file_name[9] = "pingpong\0";
// 	//uint8_t file_name[4] = "cat\0";
// 	//uint8_t file_name[10] = "frame0.txt";
// 	//uint8_t file_name[32] = "verylargetextwithverylongname.tx";
// 	//uint8_t file_name[3] = "ls\0";
// 	//uint8_t file_name[10] = "testprint\0";
// 	//uint8_t file_name[8] = "created\0";
// 	//uint8_t file_name[10] = "frame1.txt";
// 	//uint8_t file_name[6] = "hello\0";
//  	clear();		//clear the screen
//  	set_text_pos(0, 0);
// 	//500 is a random number chosen to store data in one file
// 	uint8_t file_buf[500];
// 	int32_t retVal = 0;
// 	int32_t offset = 0;
// 	int i;
// 	while (1) {
// 		//500 is a random number chosen as number of bytes need to read
// 		retVal = read_file(fd, &file_buf, 500, file_name, offset);
// 		offset += retVal;
// 		//if not found the file, or offset is invalid, the test failed
// 		if (retVal == -1) {
// 			printf("\nread failed\n");
// 			return FAIL;
// 		}
// 		//if return value is zero, has reached the end of the file, the test successes and ends
// 		else if (retVal == 0) {
// 			printf("\nreach the end\n");
// 			return PASS;
// 		}
// 		//print the characters in this buffer
// 		for (i = 0; i < retVal; i++) {
// 			printf("%c", file_buf[i]);
// 		}
// 	}
// 	//if go out of the while loop without a return, the test failed
// 	return FAIL;
// }


/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
	// TEST_OUTPUT("irq_test", irq_test());
	// TEST_OUTPUT("sys_interrupt_test", sys_interrupt_test());
	// TEST_OUTPUT("exception_test", exception_test());
	// TEST_OUTPUT("paging_test", paging_test());
	// TEST_OUTPUT("rtc_open_test", rtc_open_test());
	// TEST_OUTPUT("rtc_read_write_test", rtc_read_write_test());
	//TEST_OUTPUT("filesys_read_dir_test", filesys_read_dir_test());
	 // TEST_OUTPUT("filesys_read_file_test", filesys_read_file_test());
  // TEST_OUTPUT("terminal_read_write_test", terminal_read_write_test());
  // TEST_OUTPUT("terminal_write_test_short", terminal_write_test_short());
  // TEST_OUTPUT("terminal_write_test_long", terminal_write_test_long());
}
