//******************************************************************************                              
//name:             RF_Communication.c                 
//introduce:        ͨ��Э��          
//author:                                                          
//changetime:       2017.08.23         
//******************************************************************************   
#include "RF_Communication.h"
#include <string.h>
#include "hal_types.h"

  
/*********************�궨��************************/   
//ͨ��Э��Ĺ�����  
#define FUNC_GUA_LED_ON_OFF                             0x00    //led���صĹ�����  
  
//Ӧ�ò��¼�����Ӧ�ò㸴�ƹ�����  
#define SBP_START_DEVICE_EVT                            0x0001  
#define SBP_PERIODIC_EVT                                0x0002
#define SBP_ADV_IN_CONNECTION_EVT                       0x0004

#define SBP_GUA_RF_COMMUNICAION_PROCESS_EVT             0x0008  //ͨ�Ŵ����¼�  
#define SBP_GUA_RF_COMMUNICAION_COMMAND_ERR_EVT         0x0010  //ͨ�����ݳ����¼�  
#define SBP_GUA_LED_ON_OFF_EVT                          0x0020  //led�����¼�  
  
//ͨ��Э������ݰ�����  
#define GUA_DATA_PACKAGE_LEN                            20      //20�ֽ�һ��


//******************************************************************************                  
//name:             GUA_RF_Communication_Judgment                 
//introduce:        RF��ͨ�������ж�              
//parameter:        npGUA_Receive: ���ջ������׵�ַ         
//return:           true: ���ݰ���ȷ  
//                  false: ���ݰ�����               
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
      
  //�ж���ʼλ��ȷ��  
  if(nGUA_Sof != 0xFE)  
  {  
    return GUA_RF_COMMUNICATION_JUDGMENT_FALSE;    
  }       
  
  //�������ݳ���  
  if(nGUA_Len > 16)  
  {  
    return GUA_RF_COMMUNICATION_JUDGMENT_FALSE;    
  }   
  
  //����У���  
  nGUA_Crc_Count += nGUA_Sof;      
  nGUA_Crc_Count += nGUA_Len;     
  nGUA_Crc_Count += nGUA_Func;  
      
  while(nGUA_Len--)  
  {  
    nGUA_Crc_Count += *(npGUA_Data + nGUA_Len);  
  }  
      
  //�Ƚ�У���  
  if(nGUA_Crc != nGUA_Crc_Count)    
  {  
    return GUA_RF_COMMUNICATION_JUDGMENT_FALSE;    
  }       
  
  //���ݰ���ȷ  
  return GUA_RF_COMMUNICATION_JUDGMENT_TRUE;  
}  

extern void GUA_SimpleGATTprofile_Char6_Notify(uint16 nGUA_ConnHandle, uint8 *pGUA_Value, uint8 nGUA_Len);
//******************************************************************************                  
//name:             GUA_RF_Communication_DataPackage_Send                 
//introduce:        RF��ͨ�����ݴ��������(��ͷ+��Ч���ݳ���+������+��Ч����+�����ֽ�)              
//parameter:        nGUA_Func: ������       
//                  npGUA_ValidData: ��Ч�����׵�ַ  
//                  nGUA_ValidData_Len: Ҫ���͵����ݳ���  
//return:           none              
//author:                                                        
//changetime:       2017.08.23                        
//******************************************************************************   
void GUA_RF_Communication_DataPackage_Send(uint16 nGUA_ConnHandle, uint8 nGUA_Func, uint8 *npGUA_ValidData, uint8 nGUA_ValidData_Len)  
{  
  uint8 nbGUA_DataPackage_Data[GUA_DATA_PACKAGE_LEN];   
  uint8 nGUA_Num;  
    
  //��ʼ�����ͻ�����  
  memset(nbGUA_DataPackage_Data, 0xFF, 20);    
    
  //�������  
  nbGUA_DataPackage_Data[0] = 0xFE;                                                     //��ͷ  
  nbGUA_DataPackage_Data[1] = nGUA_ValidData_Len;                                       //��Ч���ݳ���    
  nbGUA_DataPackage_Data[2] = nGUA_Func;                                                //������  
  memcpy(nbGUA_DataPackage_Data + 3, npGUA_ValidData, nGUA_ValidData_Len);              //��Ч����  
  
    
  nbGUA_DataPackage_Data[3 + nGUA_ValidData_Len] = 0;                                   //У�������    
  for(nGUA_Num = 0; nGUA_Num < (3 + nGUA_ValidData_Len); nGUA_Num++)  
  {  
    nbGUA_DataPackage_Data[3 + nGUA_ValidData_Len] += nbGUA_DataPackage_Data[nGUA_Num]; //У����ۼ�     
  }   
    
  //��������      
  GUA_SimpleGATTprofile_Char6_Notify(nGUA_ConnHandle, nbGUA_DataPackage_Data, GUA_DATA_PACKAGE_LEN);   
}  
  
//******************************************************************************                  
//name:             GUA_RF_Communication_Process                 
//introduce:        RF��ͨ�����ݴ���              
//parameter:        npGUA_Receive: ���ջ������׵�ַ       
//                  nGUA_Event: Ҫ�������¼�  
//return:           none              
//author:                                                       
//changetime:       2017.08.23                        
//******************************************************************************   
void GUA_RF_Communication_Process(uint8 *npGUA_Receive, uint16 *npGUA_Event)  
{  
  uint8 nGUA_Func =  *(npGUA_Receive + 2);  
  
  //�жϹ�����  
  switch(nGUA_Func)  
  {   
    //led���صĹ�����  
    case FUNC_GUA_LED_ON_OFF:  
    {  
      *npGUA_Event = SBP_GUA_LED_ON_OFF_EVT;  
      break;        
    }  
  
    //��������Ч  
    default:  
    {  
      *npGUA_Event = SBP_GUA_RF_COMMUNICAION_COMMAND_ERR_EVT;  
      break;        
    }             
  }  
}

