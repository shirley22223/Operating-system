/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"
#include "idt_handler.h"


#define SLAVE_MAX_IRQ   15
#define MASTER_MAX_IRQ  7
#define MASK_INIT       0xFF

/* Interrupt masks to determine which interrupts are enabled and disabled */
/* mask all of master IRQs 0-7  */
uint8_t master_mask = MASK_INIT;
/* mask all of slave IRQs 8-15 */
uint8_t slave_mask = MASK_INIT;


/* Initialize the 8259 PIC */
/* i8259_init
 *
 * Description: initialize the 8259 master and slave PIC
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: initialize master port and slave port
 */
void i8259_init() {
  // mask all of master and slave ports
  outb(master_mask, MASTER_8259_PORT + 1);
  outb(slave_mask, SLAVE_8259_PORT + 1);

  // four initilization control words to set up the master
  outb(ICW1, MASTER_8259_PORT);
  outb(ICW2_MASTER, MASTER_8259_PORT + 1);	//mster IRQ0-7 mapped to 0x20-0x27
  outb(ICW3_MASTER, MASTER_8259_PORT + 1);
  outb(ICW4, MASTER_8259_PORT + 1);		//master expects normal EOI

  // four initilization control words to set up the slave
  outb(ICW1, SLAVE_8259_PORT);
  outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);	//slave IRQ0-7 mapped to 0x28-0x2f
  outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);
  outb(ICW4, SLAVE_8259_PORT + 1);

  //mask all of master and slave ports
  outb(master_mask, MASTER_8259_PORT + 1);
  outb(slave_mask, SLAVE_8259_PORT + 1);
}

/* enable_irq
 *
 * Description: Enable (unmask) the specified IRQ
 * Inputs: irq_num
 * Outputs: None
 * Return Value: None
 * Side Effects: enable the specified IRQ
 */
void enable_irq(uint32_t irq_num) {
    // check if irq_num is valid (less than slave port number)
    if (irq_num > SLAVE_MAX_IRQ)  return;
    if (irq_num > MASTER_MAX_IRQ) {		//if irq_num is larger than 7, slave
        // enable slave
        irq_num -= (MASTER_MAX_IRQ + 1);
        slave_mask &= ~(1 << irq_num);
        outb(slave_mask, SLAVE_8259_PORT + 1);
        // enable master
        irq_num = SLAVE_IRQ_NUM;
        master_mask &= ~(1 << irq_num);
        outb(master_mask, MASTER_8259_PORT + 1);
    }
    else {		//if irq_num is less than 8, mster
        // enable master only
        master_mask &= ~(1 << irq_num);
        outb(master_mask, MASTER_8259_PORT + 1);
    }
}

/* disable_irq
 *
 * Description: Disable (mask) the specified IRQ
 * Inputs: irq_num
 * Outputs: None
 * Return Value: None
 * Side Effects: disable the specified IRQ
 */
void disable_irq(uint32_t irq_num) {
    // check if irq_num is valid (less than slave port number)
    if (irq_num > SLAVE_MAX_IRQ)  return;
    // disable slave
    if (irq_num > MASTER_MAX_IRQ) {		//if irq_num is larger than 7, slave
        //disable specified slave
		irq_num -= (MASTER_MAX_IRQ + 1);
        slave_mask |= (1 << irq_num);
        outb(slave_mask, SLAVE_8259_PORT + 1);
    }
    else {		//if irq_num is less than 8, master
        // disable specified master
        master_mask |= (1 << irq_num);
        outb(master_mask, MASTER_8259_PORT + 1);
    }
}

/* send_eoi
 *
 * Description: Send end-of-interrupt signal for the specified IRQ
 * Inputs: irq_num
 * Outputs: None
 * Return Value: None
 * Side Effects: Send end-of-interrupt signal for the specified IRQ
 */
void send_eoi(uint32_t irq_num) {
    if (irq_num > SLAVE_MAX_IRQ)  return;
    if (irq_num > MASTER_MAX_IRQ) {		//if irq_num is larger than 7, slave
        // send eoi to slave and master
        outb(EOI | (irq_num - (MASTER_MAX_IRQ + 1)), SLAVE_8259_PORT);
        outb(EOI | SLAVE_IRQ_NUM, MASTER_8259_PORT);
    }
    // send eoi to master if irq_num is less than 8
    else outb(EOI | irq_num, MASTER_8259_PORT);
}
