#ifndef __RTC_H
#define __RTC_H

#include "types.h"


// Port 0x70 is used to specify an index or "register number", and to disable NMI
// Port 0x71 is used to read or write from/to that byte of CMOS configuration space
#define RTC_REG_PORT  0x70
#define RTC_RW_PORT   0x71
#define RTC_REG_C	    0x0C
// disable NMI
#define RTC_REG_B     0x8B
#define RTC_REG_A     0x8A
#define RTC_BOUNDARY  4



/* Initialize RTC */
void rtc_init();

/* Set RTC frequency */
int32_t rtc_set_freq(int32_t freq);

/* handler for RTC */
extern void rtc_handler();

/* Open RTC */
extern int32_t rtc_open(const uint8_t* filename);

/* Close RTC */
extern int32_t rtc_close(int32_t fd);

/* Read RTC */
extern int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

/* Write to RTC */
extern int32_t rtc_write(int32_t fd, void* buf, int32_t nbytes);


#endif
