#ifndef PWM_H_
#define PWM_H_

#include <stdint.h>

#define PWM_RECV_PIN3   BIT3
#define PWM_RECV_PIN0   BIT0
#define PWM_SEND_PIN    BIT1

#define INITIAL_STATE    0
#define RISING_EDGE      1
#define FALLING_EDGE     2
#define TIMER_OVERFLOW   3
#define TIMER_ERROR_RISE 4
#define TIMER_ERROR_FALL 5
#define TIMER_ERROR_COV  6
#define TIMER_ERROR_FLOW 7

#define MAX_SAMPLE 1
#define AVERAGE    0

void PWM_Send_Init(uint16_t lambda);
void PWM_Send(float duty);
void PWM_Send_ABS(uint16_t value);
void PWM_Recv(uint16_t *lambda, uint16_t *value, unsigned char pin);

#endif
