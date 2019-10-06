#include "schedule.h"
#include "system_calls.h"
#include "restore_ebp.h"
#include "i8259.h"
#include "lib.h"
#include "x86_desc.h"
#include "paging.h"
#include "terminal.h"

#define PIT_MODE        0x36
#define PIT_IRQ         0
#define CHANNEL_0       0x40
#define MODE_REGISTER   0x43
#define FREQ            50
#define DIVISOR         1193180
#define LOW_BYTE        0xFF
#define EIGHT_SHIFT     8
#define FIVE_LEN        5


// initialize all counters to 0
uint32_t intr_counter = 0;
uint32_t counter = 0;


/*
 * int32_t pit_init();
 * Inputs: None
 * Return Value: None
 * Function: initialize pit and enable corresponding irq
 */
void pit_init() {
    int divisor = DIVISOR / FREQ;         // calculate divisor (20 ms = 50 Hz)
    outb(PIT_MODE, MODE_REGISTER);        //set our command byte 0x36 (Mode 3)
    outb(divisor & LOW_BYTE, CHANNEL_0);  //set lower byte of divisor
    outb(divisor >> EIGHT_SHIFT, CHANNEL_0);        //set higher byte of divisor, right shift 8 bits
    enable_irq(PIT_IRQ);
    return;
}

/*
 * int32_t pit_schedule();
 * Inputs: None
 * Return Value: None
 * Function: Handle pit interrupts, calls execute shell on the first three
             interrupts to open up three terminals
 */
void pit_schedule(){
    send_eoi(PIT_IRQ);
    pcb_t * cur_pcb = get_pcb();
    cur_pcb->my_ebp = get_ebp();

    cur_terminal = counter % TERMINAL_COUNT;
    counter = (counter + 1) % TERMINAL_COUNT;   // increment counter
    uint8_t command_str[FIVE_LEN] = "shell";           // a five character string "shell" to input into execute

    if(intr_counter < TERMINAL_COUNT){
      // start all three shell
      intr_counter++;
      execute((uint8_t*)command_str);
    }
    else {
        context_switch();
    }
    return;
}

/*
 * int32_t context_switch();
 * Inputs: None
 * Return Value: None
 * Function: Do context switch, paging and restore ebp for switching terminals
 */
void context_switch() {
    int switch_pid = terminal[cur_terminal].active_process;
    map_4MB_page(switch_pid);
    tss.ss0 = KERNEL_DS;
    tss.esp0 = _8MB - switch_pid * _8KB;
    // increment switch_pid to get the next 8KB pcb index
    pcb_t * next_pcb = (pcb_t*) (_8MB - (switch_pid + 1) * _8KB);

    asm volatile(
        "movl %0, %%ebp;"   // restore ebp
        :
        :"r"(next_pcb->my_ebp)
    );
    return;
}
