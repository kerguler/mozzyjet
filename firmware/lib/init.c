#include <stdint.h>
#include "msp430g2553.h"

#define DELAY_TIME 125

volatile uint16_t WAITING = 0;

void setup_timers(unsigned char fast) {
  WAITING = 0;
  //
  P1DIR |= 0xFF;
  P1OUT &= 0x00;
  P2DIR |= 0xFF;
  P2OUT &= 0x00;
  P3DIR |= 0xFF;
  P3OUT &= 0x00;
  //
  WDTCTL  = WDTPW + WDTHOLD;
  if (fast) {
      BCSCTL1 = CALBC1_16MHZ;
      DCOCTL  = CALDCO_16MHZ;
  } else {
      BCSCTL1 = CALBC1_1MHZ;
      DCOCTL  = CALDCO_1MHZ;
  }
  //
  TA0CTL = TACLR;
  TA1CTL = TACLR;
  //
  __delay_cycles(1000);
  //
  __bis_SR_register(GIE);
}
/*
void delay_ms(uint16_t ms) {
    for (;ms;ms--)
        __delay_cycles(1000);
}
*/

void delay_ms(uint16_t ms) {
    WAITING = 0;
    TA0CCTL0 = CCIE;                   // CCR0 interrupt enabled
    TA0CTL = TASSEL_2 | MC_1 | ID_3;   // SMCLK/8 upto CCR0
    for (;ms;ms--) {
        TA0CCR0 = DELAY_TIME;          // max = 2^16 = 65536
                                       // SMCLK/8/TA0CRR0
                                       // 1 MHz / 8 / 125 = 1 mHz
        __bis_SR_register(CPUOFF);
    }
    TA0CCTL0 &= ~CCIE;
    TA0CTL = TACLR;
}

void delay_ms_nohold(uint16_t ms) {
    WAITING = ms;
    TA0CCTL0 = CCIE;                   // CCR0 interrupt enabled
    TA0CTL = TASSEL_2 | MC_1 | ID_3;   // SMCLK/8 upto CCR0
    TA0CCR0 = DELAY_TIME;              // max = 2^16 = 65536
                                       // SMCLK/8/TA0CRR0
                                       // 1 MHz / 8 / 125 = 1 mHz
}

char isWaiting() {
    return (char)(WAITING);
}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    if (WAITING) {
        TA0CCR0 = DELAY_TIME;
        if (WAITING-- == 0) {
            TA0CCTL0 &= ~CCIE;
            TA0CTL = TACLR;
        }
    }
    __bic_SR_register_on_exit(CPUOFF);
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
  if (UC0IFG & UCA0TXIFG) {
      UC0IFG &= ~UCA0TXIFG;
  }
  if (UC0IFG & UCB0TXIFG) {
      UC0IFG &= ~UCB0TXIFG;
  }
  __bic_SR_register_on_exit(CPUOFF);
}

#define BUFFER_MAX 25
unsigned char buffer[BUFFER_MAX];
unsigned char bufi = 0, bufj = 0;
unsigned char readBuffer = 0;

void UART_InitBuffer(void) {
    bufi = bufj = 0;
    readBuffer = 1;
    UC0IE |= UCA0RXIE; // Enable USCI_A0 RX interrupt
}

unsigned char UART_ReadBuffer(unsigned char *val) {
    if (bufi == bufj) return 0; // Received nothing!
    //
    *val = buffer[bufi];
    bufi = (bufi == BUFFER_MAX-1) ? 0 : bufi+1;
    //
    return 1;
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
  if (UC0IFG & UCA0RXIFG) {
      //
      if (readBuffer) {
          buffer[bufj] = UCA0RXBUF;
          bufj = (bufj == BUFFER_MAX-1) ? 0 : bufj+1;
          if (bufj == bufi)
              readBuffer = 0;
      }
      //
      UC0IFG &= ~UCA0RXIFG;
  }
  if (UC0IFG & UCB0RXIFG) {
      UC0IFG &= ~UCB0RXIFG;
  }
  __bic_SR_register_on_exit(CPUOFF);
}

