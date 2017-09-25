#include "hal_mcu.h"
#include "Delay.h"

static unsigned short timeflag; //定时标志

/*********************宏定义************************/   
#define T1STAT_CHOIF            (1<<0)  //定时器1的通道0状态位  
#define T1STAT_CH1IF            (1<<1)  //定时器1的通道1状态位  
#define T1STAT_CH2IF            (1<<2)  //定时器1的通道2状态位  
#define T1STAT_CH3IF            (1<<3)  //定时器1的通道3状态位  
#define T1STAT_CH4IF            (1<<4)  //定时器1的通道4状态位 

//******************************************************************************          
//name:             Delay_ms          
//introduce:        延时函数        
//parameter:        time         
//return:           none
//changetime:       2017.09.18
//author:           
//****************************************************************************** 
void Delay_ms(unsigned int time)
{
  //定时器1配置  
  T1CTL = (3<<2)|(2<<0);            //0000(reserved)、11(128分频，32M/128=250K、10（Modulo）  
  T1CNTL = 0;                       //清除计数器  
  //定时器1的通道0配置  
  T1CCTL0 = (1<<6)|(7<<3)|(1<<2)|(0<<0);//Enables interrupt request、Initialize output pin. CMP[2:0] is not changed、Compare mode、 No capture  
  T1CC0H = 250/256;                    //高位  1000Hz 1ms
  T1CC0L = 250%256;                    //低位   
  T1CTL &= ~0x03; //关闭定时器
  timeflag=time;
  T1CTL |= 0x02; //启动定时器 模模式
  //中断配置  
  IEN1 |= (1<<1);                       //定时器1中断使能
  while(timeflag);
  T1CTL &= ~0x03; //关闭定时器
}

//******************************************************************************    
//name:             Timer1_ISR          
//introduce:        定时器1的中断服务函数        
//parameter:        none         
//return:           none 
//changetime:       2017.08.23
//author: 
//******************************************************************************    
#pragma vector = T1_VECTOR     
__interrupt void Timer1_ISR(void)     
{             
  unsigned char flags = T1STAT;  
    
  //通道0  
  if(flags & T1STAT_CHOIF)  
  {  
    if(timeflag) timeflag--;
    //IEN1 |= (1<<0);       //关闭定时器1中断使能       
  }  
}

//******************************************************************************          
//name:             Delay_us          
//introduce:        延时函数        
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
//introduce:        延时函数        
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