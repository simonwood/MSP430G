//******************************************************************************
//  Counter timer PWM driver code.
//
//******************************************************************************

#include "msp430g2253.h"

volatile unsigned int nR = 1;  //P1.6 (CCR1)
volatile unsigned int nG = 2;  // P1.4 (CCR2)
volatile unsigned int nB = 3;  // P1.5 (CCR0) 

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

void DoPWM()
{
   // device running at 8MHz 
   BCSCTL1= CALBC1_8MHZ;
   DCOCTL = CALDCO_8MHZ;            
      
   // set outputs as high
   P3OUT = (BIT4 | BIT5 | BIT6);
   P3DIR = (BIT4 | BIT5 | BIT6);

   P3SEL = (BIT4 | BIT5 | BIT6);
   //P3SEL2 = BIT4;
   
   TACTL = TASSEL_2+ID_0+MC_0+TACLR+TAIE; // Timer clock = SMCLK = 8MHz
   
   TACCTL0 = CM_0+CCIS_2+OUTMOD_5; // All Output Units will reset PWM outputs if
   TACCTL1 = CM_0+CCIS_2+OUTMOD_5; // TACCRx=TAR. setting PWM outputs is done
   TACCTL2 = CM_0+CCIS_2+OUTMOD_5; // by software.
   
    CCR0 = nB; //50% duty og 64000
    CCR1 = nR;
    CCR2 = nG;
   
   Delay();
   
   TACTL |= MC_2 ;
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
  P3OUT = 0;

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
  CheckOverflow();
}
 

unsigned int nIdle = 0;

int DetermineIdle()
{
  // wait for an edge
  WDTCTL = WDT_ARST_1000;
  unsigned int sVal = (P3IN & BIT2);
  while (1)
  {
    unsigned int temp = (P3IN & BIT2);
    if (temp != sVal)
    {
      break;
    }
  }
  WDTCTL = WDT_ARST_1000;
  
  // got an edge
  unsigned int nStart = TAR;
  nOverflow = 0;
  sVal = P3IN & BIT2;
  while (nOverflow < 10)
  {
    unsigned int temp = (P3IN & BIT2);
    if (temp != sVal)
    {
      // record the current state in sVal
      sVal = temp;
      break;
    }
  }
  unsigned int nStop = TAR;
  unsigned int nDelta = 0;
  WDTCTL = WDT_ARST_1000;
  
  if (nStop > nStart)
  {
    nDelta = nStop - nStart;
  }
  else
  {
    nDelta = 0xffff-nStart;
    nDelta += nStop;
    nOverflow--;
  }
  if (nOverflow == 0)
  {
    nIdle = sVal;
  }
  else
  {
    nIdle = sVal ^ BIT7;
  }
  return 1;
}

unsigned int GetWidth()
{
  // wait for line to go idle
  WDTCTL = WDT_ARST_250;
  while ((P3IN & BIT2) != nIdle)
  {
  }

  WDTCTL = WDT_ARST_250;
  // wait for line to go active
  while ((P3IN & BIT2) == nIdle)
  {
  }
  
  WDTCTL = WDT_ARST_250;
  
  // got an edge
  unsigned int nStart = TAR;
  nOverflow = 0;
  while (nOverflow < 2)
  {
    if ((P3IN & BIT2) == nIdle)
    {
      break;
    }
  }
  unsigned int nStop = TAR;
  unsigned int nDelta = 0;
  
  if (nStop > nStart)
  {
    nDelta = nStop - nStart;
  }
  else
  {
    nDelta = 0xffff-nStart;
    nDelta += nStop;
    nOverflow--;
  }
  
  return nDelta;  
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

void sdelay(int i)
{
  while (i)
  {
   int j = 30000;
   while (j)
   {
      j--;
   }
   i--;
  }
}

void Test();
void Police();
void GoMental();
void Strobe();
void Strobe2();
void ColourChange();

void Headlight(int bState)
{
  if (bState)
  {
    P2OUT = BIT3;
  }
  else
  {
    P2OUT &= ~BIT3;
  }
}

 void Light(unsigned int nRed, unsigned int nGreen, unsigned int nBlue, int nHeadlight)
{
  nR = nRed;
  nB = nBlue;
  nG = nGreen;
  Headlight(nHeadlight);
}

int GetStyle(int nPulseWidth)
{
  return nPulseWidth / 50;

    /*static int nLastWidth = 0;
  if (nPulseWidth < 50)
  {
    nLastWidth = nPulseWidth;
    return 0;
  }
  
  int nBase = 50;
  while (nBase < 950)
  {
    if (((nPulseWidth > (nBase+5)) && (nLastWidth < nBase)) || 
        (nPulseWidth < (nBase+45) && nLastWidth > (nBase+49)))
    {
      nLastWidth = nPulseWidth;
      return nBase / 50;
    }
    nBase += 50;    
  }
  return -1;*/
}

int main(void)
{ 
    // Stop watchdog timer
   WDTCTL = WDT_ARST_1000;   
   
   // sET UP aclk
   BCSCTL3 |= LFXT1S_2;
   
   //Test();
    
  DoPWM();
  
  _EINT();
  
  // Determine pulse direction
  // pin 1.7 = input
  P3DIR &= ~(BIT2);
  //P1REN = 1;
  
  P2DIR = BIT3;
  P2OUT = 0xff;

  // default light pattern
  Headlight(1);
  nR = nG = nB = 0xFFF;
    
  DetermineIdle();
  
  unsigned int nCalMin = 9000;
  
  while (1)
  {
    WDTCTL = WDT_ARST_250;
    
    unsigned int nSig = GetWidth();
    if (nSig < nCalMin)
    {
      nCalMin = nSig;
    }
    nSig = (nSig - nCalMin)/8;
    
    int nRGB = 65 * nSig;
    nR = nG = nB = nRGB;
    
    continue;
    
    int nStyle = GetStyle(nSig);
    
    switch (nStyle)
    {
    case 0:       
      nR = nG = nB = 50;
      Headlight(0);
      break;
    case 1:
    case 2:
      Light(0xfe00, 0xfe00, 0xfe00, 1);
      break;
    case 3:
      Light(0xfe00, 50, 50, 0);
      break;
    case 4:
      Light(50, 0xfe00, 50, 0);
      break;
    case 5:
      Light(50, 50, 0xfe00, 0);
      break;
    case 6:
    case 7:
      LoopColour(); 
      break;
      
    case 8:
    case 9:
    case 10:
      ColourChange();
      break;
    case 11:
    case 12:
      Police(); 
      break;
    case 13:
    case 14:
      Strobe();
      break;
    case 15:
    case 16:
      Strobe2();
      break;
    case 17:
      GoMental(); 
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

void Test()
{
     // device running at 1MHz 
   BCSCTL1= CALBC1_1MHZ;
   DCOCTL = CALDCO_1MHZ;  

   P1DIR = 0xFF;
   P3DIR = 0xFF;
  P2DIR = 0xFF;
  unsigned char i = 0;
  while(1)
  {
    P3OUT = i;
    i++;
    
    int j = 30000;
    while (j)
    {
      j--;
      
    }
  }
 
}

