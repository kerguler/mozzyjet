#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <msp430.h>
#include "main.h"
#include "init.h"
#include "spi.h"
#include "mj_uart.h"
#include "MS5803.h"
#include "adc.h"
#include "pin.h"
#include "servo.h"
#include "pwm.h"
#include "mystring.h"
#include "flash.h"

extern volatile uint16_t WAITING;

datakey key;
unsigned char refresh = 0;
unsigned char initrand = 0;

unsigned char comm_state = COMM_COMMAND;

void state_change(unsigned char news) {
  key.state = news;
  refresh = 1;
}

unsigned char check_pwm(uint16_t sw, uint16_t sv) {
  if (sw > (key.lam - 1000) && sw < (key.lam + 1000)) {
    if (sv > key.pwm1 && sv < 2100)
      return TASK_OPEN;
    else if (sv > key.pwm0 && sv < 2100)
      return TASK_CLOSE;
    return TASK_SPRAY;
  }
  return TASK_NONE;
}

int main(void) {
  unsigned char task;
  double alpha = ALPHA_5;
  uint16_t servoms = 500;

  setup_timers(0);

  MS58_PIN_OUT;
  MS58_PIN_DIR;

  SSI_Init(0);
  MS58_Init(0);
  MJ_UART_Init(&key);
  //
  if (CHECK_POSITION)
    SERVO_Close_Check();
  else
    SERVO_Close();
  //
  P1OUT |= BIT3;
  P1DIR |= BIT3;
  //
  while (1) {
    P1OUT |= BIT3;
    //
    MS58_getTempPres_raw(&(key.tt), &(key.pp));
    key.sp = ADC10_Sample(POSIT_PIN);
    PWM_Recv(&(key.sw0), &(key.sv0), 0);
    PWM_Recv(&(key.sw1), &(key.sv1), 1);
    //
    switch (key.state) {
      case STATE_RUN_5: alpha = ALPHA_5; break;
      case STATE_RUN_10: alpha = ALPHA_10; break;
      case STATE_RUN_20: alpha = ALPHA_20; break;
    }
    //
    task = check_pwm(key.sw0, key.sv0);
    switch (task) {
      default:
      case TASK_NONE:
        break;
      case TASK_CLOSE:
        if (CHECK_POSITION)
          SERVO_Close_Check();
        else
          SERVO_Close();
        break;
      case TASK_OPEN:
        if (CHECK_POSITION)
          SERVO_Open_Check();
        else
          SERVO_Open();
        break;
      case TASK_SPRAY:
        {
          static double pres0 = 0;
          pres0 = (double)(MS58_getPressure_raw());
          pres0 = sqrt(pres0);
          pres0 = sqrt(pres0);
          pres0 = pres0 * pres0 * pres0;
          // For cweight<10:
          // servoms = 86.0 + 35735.0 * cweight * pres0^(-3.0/4.0);
          // For cweight>=10:
          // servoms = 62162.0 * cweight * pres0^(-3.0/4.0);
          servoms = (int16_t)(alpha / pres0);
          if (key.state == STATE_RUN_5)
            servoms += 86;
          //
          unsigned char wfull = SERVO_Open_ms(servoms);
          break;
        }
    }
    //
    P1OUT &= ~BIT3;
    //
    MJ_UART_Transfer_Key(&key);
    //
    while (MJ_UART_Read_Command(&key)) {
      switch (comm_state) {
        case COMM_COMMAND:
          switch (key.comm) {
            case 'O':
              if (CHECK_POSITION) SERVO_Open_Check(); else SERVO_Open();
              break;
            case 'C':
              if (CHECK_POSITION) SERVO_Close_Check(); else  SERVO_Close();
              break;
            case 'T':
              comm_state = COMM_TIME_L;
              break;
            case 'V':
              comm_state = COMM_VOL0_L;
              break;
            case 'S':
              comm_state = COMM_SPV_L;
              break;
            case 'L':
              comm_state = COMM_LAM_L;
              break;
            case '<':
              comm_state = COMM_PWM0_L;
              break;
            case '>':
              comm_state = COMM_PWM1_L;
              break;
          }
          break;
        case COMM_TIME_L:
          key.tm = key.comm;
          comm_state = COMM_TIME_R;
          break;
        case COMM_TIME_R:
          key.tm = (key.tm << 8) | key.comm;
          //
          key.p1 = MS58_getPressure_raw();
          //
          SERVO_Open_ms(key.tm);
          //
          key.p2 = MS58_getPressure_raw();
          //
          comm_state = COMM_COMMAND;
          break;
        case COMM_VOL0_L:
          key.v0 = key.comm;
          comm_state = COMM_VOL0_R;
          break;
        case COMM_VOL0_R:
          key.v0 = (key.v0 << 8) | key.comm;
          MJ_UART_Write_Key(&key);
          //
          comm_state = COMM_COMMAND;
          break;
        case COMM_SPV_L:
          key.spv = key.comm;
          comm_state = COMM_SPV_R;
          break;
        case COMM_SPV_R:
          key.spv = (key.spv << 8) | key.comm;
          MJ_UART_Write_Key(&key);
          //
          comm_state = COMM_COMMAND;
          break;
        case COMM_LAM_L:
          key.lam = key.comm;
          comm_state = COMM_LAM_R;
          break;
        case COMM_LAM_R:
          key.lam = (key.lam << 8) | key.comm;
          MJ_UART_Write_Key(&key);
          //
          comm_state = COMM_COMMAND;
          break;
        case COMM_PWM0_L:
          key.pwm0 = key.comm;
          comm_state = COMM_PWM0_R;
          break;
        case COMM_PWM0_R:
          key.pwm0 = (key.pwm0 << 8) | key.comm;
          MJ_UART_Write_Key(&key);
          //
          comm_state = COMM_COMMAND;
          break;
        case COMM_PWM1_L:
          key.pwm1 = key.comm;
          comm_state = COMM_PWM1_R;
          break;
        case COMM_PWM1_R:
          key.pwm1 = (key.pwm1 << 8) | key.comm;
          MJ_UART_Write_Key(&key);
          //
          comm_state = COMM_COMMAND;
          break;
        default:
          break;
      }
    }
  }
}
