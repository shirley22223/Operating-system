#include "terminal.h"
#include "lib.h"
#include "system_calls.h"


// Initialize all terminal information to 0
volatile int enter_count[TERMINAL_COUNT] = {0, 0, 0};
volatile int tread_mode[TERMINAL_COUNT] = {0, 0, 0};
volatile uint32_t key_buffer_index[TERMINAL_COUNT] = {0, 0, 0};
volatile uint32_t term_buffer_index[TERMINAL_COUNT] = {0, 0, 0};


/* void terminal_init();
 * Inputs: None
 * Return Value: None
 * Function: Initialize terminal properties to default values*/
void terminal_init(){
    int i;
    // loop from first to the last terminal
    for (i = 0; i < TERMINAL_COUNT; i++) {
        terminal[i].terminal_id = i;
        terminal[i].active_process = 0;   // Initialize active process to be default first process
        terminal[i].fish_check = 0;     // Initialize fish flag to default value
    }
    screen_terminal = 0; //set as the first terminal
}



/* void topen();
 * Inputs: None
 * Return Value: 0
 * Function: Initialize terminal properties (Nothing at this point)*/
int32_t topen(const uint8_t* filename) {
    return DEFAULT_RET_VAL;
}

/* void tclose();
 * Inputs: None
 * Return Value: 0
 * Function: Clear terminal variables (Nothing at this point)*/
int32_t tclose(int32_t fd) {
    return DEFAULT_RET_VAL;
}

/* void tread();
 * Inputs: fd -- file descriptor
           buf -- buffer that stores what is read from terminal buffer
           nbytes -- number of bytes to read
 * Return Value: number of characters read
 * Function: Read from the terminal buffer into buf */
int32_t tread(int32_t fd, void* buf, int32_t nbytes) {
    // Initialize the number of bytes read to 0
    int count = 0;
    int i;
    tread_mode[cur_terminal] = 1; //use terminal buffer
    // loop until it breaks
    while(1) {
      sti();
        // if at least one Enter was pressed
        if (enter_count[cur_terminal] != 0) {
          while(terminal_buffer[cur_terminal][count] != '\n' && count < nbytes) {
              *((uint8_t*)buf + count) = terminal_buffer[cur_terminal][count];
              count++;
          }

          if (terminal_buffer[cur_terminal][count] == '\n') {
              *((uint8_t*)buf + count) = '\n';
              count++;
          }

          // erase terminal buffer for the first to the last terminal
          for (i = 0; i < KEYBOARD_BUFFER_SIZE; i++) {
              terminal_buffer[cur_terminal][i] = NULL;
          }

          term_buffer_index[cur_terminal] = 0;  // reset buffer index to initial value
          enter_count[cur_terminal] = 0;
          break;
        }
     }
    tread_mode[cur_terminal] = 0;     // reset tread_mode for current terminal to initial value
    cli();
    return count;
}

/* void putc_to_screen();
 * Inputs: c -- character to put to the screen
 * Return Value: None
 * Function: Read from the terminal buffer into buf */
void putc_to_screen(const char c) {
  cli();
  // if enter was pressed, handle enter and return
  pcb_t* temp = get_pcb();
    if (c == '\n') {
        enter_lib(temp->terminal_id);
        return;
    }
    putkey(c, temp->terminal_id);
}


/* void twrite();
 * Inputs: fd -- file descriptor
           buf -- buffer that stores what to write to the screen
           nbytes -- number of bytes to write
 * Return Value: number of characters/bytes written or -1
 * Function: Write from buf to the screen */
int32_t twrite(int32_t fd, const void* buf, int32_t nbytes) {
    // Initialize number of bytes written to 0
    int count = 0;
    // if buf is NULL, return -1 to indicate fail
    if (buf == NULL)
        return -1;
    // while count is smaller than nbytes
    while (count < nbytes) {
        putc_to_screen(*((uint8_t*)buf + count));
        count++;
    }
    return count;
}
