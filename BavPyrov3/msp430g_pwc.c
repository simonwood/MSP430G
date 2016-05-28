//******************************************************************************
//  Counter timer PWM driver code.
//
//******************************************************************************

#include "msp430g2253.h"
#include "msp430g2_system.h"
#include "flash.h"
#include "pwm.h"
#include "ppm.h"


// definitions for V2 hardware (also works for V1)
#define PYRO1_PORT P3OUT
#define PYRO2_PORT P3OUT
#define PYRO3_PORT P3OUT
#define PYRO4_PORT P2OUT

#define PYRO1_BIT BIT4
#define PYRO2_BIT BIT5
#define PYRO3_BIT BIT6
#define PYRO4_BIT BIT3

// port definitions for hardware ...
volatile unsigned char* H2_PORTS[] = 
{
  &PYRO1_PORT,
  &PYRO2_PORT,
  &PYRO3_PORT,
  &PYRO4_PORT
};

int H2_BITS[] = 
{
  PYRO1_BIT,
  PYRO2_BIT,
  PYRO3_BIT,
  PYRO4_BIT
};

// end of V1/V2

// Definitions for V3 hardware 
// port definitions for hardware ...
volatile unsigned char* H3_PORTS[] = 
{
  &P2OUT,
  &P2OUT,
  &P3OUT,
  &P3OUT,
  &P2OUT,
  &P2OUT,
  &P3OUT,
  &P2OUT  
};

int H3_BITS[] = 
{
  BIT6,
  BIT7,
  BIT6,
  BIT5,
  BIT5,
  BIT4,
  BIT4,
  BIT3
};

int  nChannels = 4;
volatile unsigned char** nPorts = H2_PORTS;
int* nBits = H2_BITS;

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

static inline void Red()
{
  P3OUT = BIT3;
}

static inline void Green()
{
  P3OUT = BIT2;
}

static inline void White()
{
  P3OUT = 0;
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
      P3OUT = BIT3;
      DelayMS(100);
      P3OUT = BIT2;
      DelayMS(100);
    }
    nFired = 0;
  }

  // green light
  Green();
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

// Trigger state machines

void TimedTrigger(unsigned int nWidth)
{
    if (nBaseState == 3 && nLoopCount >= 20)
    {
      // if we see a low now fire
      if (nWidth < 1400)
      {
        Fire();
        nBaseState = 0;
      }
      nBaseState = 0;
      nLoopCount = 0;
    }
  
    if (nBaseState == 2 && nLoopCount >= 15)
    {
      // signal must still be high to continue
      nBaseState = 0;
      if (nWidth > 1500)
      {
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

void HighTrigger(unsigned int nWidth)
{
    if (nBaseState == 3 && nLoopCount >= 40)
    {
      nBaseState = 0;
    }
  
    if (nBaseState == 2)
    {
      // Fire on high
      Fire();
      nBaseState = 3;
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


void SetMode(int nMode)
{
  int nData[2];
  nData[0] = nMode;
  nData[1] = nChannels;
  
  WriteBufferFlash((int*)0x1040, nData, 2);  
  
  while(1)
  {
    Flash();
  }
}

void IndicateOption(int nOption)
{
  WDTCTL = WDTPW + WDTHOLD;   // no watchdog ...
  for (int i = 0; i < nOption; i++)
  {
    Red();
    DelayMS(250);
    Green();      
    DelayMS(250);
  }
}

char OptionSelected()
{
  for (int i = 0; i < 60; i++)
  {
    int nWidth = GetWidth()/8;
    if (nWidth > 1500)
    {
      return 1;
    }
  }
  return 0;
}

void SetChannels()
{
  // slightly hidden menu for output chip count
  White();
  WDTCTL = WDTPW + WDTHOLD;   // no watchdog ...
  DelayMS(1500);
  if (GetWidth()/8 < 1400)
  {
    return;
  }  
  Red();
  
  WDTCTL = WDTPW + WDTHOLD;   // no watchdog ...
  DelayMS(1500);
  if (GetWidth()/8 > 1500)
  {
     while (GetWidth()/8 > 1500)
     {
       ; // wait for low again
     }
     return;
  }
  Green();
  WDTCTL = WDTPW + WDTHOLD;   // no watchdog ...
  DelayMS(400);
  
  for (int i = 0; i < 4; i++)
  {
     IndicateOption(i+1);
     if (OptionSelected())
     {
       nChannels = (i+1)*2;
       while (GetWidth()/8 > 1500)
       {
         ; // wait for low again
       }
       return;
     }
   }
}

void RunMenu()
{
    WDTCTL = WDTPW + WDTHOLD;   // no watchdog ...
    while (1)
    {
      Green();
      
      for (int i = 0; i < 3; i++)
      {
        IndicateOption(i+1);
        if (OptionSelected())
        {
          SetMode(i); // should never return.
          while (1) ;
        }
      }
      
      SetChannels();
    }  
}

// Startup checking for mode select...
void CheckStartup()
{
    
    WDTCTL = WDTPW + WDTHOLD;   // no watchdog ...
    
    unsigned int nWidth = GetWidth()/8;

    for (int i = 0; i < 100; i++)
    {
      nWidth = GetWidth()/8;
      if (nWidth < 1400) return;
      Red();
      nWidth = GetWidth()/8;
      if (nWidth < 1400) return;
      Green();
    }
    
    // if we get here then we are in mode select
    
    // wait for low
    WDTCTL = WDTPW + WDTHOLD;   // no watchdog ...
    Red();
    while (nWidth > 1400)
    {
        nWidth = GetWidth()/8;
    }
    Green();
    DelayMS(1000);
    
    RunMenu();
 }

void CheckVersions()
{
  if (*((int*)0x1042) == -1)
  {
    // not programmed - Firmwave V2 Hardware v2

    nChannels = 4;
    nPorts = H2_PORTS;
    nBits = H2_BITS;    
  }
  else
  {
    nChannels = *((int*)0x1042);
    nPorts = H3_PORTS;
    nBits = H3_BITS;        
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
  
   // Stop watchdog timer
  // DoWatchdog(0);
   
   // sET UP aclk
   BCSCTL3 |= LFXT1S_2;
   
   // Set all pins to output to stop floating
   P1DIR = 0xFF;
   P2DIR = 0xFF;
   P3DIR = 0xFF;

   // device running at 8MHz 
   BCSCTL1= CALBC1_8MHZ;
   DCOCTL = CALDCO_8MHZ;    

   //BCSCTL1= 0x0d;//CALBC1_8MHZ;
   //DCOCTL = 0x95;//CALDCO_8MHZ;            
   //
   //BCSCTL1= 0x06;//CALBC1_8MHZ;
   //DCOCTL = 0x40;//CALDCO_8MHZ;    
   

   
  DoPWM();
  
  _EINT();
  
  // leave two rx inputs
  P2DIR = (BIT1 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
  
  // red light
  Red();
  
  DetermineIdle();
  
  // green light
  Green();
  
  int nTriggerMode = *((int*)0x1040);
  
  CheckVersions();
  CheckStartup();

    
  while (1)
  {
    DoWatchdog(1);
       
    unsigned int nWidth = GetWidth();

    nWidth = nWidth/8;
    
    switch (nTriggerMode)
    {
      case 0:  TimedTrigger(nWidth); break;
      case 2:  HighTrigger(nWidth); break;
      default: DelayHighTrigger(nWidth); break;
    }
    
  }
}
