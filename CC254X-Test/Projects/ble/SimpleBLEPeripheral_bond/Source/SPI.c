 //******************************************************************************  
//                      INCLUDES  
//******************************************************************************  
#include <ioCC2541.h>                 
#include "spi.h"     
#include "OSAL.h"  
#include "hal_mcu.h"  
#include <string.h>  
//******************************************************************************  
//                      MACROS  
//******************************************************************************        
//引脚 usart1 spi —— alt.2   
#define SPI_CS                          P1_4     
#define SPI_CLK                         P1_5  
#define SPI_MOSI                        P1_6   
#define SPI_MISO                        P1_7  
  
  
//寄存器  
#define UxCSR                           U1CSR  
#define UxUCR                           U1UCR  
#define UxDBUF                          U1DBUF  
#define UxBAUD                          U1BAUD  
#define UxGCR                           U1GCR  
  
  
#define PxSEL                           P1SEL  
#define HAL_UART_PERCFG_BIT             0x02         // USART1 on P1, Alt-2; so set this bit.  
#define HAL_UART_PRIPO                  0x40         // USART1 priority over UART0.  
#define HAL_UART_Px_SEL_S               0xF0         // Peripheral I/O Select for Slave: SO/SI/CLK/CSn.  
#define HAL_UART_Px_SEL_M               0xE0         // Peripheral I/O Select for Master: MI/MO/CLK.  
  
  
// UxCSR - USART Control and Status Register.  
#define CSR_MODE                        0x80  
#define CSR_RE                          0x40  
#define CSR_SLAVE                       0x20  
#define CSR_FE                          0x10  
#define CSR_ERR                         0x08  
#define CSR_RX_BYTE                     0x04  
#define CSR_TX_BYTE                     0x02  
#define CSR_ACTIVE                      0x01  
  
  
// UxUCR - USART UART Control Register.  
#define UCR_FLUSH                       0x80  
#define UCR_FLOW                        0x40  
#define UCR_D9                          0x20  
#define UCR_BIT9                        0x10  
#define UCR_PARITY                      0x08  
#define UCR_SPB                         0x04  
#define UCR_STOP                        0x02  
#define UCR_START                       0x01  
  
  
//其他  
#define uint8                           unsigned char    
#define uint16                          unsigned short  
  
  
//协议相关  
#define SPI_MAX_PACKET_LEN              39  
#define SPI_MAX_DATA_LEN                35  
  
  
#define SPI_SOF                         0x7E  // Start-of-frame delimiter for SPI transport.  
#define SPI_EOF                         0x7F  // End-of-frame delimiter for SPI transport.  
  
  
#define Head_CTRL                       1  
#define Head_COMMAND                    0  
#define Head(x)                         ((p_spi_protocol_data->Head >> x) & 0x01)  
  
  
#define COMMAND_RF                      1  
#define COMMAND_BLE                     0  
  
/* 
#define SPI_LEN_INCR(LEN) st (    
  if (++(LEN) >= (SPI_MAX_DATA_LEN - 1))   
  {   
    (LEN) = 0;    
  }   
)
*/ 
  
#define DATA_RESET();   {spiRxSte = spiRxSteSOF;  spiRxIndex = 0;}  
//******************************************************************************  
//                      GLOBAL VARIABLES  
//******************************************************************************  
//  
typedef enum
{
  RESPONSE_RECEIVING = 0xF4,             //正在接收  
  RESPONSE_RECEIVED = 0xF6,              //接收完毕  
  RESPONSE_RESEND = 0xF7                 //请求重发  
} SPI_RESPONSE_BYTE;
  
//  
typedef enum
{  
  SPI_MALLOC_DATA_RIGHT,    
  SPI_MALLOC_DATA_ERROR   
} SPI_MALLOC_DATA_STATUS; 
  
//协议包  
typedef enum
{  
  spiRxSteSOF,  
  spiRxSteLen,  
  spiRxSteData   
} spiRxSte_t;  
  
//SPI接收状态  
typedef enum
{  
  SPI_RxStatus_Receiving,  
  SPI_RxStatus_Received,  
  SPI_RxStatus_Right,  
  SPI_RxStatus_Error   
} SPI_RXSTATUS; 
  
