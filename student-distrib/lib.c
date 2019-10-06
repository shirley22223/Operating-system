/* lib.c - Some basic library functions (printf, strlen, etc.)
 * vim:ts=4 noexpandtab */

#include "lib.h"
#include "terminal.h"
#include "paging.h"


#define VIDEO                 0xB8000
#define NUM_COLS              80
#define NUM_ROWS              25
#define ATTRIB                0x7
#define KEYBOARD_BUFFER_SIZE  128
#define VID_B0                0xB9000
#define VID_B1                0xBA000
#define VID_B2                0xBB000
#define _4KB                  0x1000
#define VM_END_ADDR           0x8400000  // 132 MB

static int screen_x;
static int screen_y;
static char* video_mem = (char *)VIDEO;
int prev_x[KEYBOARD_BUFFER_SIZE];
// initial auto entering flags to default zeros for all terminals
int auto_flag[TERMINAL_COUNT] = {0, 0, 0};


/* void clear(void);
 * Inputs: void
 * Return Value: none
 * Function: Clears video memory */
void clear(void) {
    map_video_page((uint32_t)video_mem);
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;
    }
}

/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output. */
int32_t printf(int8_t *format, ...) {

    /* Pointer to the format string */
    int8_t* buf = format;

    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
        switch (*buf) {
            case '%':
                {
                    int32_t alternate = 0;
                    buf++;

format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            putkey('%', screen_terminal);
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    puts(conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    puts(&conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                int8_t conv_buf[36];
                                int32_t value = *((int32_t *)esp);
                                if(value < 0) {
                                    conv_buf[0] = '-';
                                    itoa(-value, &conv_buf[1], 10);
                                } else {
                                    itoa(value, conv_buf, 10);
                                }
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            putkey((uint8_t) *((int32_t *)esp), screen_terminal);
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            puts(*((int8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                putkey(*buf, screen_terminal);
                break;
        }
        buf++;
    }
    return (buf - format);
}

/* void switch_term(uint8_t new_term)
 *   Inputs: new_term -- the terminal id to switch to
 *   Return Value: None
 *   Function: switch to the corresponding terminal and remap video mem page
 */
void switch_term(uint8_t new_term) {
    int i;
    map_video_page((uint32_t)VIDEO);
    // check current screen terminal and map to video mem address
    switch(screen_terminal) {
        case 0:
            map_video_page((uint32_t)VID_B0);
            memcpy((char*)VID_B0,(char*)VIDEO, _4KB);
            break;

        case 1:
            map_video_page((uint32_t)VID_B1);
            memcpy((char*)VID_B1,(char*)VIDEO, _4KB);
            break;

        case 2:
            map_video_page((uint32_t)VID_B2);
            memcpy((char*)VID_B2,(char*)VIDEO, _4KB);
            break;

        default:
            break;
    }

    // copy new terminal video page to video mem addr 0xB8000
    switch(new_term) {
        case 0:
            map_video_page((uint32_t)VID_B0);
            memcpy((char*)VIDEO, (char*)VID_B0, _4KB);
            break;

        case 1:
            map_video_page((uint32_t)VID_B1);
            memcpy((char*)VIDEO, (char*)VID_B1, _4KB);
            break;

        case 2:
            map_video_page((uint32_t)VID_B2);
            memcpy((char*)VIDEO, (char*)VID_B2, _4KB);
            break;

        default:
            break;
    }
    // after swtiching to new terminal, check
    // if the fish program is running and the video mem needs to be updated
    screen_terminal = new_term;
    for (i = 0; i < TERMINAL_COUNT; i++) {
      if (terminal[i].fish_check != 0) {
        map_video_mem(VM_END_ADDR + (i)*_4KB);
      }
    }
    update_cursor();
}

/* int32_t puts(int8_t* s);
 *   Inputs: int_8* s = pointer to a string of characters
 *   Return Value: Number of bytes written
 *    Function: Output a string to the console */
int32_t puts(int8_t* s) {
    register int32_t index = 0;
    while (s[index] != '\0') {
        putkey(s[index], screen_terminal);
        index++;
    }
    return index;
}

/* void putc(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 *  Function: Output a character to the console */
void putc(uint8_t c) {
    if(c == '\n' || c == '\r') {
        screen_y++;
        screen_x = 0;
    } else {
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1)) = c;
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1) + 1) = ATTRIB;
        screen_x++;
        screen_x %= NUM_COLS;
        screen_y = (screen_y + (screen_x / NUM_COLS)) % NUM_ROWS;
    }
    update_cursor();
}


/* void putkey(uint8_t c);
 * Inputs: uint_8 c = character to print
 * Return Value: void
 * Function: Output a character to the console and set text to the right position
 */
void putkey(uint8_t c, uint8_t term_num) {
  cli();
    if(term_num == screen_terminal) {
         map_video_page((uint32_t)video_mem);
    }
    else{
        switch(term_num) {
            case 0:
                map_4KB_page((uint32_t)VID_B0);
                break;

            case 1:
                map_4KB_page((uint32_t)VID_B1);
                break;

            case 2:
                map_4KB_page((uint32_t)VID_B2);
                break;

            default:
                break;
        }
    }
    // if pressed key was enter, handle enter
    if (c == '\n') enter_lib(term_num);
    else{
      *(uint8_t *)(video_mem + ((NUM_COLS * terminal[term_num].screen_y + terminal[term_num].screen_x) << CONST_OFFSET)) = c;
      *(uint8_t *)(video_mem + ((NUM_COLS * terminal[term_num].screen_y + terminal[term_num].screen_x) << CONST_OFFSET) + CONST_OFFSET) = ATTRIB;
      set_text_pos(terminal[term_num].screen_x + ONE_LINE, terminal[term_num].screen_y, term_num);
    }
}

/* void remove_char(int buffer_index);
 * Inputs: index of the buffer to be removed
 * Return Value: None
 * Function: Remove a character from the console */
void remove_char(int buffer_index) {
    map_video_page((uint32_t)video_mem);
    if (buffer_index < X_START)
        return;
    if (terminal[screen_terminal].screen_x == X_START && terminal[screen_terminal].screen_y == Y_START)
        return;
    if (terminal[screen_terminal].screen_x == X_START) {
        //set_text_pos(prev_x[terminal[screen_terminal].screen_y - ONE_LINE], terminal[screen_terminal].screen_y - ONE_LINE, screen_terminal);   // set text position to the previous line (y - 1)
        set_text_pos(NUM_COLS - 1, terminal[screen_terminal].screen_y - ONE_LINE, screen_terminal);   // set text position to the previous line (y - 1)
    }
    else {
        set_text_pos(terminal[screen_terminal].screen_x - ONE_LINE, terminal[screen_terminal].screen_y, screen_terminal);
    }
    // delete previous character
    *(uint8_t *)(video_mem + ((NUM_COLS * terminal[screen_terminal].screen_y + terminal[screen_terminal].screen_x) << CONST_OFFSET)) = ' ';
    *(uint8_t *)(video_mem + ((NUM_COLS * terminal[screen_terminal].screen_y + terminal[screen_terminal].screen_x) << CONST_OFFSET) + ONE_LINE) = ATTRIB;
}


/* void enter_lib(int buffer_index);
 * Inputs: index of the buffer to be removed
 * Return Value: None
 * Function: Remove a character from the console */
void enter_lib(uint8_t term_num) {
    cli();
    if(term_num == screen_terminal) {
         map_video_page((uint32_t)video_mem);
    }
    else{
        switch(term_num) {
            case 0:
                map_4KB_page((uint32_t)VID_B0);
                break;

            case 1:
                map_4KB_page((uint32_t)VID_B1);
                break;

            case 2:
                map_4KB_page((uint32_t)VID_B2);
                break;

            default:
                break;
        }
    }
    // save previous end character position
    if(auto_flag[term_num] == 1 && terminal[term_num].screen_x == X_START){   // if auto entering next line was implemented and at the first position of the new line
      // auto entered and new enter pressed at beginning of line
      set_text_pos(X_START, terminal[term_num].screen_y, term_num);
      auto_flag[term_num] = 0;   // set auto flag back to 0 after handling
    }
    else{
      // save previous line position
      prev_x[terminal[term_num].screen_y] = terminal[term_num].screen_x;
      set_text_pos(X_START, terminal[term_num].screen_y + ONE_LINE, term_num);

    }
}


/* void set_text_pos(int x, int y);
 * Inputs: x -- the position to set as screen_x
           y -- the position to set as screen_y
 * Return Value: None
 * Function: Set the current text position to the specified location
             and update the cursor */
void set_text_pos(int x, int y, uint8_t term_num) {
    if (x >= NUM_COLS) {
        terminal[term_num].screen_x = NUM_COLS - ONE_LINE; // prevent from out of range
        auto_flag[term_num] = 1;  // set auto flag to indicate auto entering has happened
        enter_lib(term_num);
        if(term_num == screen_terminal){
          update_cursor();
        }
        return;
    }

    else terminal[term_num].screen_x = x;

    if (y >= NUM_ROWS) {
        scroll(term_num);
        terminal[term_num].screen_y = NUM_ROWS - ONE_LINE;
    }
    else {
        terminal[term_num].screen_y = y;
    }
    // set cursor
    if(term_num == screen_terminal){
      update_cursor();
    }
}


/* void update_cursor();
 * Inputs: None
 * Return Value: None
 * Function: Update cursor to current screen x, y positions */
void update_cursor() {
      uint16_t pos = terminal[screen_terminal].screen_y * NUM_COLS + terminal[screen_terminal].screen_x;
      // cursor low port to VGA index register
      outb(0x0F, 0x3D4);
      // cursor low position to VGA data register
      outb((uint8_t) (pos & 0xFF), 0x3D5);
      // cursor high port to VGA index register
      outb(0x0E, 0x3D4);
      // cursor high position to VGA data register
      outb((uint8_t) ((pos >> 8) & 0xFF), 0x3D5);
}


/* void scroll();
 * Inputs: None
 * Return Value: None
 * Function: Enable vertical scrolling to the next line*/
void scroll(uint8_t term_num) {
  cli();
    uint32_t i, j;
    // iterate through every screen position to copy from next line to current line, discarding the first line
    for (i = 0; i < NUM_ROWS - ONE_LINE; i++) {
        for (j = 0; j < NUM_COLS; j++) {
            *(uint8_t *)(video_mem + ((NUM_COLS * (i) + j) << CONST_OFFSET)) = *(uint8_t *)(video_mem + ((NUM_COLS * (i+ONE_LINE) + j) << CONST_OFFSET));
        }
    }
    // set the next entire line to ' ' for each column position
    for (i = 0; i < NUM_COLS; i++) {
        *(uint8_t *)(video_mem + ((NUM_COLS * (NUM_ROWS - ONE_LINE) + i) << CONST_OFFSET)) = ' ';
    }
    set_text_pos(X_START, terminal[term_num].screen_y, term_num);
}


/* int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix);
 * Inputs: uint32_t value = number to convert
 *            int8_t* buf = allocated buffer to place string in
 *          int32_t radix = base system. hex, oct, dec, etc.
 * Return Value: number of bytes written
 * Function: Convert a number to its ASCII representation, with base "radix" */
int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix) {
    static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int8_t *newbuf = buf;
    int32_t i;
    uint32_t newval = value;

    /* Special case for zero */
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    /* Go through the number one place value at a time, and add the
     * correct digit to "newbuf".  We actually add characters to the
     * ASCII string from lowest place value to highest, which is the
     * opposite of how the number should be printed.  We'll reverse the
     * characters later. */
    while (newval > 0) {
        i = newval % radix;
        *newbuf = lookup[i];
        newbuf++;
        newval /= radix;
    }

    /* Add a terminating NULL */
    *newbuf = '\0';

    /* Reverse the string and return */
    return strrev(buf);
}

