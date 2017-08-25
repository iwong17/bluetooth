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
#define FUNC_GUA_LED_ON_OFF                             0x00    //led开关的功能码  
  
//应用层事件（从应用层复制过来）  
#define SBP_START_DEVICE_EVT                            0x0001  
#define SBP_PERIODIC_EVT                                0x0002
#define SBP_ADV_IN_CONNECTION_EVT                       0x0004

#define SBP_GUA_RF_COMMUNICAION_PROCESS_EVT             0x0008  //通信处理事件  
#define SBP_GUA_RF_COMMUNICAION_COMMAND_ERR_EVT         0x0010  //通信数据出错事件  
#define SBP_GUA_LED_ON_OFF_EVT                          0x0020  //led开关事件  
  
//通信协议的数据包长度  
#define GUA_DATA_PACKAGE_LEN                            20      //20字节一包


//******************************************************************************                  
//name:             GUA_RF_Communication_Judgment                 
//introduce:        RF的通信数据判断              
//parameter:        npGUA_Receive: 接收缓冲区首地址         
//return:           true: 数据包正确  
//                  false: 数据包错误               
//author:                                                        
//changetime:       2017.08.23                        
//******************************************************************************   
uint8 GUA_RF_Communication_Judgment(uint8 *npGUA_Receive)  
{  
  uint8 nGUA_Sof = *npGUA_Receive;  
  uint8 nGUA_Len =  *(npGUA_Receive + 1);  
  uint8 nGUA_Func =  *(npGUA_Receive + 2);   
  uint8 *npGUA_Data = npGUA_Receive + 3;  
  uint8 nGUA_Crc =  *(npGUA_Receive + 3 + nGUA_Len);  
  uint8 nGUA_Crc_Count = 0;      
      
  //判断起始位正确性  
  if(nGUA_Sof != 0xFE)  
  {  
    return GUA_RF_COMMUNICATION_JUDGMENT_FALSE;    
  }       
  
  //接收数据长度  
  if(nGUA_Len > 16)  
  {  
    return GUA_RF_COMMUNICATION_JUDGMENT_FALSE;    
  }   
  
  //计算校验和  
  nGUA_Crc_Count += nGUA_Sof;      
  nGUA_Crc_Count += nGUA_Len;     
  nGUA_Crc_Count += nGUA_Func;  
      
  while(nGUA_Len--)  
  {  
    nGUA_Crc_Count += *(npGUA_Data + nGUA_Len);  
  }  
      
  //比较校验和  
  if(nGUA_Crc != nGUA_Crc_Count)    
  {  
    return GUA_RF_COMMUNICATION_JUDGMENT_FALSE;    
  }       
  
  //数据包正确  
  return GUA_RF_COMMUNICATION_JUDGMENT_TRUE;  
}  

extern void GUA_SimpleGATTprofile_Char6_Notify(uint16 nGUA_ConnHandle, uint8 *pGUA_Value, uint8 nGUA_Len);
//******************************************************************************                  
//name:             GUA_RF_Communication_DataPackage_Send                 
//introduce:        RF的通信数据打包并发送(包头+有效数据长度+功能码+有效数据+补齐字节)              
//parameter:        nGUA_Func: 功能码       
//                  npGUA_ValidData: 有效数据首地址  
//                  nGUA_ValidData_Len: 要发送的数据长度  
//return:           none              
//author:                                                        
//changetime:       2017.08.23                        
//******************************************************************************   
void GUA_RF_Communication_DataPackage_Send(uint16 nGUA_ConnHandle, uint8 nGUA_Func, uint8 *npGUA_ValidData, uint8 nGUA_ValidData_Len)  
{  
  uint8 nbGUA_DataPackage_Data[GUA_DATA_PACKAGE_LEN];   
  uint8 nGUA_Num;  
    
  //初始化发送缓冲区  
  memset(nbGUA_DataPackage_Data, 0xFF, 20);    
    
  //填充数据  
  nbGUA_DataPackage_Data[0] = 0xFE;                                                     //包头  
  nbGUA_DataPackage_Data[1] = nGUA_ValidData_Len;                                       //有效数据长度    
  nbGUA_DataPackage_Data[2] = nGUA_Func;                                                //功能码  
  memcpy(nbGUA_DataPackage_Data + 3, npGUA_ValidData, nGUA_ValidData_Len);              //有效数据  
  
    
  nbGUA_DataPackage_Data[3 + nGUA_ValidData_Len] = 0;                                   //校验和清零    
  for(nGUA_Num = 0; nGUA_Num < (3 + nGUA_ValidData_Len); nGUA_Num++)  
  {  
    nbGUA_DataPackage_Data[3 + nGUA_ValidData_Len] += nbGUA_DataPackage_Data[nGUA_Num]; //校验和累加     
  }   
    
  //发送数据      
  GUA_SimpleGATTprofile_Char6_Notify(nGUA_ConnHandle, nbGUA_DataPackage_Data, GUA_DATA_PACKAGE_LEN);   
}  
  
//******************************************************************************                  
//name:             GUA_RF_Communication_Process                 
//introduce:        RF的通信数据处理              
//parameter:        npGUA_Receive: 接收缓冲区首地址       
//                  nGUA_Event: 要启动的事件  
//return:           none              
//author:                                                       
//changetime:       2017.08.23                        
//******************************************************************************   
void GUA_RF_Communication_Process(uint8 *npGUA_Receive, uint16 *npGUA_Event)  
{  
  uint8 nGUA_Func =  *(npGUA_Receive + 2);  
  
  //判断功能码  
  switch(nGUA_Func)  
  {   
    //led开关的功能码  
    case FUNC_GUA_LED_ON_OFF:  
    {  
      *npGUA_Event = SBP_GUA_LED_ON_OFF_EVT;  
      break;        
    }  
  
    //功能码无效  
    default:  
    {  
      *npGUA_Event = SBP_GUA_RF_COMMUNICAION_COMMAND_ERR_EVT;  
      break;        
    }             
  }  
}

