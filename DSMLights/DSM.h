#ifndef __DSM_H
#define __DSM_H

void initDSM();
void checkBind();
unsigned char Channel(unsigned char nChan);
unsigned char NewPacket();
void ClearPacket();
void ParsePacket();


#endif