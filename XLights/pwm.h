#ifndef __PWM_H
#define __PWM_H

void DoSWPWM();
extern unsigned char bRGBValues[];
extern volatile unsigned int bTicks;
extern volatile unsigned int bLastMessage;
extern volatile unsigned int nBlackout1;
extern volatile unsigned int nBlackout2;

#endif
