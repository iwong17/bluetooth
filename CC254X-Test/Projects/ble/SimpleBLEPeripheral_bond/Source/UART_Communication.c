//******************************************************************************                              
//name:             UART_Communication.c                 
//introduce:        通信协议          
//author:                                                          
//changetime:       2017.09.21        
//******************************************************************************   
#include "UART_Communication.h"
#include <string.h>
#include "hal_types.h"
#include "npi.h"
#include "THM3070.h"
#include "App.h"
#include "osal_snv.h"//Flash读写头文件
#include "UART_Communication.h"
#include "comdef.h"

#define Delaytime 1000

unsigned short sendlen=0;	    //待发送数据长度
unsigned char HIDsendflag=0;	//1：还有数据待发送
unsigned char HIDSendBuf[128];

uint8 MACname[maxnamelen];//设备名称
uint8 MACAddr[6];//MAC地址为6字节
uint16 UARTTimeOut;//串口命令超时时间(s)
uint16 SearchTimeOut;//寻卡命令超时时间(s)
uint8 SearchCount;
extern uint8 TimeOut_data[4];
void USBSend(void)//结果发送
{
	static uint8 i = 0;
	if(sendlen > 64)//多个数据包则陆续分包发送
	{
		do
		{
			NPI_WriteTransport(HIDSendBuf+i*64,64);
			sendlen-=64;
			i++;
		}while(sendlen>0);
		i = 0;
	}
	else//只有一个数据包
	{
		memset(HIDSendBuf+sendlen,0,64-sendlen);
		NPI_WriteTransport(HIDSendBuf,64);
		sendlen=0;
		HIDsendflag=0;
	}
}

//奇偶校验
unsigned char Check(unsigned char *msg,unsigned short len)
{
	unsigned char ret = 0;
	
	while(len--)
		ret ^= msg[len];
	return ret;
}

void CmdDeal(unsigned char *cmd,unsigned short *len,unsigned char *cmdflag, uint16 *npEvent)
{
	unsigned char buf[256];
	uint8 namelen;
	static uint8 times = 0;
	if(*cmdflag == 0)
		return ;
	*cmdflag=0;
	SearchCount = SearchTimeOut*1000/Delaytime;
	
	if(Check(cmd,cmd[2]-1) != 0 || cmd[cmd[2]-1] != 3)return;
	
	switch(cmd[4])//Parameter
	{
		case 00:
		{
			signed char ret;
			do
			{
				if(THM_Write(cmd+5,cmd[2]-7) == 0)//写命令成功后才能进行读操作
				{
					ret = THM_Read(HIDSendBuf+5,&sendlen);
					if(ret > 0)
					{
						ret = 0;
					}
						
				}
				else
				{
					ret = -7;//写命令超时
				} 
				Delay_ms(Delaytime);
				times++;
				NPI_WriteTransport(&times,1);
			}while(ret<0 && times<SearchCount);  
			times = 0;
			
			HIDSendBuf[0] = 0x02;
			HIDSendBuf[1] = (sendlen + 8) >> 8;
			HIDSendBuf[2] = sendlen + 8;
			HIDSendBuf[3] = 0x01;
			HIDSendBuf[4] = 0x00;
			HIDSendBuf[5+sendlen] = (unsigned char)ret;
			HIDSendBuf[6+sendlen] = Check(HIDSendBuf,6+sendlen);
			HIDSendBuf[7+sendlen] = 0x03;
			sendlen += 8;
			USBSend();
                     
			break;
		}
		case 01://打开载波
		{
			THM_Init();
			THM_Open_RF();
			HIDSendBuf[0] = 'O';
			HIDSendBuf[1] = 'K';
			sendlen = 2;
			USBSend();
		}
			break;
		case 2://关闭载波
			THM_Close_RF();
			P1 &= ~0x08;   //关闭LED灯
			HIDSendBuf[0] = 'O';
			HIDSendBuf[1] = 'K';
			sendlen = 2;
			USBSend();
			break;
		case 3://读寄存器
			SPI_Read(cmd[5],buf,1);
			sendlen = 1;
			HIDSendBuf[0]=*buf;
			USBSend();
			break;
		case 4://写寄存器
			SPI_Write(cmd[5],cmd+6,1);
			SPI_Read(cmd[5],buf,1);
			sendlen = 1;
			HIDSendBuf[0]=*buf;
			USBSend();
			break;
		case 5:
			SPI_Write(0,"123456789012345678901111111111",30);//THM_ClrBuf();
			Delay_ms(10);
			SPI_Read(0,buf,30);
			sendlen = 30;
			memcpy(HIDSendBuf,buf,sendlen);
			USBSend();
			break;
		case 6:
			SPI_Write(TXDP1,cmd+5,1);//输出载波时PMOS驱动高电平时的输出电阻
			SPI_Write(TXDP0,cmd+6,1);//输出调制时PMOS驱动高电平时的输出电阻
			SPI_Write(TXDN1,cmd+7,1);//输出载波时PMOS驱动低电平时的输出电阻
			SPI_Write(TXDN0,cmd+8,1);//输出调制时PMOS驱动低电平时的输出电阻
			sendlen = 2;
			HIDSendBuf[0] = 'O';
			HIDSendBuf[1] = 'K';
			USBSend();
		break;
		case 7:
			sendlen = 2;
			HIDSendBuf[0] = 'O';
			HIDSendBuf[1] = 'K';
			USBSend();
		break;
		
		case 8:
			THM_Init();//THM初始化
			HIDSendBuf[0] = 'O';
			HIDSendBuf[1] = 'K';
			sendlen = 2;
			USBSend();
		break;
		
		case 9:

			RTSN = 1;//复位芯片

		break;
		
		case 0x0a:
			THM_PowerDown();//低功耗
			sendlen = 2;
			HIDSendBuf[0] = 'O';
			HIDSendBuf[1] = 'K';
			USBSend();
		break;
		
		case 0x0b:
			RTSN = 0;
			Delay_ms(20);
			RTSN = 1;//复位芯片
			THM_Init();
			THM_Open_RF();
			sendlen = 2;
			HIDSendBuf[0] = 'O';
			HIDSendBuf[1] = 'K';
			USBSend();
		break;
		
		case 0x56:
			#define Ver "NUC122_V1.0-ff"
			sendlen = sizeof(Ver)-1;
			memcpy(HIDSendBuf,Ver,sendlen);
			USBSend();
			#undef Ver
			break;
		
		case 0x58://更改设备名称
			namelen = cmd[2]-7;
			if(namelen >maxnamelen) 
				break;
			memset(MACname,0,maxnamelen);//初始化设备名
			uint8 i;
			for(i=0;i<namelen;i++) //
			{
				MACname[i] = cmd[5+i];
			}
			if(MACname[0] != 0)
			{
				*npEvent = SBP_UPDATE_SCAN_RSP_DATA_EVT;
			}
			
	    case 0x59://串口命令超时：更改超时时间，长度为两字节，单位为s
			UARTTimeOut = cmd[5]*256+cmd[6];
			TimeOut_data[0] = cmd[5];
			TimeOut_data[1] = cmd[6];
			TimeOut_data[2] = SearchTimeOut/256;
			TimeOut_data[3] = SearchTimeOut%256;
			SNV_TimeOutWriteRead(SNV_TimeOut_WRITE, TimeOut_data, sizeof(TimeOut_data));
		break;
		
		case 0x5A://寻卡命令超时：更改超时时间，长度为两字节，单位为s
			SearchTimeOut = cmd[5]*256+cmd[6];
			TimeOut_data[0] = UARTTimeOut/256;
			TimeOut_data[1] = UARTTimeOut%256;
			TimeOut_data[2] = cmd[5];
			TimeOut_data[3] = cmd[6];
			SNV_TimeOutWriteRead(SNV_TimeOut_WRITE, TimeOut_data, sizeof(TimeOut_data));
		break;
		
		case 0x5B://读MAC地址
			Read_Mac(MACAddr);//MAC地址6字节 
			sendlen = 6;
			HIDSendBuf[0] = 0x02;
			HIDSendBuf[1] = (sendlen + 7) >> 8;
			HIDSendBuf[2] = sendlen + 7;
			HIDSendBuf[3] = 0x01;
			HIDSendBuf[4] = 0x5B;
			memcpy(HIDSendBuf+5,MACAddr,6);
			HIDSendBuf[5+sendlen] = Check(HIDSendBuf,5+sendlen);
			HIDSendBuf[6+sendlen] = 0x03;
			sendlen += 7;
			USBSend();
		break;
		
		case 0x5C://写MAC地址
			MACAddr[0] = cmd[5];
			MACAddr[1] = cmd[6];
			MACAddr[2] = cmd[7];
			MACAddr[3] = cmd[8];
			MACAddr[4] = cmd[9];
			MACAddr[5] = cmd[10];
			Write_Mac(MACAddr);
			HIDSendBuf[0] = 'O';
			HIDSendBuf[1] = 'K';
			sendlen = 2;
			USBSend();
		break;
		
		default:
		break;
	}
}