//协议中的核心数据  
typedef struct
{  
  uint8 Length;    
  uint8 Head;  
  uint8 Sequence;  
  uint8 Binary[32];    
} SPI_PROTOCOL_DATA;  
  
SPI_PROTOCOL_DATA   *p_spi_protocol_data;             
    
//接收数据要的全局变量  
spiRxSte_t spiRxSte = spiRxSteSOF;   
uint16 spiRxFcs = 0;                    //校验和  
uint8 spiRxIndex = 0;                   //指向接收核心数据缓冲区的第几位  
uint8 spi_rx[SPI_MAX_PACKET_LEN] = {0};   //接收缓冲区  
SPI_RXSTATUS spi_rx_status = SPI_RxStatus_Receiving;  
static volatile uint8 check_time = 0;   //用于计数主机发送F4的次数，如果过多就超时  
//******************************************************************************  
//                      LOCAL FUNCTIONS  
//******************************************************************************  
static void SPI_Response_Byte(SPI_RESPONSE_BYTE tx);
static SPI_MALLOC_DATA_STATUS SPI_Malloc_DATA(void);
static uint8 SPI_Packet_Receive(void);
static SPI_RXSTATUS SPI_Rx_Resolution(void);
static SPI_RXSTATUS SPI_Status_Judge(uint8 status);
static void SPI_Status_Deal(SPI_RXSTATUS status);
  
//******************************************************************************  
//name:         test  
//introduce:    测试  
//parameter:    none  
//return:       none  
//******************************************************************************  
/*  
volatile uint8 tx_buf[256] = {0};  
volatile uint8 rx_buf[256] = {0};  
volatile uint8 tx_index = 0;  
volatile uint8 rx_index = 0;  
*/  
uint8 tx_buf[256] = {0};  
uint8 rx_buf[256] = {0};  
uint8 tx_index = 0;  
uint8 rx_index = 0;  
  
  
void test_init(void)  
{  
  uint16 i = 0;  
    
  for(i = 0; i < 256; i++)  
  {  
    tx_buf[i] = i;  
  }  
  
  
}  
void test(void)  
{  
  volatile uint8 aa = 0;  
  
  
  rx_buf[rx_index] = UxDBUF;  
  UxDBUF = tx_buf[tx_index];  
  
  
  if(rx_buf[rx_index] == tx_buf[rx_index]){  
    rx_index++;  
    tx_index++;  
  }  
  else{  
    aa =0x38;  
  }  
}

//******************************************************************************  
//name:         SPI_Init  
//introduce:    SPI初始化  
//parameter:    none  
//return:       none  
//******************************************************************************  
void SPI_Init(void)  
{  
  volatile uint8 receive = 0;  
    
  PERCFG |= HAL_UART_PERCFG_BIT;        // Set UART1 I/O to Alt. 2 location on P1.  
  
  PxSEL |= HAL_UART_Px_SEL_S;           // 端口设置. Peripheral I/O Select for Slave: SO/SI/CLK/CSn.  
  UxCSR = (0<<7) | (0<<5);              // 模式设置，Mode is SPI-Master Mode.  
   
  UxUCR = UCR_FLUSH;                    // 清除单元 Flush it.  
    
  UxGCR |= (1 << 5);                    // 设置传送位顺序 Set bit order to MSB.    
  UxGCR |= (1 << 6);                    // CPHA 设置SPI时钟相位 
  UxGCR |= (1 << 7);                    // CPOL 设置SPI的时钟极性为正  
    
  TCON &= ~(1 << 7);                    //清空usart1接收中断标志位  
  IEN0 |= (1 << 3);                     //使能usart1接收中断   
    
  IP0 |= (1 << 3);                      //设置spi的中断组为等级3    
  //IP0 &= ~(1 << 3);                   //设置spi的中断组为等级2  
  IP1 |= (1 << 3);    
      
  UxCSR |= CSR_RE;                      //使能  
  
  receive = UxDBUF;                     //读寄存器，防止里面有残留数据       
    
  SPI_Response_Byte(RESPONSE_RECEIVING);//将0xF4写入发送寄存器中待发送   
  
  SPI_Malloc_DATA();                    //分配内存  
    
  //测试！！！！！！！  
  //test_init();  
}

