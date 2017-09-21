#ifndef	_UART_COMMUNICATION_H_
#define _UART_COMMUNICATION_H_

#include <string.h>

extern unsigned short sendlen;
extern unsigned char Buffer_1[];

void CmdDeal(unsigned char *cmd,unsigned short *cmdlen,unsigned char *cmdflag);

#endif