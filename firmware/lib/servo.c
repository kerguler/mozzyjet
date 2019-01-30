#include <stdint.h>
#include <math.h>
#include "msp430g2553.h"
#include "servo.h"
#include "init.h"
#include "pwm.h"
#include "adc.h"
#include "uart.h"
#include "PCD8544.h"
#include "mystring.h"

extern volatile uint16_t WAITING;

void ulog(uint16_t pwm, uint16_t adc) {
    char str[25];
    ui2s(pwm,str);
    UART_OutString(str);
    UART_OutString(",");
    ui2s(adc,str);
    UART_OutString(str);
    UART_OutString("\n\r");
}

void SERVO_Init() {
    PWM_Send_Init(LAMBDA);
    //
    SERVO_Release;
}

void SERVO_Open() {
    PWM_Send_ABS(PWM_OPEN);
    delay_ms(1000);
    //
    SERVO_Release;
}

void SERVO_Close() {
    PWM_Send_ABS(PWM_CLOSE);
    delay_ms(1000);
    //
    SERVO_Release;
}

void SERVO_Close_Check() {
    uint16_t adc = ADC10_Sample(POSIT_PIN);
    if (adc < ADC_CLOSE) {
        PWM_Send_ABS(PWM_CLOSE);
        delay_ms(1000);
    }
    //
    SERVO_Release;
}

void SERVO_Open_Check() {
    uint16_t adc = ADC10_Sample(POSIT_PIN);
    if (adc > ADC_OPEN) {
        PWM_Send_ABS(PWM_OPEN);
        delay_ms(1000);
    }
    //
    SERVO_Release;
}

unsigned char SERVO_Open_ms(uint16_t ms) {
    unsigned char retval = 1;
    uint16_t adc = 0;
    //
    if (CHECK_POSITION) {
        adc = ADC10_Sample(POSIT_PIN);
        if (adc < ADC_CLOSE) {
            PWM_Send_ABS(PWM_CLOSE);
            delay_ms(1000);
        }
    }
    //
    delay_ms_nohold(ms);
    while (WAITING)
        PWM_Send_ABS(PWM_OPEN);
    //
    PWM_Send_ABS(PWM_CLOSE);
    delay_ms(1000);
    //
    SERVO_Release;
    return retval;
}

void SERVO_Calibrate() {
    SERVO_Init();
    PWM_Send_ABS(PWM_OPEN);
    delay_ms(3000);
    SERVO_Release;
    PWM_Send_ABS(PWM_CLOSE);
    delay_ms(3000);
    SERVO_Release;
    PWM_Send_ABS(PWM_HALF);
    delay_ms(3000);
    SERVO_Release;
    SERVO_Open_ms(4000);
    SERVO_Open_ms(2000);
    SERVO_Open_ms(1000);
    SERVO_Open_ms(500);
    SERVO_Open_ms(250);
}

