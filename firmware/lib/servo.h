#ifndef SERVO_H_
#define SERVO_H_

#define CHECK_WATER       0
#define CHECK_POSITION    0

#define POSIT_PIN         BIT0

// #define SERVO_SS_PIN      BIT2

#define LAMBDA (20000-1)

#define ADC_CLOSE 690
#define ADC_OPEN  470

void SERVO_Init(void);
void SERVO_Set_Open(uint16_t spO);
void SERVO_Set_Close(uint16_t spC);
void SERVO_Open(void);
void SERVO_Open_ms(uint16_t ms);
void SERVO_Open_Check(void);
void SERVO_Close(void);
void SERVO_Close_Check(void);

#endif
