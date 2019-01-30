// https://github.com/nathanpc/msp430-nokia-5110/
// http://rohitg.in/2014/11/09/Nokia5510-lcd-with-MSP430/
// https://github.com/cpb-/Article-RPi-MSP430/blob/master/msp430-spi-5.c
#include <stdint.h>
#include "msp430g2553.h"  
#include "mj_ssi.h"
#include "spi.h"
#include "init.h"
#include "mystring.h"

unsigned char a;

void MJ_SSI_Init_Key(datakey *data) {
    data->state = 0;
    data->tt    = 0;
    data->pp    = 0;
    data->sp    = 0;
    data->sw    = 0;
    data->sv    = 0;
    data->tm    = 0;
    data->p1    = 0;
    data->p2    = 0;
}

void MJ_SSI_Transfer_RxTx(char *out, unsigned char size) {
    static unsigned char ret = 0;
    // From the last byte to the first (8 bits)!
    // e.g. 0xABCDEF09 -> 0x09 0xEF 0xCD 0xAB
    for (ret=0; size; size--,out++) {
        MJ_SSI_PIN_SELECT;
        SSI_Transfer(*out,&ret,1);
        delay_ms(1);
        MJ_SSI_PIN_DESELECT;
    }
}

void MJ_SSI_Read_Command(void) {
    static unsigned char ret = 0;
    MJ_SSI_PIN_COMMAND;
    //
    MJ_SSI_PIN_SELECT;
    SSI_Transfer(0,&ret,1);
    if (ret) {
        a = ret;
    }
    delay_ms(1);
    MJ_SSI_PIN_DESELECT;
}

void MJ_SSI_Transfer_Key(datakey *data) {
    static uint32_t tmp = 0;
    MJ_SSI_PIN_DATA;
    //
    tmp = 0xEFCDAB; MJ_SSI_Transfer_RxTx((char *)(&tmp),3);
    MJ_SSI_Transfer_RxTx((char *)(&(data->state)),1);
    MJ_SSI_Transfer_RxTx((char *)(&(data->tt)),   4);
    MJ_SSI_Transfer_RxTx((char *)(&(data->pp)),   4);
    MJ_SSI_Transfer_RxTx((char *)(&(data->sp)),   2);
    MJ_SSI_Transfer_RxTx((char *)(&(data->sw)),   2);
    MJ_SSI_Transfer_RxTx((char *)(&(data->sv)),   2);
    MJ_SSI_Transfer_RxTx((char *)(&(data->tm)),   2);
    MJ_SSI_Transfer_RxTx((char *)(&(data->p1)),   4);
    MJ_SSI_Transfer_RxTx((char *)(&(data->p2)),   4);
    tmp = 0xBADCFE; MJ_SSI_Transfer_RxTx((char *)(&tmp),3);
}

void MJ_SSI_Init(unsigned char pin) {
    if (pin) {
        MJ_SSI_PIN_SETDIR;
        //
        SSI_Init(0);
    }
}
