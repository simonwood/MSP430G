// System support for MSP430

// Watchdog

/*static inline void DoWatchdog(unsigned char bFast)
{
  // Stop watchdog timer to prevent time out reset
  //WDTCTL = WDTPW + WDTHOLD;

  WDTCTL = bFast ? WDT_ARST_250 : WDT_ARST_1000;
}*/

// Delay loops
// calibrated for 8MHz clock...

//------------------------------------------------------------------------------
// Delay Loop
void Delay(void)
{ 
  unsigned int i,j;
  for(i=0;i<=10000;i++) // delay loop
    for(j=0;j<=3;j++);
}

void _Delay(unsigned int value)
{	
	if (value == 0)
	{
		return;
	}
	
    volatile unsigned int i;            // volatile to prevent optimization
    i = value;                          // SW Delay
    do
    {
    	i = i-1;
    } while (i != 0);
}

void DelayMS(unsigned int milliseconds)
{
	while (milliseconds >= 100)
	{
		_Delay(11700);
		_Delay(11700);
		_Delay(11700);
		_Delay(11700);
		milliseconds -= 100;
	}
	while (milliseconds != 0)
	{
		_Delay(117);
		milliseconds--;
	}
}

