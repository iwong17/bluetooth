#include "hal_mcu.h"
#include "Delay.h"

static unsigned short timeflag; //��ʱ��־

/*********************�궨��************************/   
#define T1STAT_CHOIF            (1<<0)  //��ʱ��1��ͨ��0״̬λ  
#define T1STAT_CH1IF            (1<<1)  //��ʱ��1��ͨ��1״̬λ  
#define T1STAT_CH2IF            (1<<2)  //��ʱ��1��ͨ��2״̬λ  
#define T1STAT_CH3IF            (1<<3)  //��ʱ��1��ͨ��3״̬λ  
#define T1STAT_CH4IF            (1<<4)  //��ʱ��1��ͨ��4״̬λ 

//******************************************************************************          
//name:             Timer1_Init          
//introduce:        ��ʱ����        
//parameter:        time         
//return:           none
//changetime:       2017.09.18
//author:           
//****************************************************************************** 
void Timer1_Init(unsigned int time)
{
  //��ʱ��1����  
  T1CTL = (3<<2)|(2<<0);                //0000(reserved)��11(128��Ƶ��32M/128=250K��10��Modulo��  
  T1CNTL = 0;                           //���������  
  //��ʱ��1��ͨ��0����  
  T1CCTL0 = (1<<6)|(7<<3)|(1<<2);//Enables interrupt request��Initialize output pin. CMP[2:0] is not changed��Compare mode�� No capture  
  T1CC0H = 250/256;                     //��λ  1000Hz 1ms
  T1CC0L = 250%256;                     //��λ   
  T1CTL &= ~0x03;                       //�رն�ʱ��
  timeflag=time;
  T1CTL |= 0x02;                        //������ʱ�� ģģʽ
  //�ж�����  
  IEN1 |= (1<<1);                       //��ʱ��1�ж�ʹ��
  while(timeflag);
  T1CTL &= ~0x03;                       //�رն�ʱ��
}

//******************************************************************************    
//name:             Timer1_ISR          
//introduce:        ��ʱ��1���жϷ�����        
//parameter:        none         
//return:           none 
//changetime:       2017.08.23
//author: 
//******************************************************************************    
#pragma vector = T1_VECTOR     
__interrupt void Timer1_ISR(void)     
{             
  unsigned char flags = T1STAT;  
    
  //ͨ��0  
  if(flags & T1STAT_CHOIF)  
  {  
    if(timeflag) timeflag--;
    //IEN1 |= (1<<0);       //�رն�ʱ��1�ж�ʹ��       
  }  
}

//******************************************************************************          
//name:             Delay_us          
//introduce:        ��ʱ����        
//parameter:        time         
//return:           none
//changetime:       2017.09.18
//author:           
//******************************************************************************
void Delay_us(unsigned int time)  
{  
  while(time--)  
  {  
    /* 32 NOPs == 1 usecs */  
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");  
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");  
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");  
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");  
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");  
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");  
    asm("nop"); asm("nop");  
  }  
}

//******************************************************************************          
//name:             Delay_ms          
//introduce:        ��ʱ����        
//parameter:        time         
//return:           none
//changetime:       2017.09.18
//author:           
//******************************************************************************
void Delay_ms11(unsigned int time)  
{  
  while(time--)  
  {  
    Delay_us(1000);
  }  
}

//******************************************************************************          
//name:             Delay_ms          
//introduce:        ��ʱ����        
//parameter:        time ms         
//return:           none
//changetime:       2017.08.23
//author:           
//******************************************************************************     
void Delay_ms(unsigned int time)      
{  
	T4CTL = (7<<5)|(1<<3)|(1<<2)|(2<<0);  //��ʱ��4 128��Ƶ������ж�ʹ�ܣ���������0��ģ����ģʽ
	T4CCTL0 = (1<<6)|(7<<3)|(1<<2);       //Enables interrupt request��Initialize output pin. CMP[2:0] is not changed��Compare mode�� No capture
	T4CC0 = 250;                          //��ʱ��4ͨ��0����/�Ƚ�ֵ 1ms
	IEN1  |= (1<<4);                      //��ʱ��4�ж�ʹ��
	timeflag = time;
	T4CTL |= 0x10;                        //������ʱ��4
	while(timeflag);
	T4CTL &= ~0x10;                       //�رն�ʱ��4
}

 /******************************************************************************
 *�� �� ����Timer4_ISR
 *��    �ܣ���ʱ��4�жϷ������
 *��ڲ�������
 *���ڲ�������
 ******************************************************************************/
 #pragma vector = T4_VECTOR 
 __interrupt void Timer4_ISR(void) 
 { 
   //IRCON &= ~0x10;       //��ʱ��4�жϱ�־λ��0
   //TIMIF &= ~0x01;       //��ʱ��4����жϱ�־λ��0
   if(timeflag) timeflag--;
 }
