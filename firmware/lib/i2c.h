#ifndef I2C_H_
#define I2C_H_

#define I2C_SCLK_PIN BIT6
#define I2C_SDA_PIN  BIT7

void I2C_Init(char adress);
unsigned char I2C_Transfer(const void *tx_buf, unsigned char tx_len, void *rx_buf, unsigned char rx_len);

#endif
