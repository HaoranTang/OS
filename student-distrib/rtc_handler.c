

#include "rtc_handler.h"


volatile int lock = 0;
/*
 *	Function: rtc_init
 *	Description: initialize the RTC
 *	input: None
 *	output: returns 0
 *	side-effect: initialize the RTC, and set the frequency to 2
 */
void rtc_init() {

  cli();
  outb(RTC_B, RTC_PORT);    /*disable NMI*/
  unsigned char prev = inb(CMOS_PORT);   /*store the current value in B reg*/
  outb(RTC_B, RTC_PORT);
  outb((prev | BIT_MASK), CMOS_PORT);   /*turn on the PIE, by bitmask*/

  unsigned char rate = 0x0F;
  rate &= 0x0F;              /*rate must be above 2 and not over 15*/
  outb(RTC_A, RTC_PORT);
  prev = inb(CMOS_PORT);
  outb(RTC_A, RTC_PORT);    /* reset index to Reg A*/
  outb((prev & 0xF0) | rate, CMOS_PORT);
  enable_irq(RTC_IRQ);   /* enable PIC to accept interrupts*/
  sti();
  outb(0x0C, RTC_PORT);
  inb(CMOS_PORT);
}
/*
 *	Function: rtc_handler
 *	Description: set lock to 1 and handle the interrupt
 *	input: None
 *	output: return 0
 *	side-effect: set the lock to 1 and display things on the screen
 */
void rtc_handler() {
  cli();   /*critical area*/
  lock = 1;
     /*test interrupts*/

  outb(0x0C, RTC_PORT);   /*select register C*/
  inb(CMOS_PORT);          /*throw contents*/
  send_eoi(RTC_IRQ);
  sti();
}

/*
 *	Function: rtc_closer()
 *	Description: function to close the RTC.
 *	input: None
 *	output: returns 0
 *	side-effect: open the RTC
 */
int32_t rtc_opener(const uint8_t* filename) {
  // set the frequency to 2HZ
  cli();
  // rtc_init();
  unsigned char prev;
  unsigned char rate = 0x0F;
  rate &= 0x0F;
  outb(RTC_A, RTC_PORT);
  prev = inb(CMOS_PORT);
  outb(RTC_A, RTC_PORT);    /* reset index to Reg A*/
  outb((prev & 0xF0) | rate, CMOS_PORT);
  sti();
  return 0;
}
/*
 *	Function: rtc_opener()
 *	Description: function to close the RTC.
 *	input: None
 *	output: returns 0
 *	side-effects: closes the RTC
 */
int32_t rtc_closer(int32_t fd) {
  // set the frequency to 2HZ

  return 0;
}
/*
 *	Function: rtc_read
 *	Description: RTC read should only happen when clock is available(the handler clear it), and always return 0
 *	input: file descriptor(fd), buf to store the frequency of rtc(buf), number of bytes(nbytes)
 *	output: returns 0
 *	side-effect: closes the RTC
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes) {
  while (lock == 0);
  lock = 0;
  return 0;
}
/* rtc_stop_interrupt
 * Description: stop the rtc interupt
 * Input: None
 * Output: None
 * Side-effects: stop the rtc interrupt by calling diable irq
*/
void rtc_stop_interrupt() {
  disable_irq(RTC_IRQ);
}
/*
 *	Function: rtc_write()
 *	Description: determine what value for rate according to the frequency
 *	input: file descriptor(fd), buff that store the frequency of the RTC(buf), number of bytes(nbytes)
 *	output: returns 0 if success, return -1 if fail
 *	side-effect: set the corresponding frequency
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes) {
  int32_t frequency;
  unsigned char prev;
  unsigned char rate;
  // char* buffer = (char*)buf;
  if (buf == NULL || nbytes != NUM_BYTE) {
    return -1;
  }
  frequency = *(int32_t*)buf;
  if (frequency == RTC_FREQUENCY_2) {
    rate = RTC_RATE_2;
  } else if (frequency == RTC_FREQUENCY_4) {
    rate = RTC_RATE_4;
  } else if (frequency == RTC_FREQUENCY_8) {
    rate = RTC_RATE_8;
  } else if (frequency == RTC_FREQUENCY_16) {
    rate = RTC_RATE_16;
  } else if (frequency == RTC_FREQUENCY_32) {
    rate = RTC_RATE_32;
  } else if (frequency == RTC_FREQUENCY_64) {
    rate = RTC_RATE_64;
  } else if (frequency == RTC_FREQUENCY_128) {
    rate = RTC_RATE_128;
  } else if (frequency == RTC_FREQUENCY_256) {
    rate = RTC_RATE_256;
  } else if (frequency == RTC_FREQUENCY_512) {
    rate = RTC_RATE_512;
  } else if (frequency == RTC_FREQUENCY_1024) {
    rate = RTC_RATE_1024;
  } else {
    return -1;
  }
  rate &= 0x0F;
  outb(RTC_A, RTC_PORT);
  prev = inb(CMOS_PORT);
  outb(RTC_A, RTC_PORT);    /* reset index to Reg A*/
  outb((prev & 0xF0) | rate, CMOS_PORT);

  return 0;
  }
