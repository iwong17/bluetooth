#define _THM3070_C_
#include "THM3070.h"
#undef _THM3070_C_

#include "hal_mcu.h"

//����Ƶ
void THM_Open_RF(void)
{
	SPI_Write(TXCON,"\x62",1);
	Delay_ms(100);										//ʱ�䳤�ȿ����ӳ�	Delay_ms(2)��
}

	
//�ر���Ƶ
void THM_Close_RF(void)
{
  SPI_Write(TXCON,"\x65",1);
  Delay_ms(2);
}
//���ó�ʱʱ��
static void SetFWT(unsigned long ulFWT)
{
	if(ulFWT == 0)ulFWT = 1;
	SPI_Write(FWTHIGH,((unsigned char *)&ulFWT)+2,1);
	SPI_Write(FWTMID,((unsigned char *)&ulFWT)+1,1);
	SPI_Write(FWTLOW,((unsigned char *)&ulFWT),1);
}
//�������ʼ�Э��
static void ChangeProtBaud(unsigned char prot, unsigned char sndbaud, unsigned char rcvbaud)
{
	prot |= sndbaud | rcvbaud;
	SPI_Write(PSEL,&prot,1);
}

void THM_PowerDown(void)
{
	P1 |= 0x02;
}



void THM_Init(void)
{
	//unsigned short i;
	P1 |= 0x01;//SPIRST HIGH
	
        P1 &= ~0x02; //��ʼ��Ϊ�ǵ͹���ģʽ LOW
        P1 &= ~0x04; //ʹ��SPI�ӿ� LOW
	Delay_ms(10);
	P1 &= ~0x01; //SPIRST LOW
	Delay_ms(10);
	P1 |= 0x01;  //SPIRST HIGH

	//for(i=0;i<48000;i++);//������ʱʹоƬ�����ȶ�
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
        //SPI_Write(TXDP1,"\x48",1);							//0x48
	//SPI_Write(TXDP0,"\x45",1);							//0x45
	//SPI_Write(TXDN1,"\x38",1);							//0x38
	//SPI_Write(TXDN0,"\x1B",1);   						//0x1B
	SPI_Write(TXDP1,"\xff",1);
	SPI_Write(TXDP0,"\x50",1);
	SPI_Write(TXDN1,"\x7f",1);
	SPI_Write(TXDN0,"\x20",1);
        
	SetFWT(1500);
	ChangeProtBaud(TYPE_B,SND_BAUD_106K,RCV_BAUD_106K);	
	THM_ClrBuf();
}

//��ջ�����
void THM_ClrBuf(void)
{
	unsigned char temp;
	
	/*SPI_Read(SCON,&temp,1);
	temp |= 4;
	SPI_Write(SCON,&temp,1);
	temp &= ~4;
	SPI_Write(SCON,&temp,1);
	
	SPI_Write(RSTAT,"\x00",1);*/
	
	temp = 5;
	SPI_Write(SCON,&temp,1);
	Delay_ms(5);
	temp = 1;
	SPI_Write(SCON,&temp,1);
	
}

//��ȡ���������ݣ����ض����ĳ���
signed char THM_Read(unsigned char *buf,unsigned short *len)
{
	unsigned char temp = 0;
	unsigned long time = 240000000;//��ʱʱ�䣬48MΪ�뼶
	
	*len = 0;
	while ((temp==0) && --time)//�ж�	RSTAT �Ĵ��������λ�Ƿ���λ�����λ����λ������λ����λ�����ϳ�ʱ��ֹ����
	{ 
		SPI_Read(RSTAT,&temp,1);//��ȡ����״̬
  } 
	//printf("zhuangtai :%x\n",temp); 
	if(time == 0)//��ʱ������оƬ�жϵĲ���ȷ��ʱ����ʱʱ��Ϊ�뼶
	{
		return -6;
	}
	if(temp & 0x01)//��ȷ
	{		    
		SPI_Read(RSCL,&temp,1);
		*len = temp;
		SPI_Read(RSCH,&temp,1);
		*len += temp*256;
		if(*len == 0)
			return 0; 
		SPI_Read(0,buf,*len);	
		return *len;       					 //*len += (unsigned short)temp << 8;
	}
	else if(temp & 4)//��ʱ				
		return (signed char)-1;
	else if(temp & 8)//���
		return (signed char)-2;
	else if(temp & 2)//CRC����
		return (signed char)-3;
	else if(temp & 16)
		return (signed char)-4;
	return (signed char)-5;
}

		 
char THM_Write(unsigned char *buffer,unsigned short num)
{
	unsigned char temp = 0;	
	unsigned long time = 120000000;//��ʱʱ�䣬48MΪ�뼶
	
	THM_ClrBuf();
     
	SPI_Write(DATABUF,buffer,num);
	
	SPI_Write(SCON,"\x03",1);
	
	while((temp != 2) && (--time))//�ڳ�ʱʱ���ڵȴ�д���
	{ 
		SPI_Read(TXFIN,&temp,1);
	}
	
	if(time == 0)//����оƬ�жϵĲ���ȷ��ʱ
	{
		return (char)(-31);//д��ʱ
	}
	else
	{
		return 0;
	}
}



