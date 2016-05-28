/**************************************************
 *
 * IAR EMBEDDED WORKBENCH TUTORIAL
 * Header file for EC++ tutorial containing
 * the declaration of the class "fibonacci"
 *
 * Copyright (C) 1996-2009 IAR Systems AB.
 *
 * $Revision: 5151 $
 *
 **************************************************/

#include <iostream>

class fibonacci
{
public:
  // A constructor.
  // This is called when a fibonacci object is created.
  // (A constructor must have the same name as the class.)
  //
  // Note that this member is declared inline (i.e. with the
  // definition of the function here rather than in "Fibonacci.cpp").
  fibonacci()
    : current(1)
  {
    cout << "A fibonacci object was created." << endl;
  }


  // A second constructor.
  // In EC++ it is possible to define several constructors and functions
  // with the same name as long as they can be separated by their signature.
  // (A "signature" is the type of and the number of arguments the
  // function accepts.)
  fibonacci(int n)
    : current(n)
  {
    cout << "A fibonacci object that starts at fibonacci number "
         << n << " was created." << endl;
  }


  // A destructor.
  // This function is called when a fibonacci object
  // is deleted or when it runs out of scope.
  ~fibonacci()
  {
    cout << "A fibonacci object was destroyed." << endl;
  }


  // Function: next
  //
  // Return the next fibonacci number for this object.
  //
  // This function can only be called when applied to a fibonacci
  // object or pointer to object.
  //
  // For example:
  //    fib.next()     or     fib_pointer->next()
  unsigned long int next();


  // Function: nth
  //
  // Return the n:th fibonacci number.
  //
  // Note that this function is declared "static", which means
  // that you don't need an object to call it.
  //
  // For example: fibonacci::nth(10);
  static unsigned long int nth(int nr);


protected:
  // The current fibonacci number, used by the function "next" to know
  // which number it should return the next time it is called.
  //
  // Each fibonacci object has it's own "current" value.
  int current;

  // The following two variables construct a cache containing
  // already calculated fibonacci numbers.
  //
  // Since they are declared "static" they are shared between all
  // fibonacci objects.
  static int initialized;
  static unsigned long int root[];

  // The largest number of entries in the root array described above.
  //
  // A variable declared as both static and const is similar to an
  // old-style preprocessor constant.  However, in EC++ this
  // construction is preferred since proper typechecking is performed.
  static const int max_fib = 100;
};
