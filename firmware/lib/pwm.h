#ifndef PWM_H_
#define PWM_H_

#include <stdint.h>

#define PWM_RECV_PIN   BIT0
#define PWM_SEND_PIN   BIT1

void PWM_Send_Init(uint16_t lambda);
void PWM_Send(float duty);
void PWM_Send_ABS(uint16_t value);
void PWM_Recv(uint16_t *lambda, uint16_t *value);

#endif
