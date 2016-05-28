//******************************************************************************
//  Counter timer PWM driver code.
//
//******************************************************************************

#include "msp430g2253.h"
#include "msp430g2_system.h"
#include "pwm.h"
#include "msp430_spi.h"
#include "mirf.h"
#include "XBusCRC.h"

#include <stdlib.h>

uint8_t Address[] = { 'G', 'u', 'y', 0xBE, 0xEF}; 


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

/*
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
   //BCSCTL3 |= LFXT1S_2;
   
   // Set all pins to output to stop floating
   P1DIR = 0xFF;
   P2DIR = 0xFF;
   P3DIR = 0xFF;

   // device running at 8MHz 
   BCSCTL1= CALBC1_8MHZ;
   DCOCTL = CALDCO_8MHZ;    
	BCSCTL2 = DIVS_2;  // SMCLK = DCOCLK/1
	// SPI (USI) uses SMCLK, prefer SMCLK=DCO (no clock division)

        
	//DCOCTL = CALDCO_1MHZ;
	//BCSCTL1 = CALBC1_1MHZ;
	//BCSCTL2 = DIVS_0;  // SMCLK = DCOCLK/1
        
   //initTimer();
   
   // set up nrf chip
   rf_crc = RF24_EN_CRC | RF24_CRCO; // CRC enabled, 16-bit
   rf_addr_width      = 5;
   rf_speed_power     = RF24_SPEED_250KBPS | RF24_POWER_0DBM;
   rf_channel         = 10;  
   
   msprf24_init();
   msprf24_set_pipe_packetsize(0, 8);
   //msprf24_set_pipe_packetsize(1, 8);
   msprf24_open_pipe(0, 0);
   //msprf24_open_pipe(1, 0);
   
   msprf24_standby();
   w_tx_addr(Address);
   w_rx_addr(0, Address);
   
     uint8_t buf[8];
     
     buf[0] = 0;
   
   while (1)
   {
//     Red();
     uint8_t buf[8];
     w_tx_payload(8, buf);
     msprf24_activate_tx();        
//     Green();
     uint8_t status = 0;
     do
     {
       status = msprf24_current_state();
     } while (status == RF24_STATE_PTX);
     LPM4;
     buf[0] ++;

    // uint8_t user = 0;
	//if (rf_irq & RF24_IRQ_FLAGGED) 
        //{
	//	user = ~(msprf24_get_irq_reason());
	//}     
     msprf24_activate_rx();
     DelayMS(10);
     
   }
    
  _EINT();
  
  // red light
  
  while (1)
  {
    //DoWatchdog(1);       
  }
}

*/

void Send(uint8_t* bData)
{
  // wait for any previous tx to finish.
  while(Mirf.isSending())
    ;

  Mirf.setTADDR(Address);
  Mirf.send(bData);
}

uint8_t buf[8];
uint8_t nCount = 0;

#pragma pack(1)
struct XBus8
{
  uint8_t Command;
  uint8_t Length;
  uint8_t NextChannel;
  uint8_t DataType;
  uint8_t Data1;
  uint8_t Data2;
  uint8_t Data3;
  uint8_t Crc;
};
#pragma pack()
uint8_t nChan = 10;
uint8_t nPackets = 0;
void BuildBuffer()
{
  nChan = (rand() % 100) + 10;
  nPackets++;
  
  if (nPackets == 15)
  {
    // force channel 10 every 150 ms;
    nChan = 10; 
  }
  
  XBus8* pData = (XBus8*)((void*)buf);

  pData->Command = 0x10;
  pData->Length = 5;
  pData->NextChannel = nChan;
  pData->DataType = 0xFE;
  pData->Data1 = nCount++;
  pData->Data2 = 0;
  pData->Data3 = 0;
  
  pData-> Crc = crc8(buf, 7);
}

int main(void)
{ 
  // don;t want to fire on power up !
  P2SEL = 0;
  P2SEL2 = 0;
  P3OUT = 0;
  P2OUT = 0;
  //P3DIR = 0xff;
  
  WDTCTL = WDTPW + WDTHOLD;
  
   // Stop watchdog timer
  // DoWatchdog(0);
   
   // sET UP aclk
   //BCSCTL3 |= LFXT1S_2;
   
   // Set all pins to output to stop floating
   P1DIR = 0x00;
   P2DIR = 0x00;
   P3DIR = 0xFF;

   // device running at 8MHz 
   BCSCTL1= CALBC1_8MHZ;
   DCOCTL = CALDCO_8MHZ;    
	BCSCTL2 = DIVS_2;  // SMCLK = DCOCLK/1
	// SPI (USI) uses SMCLK, prefer SMCLK=DCO (no clock division)

        
	//DCOCTL = CALDCO_1MHZ;
	//BCSCTL1 = CALBC1_1MHZ;
	//BCSCTL2 = DIVS_0;  // SMCLK = DCOCLK/1
        
   //initTimer();
        
  while(1)
  {
    uint8_t t = P2IN;
    if (t & BIT5)
    {
      P3OUT = BIT3;
    }
    else
    {
      P3OUT = BIT4;
    }    
  }
        
  P3OUT = BIT3;
  P3OUT = BIT4;
  P3OUT = BIT3;
  P3OUT = BIT4;
  P3OUT = BIT3;
  P3OUT = BIT4;
  P3OUT = BIT3;
  P3OUT = BIT4;
  P3OUT = BIT3;
  P3OUT = BIT4;
   
  Mirf.channel = 10;
  Mirf.payload = 8;
  Mirf.init();  
  
  srand(1325);
  


  Mirf.setRADDR(Address);
  Mirf.setTADDR(Address);
  Mirf.payload = 8;  
  
  Mirf.config();   
  

     
   buf[0] = 0;
   

   
   while (1)
   {
     BuildBuffer();
     Send(buf);
     
     while (Mirf.isSending())
     {
     }

     Mirf.powerDown();
     Mirf.setChannel(nChan);
     Mirf.powerUpRx();
     Mirf.flushRx();
    
     DelayMS(10);
   }
    
  _EINT();
  
  // red light
  
  while (1)
  {
    //DoWatchdog(1);       
  }
}
