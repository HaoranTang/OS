#ifndef _RTC_HANDLER_H
#define _RTC_HANDLER_H

#include "lib.h"
#include "i8259.h"
/* magic numbers, data port and registers for RTC*/
#define RTC_PORT 0x70
#define CMOS_PORT 0x71
#define RTC_A 0x8A
#define RTC_B 0x8B
#define RTC_C 0x8C
#define RTC_D 0x8D
#define BIT_MASK 0x40
#define RTC_IRQ 8
#define RTC_FREQUENCY_2 2
#define RTC_FREQUENCY_4 4
#define RTC_FREQUENCY_8 8
#define RTC_FREQUENCY_16 16
#define RTC_FREQUENCY_32 32
#define RTC_FREQUENCY_64 64
#define RTC_FREQUENCY_128 128
#define RTC_FREQUENCY_256 256
#define RTC_FREQUENCY_512 512
#define RTC_FREQUENCY_1024 1024
#define RTC_RATE_2 15
#define RTC_RATE_4 14
#define RTC_RATE_8 13
#define RTC_RATE_16 12
#define RTC_RATE_32 11
#define RTC_RATE_64 10
#define RTC_RATE_128 9
#define RTC_RATE_256 8
#define RTC_RATE_512 7
#define RTC_RATE_1024 6
#define NUM_BYTE 4
/* initialize the rtc*/
void rtc_init();
void rtc_handler();
void rtc_stop_interrupt();
int32_t rtc_opener(const uint8_t* filename);
int32_t rtc_closer(int32_t fd);
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
void rtc_set_freq(int32_t freq);
/*rtc handler function*/
#endif
