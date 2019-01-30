#ifndef COUNTER_H_
#define COUNTER_H_

#include <stdint.h>

#define INITIAL_STATE    0
#define RISING_EDGE      1
#define FALLING_EDGE     2
#define TIMER_OVERFLOW   3
#define TIMER_ERROR_RISE 4
#define TIMER_ERROR_FALL 5
#define TIMER_ERROR_COV  6
#define TIMER_ERROR_FLOW 7
#define TIMER_FINISH     8
#define TIMER_ON         9

#define COUNTER_PIN   BIT0

void COUNTER_Setup(void);
int COUNTER_getValue(void);

#endif
