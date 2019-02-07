// https://electronics.stackexchange.com/questions/67524/msp430-i%C2%B2c-single-write-read-example
// http://www.simplyembedded.org/tutorials/msp430-i2c-basics/
#include <stdint.h>
#include "msp430g2553.h"  
#include "init.h"
#include "i2c.h"

char I2C_Address;

unsigned char I2C_CheckACK(void) {
    /* Check for ACK */
    if (UCB0STAT & UCNACKIFG) {
        /* Stop the I2C transmission */
        UCB0CTL1 |= UCTXSTP;

        /* Clear the interrupt flag */
        UCB0STAT &= ~UCNACKIFG;

        /* Set the error code */
        return 0;
    }

    return 1;
}

unsigned char I2C_WriteTo(const unsigned char *buf, unsigned char nbytes) {
    unsigned char okay = 1;

    /* Send the start condition */
    UCB0CTL1 |= UCTR | UCTXSTT;
    /* Wait for the start condition to be sent and ready to transmit interrupt */
    //while ((UCB0CTL1 & UCTXSTT) && ((IFG2 & UCB0TXIFG) == 0));
    while (UCB0CTL1 & UCTXSTT);
    while (!(IFG2 & UCB0TXIFG));

    /* Check for ACK */
    okay = I2C_CheckACK();

    /* If no error and bytes left to send, transmit the data */
    while (okay && (nbytes > 0)) {
        UCB0TXBUF = *buf;
        while ((IFG2 & UCB0TXIFG) == 0) {
            okay = I2C_CheckACK();
            if (okay)
                break;
        }
        buf++;
        nbytes--;
    }

    return okay;
}

unsigned char I2C_ReadFrom(unsigned char *buf, unsigned char nbytes) {
    unsigned char okay = 1;

    /* Send the start and wait */
    UCB0CTL1 &= ~UCTR;
    UCB0CTL1 |= UCTXSTT;

    /* Wait for the start condition to be sent */
    while (UCB0CTL1 & UCTXSTT);

    /*
     * If there is only one byte to receive, then set the stop
     * bit as soon as start condition has been sent
     */
    if (nbytes == 1) {
        UCB0CTL1 |= UCTXSTP;
    }

    /* Check for ACK */
    okay = I2C_CheckACK();

    /* If no error and bytes left to receive, receive the data */
    while (okay && (nbytes > 0)) {
        /* Wait for the data */
        while ((IFG2 & UCB0RXIFG) == 0);

        *buf = UCB0RXBUF;
        buf++;
        nbytes--;

        /*
         * If there is only one byte left to receive
         * send the stop condition
         */
        if (nbytes == 1) {
            UCB0CTL1 |= UCTXSTP;
        }
    }

    return okay;
}

unsigned char I2C_Transfer(const void *tx_buf, unsigned char tx_len, void *rx_buf, unsigned char rx_len) {
    unsigned char okay = 1;

    /* Set the slave device address */
    UCB0I2CSA = I2C_Address;

    /* Transmit data if there is any */
    if (tx_len > 0) {
        okay = I2C_WriteTo((const unsigned char *) tx_buf, tx_len);
    }

    /* Receive data if there is any */
    if (okay && (rx_len > 0)) {
        okay = I2C_ReadFrom((unsigned char *) rx_buf, rx_len);
    } else {
        /* No bytes to receive send the stop condition */
        UCB0CTL1 |= UCTXSTP;
    }

    return okay;
}

void I2C_Init(char address) {
    /* Configure P1.6 and P1.7 for I2C */
    P1SEL  |= I2C_SCLK_PIN + I2C_SDA_PIN;
    P1SEL2 |= I2C_SCLK_PIN + I2C_SDA_PIN;

    UCB0CTL1 |= UCSWRST;
    UCB0CTL0 |= UCMODE_0 | UCMST | UCSYNC;
    UCB0CTL1 |= UCSSEL_2; // SMCLK
    /**
     * Configure the baud rate registers for 100kHz when sourcing from SMCLK
     * where SMCLK = 1MHz
     */
    UCB0BR0 = 12;
    UCB0BR1 = 0;

    UCB0CTL1 &= ~UCSWRST; // clear SW

    I2C_Address = address;

    delay_ms(500);
}
