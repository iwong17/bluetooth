//******************************************************************************                              
//name:             UART_Communication.c                 
//introduce:        ͨ��Э��          
//author:                                                          
//changetime:       2017.09.21        
//******************************************************************************   
#include "UART_Communication.h"
#include <string.h>
#include "hal_types.h"
#include "npi.h"
#include "THM3070.h"
#include "App.h"
#include "osal_snv.h"//Flash��дͷ�ļ�
#include "UART_Communication.h"
#include "comdef.h"

#define Delaytime 1000

unsigned short sendlen=0;	    //���������ݳ���
unsigned char HIDsendflag=0;	//1���������ݴ�����
unsigned char HIDSendBuf[128];

uint8 MACname[maxnamelen];//�豸����
uint8 MACAddr[6];//MAC��ַΪ6�ֽ�
uint16 UARTTimeOut;//�������ʱʱ��(s)
uint16 SearchTimeOut;//Ѱ�����ʱʱ��(s)
uint8 SearchCount;
extern uint8 TimeOut_data[4];
void USBSend(void)//�������
{
	static uint8 i = 0;
	if(sendlen > 64)//������ݰ���½���ְ�����
	{
		do
		{
			NPI_WriteTransport(HIDSendBuf+i*64,64);
			sendlen-=64;
			i++;
		}while(sendlen>0);
		i = 0;
	}
	else//ֻ��һ�����ݰ�
	{
		memset(HIDSendBuf+sendlen,0,64-sendlen);
		NPI_WriteTransport(HIDSendBuf,64);
		sendlen=0;
		HIDsendflag=0;
	}
}

//��żУ��
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
				if(THM_Write(cmd+5,cmd[2]-7) == 0)//д����ɹ�����ܽ��ж�����
				{
					ret = THM_Read(HIDSendBuf+5,&sendlen);
					if(ret > 0)
					{
						ret = 0;
					}
						
				}
				else
				{
					ret = -7;//д���ʱ
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
		case 01://���ز�
		{
			THM_Init();
			THM_Open_RF();
			HIDSendBuf[0] = 'O';
			HIDSendBuf[1] = 'K';
			sendlen = 2;
			USBSend();
		}
			break;
		case 2://�ر��ز�
			THM_Close_RF();
			P1 &= ~0x08;   //�ر�LED��
			HIDSendBuf[0] = 'O';
			HIDSendBuf[1] = 'K';
			sendlen = 2;
			USBSend();
			break;
		case 3://���Ĵ���
			SPI_Read(cmd[5],buf,1);
			sendlen = 1;
			HIDSendBuf[0]=*buf;
			USBSend();
			break;
		case 4://д�Ĵ���
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
			SPI_Write(TXDP1,cmd+5,1);//����ز�ʱPMOS�����ߵ�ƽʱ���������
			SPI_Write(TXDP0,cmd+6,1);//�������ʱPMOS�����ߵ�ƽʱ���������
			SPI_Write(TXDN1,cmd+7,1);//����ز�ʱPMOS�����͵�ƽʱ���������
			SPI_Write(TXDN0,cmd+8,1);//�������ʱPMOS�����͵�ƽʱ���������
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
			THM_Init();//THM��ʼ��
			HIDSendBuf[0] = 'O';
			HIDSendBuf[1] = 'K';
			sendlen = 2;
			USBSend();
		break;
		
		case 9:

			RTSN = 1;//��λоƬ

		break;
		
		case 0x0a:
			THM_PowerDown();//�͹���
			sendlen = 2;
			HIDSendBuf[0] = 'O';
			HIDSendBuf[1] = 'K';
			USBSend();
		break;
		
		case 0x0b:
			RTSN = 0;
			Delay_ms(20);
			RTSN = 1;//��λоƬ
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
		
		case 0x58://�����豸����
			namelen = cmd[2]-7;
			if(namelen >maxnamelen) 
				break;
			memset(MACname,0,maxnamelen);//��ʼ���豸��
			uint8 i;
			for(i=0;i<namelen;i++) //
			{
				MACname[i] = cmd[5+i];
			}
			if(MACname[0] != 0)
			{
				*npEvent = SBP_UPDATE_SCAN_RSP_DATA_EVT;
			}
			
	    case 0x59://�������ʱ�����ĳ�ʱʱ�䣬����Ϊ���ֽڣ���λΪs
			UARTTimeOut = cmd[5]*256+cmd[6];
			TimeOut_data[0] = cmd[5];
			TimeOut_data[1] = cmd[6];
			TimeOut_data[2] = SearchTimeOut/256;
			TimeOut_data[3] = SearchTimeOut%256;
			SNV_TimeOutWriteRead(SNV_TimeOut_WRITE, TimeOut_data, sizeof(TimeOut_data));
		break;
		
		case 0x5A://Ѱ�����ʱ�����ĳ�ʱʱ�䣬����Ϊ���ֽڣ���λΪs
			SearchTimeOut = cmd[5]*256+cmd[6];
			TimeOut_data[0] = UARTTimeOut/256;
			TimeOut_data[1] = UARTTimeOut%256;
			TimeOut_data[2] = cmd[5];
			TimeOut_data[3] = cmd[6];
			SNV_TimeOutWriteRead(SNV_TimeOut_WRITE, TimeOut_data, sizeof(TimeOut_data));
		break;
		
		case 0x5B://��MAC��ַ
			Read_Mac(MACAddr);//MAC��ַ6�ֽ� 
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
		
		case 0x5C://дMAC��ַ
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
//introduce:        ��SNV��ȡ�������������                 
//parameter:        WriteReadFlag:SNV_TimeOut_READ or SNV_TimeOut_WRITE       
//                  np TimeOut:�����׵�ַ  
//                   Len:���볤��  
//return:           none                
//author:                                                                          
//changetime:       2017.10.09                          
//******************************************************************************  
void SNV_TimeOutWriteRead(uint8 WriteReadFlag, uint8 *TimeOut, uint8 Len)  
{  
  uint8 Ret;  
  uint8 Default_TimeOut[4];  
    
  //��SNV������  
  if(WriteReadFlag ==  SNV_TimeOut_READ)  
  {  
     Ret = osal_snv_read(SNV_TimeOut_ID, Len, TimeOut);  
  
    //�����һ�ζ�ȡʧ�ܣ���˵��û��д����ʱʱ�䣬�ǳ������豸���������������Ϊ������ʱʱ�䡣  
    if(Ret == NV_OPER_FAILED)  
    {  
      //δ����������ó�ʱʱ��
      Default_TimeOut[0] = 0;
	  Default_TimeOut[1] = 30;
	  Default_TimeOut[2] = 0;
	  Default_TimeOut[3] = 10;  
        
      //����ʱʱ��д��snv��  
      osal_snv_write(SNV_TimeOut_ID, sizeof( Default_TimeOut), Default_TimeOut);
          
      //������ʱʱ��  
      Ret = osal_snv_read(SNV_TimeOut_ID, Len, TimeOut);
    }   
  }  
  //д�µĳ�ʱʱ�䵽SNV    
  else if( WriteReadFlag == SNV_TimeOut_WRITE)
  {  
    //д���µĳ�ʱʱ�� 
    osal_snv_write(SNV_TimeOut_ID, Len, TimeOut);
      
    //������ʱʱ�� 
    Ret = osal_snv_read(SNV_TimeOut_ID, Len, TimeOut);      
  }   
} 