#include <stdint.h>
#include "msp430g2553.h"
#include "pwm.h"

#define max(x,y) ((x)>(y)?(x):(y))
#define min(x,y) ((x)<(y)?(x):(y))

void PWM_Send_Init(uint16_t lambda) {
  P2DIR |= PWM_SEND_PIN;
  P2SEL |= PWM_SEND_PIN; // Select p2.1 for PWM
  //
  TA1CCTL1 = OUTMOD_7;
  TA1CTL   = TASSEL_2 | MC_1 | ID_0; // 1000000 / 1 / lambda Hz (per second)
  //
  TA1CCR0 = lambda;
  TA1CCR1 = 0; // Off
}

void PWM_Send(float duty) {
  TA1CCR1 = (unsigned int)(TA1CCR0 * min(1, max(0, duty)));
}

void PWM_Send_ABS(uint16_t value) {
  TA1CCR1 = value;
}

// --------------------------------------------------------------------------

unsigned char timer_state = INITIAL_STATE;
volatile uint16_t timer_value = 0;

#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer1_A0_Interrupt(void)
{
  timer_value = TA1CCR0;
  if (TA1CCTL0 & COV) { // Second capture was taken before processing the first
    timer_state = TIMER_ERROR_COV;
    TA1CCTL0 &= ~COV;
  } else if (TA1CCTL0 & CCI) { // Raising edge (indicates signal boundaries)
    timer_state = timer_state == RISING_EDGE ? TIMER_ERROR_RISE : RISING_EDGE;
  } else { // Falling edge (indicates duty cycle)
    timer_state = timer_state == FALLING_EDGE ? TIMER_ERROR_FALL : FALLING_EDGE;
  }
  if (TA1CCTL0 & CCIFG)
    TA1CCTL0 &= ~CCIFG;
  __bic_SR_register_on_exit(CPUOFF);
}

#pragma vector = TIMER1_A1_VECTOR
__interrupt void Timer1_A1_Interrupt(void)
{
  if (TA1CTL & TAIFG) {
    timer_value = 0xFFFF;
    timer_state = TIMER_OVERFLOW;
    TA1CTL &= ~TAIFG; // Clear interrupt flag - handled
  }
  if (TA1CCTL0 & CCIFG)
    TA1CCTL0 &= ~CCIFG;
  __bic_SR_register_on_exit(CPUOFF);
}

void PWM_Recv_Init(unsigned char pin) {
  P2DIR |= PWM_SEND_PIN;
  P2SEL &= ~PWM_SEND_PIN; // Unselect p2.1 for PWM
  P2OUT &= ~PWM_SEND_PIN;
  //
  // Timer 1: PWM signal detection
  switch (pin) {
    default:
    case 0:
      TA1CCTL0 = CCIE | CM_3 | CCIS_0 | CAP;    // Capture mode, both edges, input 0 (CCI0A, P2.0)
      TA1CTL   = TASSEL_2 | MC_2 | ID_0 | TAIE; // SMCLK continuous up, interrupt enabled
      //
      P2DIR  &= ~PWM_RECV_PIN0;
      P2SEL  |=  PWM_RECV_PIN0; // Select pin 2.0 as input interrupt
      P2SEL2 &= ~PWM_RECV_PIN0; // Select pin 2.0 as input interrupt
      break;
    case 1:
      TA1CCTL0 = CCIE | CM_3 | CCIS_1 | CAP;    // Capture mode, both edges, input 1 (CCI0B, P2.3)
      TA1CTL   = TASSEL_2 | MC_2 | ID_0 | TAIE; // SMCLK continuous up, interrupt enabled
      //
      P2DIR  &= ~PWM_RECV_PIN3;
      P2SEL  |=  PWM_RECV_PIN3; // Select pin 2.3 as input interrupt
      P2SEL2 &= ~PWM_RECV_PIN3; // Select pin 2.3 as input interrupt
      break;
  }
}

void PWM_Recv_Stop() {
  TA1CTL = TACLR; // Stop capture
  TA1CCTL0 = 0;
}

void PWM_Recv(uint16_t *lambda, uint16_t *value, unsigned char pin) {
  unsigned char i;
  // 1. Initialize the timer and input interrupt
  // 2. Check if the timer is on
  // 3. Process interrupt
  uint16_t hold_value = 0; // 16 bit
  uint32_t time_counter = 0; // 32 bit
  uint32_t neglambdas[MAX_SAMPLE];
  uint32_t values[MAX_SAMPLE];
  uint32_t tmp1, tmp2;
  unsigned char first = 1;
  unsigned char samples = 0;
  //
  timer_state = INITIAL_STATE;
  unsigned char timer_state0 = timer_state;
  //
  timer_value = 0;
  //
  for (i=0; i<MAX_SAMPLE; i++) {
    values[i] = 0;
    neglambdas[i] = 0;
  }
  PWM_Recv_Init(pin);
  hold_value = 0;
  while (1) {
    __bis_SR_register(CPUOFF);
    time_counter += timer_value - hold_value;
    hold_value = timer_value;
    if (time_counter > 0x8000) // Let max(lambda) = 32768 us
      timer_state = TIMER_ERROR_FLOW;
    //
    switch (timer_state) {
      case TIMER_ERROR_RISE:
      case TIMER_ERROR_FALL:
      case TIMER_ERROR_COV:
      case TIMER_ERROR_FLOW:
        PWM_Recv_Stop();
        *value = 0;
        *lambda = 0;
        return;
        break;
      case TIMER_OVERFLOW:
        timer_state = timer_state0;
        hold_value = 0;
        continue;
        break;
      case RISING_EDGE:
        if (first) { // Offset to the first rising edge
          neglambdas[samples] = 0;
          values[samples] = 0;
          first = 0;
        } else { // Sampling performed
          neglambdas[samples] += time_counter;
          if (++samples >= MAX_SAMPLE) { // 8 samples obtained
            PWM_Recv_Stop();
            for (tmp1 = 0, tmp2 = 0; samples; samples--) {
              tmp1 += values[samples - 1];
              tmp2 += values[samples - 1] + neglambdas[samples - 1];
            }
            tmp1 >>= AVERAGE; // Calculate the average (/ 2^2)
            tmp2 >>= AVERAGE;
            if (tmp1 >= 0xFFFF || tmp2 >= 0xFFFF || tmp1 > tmp2) {
              *value  = 0;
              *lambda = 0;
            } else {
              *value  = (uint16_t)(tmp1);
              *lambda = (uint16_t)(tmp2);
            }
            //
            return;
          }
          first = 1;
        }
        break;
      case FALLING_EDGE:
        values[samples] += time_counter;
        break;
    }
    if (timer_state != TIMER_OVERFLOW)
      timer_state0 = timer_state;
    time_counter = 0;
  }
}
