// https://github.com/millerlp/MS5803_30/blob/master/MS5803_30.cpp
// https://github.com/millerlp/MS5803_14/blob/master/MS5803_14.cpp#L131-L132
// https://learn.sparkfun.com/tutorials/ms5803-14ba-pressure-sensor-hookup-guide?_ga=2.213203028.17909576.1517492050-580501244.1517492050
// https://www.sparkfun.com/products/12909
// MS5803-14BA_Breakout-master
#include <stdint.h>
#include "msp430g2553.h"
#include "MS5803.h"
#include "init.h"
#include "spi.h"

#define MAX_COEF      8
uint16_t coefficients[MAX_COEF];

unsigned char p_crc;
unsigned char n_crc;

int32_t _temperature_actual;
int32_t _pressure_actual;

//------------------------------------------------------------------
// Function to check the CRC value provided by the sensor against the
// calculated CRC value from the rest of the coefficients.
// Based on code from Measurement Specialties application note AN520
// http://www.meas-spec.com/downloads/C-Code_Example_for_MS56xx,_MS57xx_%28except_analog_sensor%29_and_MS58xx_Series_Pressure_Sensors.pdf
unsigned char MS_5803_CRC(unsigned int n_prom[]) {
    int cnt;                // simple counter
    unsigned int n_rem;     // crc reminder
    unsigned int crc_read;  // original value of the CRC
    unsigned char  n_bit;
    n_rem = 0x00;
    crc_read = n_prom[7];     // save read CRC
    n_prom[7] = (0xFF00 & (n_prom[7])); // CRC byte replaced with 0
    for (cnt = 0; cnt < 16; cnt++)
    { // choose LSB or MSB
        if (cnt%2 == 1) {
            n_rem ^= (unsigned short)((n_prom[cnt>>1]) & 0x00FF);
        }
        else {
            n_rem ^= (unsigned short)(n_prom[cnt>>1] >> 8);
        }
        for (n_bit = 8; n_bit > 0; n_bit--)
        {
            if (n_rem & (0x8000))
            {
                n_rem = (n_rem << 1) ^ 0x3000;
            }
            else {
                n_rem = (n_rem << 1);
            }
        }
    }
    n_rem = (0x000F & (n_rem >> 12));// // final 4-bit reminder is CRC code
    n_prom[7] = crc_read; // restore the crc_read to its original place
    // Return n_rem so it can be compared to the sensor's CRC value
    return (n_rem ^ 0x00);
}

void MS58_WriteTo(unsigned char data) {
    uint8_t byte;
    //
    MS58_select;
    //
    SSI_Transfer(data,&byte,1);
    //
    MS58_unselect;
}

void MS58_ReadFrom(unsigned char *b, unsigned char n) {
    MS58_select;
    //
    SSI_Transfer(0x00,b,n);
    //
    MS58_unselect;
}

void MS58_ReadWrite(unsigned char data, unsigned char *b, unsigned char n) {
    uint8_t tmp;
    //
    MS58_select;
    //
    SSI_Transfer(data,&tmp,1);
    SSI_Transfer(0x00,b,n);
    //
    MS58_unselect;
}

unsigned char MS58_getCoefficients(void) {
    uint8_t i, tmp[2];
    //
    for (i=0; i<MAX_COEF; i++) {
        MS58_ReadWrite(CMD_PROM | (i << 1),tmp,2);
        coefficients[i] = ((uint16_t)tmp[0] << 8) | tmp[1];
    }
    //
    // The last 4 bits of the 7th coefficient form a CRC error checking code.
    p_crc = coefficients[7] & 0x00FF;
    // Use a function to calculate the CRC value
    n_crc = MS_5803_CRC(coefficients);
    // If the CRC value doesn't match the sensor's CRC value, then the
    // connection can't be trusted. Check your wiring.
    return p_crc == n_crc;
}

