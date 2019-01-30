#include <stdint.h>
#include "msp430g2553.h"
#include "MPL115A1.h"
#include "init.h"
#include "spi.h"

//======================//
// 	 MPL115A1 Defines	//
//======================//
#define PRESH              0x80
#define TEMPH              0x84

#define A0_H               0x88
#define B1_H               0x8C
#define B2_H               0x90
#define C12_H              0x94

#define conv_cmd           0x24

struct
{
  float A0_;
  float B1_;
  float B2_;
  float C12_;
} coefficients;

void MPL_Init(unsigned char pin) {
    if (pin) {
        MPL_PIN_OUT;
        MPL_PIN_DIR;

        SSI_Init();
    }

    MPL_unselect;
    MPL_reset;
    delay_ms(10);
    MPL_set;

    MPL_GetCoefficients();
}

void write(uint8_t address, uint8_t data) {
    uint8_t tmp;

    address &= 0x7F;

    MPL_select;
    delay_ms(3);
    SSI_Transfer(address,&tmp,1);
    delay_ms(1);
    SSI_Transfer(data,&tmp,1);
    delay_ms(1);
    MPL_unselect;
}

uint8_t read(uint8_t address) {
    uint8_t byte;

    MPL_select;
    delay_ms(10);
    SSI_Transfer(address,&byte,1);
    SSI_Transfer(0x00,&byte,1);
    MPL_unselect;

    return byte;
}

void get_bytes(uint16_t *upper, uint16_t *lower, uint8_t address)
{
  *upper = (uint16_t)read(address);
  *lower = (uint16_t)read((address + 2));
}

void MPL_GetCoefficients(void) {
  uint16_t hb = 0;
  uint16_t lb = 0;

  get_bytes(&hb, &lb, A0_H);
  coefficients.A0_ = ((hb << 5) + (lb >> 3) + ((lb & 7) / 8.0));

  get_bytes(&hb, &lb, B1_H);
  coefficients.B1_ = (((((hb & 0x1F) * 0x0100) + lb) / 8192.0) - 3.0);

  get_bytes(&hb, &lb, B2_H);
  coefficients.B2_ = (((((hb - 0x80) << 8) + lb) / 16384.0) - 2.0);

  get_bytes(&hb, &lb, C12_H);
  coefficients.C12_ = (((hb * 0x100) + lb) / 16777216.0);
}

void get_adc(uint16_t *Padc, uint16_t *Tadc) {
    uint16_t hb = 0;
    uint16_t lb = 0;

    write(conv_cmd, 0x00);

    get_bytes(&hb, &lb, PRESH);
    *Padc = (((hb << 8) + lb) >> 6);

    get_bytes(&hb, &lb, TEMPH);
    *Tadc = (((hb << 8) + lb) >> 6);
}

void get_8adc(uint16_t *Padc, uint16_t *Tadc) {
    uint16_t hb = 0;
    uint16_t lb = 0;
    unsigned char i = 0b10000000;
    *Padc = 0;
    *Tadc = 0;
    for (; i; i >>= 1) {
        write(conv_cmd, 0x00);
        get_bytes(&hb, &lb, PRESH);
        *Padc += (((hb << 8) + lb) >> 6);
        get_bytes(&hb, &lb, TEMPH);
        *Tadc += (((hb << 8) + lb) >> 6);
    }
    *Padc >>= 3;
    *Tadc >>= 3;
}

void MPL_GetPresComp(float *pres) {
    uint16_t Padc, Tadc;
    get_8adc(&Padc,&Tadc);

    *pres = ( coefficients.A0_ + (( coefficients.B1_ + ( coefficients.C12_ * Tadc)) * Padc) + ( coefficients.B2_ * Tadc));
}

void MPL_GetData(float *pres, float *temp) {
    uint16_t Padc, Tadc;
    get_8adc(&Padc,&Tadc);

    *pres = ( coefficients.A0_ + (( coefficients.B1_ + ( coefficients.C12_ * Tadc)) * Padc) + ( coefficients.B2_ * Tadc));
    *pres = (((*pres * 65.0) / 1023.0) + 50.0);

    *temp = (30.0 + ((Tadc - 472.0) / (-5.35)));
}
