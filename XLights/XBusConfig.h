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
	RGB1_CONTROL = 0,
	RGB1_RED = 1,
	RGB1_GREEN = 2,
	RGB1_BLUE = 3,
	RGB2_CONTROL = 4,
	RGB2_RED = 5,
	RGB2_GREEN = 6,
	RGB2_BLUE = 7,
        PYRO1_FIRE = 8,
        PYRO2_FIRE = 9
} RGBFunction;


#pragma pack(1)
typedef struct
{
	uint8_t nInitBE;
	uint8_t nInitEF;

	uint8_t nControlChannels[10];
} XBusConfigData;
#pragma pack()

bool HasNewConfig();
void ClearConfigChange();
void SetFunctionChannel(RGBFunction function, uint8_t nChannel);
uint8_t GetFunctionChannel(RGBFunction function);
void UpdateFunctionChannel(uint8_t nCurrent, uint8_t nNew);
void LoadConfig();
void CommitConfig();
uint8_t IsActiveChannel(uint8_t nChannel);

#endif /* INCLUDES_XBUSCONFIG_H_ */
