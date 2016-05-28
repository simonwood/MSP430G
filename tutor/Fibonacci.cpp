/**************************************************
 *
 * IAR EMBEDDED WORKBENCH TUTORIAL
 * Fibonacci class implementation for EC++ tutorial.
 * This file contains definitions of the member
 * functions and static variables that are declared
 * in the class "fibonacci".
 *
 * Copyright (C) 1996-2009 IAR Systems AB.
 *
 * $Revision: 5151 $
 *
 **************************************************/

#include "Fibonacci.h"

#include <iostream>


int fibonacci::initialized = 0;


__no_init unsigned long int fibonacci::root[max_fib];


unsigned long int fibonacci::nth(int n)
{
  // If we already have calculated the value of the n:th
  // fibonacci number we can simply return it.
  if (n <= initialized)
  {
    return (root[n - 1]);
  }

  // In EC++ it is possible to declare variables in the
  // middle of a function.
  unsigned long int value;

  // Calculate the value.
  if (n <= 2)
  {
    value = 1;
  }
  else
  {
    value = nth(n-1) + nth(n-2);
  }

  // Try to store the calculated value for future needs.
  if (   (n > 0)
      && (n == initialized + 1)
      && (n < max_fib))
  {
    root[n - 1] = value;
    ++initialized;
  }

  // Finally, return the value.
  return value;
}


unsigned long int fibonacci::next()
{
  // Increase "current" and return the corresponding fibonacci number.
  return nth(current++);
}
