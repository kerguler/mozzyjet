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

void servo_open_pres() {
  key.p1 = MS58_getPressure_raw();
  //
  SERVO_Open_ms(key.tm);
  delay_ms(1000);
  //
  key.p2 = MS58_getPressure_raw();
}

void calculate_spray_value() {
  if (key.sv1 < 1100 || key.sv1 > 1900) return;
  //
  // Set spray volume temporarily
  key.spv = (key.sv1 - 1100) >> 5;
}

unsigned char spray_and_check() {
  static uint16_t tmp = 0;
  static double pres0 = 0;
  //
  key.p1 = MS58_getPressure_raw();
  // Calculate pres^(3/4)
  pres0 = (double)(key.p1);
  pres0 = sqrt(pres0);
  pres0 = sqrt(pres0);
  pres0 = pres0 * pres0 * pres0;
  //
  if (key.spv < ((double)(key.p1)/1e4))
    key.tm = (uint16_t)(92.62 + (64917.90 * (double)(key.spv) / pres0));
  else
    key.tm = (uint16_t)(-24.45 + (150010.63 * (double)(key.spv) / pres0));
  //
  SERVO_Open_ms(key.tm);
  delay_ms(1000);
  //
  key.p2 = MS58_getPressure_raw();
  //
  // Calculate volume reduction
  if (key.p2 < key.p1) {
    tmp = (uint16_t)((double)(CONTAINER_VOLMAX-key.v0)*((double)(key.p1)/(double)(key.p2)-1.0));
    key.v0 = (tmp > key.v0) ? 0 : key.v0 - tmp;
  }
  //
  return 1;
}

int main(void) {
  unsigned char task = TASK_CLOSE;
  static unsigned char task0;
  task0 = task;

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
    key.sp = CHECK_POSITION ? ADC10_Sample(POSIT_PIN) : 0;
    PWM_Recv(&(key.sw0), &(key.sv0), 0);
    PWM_Recv(&(key.sw1), &(key.sv1), 1);
    //
    task = check_pwm(key.sw1, key.sv1);
    if (task != TASK_NONE)
      calculate_spray_value();
    //
    task = check_pwm(key.sw0, key.sv0);
    switch (task) {
      default:
      case TASK_NONE:
        break;
      case TASK_CLOSE:
        if (task0 != TASK_CLOSE) {
          if (CHECK_POSITION)
            SERVO_Close_Check();
          else
            SERVO_Close();
        }
        break;
      case TASK_OPEN:
        if (task0 != TASK_OPEN) {
          if (CHECK_POSITION)
            SERVO_Open_Check();
          else
            SERVO_Open();
        }
        break;
      case TASK_SPRAY:
        spray_and_check();
        break;
    }
    task0 = task;
    //
    P1OUT &= ~BIT3;
    //
    MJ_UART_Transfer_Key(&key);
    //
    while (MJ_UART_Read_Command(&key)) {
      switch (comm_state) {
        case COMM_COMMAND:
          switch (key.comm) {
            case 'R':
              spray_and_check();
              break;
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
          servo_open_pres();
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
