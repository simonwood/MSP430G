/*
 * XBusConfig.h
 *
 *  Created on: 3 May 2015
 *      Author: simon
 */

#ifndef INCLUDES_XBUSCONFIG_H_
#define INCLUDES_XBUSCONFIG_H_

typedef enum
{
  PYRO1_FIRE = 0,
  PYRO2_FIRE = 1,
  PYRO3_FIRE = 2,
  PYRO4_FIRE = 3
} PyroFunction;


#pragma pack(1)
typedef struct 
{
	uint8_t nInitBE;
	uint8_t nInitEF;

	uint8_t nControlChannels[4];
} XBusConfigData;
#pragma pack()

bool HasNewConfig();
void ClearConfigChange();
void SetFunctionChannel(PyroFunction function, uint8_t nChannel);
uint8_t GetFunctionChannel(PyroFunction function);
void UpdateFunctionChannel(uint8_t nCurrent, uint8_t nNew);
void LoadConfig();
void CommitConfig();
uint8_t IsActiveChannel(uint8_t nChannel);

#endif /* INCLUDES_XBUSCONFIG_H_ */
