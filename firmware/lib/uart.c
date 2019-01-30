#include <stdint.h>
#include "msp430g2553.h"
#include "init.h"
#include "uart.h"

#define USCI_INPUT_CLK      (16000000UL)  // in Hz
#define USCI_BAUD_RATE      (9600)

#define USCI_DIV_INT              (USCI_INPUT_CLK/USCI_BAUD_RATE)
#define USCI_BR0_VAL              (USCI_DIV_INT & 0x00FF)
#define USCI_BR1_VAL              ((USCI_DIV_INT >> 8) & 0xFF)

#define USCI_DIV_FRAC_NUMERATOR   (USCI_INPUT_CLK - (USCI_DIV_INT*USCI_BAUD_RATE))
#define USCI_DIV_FRAC_NUM_X_8     (USCI_DIV_FRAC_NUMERATOR*8)
#define USCI_DIV_FRAC_X_8         (USCI_DIV_FRAC_NUM_X_8/USCI_BAUD_RATE)

#if (((USCI_DIV_FRAC_NUM_X_8-(USCI_DIV_FRAC_X_8*USCI_BAUD_RATE))*10)/USCI_BAUD_RATE < 5)
#define USCI_BRS_VAL              (USCI_DIV_FRAC_X_8<< 1)
#else
#define USCI_BRS_VAL              ((USCI_DIV_FRAC_X_8+1)<< 1)
#endif

void UART_Init(unsigned char fast) {
  UCA0CTL1 |= UCSWRST; // **Reset USCI state machine**
  //
  UCA0CTL1 |= UCSSEL_2; // SMCLK
  if (fast) {
      UCA0BR0 = USCI_BR0_VAL;
      UCA0BR1 = USCI_BR1_VAL;
      UCA0MCTL = USCI_BRS_VAL;
  } else {
      UCA0BR0 = 0x68; // 1MHz 9600
      UCA0BR1 = 0x00; // 1MHz 9600
      UCA0MCTL = 0x02; // Modulation UCBRSx = 1
  }
  //
  P1SEL  |= UART_RXD | UART_TXD; // P1.1 = RXD, P1.2=TXD
  P1SEL2 |= UART_RXD | UART_TXD; // P1.1 = RXD, P1.2=TXD
  //
  UCA0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**
  //
  __delay_cycles(1000);
}

void UART_OutChar(unsigned char value) {
  UCA0TXBUF = value;
  UC0IE |= UCA0TXIE; // Enable USCI_A0 TX interrupt 
  __bis_SR_register(CPUOFF);
  UC0IE &= ~UCA0TXIE; // Disable USCI_A0 TX interrupt
}

unsigned char UART_InChar(void) {
  UC0IE |= UCA0RXIE; // Enable USCI_A0 RX interrupt
  __bis_SR_register(CPUOFF);
  UC0IE &= ~UCA0RXIE; // Disable USCI_A0 RX interrupt
  return UCA0RXBUF;
}

void UART_OutString(char *c) {
    for (;*c;c++)
        UART_OutChar(*c);
}

void UART_OutStringN(char *c, unsigned char n) {
    for (n++;n;n--)
        if (*c) {
            UART_OutChar(*c);
            c++;
        } else
            UART_OutChar(' ');
}

