#include "msp430g2253.h"
#include "msp430g2_system.h"
#include "pwm.h"

// software PWM using overflow interrupt to advance a counter 
// that is then compared to the value we want to modulate at
// performed for 6 pins
void DoSWPWM()
{
   TA1CTL = TASSEL_2+ID_0+MC_0+TACLR+TAIE; // Timer clock = SMCLK = 16MHz
   
   TA1CCTL0 = CM_0+CCIS_2+OUTMOD_0; 
   
   TA1CCR0 = 1600; // 16MHz - we need 10,000 ticks per second;
   
   Delay();
   
   TA1CTL |= MC_1 ; // start running in up mode (to value of TBCL0)

   // timer 0  at 1us ticks
   TA0CTL = TASSEL_2+ID_1+MC_0+TACLR; // Timer clock = SMCLK = 16MHz
   
   TA0CCTL0 = CM_0+CCIS_2+OUTMOD_0; 
   
   TA0CCR0 = 0xFFFF;
   
   Delay();
   
   TA0CTL |= MC_2 ; // start running in continuous
}

volatile unsigned int bTicks = 0;
volatile unsigned int bLastMessage = 0;



#pragma vector=TIMER1_A1_VECTOR
__interrupt void timer_1A1(void)
{
  // read the TAIV register
  volatile int n = TA1IV;
    
  bTicks++;
  bLastMessage++;
}

