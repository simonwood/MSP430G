#include "msp430g2253.h"
#include "ppm.h"
#include "pwm.h"
#include "msp430g2_system.h"

unsigned int nIdle = 0;
#define RX_PORT P1IN
#define RX_PORT_DIR P1DIR
#define RX_PIN BIT1

int DetermineIdle()
{
  // Make sure RX pin is input.
  RX_PORT_DIR &= ~(RX_PIN); 
  
  // wait for an edge
  DoWatchdog(0);
  unsigned int sVal = (RX_PORT & RX_PIN);
  while (1)
  {
    unsigned int temp = (RX_PORT & RX_PIN);
    if (temp != sVal)
    {
      break;
    }
  }
  DoWatchdog(0);
  
  // got an edge
  unsigned int nStart = TAR;
  nOverflow = 0;
  sVal = RX_PORT & RX_PIN;
  while (nOverflow < 10)
  {
    unsigned int temp = (RX_PORT & RX_PIN);
    if (temp != sVal)
    {
      // record the current state in sVal
      sVal = temp;
      break;
    }
  }
  unsigned int nStop = TAR;
  unsigned int nDelta = 0;
  DoWatchdog(0);
  
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
  DoWatchdog(1);
  while ((RX_PORT & RX_PIN) != nIdle)
  {
  }

  DoWatchdog(1);
  // wait for line to go active
  while ((RX_PORT & RX_PIN) == nIdle)
  {
  }
  
  DoWatchdog(1);
  
  // got an edge
  unsigned int nStart = TAR;
  nOverflow = 0;
  while (nOverflow < 2)
  {
    if ((RX_PORT & RX_PIN) == nIdle)
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
