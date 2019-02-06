#ifndef FLASH_H_
#define FLASH_H_

#include <stdint.h>

#define FLASH_ADDR ((uint16_t *)0x01000)

void flash_erase(void);
void flash_write(uint16_t *buffer, unsigned char loc, unsigned char size);
void flash_read(uint16_t *buffer, unsigned char loc, unsigned char size);

#endif
