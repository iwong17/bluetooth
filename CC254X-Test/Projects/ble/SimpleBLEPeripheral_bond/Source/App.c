#include "hal_mcu.h"
#include "App.h"

/*********************�궨��************************/   
#define T1STAT_CHOIF            (1<<0)  //��ʱ��1��ͨ��0״̬λ  
#define T1STAT_CH1IF            (1<<1)  //��ʱ��1��ͨ��1״̬λ  
#define T1STAT_CH2IF            (1<<2)  //��ʱ��1��ͨ��2״̬λ  
#define T1STAT_CH3IF            (1<<3)  //��ʱ��1��ͨ��3״̬λ  
#define T1STAT_CH4IF            (1<<4)  //��ʱ��1��ͨ��4״̬λ  

static unsigned short timeflag; //��ʱ��־

//******************************************************************************          
//name:             GPIOInit          
//introduce:        I/O�ڳ�ʼ��        
//parameter:        none         
//return:           none
//changetime:       2017.08.23
//author:           
//******************************************************************************

void GPIOInit(void)
{
    //P0SEL = 0xc0; // Configure Port 0 as GPIO
    P1SEL = 0; // Configure Port 1 as GPIO
    //P2SEL = 0; // Configure Port 2 as GPIO

    //P0DIR = 0xFF; // Port 0 pins P0.0 and P0.1 as input (buttons),
                // all others (P0.2-P0.7) as output
    P1DIR = 0xFF; // All port 1 pins (P1.0-P1.7) as output
    //P2DIR = 0x1F; // All port 1 pins (P2.0-P2.4) as output

    //P0 = 0x0c; // All pins on port 0 to low except for P0.0 and P0.1 (buttons)
    P1 = 0x00;   // All pins on port 1 to low
    //P2 = 0;   // All pins on port 2 to low
}
//******************************************************************************          
//name:             buzzerInit          
//introduce:        ��������ʼ��        
//parameter:        none         
//return:           none
//changetime:       2017.08.23
//author:           
//******************************************************************************  
void buzzerInit(void)
{
    // Buzzer connected at P1_6
    // We will use Timer 3 Channel 0 at alternate location 2
    // Channel 0 will toggle on compare with 0 and counter will
    // count in up/down mode to T3CC0.

    PERCFG |= 0x20;             // Timer 3 Alternate location 2
    P1DIR |= 0x40;              // P1_6 = output
    P1SEL |= 0x40;              // Peripheral function on P1_6

    T3CTL &= ~0x10;             // Stop timer 3 (if it was running)
    T3CTL |= 0x04;              // Clear timer 3
    T3CTL &= ~0x08;             // Disable Timer 3 overflow interrupts
    T3CTL |= 0x03;              // Timer 3 mode = 3 - Up/Down

    T3CCTL0 &= ~0x40;           // Disable channel 0 interrupts
    T3CCTL0 |= 0x04;            // Ch0 mode = compare
    T3CCTL0 |= 0x10;            // Ch0 output compare mode = toggle on compare
}

//******************************************************************************          
//name:             buzzerStart          
//introduce:        ����������        
//parameter:        Ƶ��         
//return:           true false
//changetime:       2017.08.23
//author:           
//******************************************************************************  
uint8 buzzerStart(uint16 frequency)
{
    buzzerInit();
  
    uint8 prescaler = 0;

    // Get current Timer tick divisor setting
    uint8 tickSpdDiv = (CLKCONSTA & 0x38)>>3;

    // Check if frequency too low
    if (frequency < (244 >> tickSpdDiv)){   // 244 Hz = 32MHz / 256 (8bit counter) / 4 (up/down counter and toggle on compare) / 128 (max timer prescaler)
        buzzerStop();                       // A lower tick speed will lower this number accordingly.
        return 0;
    }

    // Calculate nr of ticks required to achieve target frequency
    uint32 ticks = (8000000/frequency) >> tickSpdDiv;      // 8000000 = 32M / 4;

    // Fit this into an 8bit counter using the timer prescaler
    while ((ticks & 0xFFFFFF00) != 0)
    {
        ticks >>= 1;
        prescaler += 32;
    }

    // Update registers
    T3CTL &= ~0xE0;
    T3CTL |= prescaler;
    T3CC0 = (uint8)ticks;

    // Start timer
    T3CTL |= 0x10;
    
    return 1;
}

//******************************************************************************          
//name:             buzzerStop          
//introduce:        �������ر�        
//parameter:        none         
//return:           none
//changetime:       2017.08.23
//author:           
//******************************************************************************  
void buzzerStop(void)
{
    T3CTL &= ~0x10;             // Stop timer 3
    P1SEL &= ~0x40;
    P1_6 = 0;
}  

//******************************************************************************          
//name:             Timer1_Init          
//introduce:        ��ʱ��1�ĳ�ʼ��        
//parameter:        none         
//return:           none
//changetime:       2017.08.23
//author:           
//******************************************************************************     
void Timer1_Init(void)      
{  
  //��ʱ��1����  
  T1CTL = (3<<2)|(2<<0);            //0000(reserved)��11(128��Ƶ��32M/128=250K��10��Modulo��  
  T1CNTL = 0;                       //���������  
    
  //��ʱ��1��ͨ��0����  
  T1CCTL0 = (1<<6)|(7<<3)|(1<<2)|(0<<0);//Enables interrupt request��Initialize output pin. CMP[2:0] is not changed��Compare mode�� No capture  
  T1CC0H = 2500/256;                    //��λ  100Hz
  T1CC0L = 2500%256;                    //��λ   
   
  //�ж�����  
  IEN1 |= (1<<1);                       //��ʱ��1�ж�ʹ��  
}

//******************************************************************************          
//name:             Delay_ms          
//introduce:        ��ʱ����        
//parameter:        time         
//return:           none
//changetime:       2017.09.18
//author:           
//****************************************************************************** 
void Delay_ms(unsigned short time)
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
    //test
    if(timeflag) timeflag--;  
    P1_1 = ~P1_1;         //������Զ�ʱһ�Σ���ȡ��һ��P1_1������۲� P1_1��Ӧ��led  
    P1SEL &= ~(1 << 1);   //����Ϊ IO ��  
    P1DIR |= (1 << 1);    //����Ϊ���
    //IEN1 |= (1<<0);       //�رն�ʱ��1�ж�ʹ��  
    //test     
  }  
}

//**************************************************  
//name:         Read_Mac  
//input:        mac��Ҫ���浽��λ�ã���Ҫ6���ֽڴ�С  
//return:       none  
//**************************************************  
void Read_Mac(uint8 *ownAddress)     //������MAC �ڳ�ʼ��ʱ����ͨ��
                                     //GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);ȥ��ȡMAC����Ϊ��ʱMAC��û��д�뵽�ñ����� 
{  
  ownAddress[5] = *(unsigned char *)(0x780E); // ֱ��ָ��ָ������     
  ownAddress[4] = *(unsigned char *)(0x780F);    
  ownAddress[3] = *(unsigned char *)(0x7810);    
  ownAddress[2] = XREG(0x7811);                // define ����ֱ�Ӷ�������     
  ownAddress[1] = XREG(0x7812);    
  ownAddress[0] = XREG(0x7813);   
} 
