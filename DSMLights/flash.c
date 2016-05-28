// Flash handlers

#include "msp430g2253.h"
#include <stdint.h>
#include "flash.h"



void EraseSegment(uint8_t* addr)
{
   BCSCTL1= CALBC1_1MHZ;
   DCOCTL = CALDCO_1MHZ; 
   
  WDTCTL = WDTPW + WDTHOLD;            // No watchdog !
  _DINT();                             // Disable interrupts. This is important, otherwise,
                                       // a flash operation in progress while interrupt may
                                       // crash the system.
  while(BUSY & FCTL3);                 // Check if Flash being used
  FCTL2 = FWKEY + FSSEL0 + FN1;       // Clk = SMCLK/4
  FCTL1 = FWKEY + ERASE;               // Set Erase bit
  FCTL3 = FWKEY;                       // Clear Lock bit
  *addr = 0;                           // Dummy write to erase Flash segment
  while(BUSY & FCTL3);                 // Check if Flash being used
  FCTL1 = FWKEY;                       // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                // Set LOCK bit
  _EINT();  
}


void WriteBufferFlash(uint8_t* addr, uint8_t* nValues, int nCount)
{
   BCSCTL1= CALBC1_1MHZ;
   DCOCTL = CALDCO_1MHZ; 
   
  EraseSegment(addr);
  WDTCTL = WDTPW + WDTHOLD;            // No watchdog !
  _DINT();                             // Disable interrupts(IAR workbench).
  FCTL2 = FWKEY + FSSEL0 + FN1;       // Clk = SMCLK/4
  FCTL3 = FWKEY;                       // Clear Lock bit
  FCTL1 = FWKEY + WRT;                 // Set WRT bit for write operation

  for (int i = 0; i < nCount; i++)
  {
    *addr++ = nValues[i];
  }  
 
  FCTL1 = FWKEY;                        // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                 // Set LOCK bit
  _EINT();
  
   BCSCTL1= CALBC1_16MHZ;
   DCOCTL = CALDCO_16MHZ;   
}


   /*
   
 unsigned char volatile*Flash_ptrA = (unsigned char volatile*)0x10C0;              // Point to beginning of seg A
  FCTL2 = FWKEY + FSSEL0 + FN1;             // MCLK/3 for Flash Timing Generator
  FCTL1 = FWKEY + ERASE;                    // Set Erase bit
  FCTL3 = FWKEY + LOCKA;                    // Clear LOCK & LOCKA bits
  *Flash_ptrA = 0x00;                       // Dummy write to erase Flash seg A
  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
  //Flash_ptrA = (char *)0x10F8;              // Point to beginning of cal consts
  //for (j = 0; j < 8; j++)
  //  *Flash_ptrA++ = CAL_DATA[j];            // re-flash DCO calibration data
  
  Flash_ptrA = (unsigned char volatile*)&(CALBC1_12MHZ);
  *Flash_ptrA = 0x0d;
  Flash_ptrA = (unsigned char volatile*)&(CALDCO_12MHZ);
  *Flash_ptrA = 0x95;
  Flash_ptrA = (unsigned char volatile*)&(CALBC1_16MHZ);
  *Flash_ptrA = 0x0d;
  Flash_ptrA = (unsigned char volatile*)&(CALDCO_16MHZ);
  *Flash_ptrA = 0x95;


  Flash_ptrA = (unsigned char volatile*)&(CALBC1_8MHZ);
  *Flash_ptrA = 0x0d;
  Flash_ptrA = (unsigned char volatile*)&(CALDCO_8MHZ);
  *Flash_ptrA = 0x95;
  Flash_ptrA = (unsigned char volatile*)&(CALBC1_1MHZ);
  *Flash_ptrA = 0x06;
  Flash_ptrA = (unsigned char volatile*)&(CALDCO_1MHZ);
  *Flash_ptrA = 0x40;
  
  
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCKA + LOCK;             // Set LOCK & LOCKA bit 

  while(1)
  {
    Flash();
  }

*/

   
   /*int nVal = *((int*)0x1040);
   
   if (nVal == 23)
   {
     WriteFlash((int*)0x1040, 77);
     while(1) Green();
   }
   if (nVal == 77)
   {
     WriteFlash((int*)0x1040, 23);
     while(1) Red();
   }
   
   WriteFlash((int*)0x1040, 23);*/

