//******************************************************************************
//  Counter timer PWM driver code.
//
//******************************************************************************

#include "msp430g2253.h"
#include <stdint.h>

#include "msp430g2_system.h"
#include "flash.h"
#include "pwm.h"
#include "ppm.h"
#include "xbus.h"
#include "XBusConfig.h"

static inline void Red()
{
  P3OUT &= ~BIT3;
  P3OUT |= BIT4;
}

static inline void Green()
{
  P3OUT &= ~BIT4;
  P3OUT |= BIT3;
}

void Flash()
{

  for (int i = 0; i < 5; i++)
  {
    // red light
    P3OUT = BIT3;
    DelayMS(100);
    // green light
    P3OUT = BIT2;
    DelayMS(100);
  }
}

unsigned char Scale(unsigned char nVal)
{
  if (nVal > 200) return 100;
  return nVal / 2;
}

void Wheel1(unsigned char WheelPos)
{
   if(WheelPos < 85) 
   {
     bRGBValues[0] = Scale(WheelPos * 3);
     bRGBValues[1] = Scale(255 - WheelPos * 3);
     bRGBValues[2] = Scale(0);
   } 
   else if(WheelPos < 170) 
   {
     WheelPos -= 85;
   
     bRGBValues[0] = Scale(255 - WheelPos * 3);
     bRGBValues[1] = Scale(0);
     bRGBValues[2] = Scale(WheelPos * 3);
   
   } 
   else 
   {
   
     WheelPos -= 170;
   
     bRGBValues[0] = Scale(0);
     bRGBValues[1] = Scale(WheelPos * 3);
     bRGBValues[2] = Scale(255 - WheelPos * 3);
    
   }

}

void Wheel2(unsigned char WheelPos)
{
   if(WheelPos < 85) 
   {
     bRGBValues[3] = Scale(WheelPos * 3);
     bRGBValues[4] = Scale(255 - WheelPos * 3);
     bRGBValues[5] = Scale(0);
  
   } 
   else if(WheelPos < 170) 
   {
   
     WheelPos -= 85;
   
     bRGBValues[3] = Scale(255 - WheelPos * 3);
     bRGBValues[4] = Scale(0);
     bRGBValues[5] = Scale(WheelPos * 3);
   
   } 
   else 
   {
   
     WheelPos -= 170;
   
     bRGBValues[3] = Scale(0);
     bRGBValues[4] = Scale(WheelPos * 3);
     bRGBValues[5] = Scale(255 - WheelPos * 3);
    
   }

}

unsigned char RCScale(unsigned char nIn)
{
  uint16_t temp = nIn;
  temp *= 100;
  temp /= 256;
  
  return 100 - temp;
}

void BlackoutCheck()
{
  // top
  if (bRGBValues[3] + bRGBValues[4] + bRGBValues[5] <= 3*98)
  {
    nBlackout1 = 0;
  }
  if (nBlackout1 > 20000)
  {
    // force level increase
    bRGBValues[3] = 97;
    bRGBValues[4] = 97;
    bRGBValues[5] = 97;
    nBlackout1 = 21000;
  }

  // bottom
  if (bRGBValues[0] + bRGBValues[1] + bRGBValues[2] <= 3*98)
  {
    nBlackout2 = 0;
  }
  if (nBlackout2 > 20000)
  {
    // force level increase
    bRGBValues[0] = 97;
    bRGBValues[1] = 97;
    bRGBValues[2] = 97;
    nBlackout2 = 21000;
  }
}

int main(void)
{ 
  // don;t want to fire on power up !
  P2SEL = 0;
  P2SEL2 = 0;
  P3OUT = 0;
  P2OUT = 0;
  P3DIR = 0xff;
  
  //WDTCTL = WDTPW + WDTHOLD;
    WDTCTL = WDT_ARST_250;
   
   // sET UP aclk
   BCSCTL3 |= LFXT1S_2;
   
   // Set all pins to output to stop floating
   P1DIR = 0xC0;
   P2DIR = 0xFF;
   P3DIR = 0xFF;

   // device running at 16MHz 
   BCSCTL1= CALBC1_16MHZ;
   DCOCTL = CALDCO_16MHZ;    
   
   Green();

   bRGBValues[0] = 97;
   bRGBValues[1] = 97;
   bRGBValues[2] = 97;
   bRGBValues[3] = 97;
   bRGBValues[4] = 97;
   bRGBValues[5] = 97;
   
   LoadConfig();
   
   
   DoSWPWM(); 
   
   initXBus();
   
   _EINT();
  
   int i = 0;
  while (1)
  {
    if (NewPacket())
    {
      bLastMessage = 0;
      ParsePacket();
      bRGBValues[0] = RCScale(Channel((RGB1_RED)));
      bRGBValues[1] = RCScale(Channel((RGB1_GREEN)));
      bRGBValues[2] = RCScale(Channel((RGB1_BLUE)));
      bRGBValues[3] = RCScale(Channel((RGB2_RED)));
      bRGBValues[4] = RCScale(Channel((RGB2_GREEN)));
      bRGBValues[5] = RCScale(Channel((RGB2_BLUE)));
      
      BlackoutCheck();
     
      // check for pyro fire
      P2OUT &= ~BIT5;
      P2OUT &= ~BIT4;
      if (Channel(PYRO1_FIRE) == 0xDE)
      {
        P2OUT |= BIT5;
      }
      if (Channel(PYRO2_FIRE) == 0xDE)
      {
        P2OUT |= BIT4;
      }

      if (i)
      {
        Red();
        i = 0;
      }
      else
      {
        Green();
        i = 1;
      }
    }  
    
    
    if (bLastMessage > 5000)
    {
      bRGBValues[0] = 0xff;
      bRGBValues[1] = 0;
      bRGBValues[2] = 0xff;
      bRGBValues[3] = 0xff;
      bRGBValues[4] = 0;
      bRGBValues[5] = 0xff;
      bLastMessage = 0;
      Red();
    }
    
      WDTCTL = WDT_ARST_250;
  }
}
