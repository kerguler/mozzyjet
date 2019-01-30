#ifndef MPL115A1_H_
#define MPL115A1_H_

// P1.3 and P1.4
#define MPL_CSE_PIN   BIT3
#define MPL_SDN_PIN   BIT4

#define MPL_select    P1OUT &= ~MPL_CSE_PIN
#define MPL_unselect  P1OUT |= MPL_CSE_PIN
#define MPL_reset     P1OUT &= ~MPL_SDN_PIN
#define MPL_set       P1OUT |= MPL_SDN_PIN

#define MPL_PIN_OUT   P1OUT |= MPL_CSE_PIN | MPL_SDN_PIN
#define MPL_PIN_DIR   P1DIR |= MPL_CSE_PIN | MPL_SDN_PIN

void MPL_Init(unsigned char pin);
void MPL_GetCoefficients(void);
void MPL_GetPresComp(float *pres);
void MPL_GetData(float *pres, float *temp);

#endif
