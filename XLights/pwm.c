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
   
   TA1CCR0 = 1600; // 16MHz - we need 10,000 ticks per second
   
   Delay();
   
   TA1CTL |= MC_1 ; // start running in up mode (to value of TBCL0)
}

unsigned char bRGBValues[6];
unsigned char  bCount = 0;
volatile unsigned int bTicks = 0;
volatile unsigned int bLastMessage = 0;
volatile unsigned int nBlackout1 = 0;
volatile unsigned int nBlackout2 = 0;


#pragma vector=TIMER1_A1_VECTOR
__interrupt void timer_1A1(void)
{
  // read the TAIV register
  volatile int n = TA1IV;
  
   // top wing
  if (bRGBValues[3] == bCount) P3OUT |= BIT5;
  if (bRGBValues[4] == bCount) P2OUT |= BIT3;
  if (bRGBValues[5] == bCount) P1OUT |= BIT7;
  
  // bottom wing
  if (bRGBValues[0] == bCount) P1OUT |= BIT6;
  if (bRGBValues[1] == bCount) P3OUT |= BIT7;
  if (bRGBValues[2] == bCount) P3OUT |= BIT6;
  
  bCount++;
  
  if (bCount >= 100)
  {
    P1OUT &= ~(BIT6|BIT7);
    P2OUT &= ~(BIT3);
    P3OUT &= ~(BIT5|BIT6|BIT7);
    bCount = 0;
  }
  bTicks++;
  bLastMessage++;
  nBlackout1++;
  nBlackout2++;
}

