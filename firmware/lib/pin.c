#include "msp430g2553.h"

void PIN_Init(unsigned char pin) {
    P1OUT &= ~pin;
    P1DIR &= ~pin;
    P1SEL &= ~pin;
    P1SEL2 &= ~pin;
}

unsigned char PIN_Read(unsigned char pin) {
    return P1IN & pin;
}
