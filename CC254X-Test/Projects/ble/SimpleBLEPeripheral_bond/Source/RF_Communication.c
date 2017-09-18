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
#define FUNC_LED_ON_OFF                             0x00    //led���صĹ�����  
  
//Ӧ�ò��¼�����Ӧ�ò㸴�ƹ�����  
#define SBP_START_DEVICE_EVT                            0x0001  
#define SBP_PERIODIC_EVT                                0x0002
#define SBP_ADV_IN_CONNECTION_EVT                       0x0004

#define SBP_RF_COMMUNICAION_PROCESS_EVT             0x0008  //ͨ�Ŵ����¼�  
#define SBP_RF_COMMUNICAION_COMMAND_ERR_EVT         0x0010  //ͨ�����ݳ����¼�  
#define SBP_LED_ON_OFF_EVT                          0x0020  //led�����¼�  
  
//ͨ��Э������ݰ�����  
#define DATA_PACKAGE_LEN                            20      //20�ֽ�һ��


//******************************************************************************                  
//name:             RF_Communication_Judgment                 
//introduce:        RF��ͨ�������ж�              
//parameter:        npReceive: ���ջ������׵�ַ         
//return:           true: ���ݰ���ȷ  
//                  false: ���ݰ�����               
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
      
  //�ж���ʼλ��ȷ��  
  if(nSof != 0xFE)  
  {  
    return RF_COMMUNICATION_JUDGMENT_FALSE;    
  }       
  
  //�������ݳ���  
  if(nLen > 16)  
  {  
    return RF_COMMUNICATION_JUDGMENT_FALSE;    
  }   
  
  //����У���  
  nCrc_Count += nSof;      
  nCrc_Count += nLen;     
  nCrc_Count += nFunc;  
      
  while(nLen--)  
  {  
    nCrc_Count += *(npData + nLen);  
  }  
      
  //�Ƚ�У���  
  if(nCrc != nCrc_Count)    
  {  
    return RF_COMMUNICATION_JUDGMENT_FALSE;    
  }       
  
  //���ݰ���ȷ  
  return RF_COMMUNICATION_JUDGMENT_TRUE;  
}  

extern void SimpleGATTprofile_Char6_Notify(uint16 nConnHandle, uint8 *pValue, uint8 nLen);
//******************************************************************************                  
//name:             RF_Communication_DataPackage_Send                 
//introduce:        RF��ͨ�����ݴ��������(��ͷ+��Ч���ݳ���+������+��Ч����+�����ֽ�)              
//parameter:        nFunc: ������       
//                  npValidData: ��Ч�����׵�ַ  
//                  nValidData_Len: Ҫ���͵����ݳ���  
//return:           none              
//author:                                                        
//changetime:       2017.08.23                        
//******************************************************************************   
void RF_Communication_DataPackage_Send(uint16 nConnHandle, uint8 nFunc, uint8 *npValidData, uint8 nValidData_Len)  
{  
  uint8 nbDataPackage_Data[DATA_PACKAGE_LEN];   
  uint8 nNum;  
    
  //��ʼ�����ͻ�����  
  memset(nbDataPackage_Data, 0xFF, 20);    
    
  //�������  
  nbDataPackage_Data[0] = 0xFE;                                             //��ͷ  
  nbDataPackage_Data[1] = nValidData_Len;                                   //��Ч���ݳ���    
  nbDataPackage_Data[2] = nFunc;                                            //������  
  memcpy(nbDataPackage_Data + 3, npValidData, nValidData_Len);              //��Ч����  
  
    
  nbDataPackage_Data[3 + nValidData_Len] = 0;                                   //У�������    
  for(nNum = 0; nNum < (3 + nValidData_Len); nNum++)  
  {  
    nbDataPackage_Data[3 + nValidData_Len] += nbDataPackage_Data[nNum]; //У����ۼ�     
  }   
    
  //��������      
  SimpleGATTprofile_Char6_Notify(nConnHandle, nbDataPackage_Data, DATA_PACKAGE_LEN);   
}  
  
//******************************************************************************                  
//name:             RF_Communication_Process                 
//introduce:        RF��ͨ�����ݴ���              
//parameter:        npReceive: ���ջ������׵�ַ       
//                  nEvent: Ҫ�������¼�  
//return:           none              
//author:                                                       
//changetime:       2017.08.23                        
//******************************************************************************   
void RF_Communication_Process(uint8 *npReceive, uint16 *npEvent)  
{  
  uint8 nFunc =  *(npReceive + 2);  
  
  //�жϹ�����  
  switch(nFunc)  
  {   
    //led���صĹ�����  
    case FUNC_LED_ON_OFF:  
    {  
      *npEvent = SBP_LED_ON_OFF_EVT;  
      break;        
    }  
  
    //��������Ч  
    default:  
    {  
      *npEvent = SBP_RF_COMMUNICAION_COMMAND_ERR_EVT;  
      break;        
    }             
  }  
}

