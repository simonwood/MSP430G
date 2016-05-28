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
#include "DSM.h"
#include "LightControl.h"

#define DSM_MODE 8
#define DSM_PARAM 1
#define DSM_PYRO 5

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


unsigned char RCScale(unsigned char nIn)
{
  uint16_t temp = nIn;
  temp *= 100;
  temp /= 256;
  
  return 100 - temp;
}

void FirePyro(volatile unsigned char* port, int bit)
{
  (*port) |= bit;
  DoWatchdog(1);
  DelayMS(400);
  (*port) &= ~bit;
  DoWatchdog(1);
  DelayMS(100);
  (*port) |= bit;
  DoWatchdog(1);
  DelayMS(300);
  (*port) &= ~bit;
}
int nFired = 0;

volatile unsigned char* H4_PORTS[] = 
{
  &P2OUT,
  &P2OUT
};

int H4_BITS[] = 
{
  BIT5,
  BIT4
};
void Fire()
{
  // red light
  Red();
  
  if (nFired >= 0 && nFired < 2)
  {
    FirePyro(H4_PORTS[nFired], H4_BITS[nFired]);
    nFired++;
  }
  
  if (nFired == 2)
  {
    // flash for end of sequence
    for (int i = 0; i < 5; i++)
    {
      DoWatchdog(1);
      Red();
      DelayMS(50);
      Green();
      DelayMS(50);
    }
    nFired = 0;
  }

  // green light
  Green();
}

// Pyro handling
uint8_t nState = 0;
void CheckPyro(uint8_t nValue)
{
  if (nState == 3)
  {
    if (nValue < 120)
    {
      if (bPyro < 3000)
      {
        nState = 0;
      }
      else if (bPyro > 10000)
      {
        nState = 0;
      }
      else
      {
        Fire();
        nState = 0;
      }
    }
  }
  if (nState == 2 && nValue > 130)
  {
    nState = 3;
    bPyro = 0;
  }
  if (nState == 1)
  {
    if (bPyro > 10000)
    {
      nState = 2;
    }
  }
  if (nState == 0 && nValue < 120)
  {
    bPyro = 0;
    nState = 1;
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
   
   
   DoSWPWM(); 
   _EINT();
   
   checkBind();   
   
   initDSM();
   

  
   int i = 0;
  while (1)
  {
    if (NewPacket())
    {
      bLastMessage = 0;
      ParsePacket();
      
      ProcessLightMode(Channel(DSM_MODE), Channel(DSM_PARAM));
      CheckPyro(Channel(DSM_PYRO));
      
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
    else
    {
      if (nStepNow)
      {
        StepLights();
        nStepNow = 0;
      }
    }
    
   WDTCTL = WDT_ARST_250;
  }
}
