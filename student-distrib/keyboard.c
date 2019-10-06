#include "keyboard.h"
#include "terminal.h"
#include "idt_handler.h"
#include "lib.h"
#include "i8259.h"

#define ON    1
#define OFF   0
#define FIRST 0
#define SECOND 1
#define THIRD 2

// initialize keyboard flags for all three terminals
int shift_flag[TERMINAL_COUNT] = {OFF, OFF, OFF};
int caps_flag[TERMINAL_COUNT] = {OFF, OFF, OFF};
int ctrl_flag[TERMINAL_COUNT] = {OFF, OFF, OFF};
int alt_flag= OFF;

// keyboard http://www.plantation-productions.com/Webster/www.artofasm.com/DOS/pdf/apndxc.pdf
static uint8_t keyboard_map[KEY_ARRAY_ROW][KEY_ARRAY_COL] = {
	// no caps and no shift
	{'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0',
	 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\0', '\0', 'a', 's',
	 'd', 'f', 'g', 'h', 'j', 'k', 'l' , ';', '\'', '`', '\0', '\\', 'z', 'x', 'c', 'v',
	 'b', 'n', 'm',',', '.', '/', '\0', '*', '\0', ' ', '\0'},
	 // no caps and shift
	{'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0', '\0',
	 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\0', '\0', 'A', 'S',
	 'D', 'F', 'G', 'H', 'J', 'K', 'L' , ':', '"', '~', '\0', '|', 'Z', 'X', 'C', 'V',
	 'B', 'N', 'M', '<', '>', '?', '\0', '*', '\0', ' ', '\0'},
	// caps and no shift
	{'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0',
	 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\0', '\0', 'A', 'S',
	 'D', 'F', 'G', 'H', 'J', 'K', 'L' , ';', '\'', '`', '\0', '\\', 'Z', 'X', 'C', 'V',
	 'B', 'N', 'M', ',', '.', '/', '\0', '*', '\0', ' ', '\0'},
	// caps and shift
	{'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0', '\0',
	 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\0', '\0', 'a', 's',
	 'd', 'f', 'g', 'h', 'j', 'k', 'l' , ':', '"', '~', '\0', '\\', 'z', 'x', 'c', 'v',
	 'b', 'n', 'm', '<', '>', '?', '\0', '*', '\0', ' ', '\0'}
};

/* keyboard_handler
 *
 * Description: handler for keyboard, read character from keyboard and
 * 				put the character on screen
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: Show the character read from keyboard on screen
 */
void keyboard_handler(){
    uint32_t word;
    word = inb(KEYBOARD_PORT);		//get the character from keyboard
		// loop forever until it breaks
    while(1){
			// check if character is out of range/ invalid
      if (word <= 0) {
        word = inb(KEYBOARD_PORT); // check for next input
      }
      else break;
    }

  	//show the character on screen
    switch(word){ //to decide which case to choose according to key
			case ENTER:
				enter_count[screen_terminal]++;
				enter();
				break;

      case L_SHIFT_DOWN:
        shift_flag[screen_terminal] = ON; //set shift flag
        break;

      case R_SHIFT_DOWN:
        shift_flag[screen_terminal] = ON; //set shift flag
        break;

      case L_SHIFT_UP:
        shift_flag[screen_terminal] = OFF;
        break;

      case R_SHIFT_UP:
        shift_flag[screen_terminal] = OFF;
        break;

      case CAPS_LOCK:
        caps_flag[screen_terminal] = ON - caps_flag[screen_terminal]; //alternate caps_flag
        break;

      case CTRL_DOWN:
        ctrl_flag[screen_terminal] = ON; //set control flag
        break;

      case CTRL_UP:
        ctrl_flag[screen_terminal] = OFF; //set control flag
        break;

      case BACK:
        backspace();
        break;

			case ALT_DOWN:
				alt_flag = ON;
				break;

			case ALT_UP:
				alt_flag = OFF;
				break;

			case F1:
				if(alt_flag == ON){
					// if already in the first terminal
					if(screen_terminal == FIRST) break;
					// switch to first terminal
					switch_term(FIRST);
				}
				break;

			case F2:
				if(alt_flag == ON){
					// if already in the second terminal
					if(screen_terminal == SECOND) break;
					switch_term(SECOND);
				}
				break;

			case F3:
				if(alt_flag == ON){
					// if already in the third terminal
					if(screen_terminal == THIRD) break;
					switch_term(THIRD);
				}
				break;

      default:
        display_key(word);
        break;
      }

    send_eoi(KEYBOARD_IRQ_NUM);		//send EOI with keyboard IRQ number
}

