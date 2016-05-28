// uart XBus code

#include "msp430g2253.h"
#include <stdint.h>
#include "msp430g2_system.h"
#include "DSM.h"
#include "pwm.h"
#include <string.h>

#define DSM_PACKET_GAP 70

void delayUS(int us)
{
  volatile int i = 0;
  
  while (us)
  {
    i++;
    i--;
    i++;
    i--;
    us--;
  }
}

void checkBind()
{
  // using 2.6, will be pulled low for bind
  // init 2.6 (default to XTAL)
  P2SEL &= ~BIT6;
  P2SEL2 &= ~BIT6;
  
  P2DIR &= ~BIT6;
  
  P2REN |= BIT6; // enable resistor
  P2OUT |= BIT6; // pull up
  delayUS(10);
  
  uint8_t in = P2IN & BIT6;
  
  if (in == 0)
  {
    // bind request
    P1SEL &= ~BIT1;
    P1SEL2 &= ~BIT1;
    
    P1DIR |= BIT1;
    
    bTicks = 0;
    while (bTicks < 600)
    {
      ; // spin
    }
    
    // set output high
    P1OUT |= BIT1;
    
    int nPulses = 5;
    
    while (nPulses)
    {
      P1OUT &= ~BIT1;
      delayUS(118);
      P1OUT |= BIT1;
      delayUS(122);
      nPulses--;
    }
    
    P1DIR &= ~BIT1;
  }
  
  P2REN &= ~BIT6;
  P2OUT &= ~BIT6;
}

void initDSM()
{
  /* Configure hardware UART */
  P1SEL = BIT1 ; // P1.1 = RXD,
  P1SEL2 = BIT1 ; // P1.1 = RXD, 
  
  // device running at 16MHz 
  // UCOS16 = 0 (no oversampling)
  // divisor = 139 (16mhz to 250KHz)
  // no modulation UCBRSx = 0
  
  UCA0CTL1 |= UCSSEL_2; // Use SMCLK
  UCA0BR0 = 139; // 
  UCA0BR1 = 0; // 
  UCA0MCTL = 0; // no modulation, UCOS16 = 0
  UCA0CTL0 = 0; // no parity, 8 bits, Async UART, LSB first
  UCA0CTL1 = UCSSEL_2 ; // SMCLK, 
  IE2 |= UCA0RXIE; // Enable USCI_A0 RX interrupt
}

unsigned char DSMBuffer[32];
uint8_t DSMIndex = 0;
unsigned char nNewPacket = 0;

uint16_t ChannelData[20];

void ExtractChannels()
{
  int nChanShift = 0;
  int nChanMask = 0;
  
  if (DSMBuffer[1] & 0x01)
  {
    // 10 bit data stream
    nChanShift = 2;   
    nChanMask = 0x03; 
  }
  else if (DSMBuffer[1] & 0x02)
  {
    // 11 bit data
    nChanShift = 3;
    nChanMask = 0x07;
  }
  else
  {
    // who knows ?
    return ;
  }
  
  uint8_t bPacket0 = 0;
  
  for (int b = 3; b < 16; b += 2) 
  { 
    uint8_t spekChannel = 0x0F & (DSMBuffer[b - 1] >> nChanShift); 
    if (spekChannel < 12) 
    {
      if (spekChannel == 1 || spekChannel == 10)
      {
        bPacket0 = 1;
      }
      ChannelData[spekChannel] = ((uint32_t)(DSMBuffer[b - 1] & nChanMask) << 8) + DSMBuffer[b]; 
      if (nChanShift == 2)
      {
        // 1024 bit data - adjust to 2048
        ChannelData[spekChannel] *= 2;
      }
    }
    if (spekChannel == 12)
    {
      // X-Plus
      uint8_t xChannel = (DSMBuffer[b - 1] >> 1) & 0x03;
      if (!bPacket0)
      {
        xChannel += 4;
      }
      ChannelData[xChannel+12] = ((DSMBuffer[b - 1]) & 0x01) * 256 + DSMBuffer[b];
      ChannelData[xChannel+12] *= 4; 
    }
  }
}


void ParsePacket()
{
  ExtractChannels();
  memset(DSMBuffer, 0, sizeof(DSMBuffer));
  DSMIndex = 0;
  nNewPacket = 0;
}

void ProcessByte(uint8_t b)
{
  if (bTicks > DSM_PACKET_GAP) // more than 7ms since last byte - resync
  {
    DSMIndex = 0;
  }
  bTicks = 0;

  DSMBuffer[DSMIndex++] = b;
  
  // check for full packet
  if (DSMIndex == 16)
  {
    nNewPacket = 1;
  }
}


#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
  unsigned char data = UCA0RXBUF;
  if (nNewPacket == 0)
  {
    ProcessByte(data);
  }
}

unsigned char Channel(unsigned char nChan)
{
  if (nChan < 1 || nChan > 20)
  {
    return 0;
  }
  
  return ChannelData[nChan-1]/8;
}

unsigned char NewPacket()
{
  return nNewPacket;
}

void ClearPacket()
{
  nNewPacket = 0;
}
