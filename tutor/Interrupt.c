/************************************************************
 *
 * IAR EMBEDDED WORKBENCH TUTORIAL
 * Print the Fibonacci numbers using an interrupt handler
 * for the MSP430 Microcontroller.
 *
 * Note: This tutorial is intended to use the CLIB library.
 *       If you want to use DLIB, see instructions in the
 *       Utilities.h file
 *
 * Copyright (C) 1996-2011 IAR Systems AB.
 *
 * $Revision: 8211 $
 *
 ************************************************************/

/* enable use of extended keywords*/

/* use device msp430f149 */
/* and interrupt vector USART0RX_VECTOR */

#pragma language=extended

#include <stdio.h>
#include <intrinsics.h>
#include <io430x14x.h>

#include "Utilities.h"

void InitUart(void);
void do_foreground_process( void );
__interrupt void uartReceiveHandler( void );

/*-------------------------------------------------------------------
 DESCRIPTION: This routine initialises the UART0 on the MSP430.
 INPUTS:      None.
 OUTPUTS:     UART setup registers are modified.
 RETURNS:     None.
---------------------------------------------------------------------*/
void InitUart(void)
{
  __disable_interrupt();


  U0CTL_bit.CHAR = 1;                   // 8-bit character
  U0TCTL_bit.SSEL0 = 1;                 // UCLK = ACLK
  U0BR0 = 0x0D;                         // 32k/2400 - 13.65
  U0BR1 = 0x00;                         //
  U0MCTL = 0x6B;                        // modulation

  ME1_bit.URXE0 = ME1_bit.UTXE0 = 1;    // Enable USART0 TXD/RXD
  IE1_bit.URXIE0 = 1;                   // Enable USART0 RX interrupt

  __enable_interrupt();
}


// The showstopper
int callCount = 0;


// to have something to do, while waiting for interrupts
void do_foreground_process( void )
{
  putchar('.');
}


// define the interrupt handler
#pragma vector=USART0RX_VECTOR
__interrupt void uartReceiveHandler( void )
{
  unsigned int fib;

  // read fib value from UART0 receive buffer
  fib = U0RXBUF;
  PutFib(fib);

  // increment the show stopper
  ++callCount;
}


// Main program for Interrup simulation tutorial.
//   Prints the Fibonacci numbers.

int main( void )
{
  // initalize the UART and fibonacci numbers
  InitUart();
  InitFib();

  /* now loop 'forever', taking input when interrupted */
  while (callCount < MAX_FIB)
  {
    do_foreground_process();
  }
}
