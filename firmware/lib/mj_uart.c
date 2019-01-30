// https://github.com/nathanpc/msp430-nokia-5110/
// http://rohitg.in/2014/11/09/Nokia5510-lcd-with-MSP430/
// https://github.com/cpb-/Article-RPi-MSP430/blob/master/msp430-spi-5.c
#include <stdint.h>
#include "msp430g2553.h"  
#include "mj_uart.h"
#include "uart.h"
#include "init.h"

unsigned char a;

void MJ_UART_Init_Key(datakey *data) {
    data->state = 0;
    data->tt    = 0;
    data->pp    = 0;
    data->sp    = 0;
    data->sw    = 0;
    data->sv    = 0;
    data->tm    = 0;
    data->p1    = 0;
    data->p2    = 0;
    data->comm  = 0;
}

void MJ_UART_Transfer_RxTx(char *out, unsigned char size) {
    // From the last byte to the first (8 bits)!
    // e.g. 0xABCDEF09 -> 0x09 0xEF 0xCD 0xAB
    for (; size; size--,out++)
        UART_OutChar(*out);
}

unsigned char MJ_UART_Read_Command(datakey *data) {
    static unsigned char ret = 0;
    //
    data->comm = 0;
    if (UART_ReadBuffer(&ret)) {
        data->comm = ret;
        return 1;
    }
    return 0;
}

void MJ_UART_Transfer_Key(datakey *data) {
    static uint32_t tmp = 0;
    //
    tmp = 0xEFCDAB; MJ_UART_Transfer_RxTx((char *)(&tmp),3);
    MJ_UART_Transfer_RxTx((char *)(&(data->state)),1);
    MJ_UART_Transfer_RxTx((char *)(&(data->tt)),   4);
    MJ_UART_Transfer_RxTx((char *)(&(data->pp)),   4);
    MJ_UART_Transfer_RxTx((char *)(&(data->sp)),   2);
    MJ_UART_Transfer_RxTx((char *)(&(data->sw)),   2);
    MJ_UART_Transfer_RxTx((char *)(&(data->sv)),   2);
    MJ_UART_Transfer_RxTx((char *)(&(data->tm)),   2);
    MJ_UART_Transfer_RxTx((char *)(&(data->p1)),   4);
    MJ_UART_Transfer_RxTx((char *)(&(data->p2)),   4);
    MJ_UART_Transfer_RxTx((char *)(&(data->comm)), 1);
   tmp = 0xBADCFE; MJ_UART_Transfer_RxTx((char *)(&tmp),3);
}

void MJ_UART_Init(datakey *key) {
    MJ_UART_Init_Key(key);
    //
    UART_Init(0);
    UART_InitBuffer();
}
