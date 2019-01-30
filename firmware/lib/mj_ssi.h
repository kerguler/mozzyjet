#ifndef MJ_SSI_H_
#define MJ_SSI_H_

#define MJ_SSI_SCE_PIN  BIT4
#define MJ_SSI_DAT_PIN  BIT5

#define MJ_SSI_PIN_SELECT   P2OUT &= ~MJ_SSI_SCE_PIN
#define MJ_SSI_PIN_DESELECT P2OUT |= MJ_SSI_SCE_PIN

#define MJ_SSI_PIN_DATA     P2OUT |= MJ_SSI_DAT_PIN
#define MJ_SSI_PIN_COMMAND  P2OUT &= ~MJ_SSI_DAT_PIN

#define MJ_SSI_PIN_SETDIR   P2DIR |= MJ_SSI_SCE_PIN | MJ_SSI_DAT_PIN

typedef struct datakey_struct {
    unsigned char state; // 1 char
    int32_t tt;          // 4 char
    int32_t pp;          // 4 char
    uint16_t sp;         // 2 char
    uint16_t sw;         // 2 char
    uint16_t sv;         // 2 char
    uint16_t tm;         // 2 char
    int32_t p1;          // 4 char
    int32_t p2;          // 4 char
} datakey;

void MJ_SSI_Init_Key(datakey *data);
void MJ_SSI_Transfer_Key(datakey *data);
void MJ_SSI_Read_Command(void);

void MJ_SSI_Init(unsigned char slv);

#endif
