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

// Definitions for V4 hardware 
// port definitions for hardware ...
volatile unsigned char* H4_PORTS[] = 
{
  &P3OUT,
  &P3OUT,
  &P2OUT,
  &P3OUT
};

int H4_BITS[] = 
{
  BIT6,
  BIT7,
  BIT3,
  BIT5
};

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

int  nChannels = 4;
volatile unsigned char** nPorts = H4_PORTS;
int* nBits = H4_BITS;

void FirePyro(volatile unsigned char* port, int bit)
{
  (*port) |= bit;
  DoWatchdog(1);
  DelayMS(500);
  (*port) &= ~bit;
  DoWatchdog(1);
  DelayMS(300);
  (*port) |= bit;
  DoWatchdog(1);
  DelayMS(500);
  (*port) &= ~bit;
}
// control
int nLoopCount = 0;
int nBaseState = 0;
int nFired = 0;

void Fire()
{
  // red light
  Red();
  
  if (nFired >= 0 && nFired < nChannels)
  {
    FirePyro(nPorts[nFired], nBits[nFired]);
    nFired++;
  }
  
  if (nFired == nChannels)
  {
    // flash for end of sequence
    for (int i = 0; i < 5; i++)
    {
      DoWatchdog(1);
      Red();
      DelayMS(100);
      Green();
      DelayMS(100);
    }
    nFired = 0;
  }

  // green light
  Green();
}
void DelayHighTrigger(unsigned int nWidth)
{
    if (nBaseState == 3 && nLoopCount >= 25)
    {
      nLoopCount = 0;
      nBaseState = 0;
    }
  
    if (nBaseState == 2)
    {
      if (nWidth < 1400)
      {
        // reset
        nBaseState = 0;
        nLoopCount = 0;
      }
      if (nWidth > 1500 && nLoopCount > 15)
      {
        // Fire on high
        Fire();
        nBaseState = 3;
        nLoopCount = 0;
      }
    }
    if (nBaseState == 1)
    {
      nLoopCount = 0;
      if (nWidth > 1500)
      {
        nBaseState = 2;
      }
    }
    if (nBaseState == 0)
    {
      if (nWidth < 1400 && nLoopCount >= 15)
      {
        nBaseState = 1;
      }
      if (nWidth > 1500)
      {
        nLoopCount = 0;
      }
      
    }
    nLoopCount++;
}



void ProcessChannel(int nChannel)
{
  if (Channel((nChannel+1)) > 0x80)
  {
    volatile unsigned char* port = nPorts[nChannel];
    *port |= nBits[nChannel];
  }
  else
  {
    volatile unsigned char* port = nPorts[nChannel];
    *port &= ~(nBits[nChannel]);
  }
}

void DoXBus()
{
  while (1)
  {
    if (NewPacket())
    {
      ParsePacket();
      bLastMessage = 0;
      
      for (int i = 0; i < 4; i++)
      {
        ProcessChannel(i);
      }
    }
  }
}

void DoPWM()
{
  P1DIR = 0; // all P1 as input.  
  P1SEL = 0 ; 
  P1SEL2 = 0 ; 
  
  Red();
  
  DetermineIdle();
  // green light
  Green();  
  
  while (1)
  {
    unsigned int nWidth = GetWidth();

    nWidth = nWidth/8;
    
    DelayHighTrigger(nWidth);
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
  
  WDTCTL = WDTPW + WDTHOLD;
  //  WDTCTL = WDT_ARST_250;
   
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


   LoadConfig();
   
   DoSWPWM(); 
   
   initXBus();
   
   _EINT();
   
   // is it XBus based ???
   bLastMessage = 0;
   while (1)
   {
     if (NewPacket())
     {
       // means we have a data packet with Valid CRC
       Flash();
       DoXBus();
     }
     if (bLastMessage > 25000)
     {
       // no message in 2.5 seconds - PWM
       DoPWM();
     }
   }
   
}
