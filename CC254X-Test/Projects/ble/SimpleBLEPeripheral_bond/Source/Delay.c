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
//name:             Delay_ms          
//introduce:        ��ʱ����        
//parameter:        time         
//return:           none
//changetime:       2017.09.18
//author:           
//****************************************************************************** 
void Delay_ms11(unsigned int time)
{
  //��ʱ��1����  
  T1CTL = (3<<2)|(2<<0);            //0000(reserved)��11(128��Ƶ��32M/128=250K��10��Modulo��  
  T1CNTL = 0;                       //���������  
  //��ʱ��1��ͨ��0����  
  T1CCTL0 = (1<<6)|(7<<3)|(1<<2)|(0<<0);//Enables interrupt request��Initialize output pin. CMP[2:0] is not changed��Compare mode�� No capture  
  T1CC0H = 25000/256;                    //��λ  1000Hz 1ms
  T1CC0L = 25000%256;                    //��λ   
  T1CTL &= ~0x03; //�رն�ʱ��
  timeflag=time;
  T1CTL |= 0x02; //������ʱ�� ģģʽ
  //�ж�����  
  IEN1 |= (1<<1);                       //��ʱ��1�ж�ʹ��
  while(timeflag);
  T1CTL &= ~0x03; //�رն�ʱ��
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
    P1_1 = ~P1_1;         //������Զ�ʱһ�Σ���ȡ��һ��P1_1������۲� P1_1��Ӧ��led  
    P1SEL &= ~(1 << 1);   //����Ϊ IO ��  
    P1DIR |= (1 << 1);    //����Ϊ���
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
void Delay_ms(unsigned int time)  
{  
  while(time--)  
  {  
    Delay_us(1000);
  }  
} 