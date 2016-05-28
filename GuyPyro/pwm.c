#include "msp430g2253.h"
#include "msp430g2_system.h"
#include "pwm.h"

void initTimer()
{
   TA1CTL = TASSEL_2+ID_0+MC_0+TACLR+TAIE; // Timer clock = SMCLK = 8MHz
   
   TA1CCTL0 = CM_0+CCIS_2+OUTMOD_0; 
   
   TA1CCR0 = 8000; // 8MHz - we need 1,000 ticks per second
   
   Delay();
   
   TA1CTL |= MC_1 ; // start running in up mode (to value of TBCL0)
}

int nTicks = 0;

#pragma vector=TIMER1_A1_VECTOR
__interrupt void timer_1A1(void)
{
  nTicks++;
}