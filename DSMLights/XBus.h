#ifndef __XBUS_H
#define __XBUS_H

void initXBus();
unsigned char Channel(unsigned char nChan);
unsigned char NewPacket();
void ClearPacket();
void ParsePacket();


// XBus frame - 8 channel
#pragma pack(1)
typedef struct XBusChannel
{
  uint8_t Id;
  uint8_t Function;
  uint8_t High;
  uint8_t Low;
} XBusChannel;
#pragma pack()

#pragma pack(1)
typedef struct XBusFrame
{
  uint8_t Command;
  uint8_t Length;
  uint8_t Key;
  uint8_t Type;
  XBusChannel Channels[16];
  uint8_t Quality;
  uint8_t Other;
  uint8_t Crc;
} XBusFrame;
#pragma pack()

#pragma pack(1)
typedef struct XBusDMX64
{
  uint8_t Command;
  uint8_t Length; // always 64!
  uint8_t DMXData[64];
  uint8_t Crc;
} XBusDMX64;
#pragma pack()


#endif