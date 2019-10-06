#ifndef _KEYBOARD_H
#define _KEYBOARD_H
#include "types.h"

#ifndef ASM

#define KEYBOARD_PORT 	0x60		//the port of keyboard
#define KEY_ARRAY_COL	60			//number of columns of the keyboard array
#define KEY_ARRAY_ROW	4			//number of rows of the keyboard array
#define KEYBOARD_BUFFER_SIZE 128 // add one for \n

#define L_SHIFT_DOWN 0x2A
#define L_SHIFT_UP   0xAA
#define R_SHIFT_DOWN 0x36
#define R_SHIFT_UP   0xB6
#define CAPS_LOCK    0x3A
#define CTRL_DOWN    0x1D
#define CTRL_UP      0x9D
#define BACK         0x0E
#define ENTER        0x1C
#define ALT_DOWN     0x38
#define ALT_UP       0xB8
#define F1           0x3B
#define F2           0x3C
#define F3           0x3D

//test buffer for terminal
volatile uint8_t keyboard_buffer[3][KEYBOARD_BUFFER_SIZE];

// keyboard flags for all three terminals
extern int shift_flag[3];
extern int caps_flag[3];
extern int ctrl_flag[3];
extern int alt_flag;





/* handler for keyboard */
extern void keyboard_handler();

/* initialize keyboard*/
extern void keyboard_init();

/* function to handle backspace*/
extern void backspace();

/* function to handle enter*/
extern void enter();

/* display key to screen*/
extern void display_key(uint32_t word);

/* clear text buffer*/
extern void clear_buffer();

#endif
#endif
