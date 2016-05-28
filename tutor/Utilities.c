/**************************************************
 *
 * IAR EMBEDDED WORKBENCH TUTORIAL
 * Utility file
 *
 * Copyright (C) 1996-2009 IAR Systems AB.
 *
 * $Revision: 8176 $
 *
 **************************************************/

#include <stdio.h>
#include "Utilities.h"


unsigned int Fib[MAX_FIB];

/* Initialize MAX_FIB Fibonacci numbers. */

void InitFib( void )
{
  int i = 45;
  Fib[0] = Fib[1] = 1;

  for ( i=2 ; i<MAX_FIB ; i++)
  {
    Fib[i] = GetFib(i) + GetFib(i-1);
  }
}

/* Return the Fibonacci number 'nr'. */

unsigned int GetFib( int nr )
{
  if ( (nr>0) && (nr<=MAX_FIB) )
  {
    return ( Fib[nr-1] );
  }
  else
  {
    return ( 0 );
  }
}

/* Puts a number between 0 and 65536 to stdout. */

void PutFib( unsigned int out )
{
  unsigned int dec = 10, temp;

  if ( out >= 10000 )
  {
    putchar ( '#' ); /* To large value. */
    return;          /* Print a '#'. */
  }

  putchar ( '\n' );
  while ( dec <= out )
  {
    dec *= 10;
  }

  while ( (dec/=10) >= 10 )
  {
    temp = out/dec;
    putchar ( (int)('0' + temp) );
    out -= temp*dec;
  }

  putchar ( (int)('0' + out) );
}

