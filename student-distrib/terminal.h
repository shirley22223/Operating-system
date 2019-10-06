#ifndef _TERMINAL_H
#define _TERMINAL_H
#include "types.h"


#define DEFAULT_RET_VAL   0
#define TERMINAL_COUNT    3
#define KEYBOARD_BUFFER_SIZE  128

//terminal buffer for terminal read and write
volatile uint8_t terminal_buffer[TERMINAL_COUNT][KEYBOARD_BUFFER_SIZE];
volatile uint8_t cur_terminal;
volatile uint8_t screen_terminal;
volatile uint8_t prev_screen_terminal;

// arrays for important info for three terminals
extern volatile int enter_count[TERMINAL_COUNT];
extern volatile int tread_mode[TERMINAL_COUNT] ;
extern volatile uint32_t key_buffer_index[TERMINAL_COUNT] ;
extern volatile uint32_t term_buffer_index[TERMINAL_COUNT];

// terminal struct
typedef struct {
    uint8_t terminal_id;
    uint8_t active_process;  //pid
    uint32_t screen_x;
    uint32_t screen_y;
    int32_t fish_check;
} __attribute__((packed)) terminal_t;

terminal_t terminal[TERMINAL_COUNT];

/* initialize terminal */
extern void terminal_init();
/* function to Initialize terminal variables */
extern int32_t topen(const uint8_t* filename);
/* function to clear any terminal variables */
extern int32_t tclose(int32_t fd);
/* function to read from key buffer to buf  */
extern int32_t tread(int32_t fd, void* buf, int32_t nbytes);
/* function to write from buf to the screen */
extern int32_t twrite(int32_t fd, const void* buf, int32_t nbytes);
/* function to put a character to the screen */
extern void putc_to_screen(const char c);

#endif
