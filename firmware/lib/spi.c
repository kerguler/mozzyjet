// https://github.com/nathanpc/msp430-nokia-5110/
// http://rohitg.in/2014/11/09/Nokia5510-lcd-with-MSP430/
// https://github.com/cpb-/Article-RPi-MSP430/blob/master/msp430-spi-5.c
#include <stdint.h>
#include "msp430g2553.h"  
#include "spi.h"
#include "init.h"

unsigned char SLAVE = 0;

void SSI_WriteTo(unsigned char data) {
    //UC0IE |= UCB0TXIE; // Enable USCI_B0 TX interrupt
    UCB0TXBUF = data;
    __bis_SR_register(CPUOFF);
    //UC0IE &= ~UCB0TXIE; // Disable USCI_B0 TX interrupt
    while(UCB0STAT & BIT0); // Wait for the busy signal to clear
}

void SSI_ReadFrom(unsigned char bytes, unsigned char *array) {
    unsigned char i;
    //UC0IE |= UCB0RXIE; // Enable USCI_B0 RX interrupt
    for (i=0; i<bytes; i++) {
        //UCB0TXBUF = 0x09;
        __bis_SR_register(CPUOFF);
        array[i] = UCB0RXBUF;
    }
    //UC0IE &= ~UCB0RXIE; // Disable USCI_B0 RX interrupt
    while(UCB0STAT & BIT0); // Wait for the busy signal to clear
}

void SSI_Transfer(const unsigned char tx_buf, unsigned char *rx_buf, unsigned char rx_len) {
    unsigned char i;
    // UC0IE |= UCB0RXIE; // Enable USCI_B0 RX interrupts
    for (i=0; i<rx_len; i++) {
        if (SLAVE) {
            while(!(UC0IFG & UCB0RXIFG));
            rx_buf[i] = UCB0RXBUF;
            // __bis_SR_register(CPUOFF);
            // while(!(UC0IFG & UCB0TXIFG));
            UCB0TXBUF = i ? 0x00 : tx_buf;
            while(!(UC0IFG & UCB0TXIFG));
        } else {
            // while(!(UC0IFG & UCB0TXIFG));
            UCB0TXBUF = i ? 0x00 : tx_buf;
            while(!(UC0IFG & UCB0TXIFG));
            // __bis_SR_register(CPUOFF);
            while(!(UC0IFG & UCB0RXIFG));
            rx_buf[i] = UCB0RXBUF;
        }
    }
    // UC0IE &= ~UCB0RXIE; // Enable USCI_B0 RX interrupts
    // while(UCB0STAT & UCBUSY); // Wait for the busy signal to clear
}

void SSI_Init(unsigned char slv) {
    SLAVE = slv;
    //
    UCB0CTL1 |= UCSWRST; // set SW

    if (SLAVE) {
        UCB0CTL0 &= ~UCMST;
        UCB0CTL0 |= UCCKPH | UCMSB | UCSYNC; // 3-pin, 8-bit SPI slave (Nokia 5110 LCD)
        // UCB0CTL0 |= UCMSB | UCSYNC; // 3-pin, 8-bit SPI slave
        // UCB0CTL0 |= UCMODE_0 | UCMSB | UCSYNC; // 3-pin, 8-bit SPI slave
    } else {
        UCB0CTL0 |= UCMST | UCCKPH | UCMSB | UCSYNC; // 3-pin, 8-bit SPI master (Nokia 5110 LCD)
        // UCB0CTL0 |= UCMST | UCMSB | UCSYNC; // 3-pin, 8-bit SPI master
        // UCB0CTL0 |= UCMST | UCMODE_0 | UCMSB | UCSYNC; // 3-pin, 8-bit SPI master
        UCB0CTL1 |= UCSSEL_2; // SMCLK
        UCB0BR0 = 0x01; // 0x02; // 0x01; // 1:1
        UCB0BR1 = 0;
    }

    P1SEL  |= SSI_SCLK_PIN | SSI_SOMI_PIN | SSI_SIMO_PIN; // P1.5 = SCLK, P1.6 = SOMI, P1.7 = SIMO
    P1SEL2 |= SSI_SCLK_PIN | SSI_SOMI_PIN | SSI_SIMO_PIN;

    UCB0CTL1 &= ~UCSWRST; // clear SW

    // delay_ms(500);
    delay_ms(100);
}
