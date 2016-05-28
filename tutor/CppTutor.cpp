/**************************************************
 *
 * IAR EMBEDDED WORKBENCH TUTORIAL
 * A simple EC++ tutorial that uses fibonacci objects.
 * It creates two objects and extracts two sequences
 * of fibonacci numbers. To demonstrate that the two
 * objects are independent of each other, the numbers
 * are extracted at different speed. A number is
 * extracted from fib1 each turn in the loop while a
 * number is extracted from fib2 only every second turn.
 *
 * "fib1" is created using the default constructor
 * while the definition of "fib2" uses the constructor
 * that takes an integer as its argument.
 *
 * Copyright (C) 1996-2009 IAR Systems AB.
 *
 * $Revision: 5151 $
 *
 **************************************************/

#include <iostream>

#include "Fibonacci.h"

int main(void)
{
  // Create two fibonacci objects.
  fibonacci fib1;
  fibonacci fib2(7);            // fib2 starts at fibonacci number 7.

  // Extract two series of fibonacci numbers.
  for (int i = 1; i < 30; ++i)
  {
    cout << fib1.next();

    // If "i" is even, we print out the next fibonacci number of
    // the sequence represented by fib2.
    if (i % 2 == 0)
    {
      cout << "  " << fib2.next();
    }

    cout << endl;  // Flush the output stream to the terminal I/O
  }
}
