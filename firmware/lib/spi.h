#ifndef SPI_H_
#define SPI_H_

#define SSI_SCLK_PIN BIT5
#define SSI_SOMI_PIN BIT6
#define SSI_SIMO_PIN BIT7

void SSI_Init(unsigned char slv);
void SSI_WriteTo(unsigned char data);
void SSI_ReadFrom(unsigned char bytes, unsigned char *array);
void SSI_Transfer(const unsigned char tx_buf, unsigned char *rx_buf, unsigned char rx_len);

#endif