uint32_t MS58_getADCconversion(unsigned char _measurement, unsigned char _precision) {
    uint32_t result;
    uint8_t bytes[3];

    MS58_WriteTo(CMD_ADC_CONV + _measurement + _precision);
    delay_ms(1);
    switch( _precision )
    {
        case ADC_256 : delay_ms(1); break;
        case ADC_512 : delay_ms(3); break;
        case ADC_1024: delay_ms(4); break;
        case ADC_2048: delay_ms(6); break;
        case ADC_4096: delay_ms(10); break;
    }

    MS58_ReadWrite(CMD_ADC_READ,bytes,3);

    result = ((uint32_t)bytes[0] << 16) | ((uint32_t)bytes[1] << 8) | (uint32_t)bytes[2];

    return result;
}

void MS58_getMeasurements(unsigned char _precision) {
    //Retrieve ADC result
    int32_t temperature_raw = MS58_getADCconversion(TEMPERATURE, _precision);
    int32_t pressure_raw = MS58_getADCconversion(PRESSURE, _precision);

    //Create Variables for calculations
    int32_t temp_calc;
    int32_t pressure_calc;

    int32_t dT;

    //Now that we have a raw temperature, let's compute our actual.
    dT = temperature_raw - ((int32_t)coefficients[5] << 8);
    temp_calc = (((int64_t)dT * coefficients[6]) >> 23) + 2000;

    // TODO TESTING  _temperature_actual = temp_calc;

    //Now we have our first order Temperature, let's calculate the second order.
    int64_t T2, OFF2, SENS2, OFF, SENS; //working variables

    if (temp_calc < 2000)
    // If temp_calc is below 20.0C
    {
        T2 = 3 * (((int64_t)dT * dT) >> 33);
        OFF2 = 3 * ((temp_calc - 2000) * (temp_calc - 2000)) / 2;
        SENS2 = 5 * ((temp_calc - 2000) * (temp_calc - 2000)) / 8;

        if(temp_calc < -1500)
        // If temp_calc is below -15.0C
        {
            OFF2 = OFF2 + 7 * ((temp_calc + 1500) * (temp_calc + 1500));
            SENS2 = SENS2 + 4 * ((temp_calc + 1500) * (temp_calc + 1500));
        }
    }
    else
    // If temp_calc is above 20.0C
    {
        T2 = 7 * ((uint64_t)dT * dT) >> 37;
        OFF2 = ((temp_calc - 2000) * (temp_calc - 2000)) / 16;
        SENS2 = 0;
    }

    // Now bring it all together to apply offsets

    OFF = ((int64_t)coefficients[2] << 16) + (((coefficients[4] * (int64_t)dT)) >> 7);
    SENS = ((int64_t)coefficients[1] << 15) + (((coefficients[3] * (int64_t)dT)) >> 8);

    temp_calc = temp_calc - T2;
    OFF = OFF - OFF2;
    SENS = SENS - SENS2;

    // Now lets calculate the pressure

    pressure_calc = (((SENS * pressure_raw) / 2097152 ) - OFF) / 32768;

    _temperature_actual = temp_calc;
    _pressure_actual = pressure_calc;
}

int32_t MS58_getTemperature(void) {
    MS58_getMeasurements(ADC_512);
    return _temperature_actual;// / 100.0f;
}

int32_t MS58_getPressure(void) {
    MS58_getMeasurements(ADC_512);
    return _pressure_actual;// / 10.0f;
}

int32_t MS58_getPressure_raw(void) {
    MS58_getMeasurements(ADC_512);
    return _pressure_actual;
}

int32_t MS58_getTemperature_raw(void) {
    MS58_getMeasurements(ADC_512);
    return _temperature_actual;
}

void MS58_getTempPres_raw(int32_t *temp, int32_t *pres) {
    MS58_getMeasurements(ADC_512);
    *temp = _temperature_actual;
    *pres = _pressure_actual;
}

unsigned char MS58_Init(unsigned char pin) {
    if (pin) {
        P1DIR  |= MS58_SCE_PIN; // SS
        //
        SSI_Init(0);
    }
  //
  MS58_WriteTo(CMD_RESET);
  // delay_ms(1000);
  delay_ms(100);
  //
  return MS58_getCoefficients();
}
