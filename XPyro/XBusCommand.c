/*
 * XBusCommand.c
 *
 *  Created on: 3 May 2015
 *      Author: simon
 */

#include <msp430.h>
#include <stdint.h>

#include "XBusCommand.h"
#include "XBusConfig.h"
#include <string.h>
#include "XBusCRC.h"
#include "pwm.h"

bool bXBusTransmitting = false;

// forward declarations for internal functions
void SendResponse();

// XBus definitions
#pragma pack(1)
typedef struct XBusCommand
{
  uint8_t Command;
  uint8_t Length;
  uint8_t Key;
  uint8_t DeviceID;
  uint8_t Order;
  uint8_t Data1;
  uint8_t Data2_Crc;
  uint8_t Crc;
} XBusCommand;
#pragma pack()

typedef enum XBusCommandIDs
{
	XBus_Set = 0x20,
	XBus_Get = 0x21,
	XBus_Status = 0x22,
	XBus_Data = 0xA4,
} XBusCommandIDs;

typedef enum XBusOrderID
{
    XBus_Order_Mode =             0x01,
    XBus_Order_ID =               0x03,
    XBus_Order_Version =          0x04,           // only for get
    XBus_Order_Product =          0x05,           // only for get
    XBus_Order_Unsupported =      0x06,           // only for status
    XBus_Order_Reset =            0x07,           // only for set
    XBus_Order_ParamWrite =       0x08,           // only for set
    XBus_Order_Reverse =          0x10,
    XBus_Order_Neutral =          0x11,
    XBus_Order_H_Travel =         0x12,
    XBus_Order_L_Travel =         0x13,
    XBus_Order_H_Limit =          0x14,
    XBus_Order_L_Limit =          0x15,
    XBus_Order_P_Gain =           0x16,
    XBus_Order_I_Gain =           0x17,
    XBus_Order_D_Gain =           0x18,
    XBus_Order_DeadBand =         0x19,
    XBus_Order_PowerOffset =      0x1A,
    XBus_Order_AlarmLevel =       0x1B,
    XBus_Order_AlarmDelay =       0x1C,
    XBus_Order_Angle_180 =        0x1D,
    XBus_Order_SlowStart =        0x1E,
    XBus_Order_StopMode =         0x1F,
    XBus_Order_CurrentPos =       0x20,           // only for get
    XBus_Order_CurrentPow =       0x21,           // only for get
    XBus_Order_SpeedLimit =       0x22,
    XBus_Order_MaxInteger =       0x23,
} XBusOrderID;

typedef enum
{
	XBus_ParamIdx_Unused0 =             0x0000,
	XBus_ParamIdx_AllData_wID =         0x0001,
	XBus_ParamIdx_AllData_woID =        0x0002,
	XBus_ParamIdx_ServoID =             0x0003,
	XBus_ParamIdx_Reversed =            0x0004,
	XBus_ParamIdx_NeutralOffset =       0x0005,
	XBus_ParamIdx_TravelHigh =          0x0006,
	XBus_ParamIdx_TravelLow =           0x0007,
	XBus_ParamIdx_LimitHigh =           0x0008,
	XBus_ParamIdx_LimitLow =            0x0009,
	XBus_ParamIdx_PGainDiff =           0x000A,
	XBus_ParamIdx_IGainDiff =           0x000B,
	XBus_ParamIdx_DGainDiff =           0x000C,
	XBus_ParamIdx_DeadBandDiff =        0x000D,
	XBus_ParamIdx_PWOffsetDiff =        0x000E,
	XBus_ParamIdx_AlarmLevel =          0x000F,
	XBus_ParamIdx_AlarmDelay =          0x0010,
	XBus_ParamIdx_Angle_180 =           0x0011,
	XBus_ParamIdx_SlowStart =           0x0012,
	XBus_ParamIdx_StopMode =            0x0013,
	XBus_ParamIdx_SpeedLimit =          0x0014,
    XBus_ParamIdx_MaxIntegerDiff =      0x0015,
} XBusParamIdx;

// Data buffers for transmit operation
XBusCommand XBusResponse;
uint8_t* pXBusResponse = (uint8_t*)&(XBusResponse) ;
uint8_t nRawResponseIndex = 10;

bool IsSingleByte(XBusOrderID eID)
{
	switch (eID)
	{
	case XBus_Order_Mode:
	case XBus_Order_ID:
	case XBus_Order_Unsupported:
	case XBus_Order_P_Gain:
	case XBus_Order_I_Gain:
	case XBus_Order_D_Gain:
	case XBus_Order_DeadBand:
	case XBus_Order_AlarmLevel:
	case XBus_ParamIdx_Angle_180:
	case XBus_Order_SlowStart:
	case XBus_Order_StopMode:
	case XBus_Order_CurrentPow:
	case XBus_Order_SpeedLimit:
		return true;
  default:
    return false;
	}
	return false;
}

/**
 * @brief	 Sets up the response packet for transmission back to the TX
 * @return	None
 */
