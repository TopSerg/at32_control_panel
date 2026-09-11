#ifndef TIMEBASE_H
#define TIMEBASE_H
#include <stdint.h>
void timebase_init(void);
void delay_ms(uint32_t ms);
uint32_t timebase_millis(void);
#endif
