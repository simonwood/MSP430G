#include "msp430g2253.h"

void Decode_Init()
{
  // Set up timer A1 for free running at clock speed...
   TA1CTL = TASSEL_2+ID_0+MC_0+TACLR+TAIE; // Timer clock = SMCLK = 8MHz
   
   TA1CCTL0 = CM_0+CCIS_2+OUTMOD_0;
   TA1CCTL1 = CM_0+CCIS_2+OUTMOD_0;
   TA1CCTL2 = CM_0+CCIS_2+OUTMOD_0;
    
   TA1CTL |= MC_2 ;
}

void Decode_Start()
{
  // force a timer reset
  TA1CTL |= TACLR;
}

