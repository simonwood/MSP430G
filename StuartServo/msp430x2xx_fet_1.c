//******************************************************************************
//  MSP430x2xx Demo - Software Toggle P1.0
//
//  Description; Toggle P1.0 by xor'ing P1.0 inside of a software loop.
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//                MSP430x2xx
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |             P1.0|-->LED
//
//  A. Dannenberg
//  Texas Instruments, Inc
//  January 2006
//  Built with CCE for MSP430 Version: 3.0
//******************************************************************************

#include "msp430g2252.h"

void Delay(unsigned int value)
{	
	if (value == 0)
	{
		return;
	}
	
    volatile unsigned int i;            // volatile to prevent optimization
    i = value;                          // SW Delay
    do
    {
    	i = i-1;
    } while (i != 0);
}

void DelayMS(unsigned int milliseconds)
{
	while (milliseconds >= 100)
	{
		Delay(11700);
		milliseconds -= 100;
	}
	while (milliseconds != 0)
	{
		Delay(117);
		milliseconds--;
	}
}

void DoPWM();
int main(void)
{ 

  DoPWM();
  
  _EINT();
  
  while (1)
  {
  }
 
  
  // END
  
}
