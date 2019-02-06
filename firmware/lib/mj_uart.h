#ifndef MJ_UART_H_
#define MJ_UART_H_

typedef struct datakey_struct {
    unsigned char state; // 1 char
    int32_t tt;          // 4 char
    int32_t pp;          // 4 char
    uint16_t sp;         // 2 char
    uint16_t sw0;        // 2 char
    uint16_t sv0;        // 2 char
    uint16_t sw1;        // 2 char
    uint16_t sv1;        // 2 char
    uint16_t tm;         // 2 char
    int32_t p1;          // 4 char
    int32_t p2;          // 4 char
    uint16_t lam;        // 2 char
    uint16_t pwm0;       // 2 char
    uint16_t pwm1;       // 2 char
    uint16_t v0;         // 2 char
    uint16_t spv;        // 2 char
    unsigned char comm;  // 1 char
} datakey;

void MJ_UART_Init_Key(datakey *data);
void MJ_UART_Transfer_Key(datakey *data);
unsigned char MJ_UART_Read_Command(datakey *data);
void MJ_UART_Write_Key(datakey *data);

void MJ_UART_Init(datakey *key);

#endif
