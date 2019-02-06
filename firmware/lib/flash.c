// https://cyroforge.wordpress.com/2013/01/01/using-flash-memory-in-msp430/
#include <stdint.h>
#include "msp430g2553.h"
#include "flash.h"

// __DINT() is in IAR workbench
void flash_erase() {
  _DINT();                             // Disable interrupts. This is important, otherwise,
                                       // a flash operation in progress while interrupt may
                                       // crash the system.
  while(BUSY & FCTL3);                 // Check if Flash being used
  FCTL2 = FWKEY + FSSEL_1 + FN3;       // Clk = SMCLK/4
  FCTL1 = FWKEY + ERASE;               // Set Erase bit
  FCTL3 = FWKEY;                       // Clear Lock bit
  *(FLASH_ADDR) = 0;                   // Dummy write to erase Flash segment
  while(BUSY & FCTL3);                 // Check if Flash being used
  FCTL1 = FWKEY;                       // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                // Set LOCK bit
  _EINT();
}

void flash_write(uint16_t *buffer, unsigned char loc, unsigned char size) {
  flash_erase();
  //
  _DINT();                             // Disable interrupts(IAR workbench).
  while(FCTL3 & BUSY);                 // Check if Flash being used
  FCTL2 = FWKEY + FSSEL_1 + FN0;       // Clk = SMCLK/4
  FCTL3 = FWKEY;                       // Clear Lock bit
  FCTL1 = FWKEY + WRT;                 // Set WRT bit for write operation
  //
  for (size+=loc; loc<size; loc++) {
    *(FLASH_ADDR+loc) = *(buffer++);   // copy value to flash
    while(FCTL3 & BUSY);               // Check if Flash being used
  }
  //
  FCTL1 = FWKEY;                       // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                // Set LOCK bit
  _EINT();
}

void flash_read(uint16_t *buffer, unsigned char loc, unsigned char size) {
  _DINT();                             // Disable interrupts(IAR workbench).
  while(FCTL3 & BUSY);                 // Check if Flash being used
  FCTL2 = FWKEY + FSSEL_1 + FN0;       // Clk = SMCLK/4
  //
  for (size+=loc; loc<size; loc++)
    *(buffer++) = *(FLASH_ADDR+loc);   // copy value to buffer
  //
  _EINT();
}
