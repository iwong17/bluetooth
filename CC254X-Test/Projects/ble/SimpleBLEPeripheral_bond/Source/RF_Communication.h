//******************************************************************************                              
//name:             RF_Communication.h                
//introduce:        ͨ��Э���ͷ�ļ�          
//author:                                                       
//changetime:       2016.12.08         
//******************************************************************************   
#ifndef _RF_COMMUNICATION_H_  
#define _RF_COMMUNICATION_H_  
  
/*********************�궨��************************/   
#ifndef U8      
typedef unsigned char U8;      
#endif      
    
#ifndef U16      
typedef unsigned short U16;      
#endif      
    
#ifndef U32      
typedef unsigned long U32;      
#endif      
  
#define RF_COMMUNICATION_JUDGMENT_FALSE     0  
#define RF_COMMUNICATION_JUDGMENT_TRUE      1  
  
  
/*********************�ⲿ��������************************/    
extern U8 RF_Communication_Judgment(U8 *npReceive);  
extern void RF_Communication_DataPackage_Send(U16 nConnHandle,   
                                                  U8 nFunc, U8 *npValidData, U8 nValidData_Len);  
extern void RF_Communication_Process(U8 *npReceive, unsigned short *npEvent);
  
#endif  