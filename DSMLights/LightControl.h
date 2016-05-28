#ifndef __LIGHTCONTROL_H
#define __LIGHTCONTROL_H




void ProcessLightMode(uint8_t nMode, uint8_t nData);
void ColourWheel(uint8_t nPos, uint8_t &r, uint8_t &g, uint8_t &b);
uint8_t RCScale(unsigned char nIn);
void StepLights();

#endif

