#include "msp430g2253.h"
#include "msp430g2_system.h"
#include "pwm.h"

void DoPWM()
{   
   TACTL = TASSEL_2+ID_0+MC_0+TACLR+TAIE; // Timer clock = SMCLK = 8MHz
   
   TACCTL0 = CM_0+CCIS_2+OUTMOD_5; // All Output Units will reset PWM outputs if
   TACCTL1 = CM_0+CCIS_2+OUTMOD_5; // TACCRx=TAR. setting PWM outputs is done
   TACCTL2 = CM_0+CCIS_2+OUTMOD_5; // by software.
   
    CCR0 = 0x8000; 
    CCR1 = 0x8000;
    CCR2 = 0x8000;
   
   Delay();
   
   TACTL |= MC_2 ;
}

int nOverflow = 0;

#pragma vector=TIMER0_A1_VECTOR
__interrupt void timer_A1(void)
{
  TACTL = TASSEL_2+ID_0+MC_0+TACLR+TAIE; // stop the timer 

  TACCR0=0x8000; 
  TACCR1=0x8000; 
  TACCR2=0x8000;  
  
  //--- PWM signal generation
  TACTL &= ~TAIFG;

  //activate leds    
  TACCTL0 &= ~OUTMOD_7; // OUTMOD_0 => PWM output=L
  TACCTL1 &= ~OUTMOD_7; // OUTMOD_0 => PWM output=L
  TACCTL2 &= ~OUTMOD_7; // OUTMOD_0 => PWM output=L
  
  // move to reset mode...
  TACCTL0 |= OUTMOD_1; // OUTMOD_1 => reset PWM output TA0 as soon as TACCR0=TAR
  TACCTL1 |= OUTMOD_1; // OUTMOD_1 => reset PWM output TA1 as soon as TACCR1=TAR
  TACCTL2 |= OUTMOD_1; // OUTMOD_1 => reset PWM otuput

  // activate timer
  TACTL |= MC_2 ;
  nOverflow++;
}

