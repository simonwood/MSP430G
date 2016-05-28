//******************************************************************************
//  Counter timer PWM driver code.
//
//******************************************************************************

#include "msp430g2253.h"

// THOMAS BIKE

volatile unsigned int nR = 1;  //P1.6 (CCR1)
volatile unsigned int nG = 2;  // P1.4 (CCR2)
volatile unsigned int nB = 3;  // P1.5 (CCR0) 
volatile unsigned int nH = 32000;

//------------------------------------------------------------------------------
// Delay Loop
void Delay(void)
{ 
  unsigned int i,j;
  for(i=0;i<=10000;i++) // delay loop
    for(j=0;j<=3;j++);
}

void _Delay(unsigned int value)
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
		_Delay(11700);
		milliseconds -= 100;
	}
	while (milliseconds != 0)
	{
		_Delay(117);
		milliseconds--;
	}
}

inline void DoWatchdog(unsigned char bFast)
{
  // Stop watchdog timer to prevent time out reset
  //WDTCTL = WDTPW + WDTHOLD;

  WDTCTL = bFast ? WDT_ARST_250 : WDT_ARST_1000;
}

void DoPWM()
{
   // device running at 8MHz 
   BCSCTL1= CALBC1_8MHZ;
   DCOCTL = CALDCO_8MHZ;            
      
   // set outputs as high
   //P3OUT = (BIT4 | BIT5 | BIT6);
   P3DIR = 0xFF;

   P3SEL = (BIT4 | BIT5 | BIT6);
   P3SEL2 = 0;
   
   TACTL = TASSEL_2+ID_0+MC_0+TACLR+TAIE; // Timer clock = SMCLK = 8MHz
   
   TACCTL0 = CM_0+CCIS_2+OUTMOD_5; // All Output Units will reset PWM outputs if
   TACCTL1 = CM_0+CCIS_2+OUTMOD_5; // TACCRx=TAR. setting PWM outputs is done
   TACCTL2 = CM_0+CCIS_2+OUTMOD_5; // by software.
   
    CCR0 = nB; //50% duty og 64000
    CCR1 = nR;
    CCR2 = nG;
   
   Delay();
   
   TACTL |= MC_2 ;
   
   // Now set up for PWM on TA1.0 for headlight control
   P2DIR |= BIT3;
   P2SEL = BIT3; //enable Timer1_A3.TA0 on pin 2.3
   
   TA1CTL = TASSEL_2+ID_0+MC_0+TACLR+TAIE; // Timer clock = SMCLK = 8MHz
   TA1CCTL0 = CM_0+CCIS_2+OUTMOD_5;
   TA1CCR0 = 32000;
   TA1CTL |= MC_2;   
}

