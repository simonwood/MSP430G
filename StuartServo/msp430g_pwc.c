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

static inline void Red()
{
  P3OUT |= BIT4;
  P3OUT &= ~BIT3;
}

static inline void Green()
{
  P3OUT |= BIT3;
  P3OUT &= ~BIT4;
}

void Flash()
{

  for (int i = 0; i < 5; i++)
  {
    // red light
    Red();
    DelayMS(100);
    // green light
    Green();
    DelayMS(100);
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
   P2DIR = (BIT1 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
   P3DIR = 0xFF;

   // device running at 16MHz 
   BCSCTL1= CALBC1_16MHZ;
   DCOCTL = CALDCO_16MHZ;    

  //DoPWM();
  
  //_EINT();
  
  // red light
  Red();
  DelayMS(300);
  Green();

    
  while (1)
  {
    //DoWatchdog(1);
    
    // set lines low (outputs go hihg)
    P3OUT &= ~(BIT7 | BIT6 | BIT5);
    P2OUT &= ~BIT3; 
    
    
    volatile unsigned int i = 0;
    while (i < 1700)
    {
      i++;
    }
    P3OUT |= BIT7;
    while (i < 2000)
    {
      i++;
    }
    P3OUT |= BIT6;
    while (i < 2300)
    {
      i++;
    }
    P3OUT |= BIT5;
    while (i < 2600)
    {
      i++;
    }
    P2OUT |= BIT3;
       
    i = 0;
    while (i < 24366) // 12183
    {
      i++;
    }
  }
}
