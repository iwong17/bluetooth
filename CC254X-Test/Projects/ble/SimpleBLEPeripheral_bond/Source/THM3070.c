#define _THM3070_C_
#include "THM3070.h"
#undef _THM3070_C_

#include "hal_mcu.h"
#include "npi.h"

#define RTSN                            P1_0     
#define STANDBY                         P1_1  
#define MOD0                            P1_2

//打开射频
void THM_Open_RF(void)
{
  SPI_Write(TXCON,"\x62",1);
  Delay_ms(100);   //时间长度可以延长	Delay_ms(2)；
}

	
//关闭射频
void THM_Close_RF(void)
{
  SPI_Write(TXCON,"\x65",1);
  Delay_ms(2);
}

void THM_CloseCarrier(void)
{
	SPI_Write(SCON,"\x04",1);//收发缓冲区清空，收发接收停止，关闭载波
}

void THM_OpenCarrier(void)
{
	SPI_Write(SCON,"\x03",1);//收发缓冲区清空，收发接收开始，打开载波
}

//设置超时时间
static void SetFWT(unsigned long ulFWT)
{
	if(ulFWT == 0)ulFWT = 1;
	SPI_Write(FWTHIGH,((unsigned char *)&ulFWT)+2,1);
	SPI_Write(FWTMID,((unsigned char *)&ulFWT)+1,1);
	SPI_Write(FWTLOW,((unsigned char *)&ulFWT),1);
}
//设置速率及协议
static void ChangeProtBaud(unsigned char prot, unsigned char sndbaud, unsigned char rcvbaud)
{
	prot |= sndbaud | rcvbaud;
	SPI_Write(PSEL,&prot,1);
}

void THM_PowerDown(void)
{
	STANDBY = 1;
}



void THM_Init(void)
{
	RTSN = 1;//SPIRST HIGH
	
    STANDBY = 0;//初始化为非低功耗模式 LOW
    MOD0 = 0;//使用SPI接口 LOW
	Delay_ms(10);
	RTSN = 0;//SPIRST LOW
	Delay_ms(10);
	RTSN = 1;//SPIRST HIGH
	Delay_ms(50);
	
	SPI_Write(FCONB,"\x2a",1);
	SPI_Write(EGT,"\x40",1);
	SPI_Write(CRCSEL,"\xc1",1);
	SPI_Write(INTCON,"\x03",1);
	SPI_Write(EMVEN,"\xfd",1);
	SPI_Write(TXCON,"\x63",1);
	SPI_Write(RXCON,"\x41",1);
	SPI_Write(TR0MINH,"\x04",1);
	SPI_Write(TR0MINL,"\x20",1);
	SPI_Write(TR1MINH,"\x04",1);
	SPI_Write(TR1MINL,"\x4A",1);
    //SPI_Write(TXDP1,"\x48",1);						//0x48
	//SPI_Write(TXDP0,"\x45",1);						//0x45
	//SPI_Write(TXDN1,"\x38",1);						//0x38
	//SPI_Write(TXDN0,"\x1B",1);   						//0x1B
	SPI_Write(TXDP1,"\xff",1);
	SPI_Write(TXDP0,"\x50",1);
	SPI_Write(TXDN1,"\x7f",1);
	SPI_Write(TXDN0,"\x20",1);
        
	SetFWT(1500);
	ChangeProtBaud(TYPE_B,SND_BAUD_106K,RCV_BAUD_106K);	
	THM_ClrBuf();
}

//清空缓冲区
void THM_ClrBuf(void)
{
	unsigned char temp;
	
	/*SPI_Read(SCON,&temp,1);
	temp |= 4;
	SPI_Write(SCON,&temp,1);
	temp &= ~4;
	SPI_Write(SCON,&temp,1);
	
	SPI_Write(RSTAT,"\x00",1);*/
	
	temp = 5;//收发缓冲区清空 打开载波
	SPI_Write(SCON,&temp,1);
	Delay_ms(5);
	temp = 1;//打开射频载波
	SPI_Write(SCON,&temp,1);
}

//读取缓冲区数据，返回读到的长度
signed char THM_Read(unsigned char *buf,unsigned short *len)
{
	unsigned char temp = 0;
	unsigned long time = 100000;//240000000;//超时时间，48M为秒级
	
	*len = 0;
	while ((temp==0) && --time)//判断	RSTAT 寄存器的最高位是否置位，最高位的置位比其他位的置位晚。加上超时防止死机
	{ 
		SPI_Read(RSTAT,&temp,1);//读取接收状态          
    }
	//printf("zhuangtai :%x\n",temp); 
	if(time == 0)//超时，主控芯片判断的不精确超时，超时时间为秒级
	{
		return -6;
	}
	if(temp & 0x01)//正确
	{		    
		SPI_Read(RSCL,&temp,1);//发送低字节数
		*len = temp;
		SPI_Read(RSCH,&temp,1);//发送高字节数
		*len += temp*256;
		if(*len == 0)
			return 0; 
		SPI_Read(0,buf,*len);	
		return *len;          //*len += (unsigned short)temp << 8;
	}
	else if(temp & 4)//超时				
		return (signed char)-1;
	else if(temp & 8)//溢出
		return (signed char)-2;
	else if(temp & 2)//CRC错误
		return (signed char)-3;
	else if(temp & 16)
		return (signed char)-4;
	return (signed char)-5;
}

		 
char THM_Write(unsigned char *buffer,unsigned short num)
{
	unsigned char temp = 0;	
	unsigned long time = 100000;//120000000;//超时时间，48M为秒级
	
    THM_ClrBuf();
	SPI_Write(DATABUF,buffer,num);//写入数据
	SPI_Write(SCON,"\x03",1);     //发送接收开始
	while((temp != 2) && (--time))//在超时时间内等待写完成
	{ 
		SPI_Read(TXFIN,&temp,1);//发生完成
	}
	
	if(time == 0)//主控芯片判断的不精确超时
	{
		return (char)(-31);//写超时
	}
	else
	{
		return 0;
	}
}



