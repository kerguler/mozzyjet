#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <msp430.h>
#include "init.h"
#include "spi.h"
#include "mj_uart.h"
#include "MS5803.h"
#include "adc.h"
#include "pin.h"
#include "servo.h"
#include "pwm.h"
#include "mystring.h"

#define STATE_PIN         BIT3

#define STATE_TPRES             0
#define STATE_RUN_5             1
#define STATE_RUN_10            2
#define STATE_RUN_20            3
#define STATE_CALIBFLOW         4
#define STATE_EMPTY             5
#define STATE_HOLD              6
#define STATE_CALIBFLOW_PROCEED 50

uint16_t STATE_SUM = 5;

#define TASK_NONE         0
#define TASK_SPRAY        1
#define TASK_OPEN         2

// 35735.0 * cweight:
#define ALPHA_5   178675.0
// 62162.0 * cweight:
#define ALPHA_10  621620.0
#define ALPHA_20  1243240.0

extern volatile uint16_t WAITING;

datakey key;
unsigned char refresh = 0;
unsigned char initrand = 0;

#define COMM_COMMAND 0
#define COMM_TIME_L  1
#define COMM_TIME_R  2
unsigned char comm_state = COMM_COMMAND;

void state_change(unsigned char news) {
  key.state = news;
  refresh = 1;
}

unsigned char check_pwm(uint16_t sw, uint16_t sv) {
  if (sw > 13000 && sw < 15000) {
    if (sv > 1750 && sv < 2100)
      return TASK_OPEN;
    else if (sv > 1250 && sv < 2100)
      return TASK_SPRAY;
  }
  return TASK_NONE;
}

int main(void) {
  unsigned char state0 = 0;
  unsigned char counter = 0;
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
  SERVO_Init();
  if (CHECK_POSITION)
    SERVO_Close_Check();
  else
    SERVO_Close();
  //
  P1OUT |= BIT3;
  P1DIR |= BIT3;
  //
  state_change(STATE_TPRES);
  // state_change(STATE_CALIBFLOW_PROCEED);
  while (1) {
    P1OUT |= BIT3;
    //
    switch (key.state) {
      case STATE_EMPTY:
        break;
      case STATE_RUN_5:
      case STATE_RUN_10:
      case STATE_RUN_20:
        if (refresh) {
          switch (key.state) {
            case STATE_RUN_5: alpha = ALPHA_5; break;
            case STATE_RUN_10: alpha = ALPHA_10; break;
            case STATE_RUN_20: alpha = ALPHA_20; break;
          }
          refresh = 0;
        }
        PWM_Recv(&(key.sw), &(key.sv));
        task = check_pwm(key.sw, key.sv);
        switch (task) {
          default:
          case TASK_NONE:
            SERVO_Init();
            if (CHECK_POSITION)
              SERVO_Close_Check();
            else
              SERVO_Close();
            break;
          case TASK_OPEN:
            SERVO_Init();
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
              SERVO_Init();
              unsigned char wfull = SERVO_Open_ms(servoms);
              delay_ms(1000);
              if (!wfull)
                state_change(STATE_EMPTY);
              break;
            }
        }
        break;
      case STATE_TPRES:
        MS58_getTempPres_raw(&(key.tt), &(key.pp));
        key.sp = ADC10_Sample(POSIT_PIN);
        PWM_Recv(&(key.sw), &(key.sv));
        break;
      case STATE_CALIBFLOW:
        for (counter = 39; counter && (key.state == STATE_CALIBFLOW); counter--) {
          delay_ms(100);
        }
        if (key.state == STATE_CALIBFLOW) {
          if (!initrand) {
            int32_t pres = MS58_getTemperature_raw();
            srand(pres);
            initrand = 1;
            //
            delay_ms(1000);
          }
          STATE_SUM = 1000;
          state0 = STATE_CALIBFLOW_PROCEED;
          state_change(STATE_CALIBFLOW_PROCEED);
        }
        break;
      //
      default:
      //
      case STATE_CALIBFLOW_PROCEED:
        if (state0 != key.state) {
          key.tm = 100 + (uint16_t)(rand() % 900);
          //
          SERVO_Init();
          key.p1 = MS58_getPressure_raw();
          //
          SERVO_Open_ms(key.tm);
          delay_ms(3000);
          //
          key.p2 = MS58_getPressure_raw();
          //
          state0 = key.state;
        }
        delay_ms(500);
        //
        break;
      case STATE_HOLD:
        delay_ms(500);
        break;
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
              SERVO_Init();
              if (CHECK_POSITION) SERVO_Open_Check(); else SERVO_Open();
              break;
            case 'C':
              SERVO_Init();
              if (CHECK_POSITION) SERVO_Close_Check(); else  SERVO_Close();
              break;
            case 'T':
              comm_state = COMM_TIME_L;
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
          SERVO_Init();
          key.p1 = MS58_getPressure_raw();
          //
          SERVO_Open_ms(key.tm);
          delay_ms(3000);
          //
          key.p2 = MS58_getPressure_raw();
          //
          comm_state = COMM_COMMAND;
          break;
        default:
          break;
      }
    }
  }
}