void InitResponse(uint8_t nDeviceID, uint8_t nKey, XBusOrderID eID, uint8_t Data1, uint8_t Data2)
{
	XBusResponse.Command = XBus_Status;
	XBusResponse.Length = 5;
	XBusResponse.Key = nKey;
	XBusResponse.DeviceID = nDeviceID;
	XBusResponse.Order = eID;
	XBusResponse.Data1 = Data1;
	XBusResponse.Data2_Crc = Data2;

	if (IsSingleByte(eID))
	{
		XBusResponse.Length = 4;
		XBusResponse.Data2_Crc = crc8((uint8_t*)&XBusResponse, XBusResponse.Length+2);
	}
	else
	{
		XBusResponse.Crc = crc8((uint8_t*)&XBusResponse, XBusResponse.Length+2);
	}

	pXBusResponse = (uint8_t*)&(XBusResponse) ;
	nRawResponseIndex = 0;
}


void CommitConfig();

/**
 * @brief	Handles the set (ParamWrite) command to store changes to flash
 * @Param	nParamIdx - the XBus parameter index to save
 * @return	None
 */
void DoSet(uint8_t nDeviceID, uint8_t nKey, uint8_t nParamIdx)
{
	// response packet
	if (nParamIdx == XBus_ParamIdx_Reversed)
	{
		CommitConfig();
		InitResponse(nDeviceID, nKey, XBus_Order_ParamWrite, 0, XBus_ParamIdx_Reversed);
	}
	else
	{
		InitResponse(nDeviceID, nKey, XBus_Order_Unsupported, 0, 0);
	}
}

void DoIDChange(uint8_t nKey, uint8_t nCurrentID, uint8_t nNewID)
{
  UpdateFunctionChannel(nCurrentID & 0x4F, nNewID & 0x4F);
  CommitConfig();
  InitResponse(nCurrentID, nKey, XBus_Order_ID, nNewID, 0);
}

/**
 * @brief	Parses incoming command packet from RX and actions as appropriate
 * @Param	pXBusPacket - pointer to a validated XBus packet
 * @return	None
 */
void PerformCommand(const uint8_t* pXBusPacket)
{
	const XBusCommand* pCommand = (const XBusCommand*)pXBusPacket;

	if (!IsActiveChannel(pCommand->DeviceID) &&
		pCommand->DeviceID != 0)
	{
		// not ID 1.1
		return;
	}
	switch (pCommand->Order)
	{
		case XBus_Order_Mode: InitResponse(pCommand->DeviceID, pCommand->Key, XBus_Order_Mode, 2, 0); break;
		case XBus_Order_ID: DoIDChange(pCommand->Key, pCommand->DeviceID, pCommand->Data1); break;
		case XBus_Order_ParamWrite: DoSet(pCommand->DeviceID, pCommand->Key, pCommand->Data2_Crc); break;

		default: InitResponse(pCommand->DeviceID, pCommand->Key, XBus_Order_Unsupported, 0, 0); break;
	}
  
  SendResponse();
}

/**
 * @brief	Parses incomming command packet from RX (get command - 0x21) and act
 * @Param	pXBusPacket - pointer to a validaated XBus packet
 * @return	None
 */
void PerformStatus(const uint8_t* pXBusPacket)
{
	const XBusCommand* pCommand = (const XBusCommand*)pXBusPacket;

	if (!IsActiveChannel(pCommand->DeviceID))
	{
		return;
	}
	switch (pCommand->Order)
	{
		case XBus_Order_Version: InitResponse(pCommand->DeviceID, pCommand->Key, XBus_Order_Version, 1, 0); break;
		case XBus_Order_Product: InitResponse(pCommand->DeviceID, pCommand->Key, XBus_Order_Product, 0, 0); break;

		default:  InitResponse(pCommand->DeviceID, pCommand->Key, XBus_Order_Unsupported, 0, 0);
	}

  SendResponse();
}

/**
 * @brief	Switches the hardware to transmit mode
 * @return	None
 */
void InitTransmit()
{
	bXBusTransmitting = true;

  P1DIR = 0; // all P1 as input.  
  P1SEL = BIT2 ; // P1.2 = TXD,
  P1SEL2 = BIT2 ; // P1.2 = TXD, 
}

/**
 * @brief	Switches the hardware back to receive mode.
 * @return	None
 */
void ResumeRX()
{
  
  /* Configure hardware UART */
  P1DIR = 0; // all P1 as input.  
  P1SEL = BIT1 ; // P1.1 = RXD,
  P1SEL2 = BIT1 ; // P1.1 = RXD, 
	bXBusTransmitting = false;
}

void Delay100uS(uint8_t nTimes)
{
  volatile int nLoop = 120;
  while (nTimes != 0)
  {
    while (nLoop > 0)
    {
      nLoop--;
    }
    nLoop = 120;
    nTimes--;
  }
}

void SendResponse()
{
  InitTransmit();
  
  //Delay100uS(7);
  
  // feed XBus bytes...
	while (nRawResponseIndex < (pXBusResponse[1]+3))
	{
    // wait for empty buffer...
    while ((IFG2 & UCA0TXIFG) == 0)
    {
      ; // spin - wait for data to go
    }
    UCA0TXBUF = pXBusResponse[nRawResponseIndex++];
	}  
  // wait for end of TX.
  while ((UCA0STAT & UCBUSY) == UCBUSY)
  {
    ; // spin - wait for data to go
  }  
  ResumeRX();
}