/*********************SPI写字节************************************/ 
void SPI_SendByte(unsigned char dat)     
{
    unsigned char i=8, temp=0;
    for(i=0;i<8;i++) //发送一个八位数据 
    {
        SPI_CLK=0;  
        
        temp = dat&0x80;
        if (temp == 0)
        {
            SPI_MOSI = 0;
        }
        else
        {
            SPI_MOSI = 1;
        }
        SPI_CLK=1;             
        dat<<=1;    
    }
}

/*********************SPI读字节************************************/ 
uint8 SPI_ReadByte(void)
{
    unsigned char i=0, in=0, temp=0;

    //先拉低, 然后拉高的时候, 读取MISO
    SPI_CLK = 0;
    for(i=0;i<8;i++){
        in = (in << 1);
        SPI_CLK = 1; 
        //SPI_DLY_us(1);
        temp = SPI_MISO;
        if (temp == 1){
            in = in | 0x01;
        }
        SPI_CLK = 0;
        //SPI_DLY_us(1);   
    }
    //SPI_CS=1;
    return in;
}

/*********************SPI写命令************************************/ 
signed char SPI_Write(unsigned char add, unsigned char *buf,unsigned short len)
{
	unsigned short TimeOut;
	unsigned short i;
	
	TimeOut = 4200;
	while((UxCSR&&CSR_ACTIVE) && --TimeOut);

	if(TimeOut == 0)return -1;
	
	SPI_CS = 1;
	
	TimeOut = 6;
	while(--TimeOut);
	
	UxDBUF = add | 0x80;
	TimeOut = 4200;
	while((UxCSR&&CSR_ACTIVE) && --TimeOut);
	if(TimeOut == 0)return -2;
	
	for(i=0;i<len;i++)
	{
		UxDBUF = buf[i];
		TimeOut = 4200;
		while((UxCSR&&CSR_ACTIVE) && --TimeOut);
		if(TimeOut == 0)return -3;
	}
	
	SPI_CS = 0;
	
	return 0;
}

/*********************SPI读命令************************************/
signed char SPI_Read(unsigned char add, unsigned char *buf,unsigned short len)
{
	unsigned short TimeOut;
	unsigned short i;
	
	TimeOut = 4200;
	while((UxCSR&&CSR_ACTIVE) && --TimeOut);
	if(TimeOut == 0)return -1;
	
	SPI_CS = 1;
	
	TimeOut = 6;
	while(--TimeOut);

	UxDBUF = add & 0x7f;
	TimeOut = 4200;
	while((UxCSR&&CSR_ACTIVE) && --TimeOut);
	if(TimeOut == 0)return -2;
	
	for(i=0;i<len;i++)
	{
		UxDBUF = 0x00;
		TimeOut = 4200;
		while((UxCSR&&CSR_ACTIVE) && --TimeOut);
		if(TimeOut == 0)return -4;
		buf[i] = UxDBUF;
	}
	
	SPI_CS = 0;
	
	return 0;
}
 
//******************************************************************************  
//name:         SPI_Response_Byte  
//introduce:    SPI从机应答函数  
//parameter:    none  
//return:       none  
//******************************************************************************  
static void SPI_Response_Byte(SPI_RESPONSE_BYTE tx)   
{  
  UxDBUF = tx;                          //写到寄存器里  
}    
  
//******************************************************************************  
//name:         SPI_Malloc_DATA  
//introduce:    给通信协议中的核心数据开辟一个缓冲区  
//parameter:    none  
//return:       none  
//******************************************************************************  
static SPI_MALLOC_DATA_STATUS SPI_Malloc_DATA(void)   
{  
  //分配内存  
  p_spi_protocol_data = osal_mem_alloc(sizeof(SPI_PROTOCOL_DATA));         //申请缓冲区buffer  
    
  //判断是否分配成功  
  if(p_spi_protocol_data){            
      
      //内存分配成功，缓冲区置0  
      osal_memset(p_spi_protocol_data, 0, sizeof(SPI_PROTOCOL_DATA));  
        
      return SPI_MALLOC_DATA_RIGHT;  
  }else{  
      
      //内存分配失败，返回错误信息  
      return SPI_MALLOC_DATA_ERROR;    
  }  
}    
  
