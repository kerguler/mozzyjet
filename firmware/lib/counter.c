#include <stdint.h>
#include "msp430g2553.h"
#include "counter.h"
#include "init.h"

int degree = 1000;

#define STATE_PIN0 BIT3
#define STATE_PIN1 BIT4

void COUNTER_Setup() {
    P1OUT |= STATE_PIN0 | STATE_PIN1;
    P1REN |= STATE_PIN0 | STATE_PIN1;
    P1DIR &= ~STATE_PIN0;
    P1DIR &= ~STATE_PIN1;

    P1IE  |= STATE_PIN0;
    P1IES |= STATE_PIN0;
    P1IFG &= ~STATE_PIN0;
}

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
    if (!(P1IFG & STATE_PIN0)) return;
    if (P1IN & STATE_PIN1) { // CCW
        P1OUT |= BIT0;
        P1OUT &= ~BIT6;
        degree++;
    } else { // CW
        P1OUT |= BIT6;
        P1OUT &= ~BIT0;
        degree--;
    }
    P1IFG &= ~STATE_PIN0;
}

int COUNTER_getValue() {
    return degree;
}

