#ifndef UART_H_
#define UART_H_

#define UART_RXD     BIT1
#define UART_TXD     BIT2

void UART_Init(unsigned char fast);
void UART_OutChar(unsigned char value);
unsigned char UART_InChar();
void UART_OutString(char *c);
void UART_OutStringN(char *c, unsigned char n);

#endif
