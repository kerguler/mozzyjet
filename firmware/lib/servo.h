#ifndef SERVO_H_
#define SERVO_H_

#define CHECK_WATER       0
#define CHECK_POSITION    1

#define POSIT_PIN         BIT0

// #define SERVO_SS_PIN      BIT2

#define LAMBDA (20000-1)

#define SERVO_Release     PWM_Send_ABS(LAMBDA+1)

/*
 * Cap design: Glue
 */
#define PWM_OPEN  1300
#define PWM_HALF  1745
#define PWM_CLOSE 2190

#define ADC_CLOSE 690
#define ADC_OPEN  440

void SERVO_Calibrate(void);

void SERVO_Init(void);
void SERVO_Open(void);
unsigned char SERVO_Open_ms(uint16_t ms);
void SERVO_Open_Check(void);
void SERVO_Close(void);
void SERVO_Close_Check(void);

#endif