/* display_key
 *
 * Description: to diplay key to the terminal screen
 * Inputs: the correpsonding hex value of the key
 * Outputs: correpsonding key to be displayed
 * Return Value: None
 * Side Effects: Show the character read from keyboard on screen and
 *							 also add character to the text buffer if within range
 */
void display_key(uint32_t word) {
    if (word >= KEY_ARRAY_COL)    return; //check if out of bound

    uint8_t key;
    if (caps_flag[screen_terminal]==0 && shift_flag[screen_terminal]==0) // no caps and no shift
        key = keyboard_map[0][(uint32_t)word];		// get character from corresponding keyborad map
    else if (caps_flag[screen_terminal]==0 && shift_flag[screen_terminal]==1) // no caps and shift
        key = keyboard_map[1][(uint32_t)word];		// get character from corresponding keyborad map
    else if (caps_flag[screen_terminal]==1 && shift_flag[screen_terminal]==0) // caps and no shift
        key = keyboard_map[2][(uint32_t)word];		// get character from corresponding keyborad map
    else if (caps_flag[screen_terminal]==1 && shift_flag[screen_terminal]==1) // caps and shift
        key = keyboard_map[3][(uint32_t)word];		// get character from corresponding keyborad map


    if (ctrl_flag[screen_terminal] && (key == 'l' || key == 'L')) { // to clear the screen
        clear(); // clear text buffer
        set_text_pos(0, 0, screen_terminal);	// set cursor to the upper left corner position
    }

    else{
				if (key_buffer_index[screen_terminal] < KEYBOARD_BUFFER_SIZE-1)  { // if within range then add to text buffer
		        keyboard_buffer[screen_terminal][key_buffer_index[screen_terminal]] = key;
						key_buffer_index[screen_terminal]++;
						putkey(key, screen_terminal);
						update_cursor();
		    }
				if (term_buffer_index[screen_terminal] < KEYBOARD_BUFFER_SIZE-1)  { // if within range then add to text buffer
						terminal_buffer[screen_terminal][term_buffer_index[screen_terminal]] = key;
						term_buffer_index[screen_terminal]++;
				}
		}
}


/* keyboard_init
 *
 * Description: initialize the keyboard irq to get keyboard ready
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: Enable the keyboard irq
 */
void keyboard_init() {
  enable_irq(KEYBOARD_IRQ_NUM);
}

/* backspace
 *
 * Description: to handle the case if the pressed key is backspace
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: The cursor will move to a character ahead and the previous
 *							character will be cleared
 */
void backspace() {
		if (key_buffer_index[screen_terminal] > 0) {
        remove_char(key_buffer_index[screen_terminal]);
        key_buffer_index[screen_terminal]--;
        keyboard_buffer[screen_terminal][key_buffer_index[screen_terminal]] = NULL; // set previous character to NULL
    }
		if (term_buffer_index[screen_terminal] > 0 && tread_mode[screen_terminal] == 1) {
        term_buffer_index[screen_terminal]--;
        terminal_buffer[screen_terminal][term_buffer_index[screen_terminal]] = NULL; // set previous character to NULL
    }
}

/* clear_buffer
 *
 * Description: to clear the text buffer
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: clear the text buffer
 */
void clear_buffer() {
	// set all buffer characters to NULL
	int i = 0;
	for (i = 0; i < key_buffer_index[screen_terminal]; i++) {
		keyboard_buffer[screen_terminal][i] = NULL;
	}
	// initialize buffer indices to default zeros
	key_buffer_index[screen_terminal] = 0;
}

/* enter
 *
 * Description: to handle the case if the pressed key is enter
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: The cursor will move to a new line and the text buffer
 *							 will be cleared
 */
void enter() {
		clear_buffer();
		if(term_buffer_index[screen_terminal] < KEYBOARD_BUFFER_SIZE) {
				terminal_buffer[screen_terminal][term_buffer_index[screen_terminal]] = '\n'; // save '\n' into terminal buffer
				term_buffer_index[screen_terminal]++;
		}
    enter_lib(screen_terminal);
}
