#ifndef __FLASH_H
#define __FLASH_H

void WriteFlash(int* addr, int nValue);
void WriteBufferFlash(uint8_t* addr, uint8_t* nValues, int nCount);

#endif
