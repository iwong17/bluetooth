//******************************************************************************                              
//name:             RF_Communication.h                
//introduce:        通信协议的头文件          
//author:                                                       
//changetime:       2016.12.08         
//******************************************************************************   
#ifndef _RF_COMMUNICATION_H_  
#define _RF_COMMUNICATION_H_  
  
/*********************宏定义************************/   
#ifndef GUA_U8      
typedef unsigned char GUA_U8;      
#endif      
    
#ifndef GUA_U16      
typedef unsigned short GUA_U16;      
#endif      
    
#ifndef GUA_U32      
typedef unsigned long GUA_U32;      
#endif      
  
#define GUA_RF_COMMUNICATION_JUDGMENT_FALSE     0  
#define GUA_RF_COMMUNICATION_JUDGMENT_TRUE      1  
  
  
/*********************外部函数声明************************/    
extern GUA_U8 GUA_RF_Communication_Judgment(GUA_U8 *npGUA_Receive);  
extern void GUA_RF_Communication_DataPackage_Send(GUA_U16 nGUA_ConnHandle,   
                                                  GUA_U8 nGUA_Func, GUA_U8 *npGUA_ValidData, GUA_U8 nGUA_ValidData_Len);  
extern void GUA_RF_Communication_Process(GUA_U8 *npGUA_Receive, unsigned short *npGUA_Event);
  
#endif  