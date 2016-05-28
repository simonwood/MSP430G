#include "msp430g2253.h"
#include <stdint.h>

#include "msp430g2_system.h"
#include "pwm.h"

#include "LightControl.h"

#define RAND_MAX 32655
static int
do_rand(unsigned long *ctx)
{
    return ((*ctx = *ctx * 1103515245 + 12345) % ((unsigned long int)RAND_MAX + 1));
}


int
rand_r(unsigned int *ctx)
{
    unsigned long int val = (unsigned long int) *ctx;
    *ctx = do_rand(&val);
    return (int) *ctx;
}


static unsigned long int next = 1;

int
rand(void)
{
    return do_rand(&next);
}


uint16_t nTimeStep = 0;

void Sync()
{
  nTimeStep = 0;
}


void Wheel(unsigned char WheelPos, uint8_t base)
{
   if(WheelPos < 85) 
   {
     bRGBValues[base+0] = (WheelPos * 3);
     bRGBValues[base+1] = (255 - WheelPos * 3);
     bRGBValues[base+2] = (0);
   } 
   else if(WheelPos < 170) 
   {
     WheelPos -= 85;
   
     bRGBValues[base+0] = (255 - WheelPos * 3);
     bRGBValues[base+1] = (0);
     bRGBValues[base+2] = (WheelPos * 3);
   
   } 
   else 
   {
   
     WheelPos -= 170;
   
     bRGBValues[base+0] = (0);
     bRGBValues[base+1] = (WheelPos * 3);
     bRGBValues[base+2] = (255 - WheelPos * 3);    
   }
 }


enum LightMode
{
  LM_DIM_WHITE = 0,
  LM_BRIGHT_WHITE = 1,
  LM_COLOR = 2,
  LM_STROBE = 3,
  LM_POLICE = 4,
  LM_WHEEL = 5,
  LM_RANDOM = 7,
  LM_DMX = 8,
  
  LM_UNKNOWN = 100
};

typedef struct ModePoint
{
  uint8_t Value;
  LightMode Mode;  
} ModePoint;

ModePoint ModeArray[] = 
{
  { 35,  LM_BRIGHT_WHITE },
  { 50,  LM_DIM_WHITE },
  { 70,  LM_COLOR },
  { 90,  LM_STROBE },
  { 110, LM_POLICE },
  { 130, LM_WHEEL },
  { 150, LM_RANDOM },
};

LightMode CurrentMode = LM_BRIGHT_WHITE;

uint8_t ModeData = 0;
void ProcessLightMode(uint8_t nMode, uint8_t nData)
{
  uint8_t i;
  LightMode Mode = LM_UNKNOWN;
  for (i = 0; i < sizeof(ModeArray)/sizeof(ModePoint); i++)
  {
    if (nMode < ModeArray[i].Value)
    {
      Mode = ModeArray[i].Mode;
      break;
    }
  }   
  
  if (Mode != CurrentMode)
  {
    Sync();
    CurrentMode = Mode;
  }
  
  ModeData = nData;
}


void Top(uint8_t r, uint8_t g, uint8_t b)
{
  bRGBValues[3] = RCScale(r);
  bRGBValues[4] = RCScale(g);
  bRGBValues[5] = RCScale(b);
}
void Bottom(uint8_t r, uint8_t g, uint8_t b)
{
  bRGBValues[0] = RCScale(r);
  bRGBValues[1] = RCScale(g);
  bRGBValues[2] = RCScale(b);
}

void Airframe(uint8_t r, uint8_t g, uint8_t b)
{
  Top(r,g,b);
  Bottom(r,g,b);
}

void DoWhite(uint8_t nBrightness)
{
  Airframe(nBrightness, nBrightness, nBrightness);
}

void DoColor(int strobe)
{
  uint8_t r,g,b;
  ColourWheel(ModeData, r, g, b);
  
  if (strobe)
  {
    if (nTimeStep < 8)
    {
      r = g = b = 0;
    }
    if (nTimeStep >=9)
    {
      nTimeStep = 0;
    }
  }
  Airframe(r,g,b);
}

void Police_Light(uint8_t r, uint8_t b)
{
  Top(r, 0, b);

  Bottom(b, 0, r);
}

void Police()
{
  if (nTimeStep < 5)
  {
    Police_Light(0xff, 0);
  }  
  else if (nTimeStep < 15)
  {
    Police_Light(0, 0);
  }
  else if (nTimeStep < 20)
  {
    Police_Light(0xff, 0);
  }  
  else if (nTimeStep < 30)
  {
    Police_Light(0, 0);
  }
  else if (nTimeStep < 35)
  {
    Police_Light(0, 0xff);
  }  
  else if (nTimeStep < 45)
  {
    Police_Light(0, 0);
  }
  else if (nTimeStep < 50)
  {
    Police_Light(0, 0xff);
  }  
  else if (nTimeStep < 60)
  {
    Police_Light(0, 0);
  }
  else
  {
    nTimeStep = 0;
  }
}

void Wheel()
{
  if (nTimeStep >= 512)
  {
    nTimeStep = 1;
  }
  
  if (ModeData < 128)
  {
    uint8_t r,g,b;
    ColourWheel(nTimeStep/2, r, g, b);  
  
    Airframe(r,g,b);
  }
  else
  {
    uint8_t r,g,b;
    ColourWheel(nTimeStep/2, r, g, b);  
    
    Top(r,g,b);
    
    ColourWheel((nTimeStep/2)+128, r, g, b);  
    
    Bottom(r,g,b);
  }
}

void Random()
{
  if (nTimeStep < 3)
  {
    return;
  }
  if (rand()%20 > 1)
  {
    return;
  }
  uint8_t r,g,b;
  ColourWheel(rand()%256, r, g, b);  
  
  switch(rand()%2)
  {
    case 0: Top(r,g,b); break;
    case 1: Bottom(r,g,b); break;
  }
  nTimeStep = 0;
}

void StepLights()
{
  // called every 10 ms
  nTimeStep++;
  
  switch (CurrentMode)
  {
    case LM_DIM_WHITE:    DoWhite(0x15); break;
    case LM_BRIGHT_WHITE: DoWhite(0xff); break;
    case LM_COLOR:        DoColor(0); break;
    case LM_STROBE:       DoColor(1); break;
    case LM_POLICE:       Police(); break;
    case LM_WHEEL:        Wheel(); break;
    case LM_RANDOM:       Random(); break;
  }
}
