/**************************************************
 *
 * IAR EMBEDDED WORKBENCH TUTORIAL
 * Utility header file
 *
 * Copyright (C) 1996-2009 IAR Systems AB.
 *
 * $Revision: 8176 $
 *
 **************************************************/

#define MAX_FIB 10

//-----------------------------------------------------------
// when running the tutorials using the DLIB libs you have
// to use the unbuffered __putchar function instead of the
// buffered putchar. In this case 'activate' the lines
// below.
#if 0
#include <yfuns.h>
#define putchar __putchar
#endif
//-----------------------------------------------------------

void InitFib( void );
unsigned int GetFib( int nr );
void PutFib( unsigned int out );