//******************************************************************************  
//name:         SPI_Poll  
//introduce:    扫描SPI  
//parameter:    none  
//return:       none  
//******************************************************************************  
void SPI_Poll(void)   
{  
  switch(spi_rx_status)  
  {  
    case SPI_RxStatus_Received:    
        
      spi_rx_status = SPI_Rx_Resolution();  
        
      break;  
      
    default:  
      //spi_rx_status = SPI_RxStatus_Receiving;        
      break;  
  }  
}   
  
//******************************************************************************  
//name:         SPI_Rx_Resolution  
//introduce:    解析接收到的SPI数据  
//parameter:    none  
//return:       none  
//******************************************************************************  
static SPI_RXSTATUS SPI_Rx_Resolution(void)   
{  
  uint8 length = spi_rx[0];  
  uint8 xox_h = spi_rx[length + 1];  
  uint8 xox_l = spi_rx[length + 2];  
  uint16 xox = 0;  
  uint8 eof = spi_rx[length + 3];  
  
  
  //清除缓冲区  
  osal_memset(p_spi_protocol_data, 0, sizeof(SPI_PROTOCOL_DATA));   
      
      
  //拷贝接收缓冲区  
  osal_memcpy(p_spi_protocol_data, spi_rx, (length + 3));  
  
  
    
  //计算校验和  
  for(uint8 i = 0; i < (length + 1); i++){  
      
      
    //length  
    if(i == 0){  
      xox = p_spi_protocol_data->Length;  
    }  
  
  
    //head      
    else if(i == 1){  
      xox ^= p_spi_protocol_data->Head;      
    }  
  
  
    //sequence      
    else if(i == 2){  
      xox ^= p_spi_protocol_data->Sequence;      
    }  
  
  
    //binary  
    else{  
      xox ^= p_spi_protocol_data->Binary[i - 3];        
    }  
  }  
    
  //比较校验和  
  if((((uint8)(xox >> 8) & 0xff) != xox_h) || ((uint8)(xox & 0xff) != xox_l)){   //校验和出错  
    return SPI_RxStatus_Error;       
  }  
    
  //比较EOF  
  if(eof == SPI_EOF){  
      
      //处理数据  
      if(Head(7) == Head_CTRL){      //CTRL  
        switch((p_spi_protocol_data->Head) & 0x7F)  //内部指令  
        {  
          case 0x00:break;  
          case 0x01:break;  
          case 0x02:break;  
          case 0x03:break;  
          case 0x04:break;     
          default:break;  
        }  
      }  
      else{                                          //COMMAND  
         switch((p_spi_protocol_data->Head) & (1 << 5))  
         {  
            case COMMAND_BLE:  
              break;  
                
            case COMMAND_RF:  
              break;    
                
            default:  
              break;       
         }  
      }            
        
    return SPI_RxStatus_Right;       
  }   
  else{  
    return SPI_RxStatus_Error;     
  }  
    
   
}  
  
  
//******************************************************************************  
//name:         SPI_Packet_Receive  
//introduce:    SPI数据包接收  
//parameter:    none  
//return:       0:接收正常，1数据全部接收完毕，2数据出错，3扫描超时  
//******************************************************************************  
static uint8 SPI_Packet_Receive(void)   
{  
    uint8 ch = UxDBUF;          //接收数据                  
      
    switch (spiRxSte)  
    {  
      //包头  
      case spiRxSteSOF:                                 //包头  
        DATA_RESET();  
          
        if (ch == SPI_SOF){  
          spiRxSte = spiRxSteLen;                       //包头正常，指向长度段  
          check_time = 0;  
        }  
        else if (ch == 0xF5){                           //超时处理  
          if(check_time++ > 200){  
            check_time = 0;  
            return 3;  
          }  
        }  
        else{  
          return 2;                                     //包头接收出错  
        }  
            
        break;  
        
      //数据长度    
      case spiRxSteLen:                                 //数据长度  
        if((ch >= 3) && (ch <= 34)){  
          spi_rx[spiRxIndex++] = ch;                    //保存数据长度  
          spiRxSte = spiRxSteData;                      //指向数据接收            
        }  
        else{  
          DATA_RESET();  
          return 2;                                     //包头接收出错            
        }  
          
        break;  
  
  
      //其他数据  
      case spiRxSteData:      
        spi_rx[spiRxIndex] = ch;                        //保存数据  
          
        if(spiRxIndex++ == (spi_rx[0] + 3)){            //判断数据是否接收完  
          DATA_RESET();     
            
          return 1;                                     //接收完毕  
        }  
          
        break;  
  
  
      default:  
        DATA_RESET();   
          
        return 2;                      //数据出错      
          
        break;  
        
  }   
    
  return 0;             //正常   
}  
  
  
//******************************************************************************  
//name:         SPI_Status_Judge  
//introduce:    状态判断  
//parameter:    status:当前状态  
//return:       0:接收正常，1数据全部接收完毕，2数据出错  
//******************************************************************************  
static SPI_RXSTATUS SPI_Status_Judge(uint8 status)  
{  
  switch(status)  
  {  
    //正在接收  
    case 0:  
      return SPI_RxStatus_Receiving;  
  
  
    //接收完毕    
    case 1:  
      return SPI_RxStatus_Received;      
        
    //接收出错  
    case 2:        
      return SPI_RxStatus_Error;  
  
  
    //扫描超时出错  
    case 3:        
      return SPI_RxStatus_Error;  
        
    default:  
      return SPI_RxStatus_Error;        
  }  
}  
  
  
//******************************************************************************  
//name:         SPI_Status_Deal  
//introduce:    状态处理  
//parameter:    none  
//return:       none  
//******************************************************************************  
static void SPI_Status_Deal(SPI_RXSTATUS status)  
{  
  switch(status)  
  {  
    //正在接收  
    case SPI_RxStatus_Receiving:  
        
      SPI_Response_Byte(RESPONSE_RECEIVING);   
        
      break;  
  
  
    //接收完毕    
    case SPI_RxStatus_Received:  
        
      SPI_Response_Byte(RESPONSE_RECEIVING);   
        
      break;     
        
    //接收出错  
    case SPI_RxStatus_Error:     
        
      spi_rx_status = SPI_RxStatus_Receiving;      //下次状态改回接收  
        
      SPI_Response_Byte(RESPONSE_RESEND);   
        
      break;  
        
    default:break;      
  }  
}  
  
  
  
  
//******************************************************************************  
//name:         SPI_Rx_ISR  
//introduce:    SPI接收中断函数  
//parameter:    none  
//return:       none  
//******************************************************************************  
#pragma vector = URX1_VECTOR   
__interrupt void SPI_Rx_ISR(void)   
{       
  uint8 status = 0;  
    
  HAL_ENTER_ISR();       
  
  
  if(UxCSR & CSR_RX_BYTE){          //receive a byte   
  
  
     //test();       
   
    switch(spi_rx_status)  
    {  
        case SPI_RxStatus_Receiving:  
            
          status = SPI_Packet_Receive();                //接收数据  
            
          spi_rx_status = SPI_Status_Judge(status);     //判断状态  
            
          SPI_Status_Deal(spi_rx_status);               //处理数据  
            
          break;  
            
        case SPI_RxStatus_Received:  
            
          SPI_Response_Byte(RESPONSE_RECEIVING);        //在接收完毕的状态，一直回0xF4。等待轮询改变状态  
            
          break;    
  
  
        case SPI_RxStatus_Right:  
            
          spi_rx_status = SPI_RxStatus_Receiving;      //下次状态改回接收  
            
          SPI_Response_Byte(RESPONSE_RECEIVED);   
                      
          break;  
            
        case SPI_RxStatus_Error:  
            
          spi_rx_status = SPI_RxStatus_Receiving;      //下次状态改回接收  
            
          SPI_Response_Byte(RESPONSE_RESEND);   
            
          break;    
            
        default:  
          break;       
    }       
  }  
       
    
  HAL_EXIT_ISR();  
}