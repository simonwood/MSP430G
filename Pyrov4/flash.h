#ifndef __FLASH_H
#define __FLASH_H

void WriteFlash(int* addr, int nValue);
void WriteBufferFlash(int* addr, int* nValues, int nCount);

#endif