int nOverflow = 0;
void CheckOverflow()
{
  TACTL = TASSEL_2+ID_0+MC_0+TACLR+TAIE; // stop the timer 

TACCR0=0xffff-nB; 
TACCR1=0xffff-nR; 
TACCR2=0xffff-nG;  
  
//--- PWM signal generation
TACTL &= ~TAIFG;

//TACCTL0 &= ~OUTMOD_7; // OUTMOD_0 => PWM output=L
//TACCTL0 |= OUTMOD_1; // OUTMOD_1 => set PWM output TA0 as soon as TACCR0=TAR
//TACCTL1 &= ~OUTMOD_7; // OUTMOD_0 => PWM output=L
//TACCTL1 |= OUTMOD_1; // OUTMOD_1 => set PWM output TA1 as soon as TACCR1=TAR
//TACCTL2 &= ~OUTMOD_7; // OUTMOD_0 => PWM output=L
//TACCTL2 |= OUTMOD_1; // OUTMOD_1 => set PWM otuput

  //activate leds    
  TACCTL0 &= ~OUTMOD_7; // OUTMOD_0 => PWM output=L
  TACCTL1 &= ~OUTMOD_7; // OUTMOD_0 => PWM output=L
  TACCTL2 &= ~OUTMOD_7; // OUTMOD_0 => PWM output=L
  
  // SDW CHANGE v2
  // why clear here - time hw should do that
//  P3OUT = BIT3;

// move to reset mode...
TACCTL0 |= OUTMOD_1; // OUTMOD_1 => reset PWM output TA0 as soon as TACCR0=TAR
TACCTL1 |= OUTMOD_1; // OUTMOD_1 => reset PWM output TA1 as soon as TACCR1=TAR
TACCTL2 |= OUTMOD_1; // OUTMOD_1 => reset PWM otuput

  // activate timer
  TACTL |= MC_2 ;
  nOverflow++;
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void timer_A1(void)
{
  TACTL = TASSEL_2+ID_0+MC_0+TACLR+TAIE; // stop the timer 

  TACCR0=0xffff-nB; 
  TACCR1=0xffff-nR; 
  TACCR2=0xffff-nG;  
  
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
 
#pragma vector=TIMER1_A1_VECTOR
__interrupt void timer1_A1(void)
{
  TA1CTL = TASSEL_2+ID_0+MC_0+TACLR+TAIE; // stop the timer 

  TA1CCR0=0xffff-nH; 
  
  //--- PWM signal generation
  TA1CTL &= ~TAIFG;

  //activate leds    
  TA1CCTL0 &= ~OUTMOD_7; // OUTMOD_0 => PWM output=L
  
  // move to reset mode...
  TA1CCTL0 |= OUTMOD_1; // OUTMOD_1 => reset PWM output TA0 as soon as TACCR0=TAR

  // activate timer
  TA1CTL |= MC_2 ;
}

unsigned int nIdle = 0;
#define RX_PORT P2IN
#define RX_PORT_DIR P2DIR
#define RX_PIN BIT0

unsigned int GetWidth()
{
  // wait for line to go idle
  DoWatchdog(1);
  
  if ((RX_PORT & RX_PIN))
  {
    return 1;
  }
  
  return 0;
}

void LoopColour()
{
  static int nBd = 1, nRd = 1, nGd = 1;
  
    nB += nBd * 107;
    nR += nRd * 257;
    nG += nGd * 491;

    if (nBd == 1 && nB > 0xfe00)
    {
      nBd = -1;
    }
    if (nBd == -1 && nB < 170)
    {
      nBd = 1;
    }
    if (nRd == 1 && nR > 0x8000)
    {
      nRd = -1;
    }
    if (nRd == -1 && nR < 320)
    {
      nRd = 1;
    }
    if (nGd == 1 && nG > 0x8000)
    {
      nGd = -1;
    }
    if (nGd == -1 && nG < 550)
    {
      nGd = 1;
    }
}

void Police();
void GoMental();
void Strobe();
void Strobe2();
void ColourChange();
void Chase();

inline void Headlight(int bState)
{
  if (bState)
  {
    nH = 0x30A0;
  }
  else
  {
    nH = 0x0030;
  }
}

inline void Light(unsigned int nRed, unsigned int nGreen, unsigned int nBlue, int nHeadlight)
{
  nR = nRed;
  nB = nBlue;
  nG = nGreen;
  Headlight(nHeadlight);
}

inline int GetStyle(int nPulseWidth)
{
  return nPulseWidth / 50;
}


int nCount = 0;
char nMode = 0;

int main(void)
{ 
   // Stop watchdog timer
   DoWatchdog(0);
   
   // sET UP aclk
   BCSCTL3 |= LFXT1S_2;
   
   // Set all pins to output to stop floating
   P1DIR = 0xFF;
   P2DIR = 0xFF;
   P3DIR = 0xFF;
    
  DoPWM();
  
  _EINT();
  
  // Determine pulse direction
  RX_PORT_DIR &= ~(RX_PIN);
  
  // leave two rx inputs
  P2DIR = (BIT1 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
  
  P2OUT = 0xff;
  
  // red light
  P3OUT = BIT2;


  nR = nG = nB = 0x2FFF;
    
  
  // green light
  P3OUT = BIT3;
  
  while (1)
  {
    DelayMS(90);
    DoWatchdog(1);
       
    unsigned int nSig = GetWidth();

    if (nSig == 1)
    {
      P3OUT = BIT2;
      if (nCount > 10)
      {
        nMode++;
        if (nCount > 140)
        {
          nMode = 0;
        }
      }
      nCount = 0;
    }
    else
    {
       P3OUT = BIT3;
      nCount++;
    }
    
    if (nMode > 11)
    {
      nMode = 1;
    }
    
    switch (nMode)
    {
    case 0:       
      nR = nG = nB = 50;
      Headlight(0);
      break;
    case 1:
      Light(0xfe00, 0xfe00, 0xfe00, 1);
      break;
    case 2:
      Light(0xfe00, 50, 50, 0);
      break;
    case 3:
      Light(50, 0xfe00, 50, 0);
      break;
    case 4:
      Light(50, 50, 0xfe00, 0);
      break;
    case 5:
      LoopColour(); 
      break;
      
    case 6:
      ColourChange();
      break;
    case 7:
      Police(); 
      break;
    case 8:
      Strobe();
      break;
    case 9:
      Strobe2();
      break;
    case 10:
      GoMental(); 
      break;
    case 11:
      Chase();
      break;
    }   
    
  }
}

static int nPolCount = 0;
void Police()
{
  if (nPolCount == 0)
  {
    nR = 0xfe00; nB = 50; nG = 50;
  }
  if (nPolCount == 4)
  {
    nR = 50; nB = 50; nG = 50;
  }
  if (nPolCount == 6)
  {
    nR = 0xfe00; nB = 50; nG = 50;
  }
  if (nPolCount == 10)
  {
    nR = 50; nB = 50; nG = 50;
  }
  if (nPolCount == 24)
  {
    nB = 0xfe00; nR = 50; nG = 50;
  }
  if (nPolCount == 28)
  {
    nR = 50; nB = 50; nG = 50;
  }
  if (nPolCount == 30)
  {
    nB = 0xfe00; nR = 50; nG = 50;
  }
  if (nPolCount == 34)
  {
    nR = 50; nB = 50; nG = 50;
  }
  nPolCount++;
  if (nPolCount > 48)
  {
    nPolCount = 0;
  }
}

void Chase()
{
  if (nPolCount < 10)
  {
    nR = 0xfe00; nB = 50; nG = 50;
  }
  else if (nPolCount < 20)
  {
    nG = 0xfe00; nB = 50; nR = 50;
  }
  else 
  {
    nB = 0xfe00; nR = 50; nG = 50;
  }
  nPolCount++;
  if (nPolCount > 29)
  {
    nPolCount = 0;
  }
}

void Strobe()
{
  if (nPolCount > 3)
  {
    nPolCount = 0;
  }
  
  if (nPolCount == 0)
  {
    nB = 0xff40; nR = 0xff40; nG = 0xff40;
    Headlight(1);
  }
  else
  {
    nB = 50; nR = 50; nG = 50;
    Headlight(0);
  }
  nPolCount++;
}

void Strobe2()
{
  if (nPolCount > 15)
  {
    nPolCount = 0;
  }
  
  if (nPolCount == 0)
  {
    nB = 0xff40; nR = 0xff40; nG = 0xff40;
    Headlight(1);
  }
  else if (nPolCount == 4)
  {
    nB = 0xff40; nR = 50; nG = 50;
    Headlight(0);
  }
  else if (nPolCount == 8)
  {
    nR = 0xff40; nB = 50; nG = 50;
    Headlight(0);
  }
  else if (nPolCount == 12)
  {
    nG = 0xff40; nB = 50; nR = 50;
    Headlight(0);
  }
  else
  {
    nB = 50; nR = 50; nG = 50;
    Headlight(0);
  }
  nPolCount++;
}


void ColourChange()
{
  if (nPolCount > 127)
  {
    nPolCount = 0;
  }
  
  nR = (nPolCount & 16) ? 0xfe00 : 50;
  nB = (nPolCount & 32) ? 0xfe00 : 50;
  nG = (nPolCount & 64) ? 0xfe00 : 50;

  Headlight(0);
  
  nPolCount++;
}

void GoMental()
{
  unsigned char nVal = TAR & 0xFF;
  
  if (nVal%2)
  {
    Headlight(0);
    nR = 50; nB = 50; nG = 50;
    return;
  }
  
  nR = ((nVal%4 == 0) ? 0xff40 : 0x0032);
  nG = ((nVal%5 == 0) ? 0xff40 : 0x0032);
  nB = ((nVal%6 == 0) ? 0xff40 : 0x0032);
  
  Headlight(nVal%7 == 0);
}

