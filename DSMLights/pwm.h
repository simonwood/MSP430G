#ifndef __PWM_H
#define __PWM_H

void DoSWPWM();
extern unsigned char bRGBValues[];
extern volatile unsigned int bTicks;
extern volatile unsigned int bLastMessage;
extern volatile uint8_t nStepNow;
extern volatile unsigned int bPyro;

#endif
