//******************************************************************************                              
//name:             RF_Communication.c                 
//introduce:        通信协议          
//author:                                                          
//changetime:       2017.08.23         
//******************************************************************************   
#include "RF_Communication.h"
#include <string.h>
#include "hal_types.h"

  
/*********************宏定义************************/   
//通信协议的功能码  
#define FUNC_LED_ON_OFF                             0x00    //led开关的功能码  
  
//应用层事件（从应用层复制过来）  
#define SBP_START_DEVICE_EVT                            0x0001  
#define SBP_PERIODIC_EVT                                0x0002
#define SBP_ADV_IN_CONNECTION_EVT                       0x0004

#define SBP_RF_COMMUNICAION_PROCESS_EVT             0x0008  //通信处理事件  
#define SBP_RF_COMMUNICAION_COMMAND_ERR_EVT         0x0010  //通信数据出错事件  
#define SBP_LED_ON_OFF_EVT                          0x0020  //led开关事件  
  
//通信协议的数据包长度  
#define DATA_PACKAGE_LEN                            20      //20字节一包


//******************************************************************************                  
//name:             RF_Communication_Judgment                 
//introduce:        RF的通信数据判断              
//parameter:        npReceive: 接收缓冲区首地址         
//return:           true: 数据包正确  
//                  false: 数据包错误               
//author:                                                        
//changetime:       2017.08.23                        
//******************************************************************************   
uint8 RF_Communication_Judgment(uint8 *npReceive)  
{  
  uint8 nSof = *npReceive;  
  uint8 nLen =  *(npReceive + 1);  
  uint8 nFunc =  *(npReceive + 2);   
  uint8 *npData = npReceive + 3;  
  uint8 nCrc =  *(npReceive + 3 + nLen);  
  uint8 nCrc_Count = 0;      
      
  //判断起始位正确性  
  if(nSof != 0xFE)  
  {  
    return RF_COMMUNICATION_JUDGMENT_FALSE;    
  }       
  
  //接收数据长度  
  if(nLen > 16)  
  {  
    return RF_COMMUNICATION_JUDGMENT_FALSE;    
  }   
  
  //计算校验和  
  nCrc_Count += nSof;      
  nCrc_Count += nLen;     
  nCrc_Count += nFunc;  
      
  while(nLen--)  
  {  
    nCrc_Count += *(npData + nLen);  
  }  
      
  //比较校验和  
  if(nCrc != nCrc_Count)    
  {  
    return RF_COMMUNICATION_JUDGMENT_FALSE;    
  }       
  
  //数据包正确  
  return RF_COMMUNICATION_JUDGMENT_TRUE;  
}  

extern void SimpleGATTprofile_Char6_Notify(uint16 nConnHandle, uint8 *pValue, uint8 nLen);
//******************************************************************************                  
//name:             RF_Communication_DataPackage_Send                 
//introduce:        RF的通信数据打包并发送(包头+有效数据长度+功能码+有效数据+补齐字节)              
//parameter:        nFunc: 功能码       
//                  npValidData: 有效数据首地址  
//                  nValidData_Len: 要发送的数据长度  
//return:           none              
//author:                                                        
//changetime:       2017.08.23                        
//******************************************************************************   
void RF_Communication_DataPackage_Send(uint16 nConnHandle, uint8 nFunc, uint8 *npValidData, uint8 nValidData_Len)  
{  
  uint8 nbDataPackage_Data[DATA_PACKAGE_LEN];   
  uint8 nNum;  
    
  //初始化发送缓冲区  
  memset(nbDataPackage_Data, 0xFF, 20);    
    
  //填充数据  
  nbDataPackage_Data[0] = 0xFE;                                             //包头  
  nbDataPackage_Data[1] = nValidData_Len;                                   //有效数据长度    
  nbDataPackage_Data[2] = nFunc;                                            //功能码  
  memcpy(nbDataPackage_Data + 3, npValidData, nValidData_Len);              //有效数据  
  
    
  nbDataPackage_Data[3 + nValidData_Len] = 0;                                   //校验和清零    
  for(nNum = 0; nNum < (3 + nValidData_Len); nNum++)  
  {  
    nbDataPackage_Data[3 + nValidData_Len] += nbDataPackage_Data[nNum]; //校验和累加     
  }   
    
  //发送数据      
  SimpleGATTprofile_Char6_Notify(nConnHandle, nbDataPackage_Data, DATA_PACKAGE_LEN);   
}  
  
//******************************************************************************                  
//name:             RF_Communication_Process                 
//introduce:        RF的通信数据处理              
//parameter:        npReceive: 接收缓冲区首地址       
//                  nEvent: 要启动的事件  
//return:           none              
//author:                                                       
//changetime:       2017.08.23                        
//******************************************************************************   
void RF_Communication_Process(uint8 *npReceive, uint16 *npEvent)  
{  
  uint8 nFunc =  *(npReceive + 2);  
  
  //判断功能码  
  switch(nFunc)  
  {   
    //led开关的功能码  
    case FUNC_LED_ON_OFF:  
    {  
      *npEvent = SBP_LED_ON_OFF_EVT;  
      break;        
    }  
  
    //功能码无效  
    default:  
    {  
      *npEvent = SBP_RF_COMMUNICAION_COMMAND_ERR_EVT;  
      break;        
    }             
  }  
}

