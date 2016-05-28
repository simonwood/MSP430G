/**************************************************
 *
 * IAR EMBEDDED WORKBENCH TUTORIAL
 * C tutorial. Print the Fibonacci numbers.
 *
 * Copyright (C) 1996-2009 IAR Systems AB.
 *
 * $Revision: 8176 $
 *
 **************************************************/

#include "Tutor.h"

int call_count;


/* Increase the 'call_count' variable by one. */

void NextCounter(void)
{
  call_count += 1;      /* from d_f_p */
}


/* Increase the 'call_count' variable.
   Get and print the associated Fibonacci number. */

void DoForegroundProcess(void)
{
  unsigned int fib;
  NextCounter();
  fib = GetFib( call_count );
  PutFib( fib );
}


/* Main program.
   Prints the Fibonacci numbers. */

int main(void)
{
  call_count=0;

  InitFib();

  while( call_count < MAX_FIB )
  {
    DoForegroundProcess();
  }
}
