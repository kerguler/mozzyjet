#include <stdint.h>
#include <math.h>
#include "msp430g2553.h"
#include "servo.h"
#include "init.h"
#include "pwm.h"
#include "adc.h"
#include "uart.h"
#include "mystring.h"

extern volatile uint16_t WAITING;

uint16_t PWM_SIGNAL = 0;

void SERVO_Init() {
    PWM_Send_Init(LAMBDA);
}

void SERVO_Open() {
    SERVO_Init();
    PWM_Send_ABS(PWM_SIGNAL=PWM_OPEN);
    delay_ms(1000);
}

void SERVO_Close() {
    SERVO_Init();
    PWM_Send_ABS(PWM_SIGNAL=PWM_CLOSE);
    delay_ms(1000);
}

void SERVO_Close_Check() {
    uint16_t adc = ADC10_Sample(POSIT_PIN);
    if (adc < ADC_CLOSE) {
        SERVO_Init();
        PWM_Send_ABS(PWM_SIGNAL=PWM_CLOSE);
        delay_ms(1000);
    }
}

void SERVO_Open_Check() {
    uint16_t adc = ADC10_Sample(POSIT_PIN);
    if (adc > ADC_OPEN) {
        SERVO_Init();
        PWM_Send_ABS(PWM_SIGNAL=PWM_OPEN);
        delay_ms(1000);
    }
}

unsigned char SERVO_Open_ms(uint16_t ms) {
    unsigned char retval = 1;
    uint16_t adc = 0;
    SERVO_Init();
    //
    if (CHECK_POSITION) {
        adc = ADC10_Sample(POSIT_PIN);
        if (adc < ADC_CLOSE) {
            PWM_Send_ABS(PWM_SIGNAL=PWM_CLOSE);
            delay_ms(1000);
        }
    }
    //
    delay_ms_nohold(ms);
    while (WAITING)
        PWM_Send_ABS(PWM_SIGNAL=PWM_OPEN);
    //
    PWM_Send_ABS(PWM_SIGNAL=PWM_CLOSE);
    delay_ms(1000);
    //
    return retval;
}
