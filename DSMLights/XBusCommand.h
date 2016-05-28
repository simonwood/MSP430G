/*
 * XBusCommand.h
 *
 *  Created on: 3 May 2015
 *      Author: simon
 */

#ifndef INCLUDES_XBUSCOMMAND_H_
#define INCLUDES_XBUSCOMMAND_H_

void PerformCommand(const uint8_t* pXBusPacket);
void PerformStatus(const uint8_t* pXBusPacket);
void ResumeRX();
void InitTransmit();

extern bool bXBusTransmitting;



#endif /* INCLUDES_XBUSCOMMAND_H_ */
