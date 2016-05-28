#ifndef __MSP430G2_SYSTEM_H
#define __MSP430G2_SYSTEM_H

static inline void DoWatchdog(unsigned char bFast)
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;

  //WDTCTL = bFast ? WDT_ARST_250 : WDT_ARST_1000;
}


void Delay(void);
void _Delay(unsigned int value);
void DelayMS(unsigned int milliseconds);

#endif


