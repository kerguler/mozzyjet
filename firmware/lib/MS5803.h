#ifndef MS5803_H_
#define MS5803_H_

#include <stdint.h>

#define MS58_SCE_PIN  BIT4

#define MS58_select   P1OUT &= ~MS58_SCE_PIN
#define MS58_unselect P1OUT |= MS58_SCE_PIN

#define MS58_PIN_OUT  P1OUT |= MS58_SCE_PIN
#define MS58_PIN_DIR  P1DIR |= MS58_SCE_PIN

#define CMD_RESET    0x1E // reset command
#define CMD_ADC_READ 0x00 // ADC read command
#define CMD_ADC_CONV 0x40 // ADC conversion command
#define CMD_PROM     0xA0 // Coefficient location

#define PRESSURE     0x00
#define TEMPERATURE  0x10

#define CELSIUS      0
#define FAHRENHEIT   1

#define ADC_256      0x00
#define ADC_512      0x02
#define ADC_1024     0x04
#define ADC_2048     0x06
#define ADC_4096     0x08

unsigned char MS58_Init(unsigned char pin);
int32_t MS58_getTemperature(void);
int32_t MS58_getTemperature_raw(void);
int32_t MS58_getPressure(void);
int32_t MS58_getPressure_raw(void);
void MS58_getTempPres_raw(int32_t *temp, int32_t *pres);

#endif
