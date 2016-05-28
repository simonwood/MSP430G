// uart XBus code

#include "msp430g2253.h"
#include <stdint.h>
#include "msp430g2_system.h"
#include "xbus.h"
#include "pwm.h"
#include "XBusCRC.h"
#include "XBusCommand.h"
#include "XBusConfig.h"
#include <string.h>

#define XBUS_PACKET_GAP 30
#define XBUS_MAX_CHANNELS 64

extern volatile XBusConfigData LiveConfig;


void initXBus()
{
  /* Configure hardware UART */
  P1SEL = BIT1 ; // P1.1 = RXD,
  P1SEL2 = BIT1 ; // P1.1 = RXD, 
  
  // device running at 16MHz 
  // UCOS16 = 0 (no oversampling)
  // divisor = 64 (16mhz to 250KHz)
  // no modulation UCBRSx = 0
  
  UCA0CTL1 |= UCSSEL_2; // Use SMCLK
  UCA0BR0 = 64; // 
  UCA0BR1 = 0; // 
  UCA0MCTL = 0; // no modulation, UCOS16 = 0
  UCA0CTL0 = 0; // no parity, 8 bits, Async UART, LSB first
  UCA0CTL1 = UCSSEL_2 ; // SMCLK, 
  IE2 |= UCA0RXIE; // Enable USCI_A0 RX interrupt
}

unsigned char XBusBuffer[72];
uint8_t nXBusIndex = 0;
unsigned char nNewPacket = 0;

uint8_t XData[10]; // byte channel values as mapped by config

bool StoreChannelData(uint8_t bChannel, uint8_t bValue)
{
  for (uint8_t i = 0; i < 10; i++)
  {
    if (LiveConfig.nControlChannels[i] == bChannel)
    {
      XData[i] = bValue;
      return true;
    }
  }
  return false;
}

void ExtractChannels()
{
	XBusFrame* pXBus = (XBusFrame*)XBusBuffer;

	int nChannels = (pXBus->Length - 4)/4;
        uint8_t nStored = 0;

	register int i;
	for (i = 0; i < nChannels; i++)
	{
		uint8_t nChannel = (pXBus->Channels[i].Id & 0x4F);
		uint16_t nValue  = pXBus->Channels[i].High << 8;
		nValue |= pXBus->Channels[i].Low;

		if (pXBus->Channels[i].Function & 0x80)
		{
			// drop the failsafe value...
			continue;
		}

		// chan 0 is magic, first data channel is 1
		if (nChannel > 0 && nChannel <= XBUS_MAX_CHANNELS)
		{
                  if (StoreChannelData(nChannel-1, (uint8_t)(nValue/256)))
                  {
                    nStored++;
                  }
		}
                if (nStored == 10)
                {
                  break;
                }
	}
}

void ExtractDMX()
{
  XBusDMX64* pXBusDMX = (XBusDMX64*)XBusBuffer;
  
  for (int i = 0; i < 10; i++)
  {
    XData[i] = pXBusDMX->DMXData[LiveConfig.nControlChannels[i]];
  }
}

void ParsePacket()
{
  if (crc8(XBusBuffer, nXBusIndex-1) == XBusBuffer[nXBusIndex-1])
  {
	  if (XBusBuffer[0] == 0xA4)
	  {
		  ExtractChannels();
	  }
          if (XBusBuffer[0] == 0xD1)
          {
            ExtractDMX();
          }
	  if (XBusBuffer[0] == 0x20)
	  {
		  PerformCommand(XBusBuffer);
	  }
	  if (XBusBuffer[0] == 0x21)
	  {
		  PerformStatus(XBusBuffer);
	  }
  }
  memset(XBusBuffer, 0, sizeof(XBusBuffer));
  nXBusIndex = 0;
  nNewPacket = 0;
}

void ProcessByte(uint8_t b)
{
  if (bTicks > XBUS_PACKET_GAP) // more than 3ms since last byte - resync
  {
    nXBusIndex = 0;
  }
  bTicks = 0;

  XBusBuffer[nXBusIndex++] = b;
  
  // check for full packet
  if (nXBusIndex > 2)
  {
    if (nXBusIndex == XBusBuffer[1]+3)
    {
      nNewPacket = 1;
    }
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
  if (nChan < 1 || nChan > 10)
  {
    return 0;
  }
  
  return XData[nChan-1];
}

unsigned char NewPacket()
{
  return nNewPacket;
}

void ClearPacket()
{
  nNewPacket = 0;
}
