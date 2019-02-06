// https://github.com/nathanpc/msp430-nokia-5110/
// http://rohitg.in/2014/11/09/Nokia5510-lcd-with-MSP430/
// https://github.com/cpb-/Article-RPi-MSP430/blob/master/msp430-spi-5.c
#include <stdint.h>
#include "msp430g2553.h"  
#include "mj_uart.h"
#include "uart.h"
#include "init.h"
#include "flash.h"

unsigned char a;

void MJ_UART_Init_Key(datakey *data) {
    data->state = 0;
    data->tt    = 0;
    data->pp    = 0;
    data->sp    = 0;
    data->sw0   = 0;
    data->sv0   = 0;
    data->sw1   = 0;
    data->sv1   = 0;
    data->tm    = 0;
    data->p1    = 0;
    data->p2    = 0;
    data->lam   = 0;
    data->pwm0  = 0;
    data->pwm1  = 0;
    data->v0    = 0;
    data->spv   = 0;
    data->comm  = 0;
    //
    flash_read(&(data->lam),0,1);  // 14000
    flash_read(&(data->pwm0),1,1); //  1250
    flash_read(&(data->pwm1),2,1); //  1750
    flash_read(&(data->v0),3,1);   //   300
    flash_read(&(data->spv),4,1);  //     5
}

void MJ_UART_Transfer_RxTx(char *out, unsigned char size) {
    // From the last byte to the first (8 bits)!
    // e.g. 0xABCDEF09 -> 0x09 0xEF 0xCD 0xAB
    for (; size; size--,out++)
        UART_OutChar(*out);
}

void MJ_UART_Write_Key(datakey *data) {
    static uint16_t buf[5];
    //
    buf[0] = (uint16_t)(data->lam);
    buf[1] = (uint16_t)(data->pwm0);
    buf[2] = (uint16_t)(data->pwm1);
    buf[3] = (uint16_t)(data->v0);
    buf[4] = (uint16_t)(data->spv);
    //
    flash_write(buf,0,5);
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
    MJ_UART_Transfer_RxTx((char *)(&(data->sw0)),  2);
    MJ_UART_Transfer_RxTx((char *)(&(data->sv0)),  2);
    MJ_UART_Transfer_RxTx((char *)(&(data->sw1)),  2);
    MJ_UART_Transfer_RxTx((char *)(&(data->sv1)),  2);
    MJ_UART_Transfer_RxTx((char *)(&(data->tm)),   2);
    MJ_UART_Transfer_RxTx((char *)(&(data->p1)),   4);
    MJ_UART_Transfer_RxTx((char *)(&(data->p2)),   4);
    MJ_UART_Transfer_RxTx((char *)(&(data->lam)),  2);
    MJ_UART_Transfer_RxTx((char *)(&(data->pwm0)), 2);
    MJ_UART_Transfer_RxTx((char *)(&(data->pwm1)), 2);
    MJ_UART_Transfer_RxTx((char *)(&(data->v0)),   2);
    MJ_UART_Transfer_RxTx((char *)(&(data->spv)),  2);
    MJ_UART_Transfer_RxTx((char *)(&(data->comm)), 1);
   tmp = 0xBADCFE; MJ_UART_Transfer_RxTx((char *)(&tmp),3);
}

void MJ_UART_Init(datakey *key) {
    MJ_UART_Init_Key(key);
    //
    UART_Init(0);
    UART_InitBuffer();
}
