#include "hal_mcu.h"
#include "App.h"
#include "hci.h"


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
    //P0SEL = 0xc0;   // Configure Port 0 as GPIO
    P1SEL |= 0xE0;    // Configure Port 1(0~4) as GPIO,(5~7) as SPI
    //P2SEL = 0;      // Configure Port 2 as GPIO

    //P0DIR = 0xFF;   // Port 0 pins P0.0 and P0.1 as input (buttons),
                      // all others (P0.2-P0.7) as output
    P1DIR |= 0x1F;     // All port 1 pins (P1.4-P1.7) as output
    //P2DIR = 0x1F;   // All port 1 pins (P2.0-P2.4) as output

    //P0 = 0x0c;      
    P1 |= 0x08;       // P1.3 low
    //P2 = 0;         
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

//**************************************************  
//name:         Read_Mac  
//input:        mac��Ҫ���浽��λ�ã���Ҫ6���ֽڴ�С  
//return:       none  
//**************************************************  
void Read_Mac(uint8 *ownAddress)     //������MAC �ڳ�ʼ��ʱ����ͨ��                                    
{                                    //GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);ȥ��ȡMAC����Ϊ��ʱMAC��û��д�뵽�ñ�����   
  ownAddress[5] = *(unsigned char *)(0x780E); // ֱ��ָ��ָ������     
  ownAddress[4] = *(unsigned char *)(0x780F);    
  ownAddress[3] = *(unsigned char *)(0x7810);    
  ownAddress[2] = XREG(0x7811);                // define ����ֱ�Ӷ�������     
  ownAddress[1] = XREG(0x7812);    
  ownAddress[0] = XREG(0x7813);   
}
//**************************************************  
//name:         Write_Mac  
//input:        �豸MAC��ַ  
//return:       none  
//**************************************************  
void Write_Mac(uint8 *bdAddr)    //primary MACֻ����secondary�ɶ���д                                    
{  
  uint8 status;
  status = HCI_EXT_SetBDADDRCmd(bdAddr);
}

