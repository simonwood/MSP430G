/*
 * XBusConfig.c
 *
 *  Created on: 3 May 2015
 *      Author: simon
 */
#include <msp430.h>
#include <stdint.h>

#include "XBusConfig.h"
#include <string.h>
#include "flash.h"


// mapping to flash address - 64 byte limit
volatile XBusConfigData* pStoredData = (XBusConfigData*)0x1040;

// data buffers for operation
volatile XBusConfigData LiveConfig;
bool bNewConfig;

/**
 * @brief	Check for new config values
 * @return	true if changed data
 */
bool HasNewConfig()
{
	return bNewConfig;
}

/**
 * @brief	Clears changed config values flag
 * @return	None
 */
void ClearConfigChange()
{
	bNewConfig = false;
}

/**
 * @brief	Get channel number for function
 * @return	true if reversed
 */
uint8_t GetFunctionChannel(RGBFunction function)
{
  return LiveConfig.nControlChannels[function];
}

void SetFunctionChannel(RGBFunction function, uint8_t nChannel)
{
  LiveConfig.nControlChannels[function] = nChannel;
}

void ResetConfig()
{
  LiveConfig.nInitBE = 0xBE;
  LiveConfig.nInitEF = 0xEF;
  
  //uint8_t nBase = 0; // left wing
  //uint8_t nBase = 10; // right wing
  //uint8_t nBase = 20; // left tail
       vuint8_t nBase = 28; // right tail
  //uint8_t nBase = 36; // Canopy
  //uint8_t nBase = 40; // Front controller
  //uint8_t nBase = 44; // left strip
  //uint8_t nBase = 48; // right strip
  
  LiveConfig.nControlChannels[RGB1_CONTROL] = nBase + 1;
  LiveConfig.nControlChannels[RGB1_RED] = nBase + 2;
  LiveConfig.nControlChannels[RGB1_GREEN] = nBase + 3;
  LiveConfig.nControlChannels[RGB1_BLUE] = nBase + 4;
  LiveConfig.nControlChannels[RGB2_CONTROL] = nBase + 5;
  LiveConfig.nControlChannels[RGB2_RED] = nBase + 6;
  LiveConfig.nControlChannels[RGB2_GREEN] = nBase + 7;
  LiveConfig.nControlChannels[RGB2_BLUE] = nBase + 8;
  LiveConfig.nControlChannels[PYRO1_FIRE] = nBase + 9;
  LiveConfig.nControlChannels[PYRO2_FIRE] = nBase + 10;
}
  
void UpdateFunctionChannel(uint8_t nCurrent, uint8_t nNew)
{
  if (nCurrent == 0)
  {
    ResetConfig();
    return;
  }
  for (int i = 0; i < sizeof(LiveConfig.nControlChannels); i++)
  {
    if (LiveConfig.nControlChannels[i] == nCurrent)
    {
      LiveConfig.nControlChannels[i] = nNew;
      break;
    }
  }
}

uint8_t IsActiveChannel(uint8_t nChannel)
{
  for (int i = 0; i < sizeof(LiveConfig.nControlChannels); i++)
  {
    if (nChannel == LiveConfig.nControlChannels[i])
    {
      return 1;
    }
  }
  return 0;
}

/**
 * @brief	Loads the live config from flash. Used to run from and determine if there are changes to write
 * @return	None
 */
void LoadConfig()
{
	memcpy((void*)&LiveConfig, (void*)pStoredData, sizeof(LiveConfig));

	if (LiveConfig.nInitBE != 0xBE || LiveConfig.nInitEF != 0xEF)
	{
          ResetConfig();
      	}
	bNewConfig = false;
}

/**
 * @brief	Stores the live config to flash. Only writes if there are deltas
 * @return	None
 * @Note	Will disable interrupts whilst writing
 */
void CommitConfig()
{
	if (memcmp((void*)pStoredData, (void*)&LiveConfig, sizeof(LiveConfig)) == 0)
	{
		// nothing to do... but we will let the user know what they have asked for
		bNewConfig = true;
		return;
	}
  
  WriteBufferFlash((uint8_t*)pStoredData, (uint8_t*)&LiveConfig, sizeof(LiveConfig));    

	bNewConfig = true;
}
