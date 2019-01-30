#include "msp430g2553.h"
#include "init.h"
#include "PCD8544.h"
#include "spi.h"

void LCD_Init(unsigned char pin) {
  if (pin) {
      LCD_PIN_OUT;
      LCD_PIN_DIR;
      //
      SSI_Init(0);
  }
  //
  LCD_reset;
  delay_ms(1000);
  LCD_set;
  //
  LCD_WriteTo(COMMAND, PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION);
  LCD_WriteTo(COMMAND, PCD8544_SETVOP | 0x3F);
  LCD_WriteTo(COMMAND, PCD8544_SETTEMP | 0x02);
  LCD_WriteTo(COMMAND, PCD8544_SETBIAS | 0x03);
  LCD_WriteTo(COMMAND, PCD8544_FUNCTIONSET);
  LCD_WriteTo(COMMAND, PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);
  //
  delay_ms(1);
}

void LCD_WriteTo(unsigned char isData, unsigned char data) {
    unsigned char tmp;
    //
    LCD_select; // LCD_Select
    if (isData) {
        LCD_data; // DATA
    } else {
        LCD_command; // COMMAND
    }
    //
    SSI_Transfer(data,&tmp,1);
    //
    LCD_unselect; // LCD_Unselect
}

void LCD_OutChar(unsigned char c) {
    unsigned int i;
    LCD_WriteTo(DATA, 0x00);
    for(i = 0; i < 5; i++) {
        LCD_WriteTo(DATA, font[(unsigned int)(c - 0x20)][i]);
     }
    LCD_WriteTo(DATA, 0x00);
}

void LCD_OutString(char *c) {
    for (;*c;c++)
        LCD_OutChar(*c);
}

void LCD_OutStringN(char *c, unsigned char n) {
    for (n++;n;n--)
        if (*c) {
            LCD_OutChar(*c);
            c++;
        } else
            LCD_OutChar(' ');
}

void LCD_SetCursor(unsigned char newX, unsigned char newY) {
  if ((newX > PCD8544_MAX_X) || (newY > PCD8544_MAX_Y))
      return;  // multiply newX by 7 because each character is 7 columns wide
  LCD_WriteTo(COMMAND, 0x80 | (newX*7));     // setting bit 7 updates X-position
  LCD_WriteTo(COMMAND, 0x40 | newY);         // setting bit 6 updates Y-position
}

void LCD_Clear(void) {
  int i;
  for(i=1+(PCD8544_MAX_X+1)*(PCD8544_MAX_Y+1)*7; i; i--){
    LCD_WriteTo(DATA, 0x00);
  }
  LCD_SetCursor(0, 0);
}
