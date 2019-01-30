#ifndef INIT_H_
#define INIT_H_

#include <stdint.h>

#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((x)>(y)?(x):(y))

char isWaiting(void);
void delay_ms_nohold(uint16_t ms);
void delay_ms(uint16_t ms);
void setup_timers(unsigned char fast);
void UART_InitBuffer(void);
unsigned char UART_ReadBuffer(unsigned char *val);

__interrupt void USCI0TX_ISR(void);
__interrupt void USCI0RX_ISR(void);

#endif
