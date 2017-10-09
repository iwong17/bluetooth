#ifndef	_UART_COMMUNICATION_H_
#define _UART_COMMUNICATION_H_

#include <string.h>
#include "hal_types.h"

#define SBP_UPDATE_SCAN_RSP_DATA_EVT                  0x0080  //���Ĺ㲥�豸���¼�
#define maxnamelen 20

//snvʹ��  
#define SNV_TimeOut_READ           0x00  
#define SNV_TimeOut_WRITE          0x01  
#define SNV_TimeOut_ID             0x81

extern unsigned short sendlen;
extern unsigned char Buffer_1[];

void CmdDeal(unsigned char *cmd,unsigned short *cmdlen,unsigned char *cmdflag, uint16 *npEvent);

void SNV_TimeOutWriteRead(uint8 WriteReadFlag, uint8 *TimeOut, uint8 Len);

#endif