/* int8_t* strrev(int8_t* s);
 * Inputs: int8_t* s = string to reverse
 * Return Value: reversed string
 * Function: reverses a string s */
int8_t* strrev(int8_t* s) {
    register int8_t tmp;
    register int32_t beg = 0;
    register int32_t end = strlen(s) - 1;

    while (beg < end) {
        tmp = s[end];
        s[end] = s[beg];
        s[beg] = tmp;
        beg++;
        end--;
    }
    return s;
}

/* uint32_t strlen(const int8_t* s);
 * Inputs: const int8_t* s = string to take length of
 * Return Value: length of string s
 * Function: return length of string s */
uint32_t strlen(const int8_t* s) {
    register uint32_t len = 0;
    while (s[len] != '\0')
        len++;
    return len;
}

/* void* memset(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive bytes of pointer s to value c */
void* memset(void* s, int32_t c, uint32_t n) {
    c &= 0xFF;
    asm volatile ("                 \n\
            .memset_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memset_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memset_aligned \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memset_top     \n\
            .memset_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     stosl           \n\
            .memset_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memset_done    \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%edx       \n\
            jmp     .memset_bottom  \n\
            .memset_done:           \n\
            "
            :
            : "a"(c << 24 | c << 16 | c << 8 | c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_word(void* s, int32_t c, uint32_t n);
 * Description: Optimized memset_word
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set lower 16 bits of n consecutive memory locations of pointer s to value c */
void* memset_word(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosw           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_dword(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive memory locations of pointer s to value c */
void* memset_dword(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosl           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memcpy(void* dest, const void* src, uint32_t n);
 * Inputs:      void* dest = destination of copy
 *         const void* src = source of copy
 *              uint32_t n = number of byets to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of src to dest */
void* memcpy(void* dest, const void* src, uint32_t n) {
    asm volatile ("                 \n\
            .memcpy_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memcpy_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memcpy_aligned \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memcpy_top     \n\
            .memcpy_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     movsl           \n\
            .memcpy_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memcpy_done    \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%edx       \n\
            jmp     .memcpy_bottom  \n\
            .memcpy_done:           \n\
            "
            :
            : "S"(src), "D"(dest), "c"(n)
            : "eax", "edx", "memory", "cc"
    );
    return dest;
}

/* void* memmove(void* dest, const void* src, uint32_t n);
 * Description: Optimized memmove (used for overlapping memory areas)
 * Inputs:      void* dest = destination of move
 *         const void* src = source of move
 *              uint32_t n = number of byets to move
 * Return Value: pointer to dest
 * Function: move n bytes of src to dest */
void* memmove(void* dest, const void* src, uint32_t n) {
    asm volatile ("                             \n\
            movw    %%ds, %%dx                  \n\
            movw    %%dx, %%es                  \n\
            cld                                 \n\
            cmp     %%edi, %%esi                \n\
            jae     .memmove_go                 \n\
            leal    -1(%%esi, %%ecx), %%esi     \n\
            leal    -1(%%edi, %%ecx), %%edi     \n\
            std                                 \n\
            .memmove_go:                        \n\
            rep     movsb                       \n\
            "
            :
            : "D"(dest), "S"(src), "c"(n)
            : "edx", "memory", "cc"
    );
    return dest;
}

/* int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n)
 * Inputs: const int8_t* s1 = first string to compare
 *         const int8_t* s2 = second string to compare
 *               uint32_t n = number of bytes to compare
 * Return Value: A zero value indicates that the characters compared
 *               in both strings form the same string.
 *               A value greater than zero indicates that the first
 *               character that does not match has a greater value
 *               in str1 than in str2; And a value less than zero
 *               indicates the opposite.
 * Function: compares string 1 and string 2 for equality */
int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n) {
    int32_t i;
    for (i = 0; i < n; i++) {
        if ((s1[i] != s2[i]) || (s1[i] == '\0') /* || s2[i] == '\0' */) {

            /* The s2[i] == '\0' is unnecessary because of the short-circuit
             * semantics of 'if' expressions in C.  If the first expression
             * (s1[i] != s2[i]) evaluates to false, that is, if s1[i] ==
             * s2[i], then we only need to test either s1[i] or s2[i] for
             * '\0', since we know they are equal. */
            return s1[i] - s2[i];
        }
    }
    return 0;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 * Return Value: pointer to dest
 * Function: copy the source string into the destination string */
int8_t* strcpy(int8_t* dest, const int8_t* src) {
    int32_t i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src, uint32_t n)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 *                uint32_t n = number of bytes to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of the source string into the destination string */
int8_t* strncpy(int8_t* dest, const int8_t* src, uint32_t n) {
    int32_t i = 0;
    while (src[i] != '\0' && i < n) {
        dest[i] = src[i];
        i++;
    }
    while (i < n) {
        dest[i] = '\0';
        i++;
    }
    return dest;
}

/* void test_interrupts(void)
 * Inputs: void
 * Return Value: void
 * Function: increments video memory. To be used to test rtc */
void test_interrupts(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        video_mem[i << 1]++;
    }
}
