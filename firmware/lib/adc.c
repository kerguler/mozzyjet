// http://coder-tronics.com/msp430-adc-tutorial/
#include <stdlib.h>
#include <stdint.h>
#include "msp430g2553.h"

uint16_t adc10[10] = {0};

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
  __bic_SR_register_on_exit(CPUOFF);
}
 
void ADC10_Init(unsigned char pin) {
  P1DIR &= ~pin;
  ADC10CTL1 = CONSEQ_2;                             // Repeat single channel
  switch (pin) {
  case BIT0: ADC10CTL1 |= INCH_0; break;
  case BIT1: ADC10CTL1 |= INCH_1; break;
  case BIT2: ADC10CTL1 |= INCH_2; break;
  case BIT3: ADC10CTL1 |= INCH_3; break;
  case BIT4: ADC10CTL1 |= INCH_4; break;
  case BIT5: ADC10CTL1 |= INCH_5; break;
  case BIT6: ADC10CTL1 |= INCH_6; break;
  case BIT7: ADC10CTL1 |= INCH_7; break;
  }
  ADC10CTL0 = ADC10SHT_2 + MSC + ADC10ON + ADC10IE; // Sample & Hold Time + ADC10 ON + Interrupt Enable
  ADC10DTC1 = 0x0A;                                 // 10 conversions
  ADC10AE0 |= pin;                                  // ADC option select
}

void ADC10_Clear(unsigned char pin) {
    ADC10CTL1 = 0;
    ADC10CTL0 = 0;
    ADC10DTC1 = 0;
    ADC10AE0 &= ~pin;                               // ADC option clear
}

uint16_t ADC10_Sample(unsigned char pin) {
  unsigned char i;
  for (i=0;i<10;i++)
      adc10[i] = 0;
  ADC10_Init(pin);
  //
  ADC10CTL0 &= ~ENC;               // Disable Conversion
  while (ADC10CTL1 & BUSY);        // Wait if ADC10 busy
  ADC10SA = (uint16_t)adc10;       // Transfers data to next array (DTC auto increments address)
  ADC10CTL0 |= ENC + ADC10SC;      // Enable Conversion and conversion start
  //
  __bis_SR_register(CPUOFF);       // Low Power Mode 0, ADC10_ISR
  //
  ADC10_Clear(pin);
  for (i=0;i<10;i++)
      adc10[0] += adc10[i];
  return adc10[0] / 10.0;
}
