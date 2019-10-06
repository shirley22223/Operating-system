#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "terminal.h"

// Reference: https://wiki.osdev.org/RTC
// default RTC frequency = 2Hz
#define RTC_DEFAULT_FREQ    2
#define RTC_MIN_FREQ    		2
#define RTC_MAX_FREQ        1024
#define RTC_DEFAULT_RATE    RTC_MAX_FREQ / RTC_DEFAULT_FREQ
// magic number (2^15) used to convert frequency to rate
#define LOG_RATE_LIMIT      32768

// RTC interrupt flags for each terminal
volatile int32_t rtc_interrupt_flags[TERMINAL_COUNT];
// RTC rtc rates for each eterminal
volatile int32_t rtc_rates[TERMINAL_COUNT] = {RTC_DEFAULT_RATE, RTC_DEFAULT_RATE, RTC_DEFAULT_RATE};
// counter used to virtualize RTC
int32_t rtc_counter;

/* rtc_handler
 *
 * Description: handler for RTC
 *							output data to the port and print "RTC"
 * Inputs: None
 * Outputs: None
 * Return Value: None
 * Side Effects: print "RTC"
 */
void rtc_handler(){
  cli();
	// select register C
	outb(RTC_REG_C, RTC_REG_PORT);
	// throw away contents
	inb(RTC_RW_PORT);
	int i;
  rtc_counter ++;
  // reset interrupt flags
	for (i = 0; i < TERMINAL_COUNT; i++){
    if (rtc_counter % rtc_rates[i] == 0)
		  rtc_interrupt_flags[i] = 0;
	}
	// send EOI with RTC IRQ number
	send_eoi(RTC_IRQ_NUM);
  sti();
}

/* rtc_init
*
* Description: Initialize RTC
* Inputs: None
* Outputs: None
* Return Value: None
* Side Effects: RTC initialized, turning on IRQ8
*/
void rtc_init(){
  rtc_counter = 0;
  // select register B, and disable NMI
  outb(RTC_REG_B, RTC_REG_PORT);
  // read the current value of register B
  char prev = inb(RTC_RW_PORT);
  // set the index again (a read will reset the index to register D)
  outb(RTC_REG_B, RTC_REG_PORT);
  // write the previous value ORed with 0x40 to turn on bit 6 of register B
  outb(prev | 0x40, RTC_RW_PORT);
}

/* rtc_open
*
* Description: Open RTC
* Inputs: None
* Outputs: None
* Return Value: 0 if successful
* Side Effects: Opens RTC and set frequency to default rate
*/
int32_t rtc_open(const uint8_t* filename){
  // set RTC frequency to default frequency
  rtc_rates[cur_terminal] = (int32_t) RTC_DEFAULT_RATE;
  return 0;
}

/* rtc_close
*
* Description: Close RTC
* Inputs: None
* Outputs: None
* Return Value: 0 if successful
*/
int32_t rtc_close(int32_t fd){
  return 0;
}

/* rtc_read
*
* Description: Read from RTC, set a flag and return 0
*							 after the interrupt handler clears the flag
* Inputs: None
* Outputs: None
* Return Value: 0 if successful
*/
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
	// set flag to be 1
  rtc_interrupt_flags[cur_terminal] = 1;
	sti();
	// read until interrupt handler clears the flag
	while (rtc_interrupt_flags[cur_terminal]){}
	// in case the flag is changed
	rtc_interrupt_flags[cur_terminal] = 1;
  cli();
	return 0;
}

/* rtc_write
*
* Description: sets freuqncy of RTC with the helper function rtc_set_freq
* Inputs: buf -- 4-byte integer specifying the interrupt rate in Hz
*					nbytes -- number of bytes written (4 byte in this case)
* Outputs: None
* Return Value: nbytes if successful, -1 if failed
*/
int32_t rtc_write(int32_t fd, void* buf, int32_t nbytes){
  // boundary condition check
  if (nbytes != RTC_BOUNDARY || buf == NULL) return -1;
	// obtain frequyency from the buffer
  int32_t freq = *(int32_t*)buf;
	// write frequency
  rtc_rates[cur_terminal] = (int32_t) RTC_MAX_FREQ / freq;

  return nbytes;

}


/* rtc_set_freq
*
* Description: helper function that converts frequency to rate
* Inputs: freq -- freuqncy to set to RTC
* Outputs: None
* Return Value: 0 if successful, -1 if failed
* NOT USED FOR THE FINAL CHECK PT.
*/
int32_t rtc_set_freq(int32_t freq){
  // freq range [2,1024] and freq has to be a power of 2
  if (freq < RTC_MIN_FREQ || freq > RTC_MAX_FREQ || ((freq & (freq - 1)) != 0) ) return -1;
  // freq and rate conversion: freq = 32768 >> (rate-1)
  // rate has to be between 2 and 15
  // counter used to keep track which bit of rate is 1
  int counter = 0;
  // calculate 2^(rate-1)
  int quotient = LOG_RATE_LIMIT / freq;
  while ((quotient >>= 1 & 1) != 0) {counter ++;}
  int rate = (counter + 1) & 0x0F;

	// set index to register A, disable NMI
	outb(RTC_REG_A, RTC_REG_PORT);
	// get initial value of register A
	char prev = inb(RTC_RW_PORT);
	// reset index to A
	outb(RTC_REG_A, RTC_REG_PORT);
	// write only our rate to A.
	// rate is the bottom 4 bits.
	outb((prev & 0xF0) | rate, RTC_RW_PORT);
  return 0;
}