//******************************************************************************                    
//name:             SNV_TimeOutWriteRead                   
//introduce:        从SNV读取或更新连接密码                 
//parameter:        WriteReadFlag:SNV_TimeOut_READ or SNV_TimeOut_WRITE       
//                  np TimeOut:密码首地址  
//                   Len:密码长度  
//return:           none                
//author:                                                                          
//changetime:       2017.10.09                          
//******************************************************************************  
void SNV_TimeOutWriteRead(uint8 WriteReadFlag, uint8 *TimeOut, uint8 Len)  
{  
  uint8 Ret;  
  uint8 Default_TimeOut[4];  
    
  //从SNV读密码  
  if(WriteReadFlag ==  SNV_TimeOut_READ)  
  {  
     Ret = osal_snv_read(SNV_TimeOut_ID, Len, TimeOut);  
  
    //如果第一次读取失败，则说明没有写过超时时间，是出厂的设备。因此在这里设置为出厂超时时间。  
    if(Ret == NV_OPER_FAILED)  
    {  
      //未保存过，设置超时时间
      Default_TimeOut[0] = 0;
	  Default_TimeOut[1] = 30;
	  Default_TimeOut[2] = 0;
	  Default_TimeOut[3] = 10;  
        
      //将超时时间写入snv中  
      osal_snv_write(SNV_TimeOut_ID, sizeof( Default_TimeOut), Default_TimeOut);
          
      //读出超时时间  
      Ret = osal_snv_read(SNV_TimeOut_ID, Len, TimeOut);
    }   
  }  
  //写新的超时时间到SNV    
  else if( WriteReadFlag == SNV_TimeOut_WRITE)
  {  
    //写进新的超时时间 
    osal_snv_write(SNV_TimeOut_ID, Len, TimeOut);
      
    //读出超时时间 
    Ret = osal_snv_read(SNV_TimeOut_ID, Len, TimeOut);      
  }   
} 