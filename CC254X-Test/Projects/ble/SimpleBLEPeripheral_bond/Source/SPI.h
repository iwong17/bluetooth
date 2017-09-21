#ifndef _SPI_H_  
#define _SPI_H_ 

#include "hal_types.h"
  
void SPI_Init(void);  
void SPI_Poll(void);
extern void SPI_SendByte(unsigned char dat);
extern uint8 SPI_ReadByte(void);
extern signed char SPI_Write(unsigned char add, unsigned char *buf,unsigned short len);
extern signed char SPI_Read(unsigned char add, unsigned char *buf,unsigned short len);

  
#endif  