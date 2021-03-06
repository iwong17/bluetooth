/**************************************************************************************************
  Filename:       simpleBLEPeripheral.c wy
  Revised:        $Date: 2010-08-06 08:56:11 -0700 (Fri, 06 Aug 2010) $
  Revision:       $Revision: 23333 $

  Description:    This file contains the Simple BLE Peripheral sample application
                  for use with the CC2540 Bluetooth Low Energy Protocol Stack.

  Copyright 2010 - 2013 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"

#include "OnBoard.h"
#include "hal_adc.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_lcd.h"

#include "gatt.h"

#include "hci.h"

#include "npi.h"

#include "app.h" //增加自己函数库
#include "SPI.h"
#include "Delay.h"
#include "RF_Communication.h" //增加通讯协议
#include "UART_Communication.h" //增加通讯协议
#include "THM3070.H"
#include "osal_snv.h"//Flash读写头文件

#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "simpleGATTprofile.h"

#if defined( CC2540_MINIDK )
  #include "simplekeys.h"
#endif

#if defined ( PLUS_BROADCASTER )
  #include "peripheralBroadcaster.h"
#else
  #include "peripheral.h"
#endif

#include "gapbondmgr.h"

#include "simpleBLEPeripheral.h"

#if defined FEATURE_OAD
  #include "oad.h"
  #include "oad_target.h"
#endif

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
//增加通知宏开关
#define ATTRTBL_CHAR6_VALUE_IDX             18  
#define ATTRTBL_CHAR6_CCC_IDX               19

// How often to perform periodic event
#define SBP_PERIODIC_EVT_PERIOD                   1000 //原先为5000

// What is the advertising interval when device is discoverable (units of 625us, 160=100ms)
#define DEFAULT_ADVERTISING_INTERVAL          160

// Limited discoverable mode advertises for 30.72s, and then stops
// General discoverable mode advertises indefinitely

#if defined ( CC2540_MINIDK )
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_LIMITED
#else
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_GENERAL
#endif  // defined ( CC2540_MINIDK )

// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     80

// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     800

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         0

// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          1000

// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         TRUE

// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL         6

// Company Identifier: Texas Instruments Inc. (13)
#define TI_COMPANY_ID                         0x000D

#define INVALID_CONNHANDLE                    0xFFFF

// Length of bd addr as a string
#define B_ADDR_STR_LEN                        15

#if defined ( PLUS_BROADCASTER )
  #define ADV_IN_CONN_WAIT                    500 // delay 500 ms
#endif					  

uint8 UARTTimeOutFlag = 1; //开机即判断超时　所以设为１
uint8 newUARTTimeOutFlag = 0;
extern uint16 UARTTimeOut;
extern uint16 SearchTimeOut;
uint8 TimeOut_data[4];
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8 simpleBLEPeripheral_TaskID;   // Task ID for internal task/event processing

static gaprole_States_t gapProfileState = GAPROLE_INIT;

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8 scanRspData[] =
{
  // complete name
  0x0d,//0x14,   // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,
  //0x53,   // 'S'
  //0x69,   // 'i'
  //0x6d,   // 'm'
  //0x70,   // 'p'
  //0x6c,   // 'l'
  //0x65,   // 'e'
  //0x42,   // 'B'
  //0x4c,   // 'L'
  //0x45,   // 'E'
  //0x50,   // 'P'
  //0x65,   // 'e'
  //0x72,   // 'r'
  //0x69,   // 'i'
  //0x70,   // 'p'
  //0x68,   // 'h'
  //0x65,   // 'e'
  //0x72,   // 'r'
  //0x61,   // 'a'
  //0x6c,   // 'l'
  
  0x42,   // 'B'
  0x4c,   // 'L'
  0x45,   // 'E'
  0x2d,   // '-'
  0x42,   // 'B'
  0x4F,   // 'O'
  0x4C,   // 'L'
  0x55,   // 'U'
  0x54,   // 'T'
  0x45,   // 'E'
  0x4B,   // 'K'
  0x31,   // '1'

  // connection interval range
  0x05,   // length of this data
  GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
  LO_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),   // 100ms
  HI_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),
  LO_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),   // 1s
  HI_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),

  // Tx power level
  0x02,   // length of this data
  GAP_ADTYPE_POWER_LEVEL,
  0       // 0dBm
};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static uint8 advertData[] =
{
  // Flags; this sets the device to use limited discoverable
  // mode (advertises for 30 seconds at a time) instead of general
  // discoverable mode (advertises indefinitely)
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,
  DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

  // service UUID, to notify central devices what services are included
  // in this peripheral
  0x03,   // length of this data
  GAP_ADTYPE_16BIT_MORE,      // some of the UUID's, but not all
  LO_UINT16( SIMPLEPROFILE_SERV_UUID ),
  HI_UINT16( SIMPLEPROFILE_SERV_UUID ),

};

// GAP GATT Attributes
static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "BLE-BOLUTEK2";

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void simpleBLEPeripheral_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void peripheralStateNotificationCB( gaprole_States_t newState );
static void performPeriodicTask( void );
static void simpleProfileChangeCB( uint8 paramID );
static void NpiSerialCallback( uint8 port, uint8 events );//声明串口回调函数

// Application states
enum
{
  BLE_STATE_IDLE,
  BLE_STATE_CONNECTED,
};
// Application state
static uint8 simpleBLEState = BLE_STATE_IDLE;
static uint8 gPairStatus=0;/*用来管理当前的状态，如果密码不正确，立即取消连接，0表示未配对，1表示已配对*/
void ProcessPasscodeCB(uint8 *deviceAddr,uint16 connectionHandle,uint8 uiInputs,uint8 uiOutputs );
static void ProcessPairStateCB( uint16 connHandle, uint8 state, uint8 status );

#if defined( CC2540_MINIDK )
static void simpleBLEPeripheral_HandleKeys( uint8 shift, uint8 keys );
#endif

#if (defined HAL_LCD) && (HAL_LCD == TRUE)
static char *bdAddr2Str ( uint8 *pAddr );
#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)



/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t simpleBLEPeripheral_PeripheralCBs =  
{
  peripheralStateNotificationCB,  // Profile State Change Callbacks
  NULL                            // When a valid RSSI is read from controller (not used by application)
};


// GAP Bond Manager Callbacks
static gapBondCBs_t simpleBLEPeripheral_BondMgrCBs =
{
  ProcessPasscodeCB,                     // 密码回调
  ProcessPairStateCB                     // 绑定状态回调
};

// Simple GATT Profile Callbacks
static simpleProfileCBs_t simpleBLEPeripheral_SimpleProfileCBs =
{
  simpleProfileChangeCB    // Charactersitic value change callback
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SimpleBLEPeripheral_Init
 *
 * @brief   Initialization function for the Simple BLE Peripheral App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void SimpleBLEPeripheral_Init( uint8 task_id )
{
  simpleBLEPeripheral_TaskID = task_id;
  
  GPIOInit(); //关闭P1_3 LED灯
  
  NPI_InitTransport( NpiSerialCallback );//初始化串口
  
  SPI_Init();//SPI初始化
  
  THM_Init();//THM初始化
  
  THM_Open_RF();//打开射频
  
  SNV_TimeOutWriteRead(SNV_TimeOut_READ, TimeOut_data, sizeof(TimeOut_data));//初始化超时时间，30s，10s
  UARTTimeOut = TimeOut_data[0]*256 + TimeOut_data[1];
  NPI_WriteTransport(&TimeOut_data[1],1);
  SearchTimeOut = TimeOut_data[2]*256 + TimeOut_data[3];
  Delay_ms(100);
   
  // Setup the GAP
  // VOID GAP_SetParamValue( TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL );
  
  // Setup the GAP Peripheral Role Profile
  {
    #if defined( CC2540_MINIDK )
      // For the CC2540DK-MINI keyfob, device doesn't start advertising until button is pressed
      uint8 initial_advertising_enable = FALSE;
    #else
      // For other hardware platforms, device starts advertising upon initialization
      uint8 initial_advertising_enable = TRUE;
    #endif

    // By setting this to zero, the device will go into the waiting state after
    // being discoverable for 30.72 second, and will not being advertising again
    // until the enabler is set back to TRUE
    uint16 gapRole_AdvertOffTime = 0;

    uint8 enable_update_request = DEFAULT_ENABLE_UPDATE_REQUEST;
    uint16 desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    uint16 desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
    uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;

    // Set the GAP Role Parameters
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );//广播使能
    GAPRole_SetParameter( GAPROLE_ADVERT_OFF_TIME, sizeof( uint16 ), &gapRole_AdvertOffTime );

    GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( scanRspData ), scanRspData );
    GAPRole_SetParameter( GAPROLE_ADVERT_DATA, sizeof( advertData ), advertData );

    GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_ENABLE, sizeof( uint8 ), &enable_update_request );
    GAPRole_SetParameter( GAPROLE_MIN_CONN_INTERVAL, sizeof( uint16 ), &desired_min_interval );
    GAPRole_SetParameter( GAPROLE_MAX_CONN_INTERVAL, sizeof( uint16 ), &desired_max_interval );
    GAPRole_SetParameter( GAPROLE_SLAVE_LATENCY, sizeof( uint16 ), &desired_slave_latency );
    GAPRole_SetParameter( GAPROLE_TIMEOUT_MULTIPLIER, sizeof( uint16 ), &desired_conn_timeout );
  }

  // Set the GAP Characteristics
  GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName );

  // Set advertising interval
  {
    uint16 advInt = DEFAULT_ADVERTISING_INTERVAL;

    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, advInt );
	
  }

  //下面是与配对相关的设置  
  // Setup the GAP Bond Manager
  {
    uint32 passkey = 0; // passkey "000000"
    uint8 pairMode = GAPBOND_PAIRING_MODE_INITIATE;
    uint8 mitm = TRUE;
    uint8 ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;  //显示密码， 以便主机输入配对的密码
    uint8 bonding = TRUE;
    GAPBondMgr_SetParameter( GAPBOND_DEFAULT_PASSCODE, sizeof ( uint32 ), &passkey );
    GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof ( uint8 ), &pairMode );
    GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof ( uint8 ), &mitm );
    GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof ( uint8 ), &ioCap );
    GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof ( uint8 ), &bonding );
  }

  // Initialize GATT attributes
  GGS_AddService( GATT_ALL_SERVICES );            // GAP
  GATTServApp_AddService( GATT_ALL_SERVICES );    // GATT attributes
  DevInfo_AddService();                           // Device Information Service
  SimpleProfile_AddService( GATT_ALL_SERVICES );  // Simple GATT Profile
#if defined FEATURE_OAD
  VOID OADTarget_AddService();                    // OAD Profile
#endif

  // Setup the SimpleProfile Characteristic Values
  {
    uint8 charValue1 = 1;
    uint8 charValue2 = 2;
    uint8 charValue3 = 3;
    uint8 charValue4 = 4;
    uint8 charValue5[SIMPLEPROFILE_CHAR5_LEN] = { 1, 2, 3, 4, 5 };
    uint8 charValue6[SIMPLEPROFILE_CHAR6_LEN] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,20};
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR1, sizeof ( uint8 ), &charValue1 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR2, sizeof ( uint8 ), &charValue2 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR3, sizeof ( uint8 ), &charValue3 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR4, sizeof ( uint8 ), &charValue4 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR5, SIMPLEPROFILE_CHAR5_LEN, charValue5 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR6, SIMPLEPROFILE_CHAR6_LEN, charValue6 );
  }


#if defined( CC2540_MINIDK )

  SK_AddService( GATT_ALL_SERVICES ); // Simple Keys Profile

  // Register for all key events - This app will handle all key events
  RegisterForKeys( simpleBLEPeripheral_TaskID );

  // makes sure LEDs are off
  HalLedSet( (HAL_LED_1 | HAL_LED_2), HAL_LED_MODE_OFF );

  // For keyfob board set GPIO pins into a power-optimized state
  // Note that there is still some leakage current from the buzzer,
  // accelerometer, LEDs, and buttons on the PCB.

  P0SEL = 0; // Configure Port 0 as GPIO
  P1SEL = 0; // Configure Port 1 as GPIO
  P2SEL = 0; // Configure Port 2 as GPIO

  P0DIR = 0xFC; // Port 0 pins P0.0 and P0.1 as input (buttons),
                // all others (P0.2-P0.7) as output
  P1DIR = 0xFF; // All port 1 pins (P1.0-P1.7) as output
  P2DIR = 0x1F; // All port 1 pins (P2.0-P2.4) as output

  P0 = 0x03; // All pins on port 0 to low except for P0.0 and P0.1 (buttons)
  P1 = 0;   // All pins on port 1 to low
  P2 = 0;   // All pins on port 2 to low

#endif // #if defined( CC2540_MINIDK )

#if (defined HAL_LCD) && (HAL_LCD == TRUE)

#if defined FEATURE_OAD
  #if defined (HAL_IMAGE_A)
    HalLcdWriteStringValue( "BLE Peri-A", OAD_VER_NUM( _imgHdr.ver ), 16, HAL_LCD_LINE_1 );
  #else
    HalLcdWriteStringValue( "BLE Peri-B", OAD_VER_NUM( _imgHdr.ver ), 16, HAL_LCD_LINE_1 );
  #endif // HAL_IMAGE_A
#else
  HalLcdWriteString( "BLE Peripheral", HAL_LCD_LINE_1 );
#endif // FEATURE_OAD

#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

  // Register callback with SimpleGATTprofile
  VOID SimpleProfile_RegisterAppCBs( &simpleBLEPeripheral_SimpleProfileCBs );

  // Enable clock divide on halt
  // This reduces active current while radio is active and CC254x MCU
  // is halted
  //HCI_EXT_ClkDivOnHaltCmd( HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT ); //WY 2017.9.19修改 不自动切换频率
  HCI_EXT_HaltDuringRfCmd(HCI_EXT_HALT_DURING_RF_DISABLE);//WY 2017.9.19添加 ENABLE会让RF期间停止MCU 所以关闭

#if defined ( DC_DC_P0_7 )

  // Enable stack to toggle bypass control on TPS62730 (DC/DC converter)
  HCI_EXT_MapPmIoPortCmd( HCI_EXT_PM_IO_PORT_P0, HCI_EXT_PM_IO_PORT_PIN7 );

#endif // defined ( DC_DC_P0_7 )
  
  // Setup a delayed profile startup
  osal_set_event( simpleBLEPeripheral_TaskID, SBP_START_DEVICE_EVT );

}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_ProcessEvent
 *
 * @brief   Simple BLE Peripheral Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16 SimpleBLEPeripheral_ProcessEvent( uint8 task_id, uint16 events )
{

  VOID task_id; // OSAL required parameter that isn't used in this function

  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;

    if ( (pMsg = osal_msg_receive( simpleBLEPeripheral_TaskID )) != NULL )
    {
      simpleBLEPeripheral_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );

      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & SBP_START_DEVICE_EVT )
  {
    // Start the Device
    VOID GAPRole_StartDevice( &simpleBLEPeripheral_PeripheralCBs );

    // Start Bond Manager密码绑定管理
    VOID GAPBondMgr_Register( &simpleBLEPeripheral_BondMgrCBs );

    // Set timer for first periodic event
    osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD );

    return ( events ^ SBP_START_DEVICE_EVT );
  }

  if ( events & SBP_PERIODIC_EVT )
  {
    // Restart timer
    if ( SBP_PERIODIC_EVT_PERIOD )
    {
      osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD );
    }

    // Perform periodic application task
    performPeriodicTask();

    return (events ^ SBP_PERIODIC_EVT);
  }

#if defined ( PLUS_BROADCASTER ) //连接后广播
  if ( events & SBP_ADV_IN_CONNECTION_EVT )
  {
    uint8 turnOnAdv = TRUE;
    // Turn on advertising while in a connection
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &turnOnAdv );

    return (events ^ SBP_ADV_IN_CONNECTION_EVT);
  }
#endif // PLUS_BROADCASTER
  
  //通信处理事件  
 if(events & SBP_RF_COMMUNICAION_PROCESS_EVT)  
 {  
   uint16 nEvent;  
   uint8 nRet;      
   uint8 nbChar6[SIMPLEPROFILE_CHAR6_LEN] = {0};     
     
   //读取特征值6的数值  
   SimpleProfile_GetParameter(SIMPLEPROFILE_CHAR6, nbChar6);  
         
   //判断接收到的RF值是否正确  
   nRet = RF_Communication_Judgment(nbChar6); 
   
   //数据正确，则执行对应事件  
   if(nRet == RF_COMMUNICATION_JUDGMENT_TRUE)  
   {  
     //根据功能码判断该执行什么事件  
     RF_Communication_Process(nbChar6, &nEvent);                                 
      
     //定时器执行对应的功能事件  
     osal_start_timerEx(simpleBLEPeripheral_TaskID, nEvent, 0);                     
   }  
   //数据不正确，则反馈报错      
   else  
   {  
     //定时器执行RF通信数据出错事件  
     osal_start_timerEx(simpleBLEPeripheral_TaskID, SBP_RF_COMMUNICAION_COMMAND_ERR_EVT, 0);           
   }  
  
   return (events ^ SBP_RF_COMMUNICAION_PROCESS_EVT);  
 }  
  
 //通信数据出错事件  
 if (events & SBP_RF_COMMUNICAION_COMMAND_ERR_EVT)  
 {   
   uint16 nConnHandle;         
   uint8 nFunc;      
   uint8 nbValidData[16];   
   uint8 nValidData_Len;  
     
   //获得连接句柄  
   GAPRole_GetParameter(GAPROLE_CONNHANDLE, &nConnHandle);  
  
   //功能码填充    
   nFunc = 0x80;         
     
   //有效数据的长度  
   nValidData_Len = 0;  
  
   //发送数据     
   RF_Communication_DataPackage_Send(nConnHandle, nFunc, nbValidData, nValidData_Len);      
     
   return (events ^ SBP_RF_COMMUNICAION_COMMAND_ERR_EVT);  
 }   
     
 //功能码00 led开关事件  
 if ( events & SBP_LED_ON_OFF_EVT )  
 {  
   uint16 nConnHandle;         
   uint8 nFunc;      
   uint8 nbValidData[16];   
   uint8 nValidData_Len;  
   uint8 nbChar6[SIMPLEPROFILE_CHAR6_LEN] = {0};    
     
   /*****************处理指令************************/  
   //读出RF接收到的数据到缓冲区  
   SimpleProfile_GetParameter(SIMPLEPROFILE_CHAR6, nbChar6);  
  
   //如果为0则关灯  
   switch(nbChar6[3])  
   {  
     //关灯  
     case 0x00:  
     {  
       P1_3 = 0;               //拉低P13       
       P1SEL &= ~(1 << 3);     //设置P13为IO口      
       P1DIR |= (1 << 3);      //设置P13为输出   
         
       break;  
     }  
  
     //开灯  
     case 0x01:  
     {  
       P1_3 = 1;               //拉高P13       
       P1SEL &= ~(1 << 3);     //设置P13为IO口      
       P1DIR |= (1 << 3);      //设置P13为输出   
         
       break;  
     }  
       
     //其它  
     default:break;  
   }  
     
   /*****************应答************************/  
   //获得连接句柄  
   GAPRole_GetParameter(GAPROLE_CONNHANDLE, &nConnHandle);  
  
   //功能码填充    
   nFunc = 0x00;         
  
   //有效数据填充  
   //nbValidData[0] = 0;  
    
   //有效数据的长度  
   nValidData_Len = 0;  
  
   //发送数据     
   RF_Communication_DataPackage_Send(nConnHandle, nFunc, nbValidData, nValidData_Len);       
  
   return (events ^ SBP_LED_ON_OFF_EVT);  
 }

 if ( events & NpiSerial_EVT )//串口回调事件  
 {
   uint8 numBytes = 0;
   uint16 nConnHandle;
   uint16 nEvent; 
   unsigned short cmdlen=0;//命令长度
   unsigned char cmdflag=0;//命令标志
   numBytes = NPI_RxBufLen();           //读出串口缓冲区有多少字节
  
   if(numBytes == 0)
   {
     return (events ^ NpiSerial_EVT);
   }
   else
   {
      UARTTimeOutFlag = 1;
	  newUARTTimeOutFlag = 1;
	  THM_Open_RF();//开射频
      //申请缓冲区buffer  
      uint8 *buffer = osal_mem_alloc(numBytes);  
      if(buffer)//缓冲区申请成功  
      {  
        //读取读取串口缓冲区数据，释放串口数据     
        NPI_ReadTransport(buffer,numBytes);

        if(buffer[0] == 0x02 && buffer[numBytes-1] == 0x03)//判断帧头帧尾
        {  
          cmdflag = 1;
          cmdlen = buffer[1]*256+buffer[2];
          CmdDeal(buffer,&cmdlen,&cmdflag,&nEvent);//串口命名处理函数
		  
		  osal_start_timerEx(simpleBLEPeripheral_TaskID, nEvent, 0);
     
          /*
          uint8 nbDataPackage_Data[20];
          //初始化发送缓冲区  
          osal_memset(nbDataPackage_Data, 0xFF, 20); 
     
          osal_memcpy(nbDataPackage_Data,buffer,numBytes);
     
          if(nbDataPackage_Data[0] != 0xFF)
          {
            //获得连接句柄  
            GAPRole_GetParameter(GAPROLE_CONNHANDLE, &nConnHandle);
     
            SimpleGATTprofile_Char4_Notify(nConnHandle,nbDataPackage_Data,20);
          }
          */
         }
         //释放申请的缓冲区  
         osal_mem_free(buffer);
      }
   }
   return (events ^ NpiSerial_EVT);
 }
 
 if ( events & SBP_UPDATE_SCAN_RSP_DATA_EVT )        //更改广播设备名事件    
 {   
	extern uint8 MACname[maxnamelen];//名称长度最大为20
	uint8 initial_advertising_enable = FALSE;    
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );//关广播
	uint8 scanRspData_Update[] =    
    {    
      	maxnamelen+1,     //自定义设备名的长度    
      	GAP_ADTYPE_LOCAL_NAME_COMPLETE,    
      	MACname[0], MACname[1], MACname[2], MACname[3], MACname[4], MACname[5],    
		MACname[6], MACname[7], MACname[8], MACname[9], MACname[10],MACname[11],
		MACname[12],MACname[13],MACname[14],MACname[15],MACname[16],MACname[17],
		MACname[18],MACname[19],    
  
      // connection interval range  
      0x05,   // length of this data  
      GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,  
      LO_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),   // 100ms  
      HI_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),  
      LO_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),   // 1s  
      HI_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),  
  
      // Tx power level  
      0x02,   // length of this data  
      GAP_ADTYPE_POWER_LEVEL, //发射功率，可调范围为：-127~127dbm  
      0       // 0dBm    
    };       
    GAP_UpdateAdvertisingData(simpleBLEPeripheral_TaskID,     
                             FALSE,    
                             sizeof(scanRspData_Update),    
                             scanRspData_Update );      //更新扫描应答数据
    
    initial_advertising_enable = TRUE;    
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );//开广播    
     
    return (events ^ SBP_UPDATE_SCAN_RSP_DATA_EVT);
 }

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * @fn      simpleBLEPeripheral_ProcessOSALMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void simpleBLEPeripheral_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
  switch ( pMsg->event )
  {
  #if defined( CC2540_MINIDK )
    case KEY_CHANGE:
      simpleBLEPeripheral_HandleKeys( ((keyChange_t *)pMsg)->state, ((keyChange_t *)pMsg)->keys );
      break;
  #endif // #if defined( CC2540_MINIDK )

  default:
    // do nothing
    break;
  }
}

#if defined( CC2540_MINIDK )
/*********************************************************************
 * @fn      simpleBLEPeripheral_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void simpleBLEPeripheral_HandleKeys( uint8 shift, uint8 keys )
{
  uint8 SK_Keys = 0;

  VOID shift;  // Intentionally unreferenced parameter

  if ( keys & HAL_KEY_SW_1 )
  {
    SK_Keys |= SK_KEY_LEFT;
  }

  if ( keys & HAL_KEY_SW_2 )
  {

    SK_Keys |= SK_KEY_RIGHT;

    // if device is not in a connection, pressing the right key should toggle
    // advertising on and off
    if( gapProfileState != GAPROLE_CONNECTED )
    {
      uint8 current_adv_enabled_status;
      uint8 new_adv_enabled_status;

      //Find the current GAP advertisement status
      GAPRole_GetParameter( GAPROLE_ADVERT_ENABLED, &current_adv_enabled_status );

      if( current_adv_enabled_status == FALSE )
      {
        new_adv_enabled_status = TRUE;
      }
      else
      {
        new_adv_enabled_status = FALSE;
      }

      //change the GAP advertisement status to opposite of current status
      GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &new_adv_enabled_status );
    }

  }

  // Set the value of the keys state to the Simple Keys Profile;
  // This will send out a notification of the keys state if enabled
  SK_SetParameter( SK_KEY_ATTR, sizeof ( uint8 ), &SK_Keys );
}
#endif // #if defined( CC2540_MINIDK )

/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void peripheralStateNotificationCB( gaprole_States_t newState )
{
  switch ( newState )
  {
    case GAPROLE_STARTED:
      {
        uint8 ownAddress[B_ADDR_LEN];
        uint8 systemId[DEVINFO_SYSTEM_ID_LEN];

        GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);

        // use 6 bytes of device address for 8 bytes of system ID value
        systemId[0] = ownAddress[0];
        systemId[1] = ownAddress[1];
        systemId[2] = ownAddress[2];

        // set middle bytes to zero
        systemId[4] = 0x00;
        systemId[3] = 0x00;

        // shift three bytes up
        systemId[7] = ownAddress[5];
        systemId[6] = ownAddress[4];
        systemId[5] = ownAddress[3];

        DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);

        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          // Display device address
          HalLcdWriteString( bdAddr2Str( ownAddress ),  HAL_LCD_LINE_2 );
          HalLcdWriteString( "Initialized",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

    case GAPROLE_ADVERTISING:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Advertising",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
        simpleBLEState = BLE_STATE_IDLE;
      }
      break;

    case GAPROLE_CONNECTED:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Connected",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
        simpleBLEState = BLE_STATE_CONNECTED;
      }
      break;

    case GAPROLE_WAITING:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Disconnected",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

    case GAPROLE_WAITING_AFTER_TIMEOUT:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Timed Out",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

    case GAPROLE_ERROR:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Error",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

    default:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

  }

  gapProfileState = newState;

#if !defined( CC2540_MINIDK )
  VOID gapProfileState;     // added to prevent compiler warning with
                            // "CC2540 Slave" configurations
#endif


}

/*********************************************************************
 * @fn      performPeriodicTask
 *
 * @brief   Perform a periodic application task. This function gets
 *          called every five seconds as a result of the SBP_PERIODIC_EVT
 *          OSAL event. In this example, the value of the third
 *          characteristic in the SimpleGATTProfile service is retrieved
 *          from the profile, and then copied into the value of the
 *          the fourth characteristic.
 *
 * @param   none
 *
 * @return  none
 */
static void performPeriodicTask( void )
{
  uint8 valueToCopy;
  uint8 stat;
  static uint8 i = 0;
  uint8 times = UARTTimeOut*1000/SBP_PERIODIC_EVT_PERIOD;//根据周期时间计算循环次数
  stat = SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR3, &valueToCopy);
  if( stat == SUCCESS )
  {
    /*
     * Call to set that value of the fourth characteristic in the profile. Note
     * that if notifications of the fourth characteristic have been enabled by
     * a GATT client device, then a notification will be sent every time this
     * function is called.
     */
    //SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR4, sizeof(uint8), &valueToCopy);
  }
  
  if (gapProfileState == GAPROLE_CONNECTED)//判断蓝牙是否有连接，连接时开启LED
  {
	LED3 = 1;
  }
  else//未连接的时候LED闪烁
  {
	LED3 = ~LED3;
  }
  if(newUARTTimeOutFlag == 1)//串口有新数据重新计时
  {
	  i = 0;
	  newUARTTimeOutFlag = 0;
	  //已在串口回调函数中开射频，快速
	  //THM_Open_RF();//开射频
  }
  if(UARTTimeOutFlag == 1)//串口有数据开始计时
  {
	  if(i<times)
	  {
	  	i++;
		NPI_WriteTransport(&i,1);
  	  }
      else
      {  
		 i = 0;
		 UARTTimeOutFlag = 0;//到达计时时间关闭标志
		 THM_Close_RF();     //到达计时时间关闭射频
	  }
  }
  
}

/*********************************************************************
 * @fn      simpleProfileChangeCB
 *
 * @brief   Callback from SimpleBLEProfile indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void simpleProfileChangeCB( uint8 paramID )
{
  uint8 newValue;
  
  uint8 str[20];

  switch( paramID )
  {
    case SIMPLEPROFILE_CHAR1:
      SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR1, &newValue );

      #if (defined HAL_LCD) && (HAL_LCD == TRUE)
        HalLcdWriteStringValue( "Char 1:", (uint16)(newValue), 10,  HAL_LCD_LINE_3 );
      #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

      break;

    case SIMPLEPROFILE_CHAR3:
      SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR3, &newValue );
      
      str[0]=newValue;
      str[1]='\0';
      NPI_WriteTransport(str, osal_strlen((char*)str));//将CHAR3接收到的字符通过串口发送出去

      #if (defined HAL_LCD) && (HAL_LCD == TRUE)
        HalLcdWriteStringValue( "Char 3:", (uint16)(newValue), 10,  HAL_LCD_LINE_3 );
      #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

      break;
      
     //char6的处理  
    case SIMPLEPROFILE_CHAR6:    
    {   
      //启动RF通信处理事件  
      osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_RF_COMMUNICAION_PROCESS_EVT, 0 );          
      break;        
    }  

    default:
      // should not reach here!
      break;
  }
}

#if (defined HAL_LCD) && (HAL_LCD == TRUE)
/*********************************************************************
 * @fn      bdAddr2Str
 *
 * @brief   Convert Bluetooth address to string. Only needed when
 *          LCD display is used.
 *
 * @return  none
 */
char *bdAddr2Str( uint8 *pAddr )
{
  uint8       i;
  char        hex[] = "0123456789ABCDEF";
  static char str[B_ADDR_STR_LEN];
  char        *pStr = str;

  *pStr++ = '0';
  *pStr++ = 'x';

  // Start from end of addr
  pAddr += B_ADDR_LEN;

  for ( i = B_ADDR_LEN; i > 0; i-- )
  {
    *pStr++ = hex[*--pAddr >> 4];
    *pStr++ = hex[*pAddr & 0x0F];
  }

  *pStr = 0;

  return str;
}
#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

//绑定过程中的密码管理回调函数
static void ProcessPasscodeCB(uint8 *deviceAddr,uint16 connectionHandle,uint8 uiInputs,uint8 uiOutputs )
{
  uint32  passcode=123456; //初始密码
  uint8   str[7];

  //在这里可以设置存储，保存之前设定的密码，这样就可以动态修改配对密码了。
  // Create random passcode
  //LL_Rand( ((uint8 *) &passcode), sizeof( uint32 ));//开辟一个密码空间
  passcode %= 1000000;//保存6位数

  //在lcd上显示当前的密码，这样手机端，根据此密码连接。
  // Display passcode to user
  if ( uiOutputs != 0 )
  {
    HalLcdWriteString( "Passcode:",  HAL_LCD_LINE_1 );//显示密码
    HalLcdWriteString( (char *) _ltoa(passcode, str, 10),  HAL_LCD_LINE_2 );//字符显示
  }
  
  // Send passcode response  发送密码请求给主机
  GAPBondMgr_PasscodeRsp( connectionHandle, SUCCESS, passcode );
}

//绑定过程中的状态管理，在这里可以设置标志位，当密码不正确时不允许连接。
static void ProcessPairStateCB( uint16 connHandle, uint8 state, uint8 status )
{
  if ( state == GAPBOND_PAIRING_STATE_STARTED )//主机发起连接，会进入开始绑定状态Pairing started
  {
    gPairStatus = 0;
  }
  else if ( state == GAPBOND_PAIRING_STATE_COMPLETE )/*当主机提交密码后，会进入完成*/
  {
    if ( status == SUCCESS )/*密码正确*/
    {
	  gPairStatus = 1;
    }
    else if(status == SMP_PAIRING_FAILED_UNSPECIFIED)//Paired device
    {   
      gPairStatus = 1; 
	}
    else//Pairing fail
    {
	  gPairStatus = 0;
	}
	//判断配对结果，如果不正确立刻停止连接。
    if(simpleBLEState == BLE_STATE_CONNECTED && gPairStatus !=1)
    {
      GAPRole_TerminateConnection();  // 终止连接
      // 终止连接后， 需要复位从机
    }
  }
  else if ( state == GAPBOND_PAIRING_STATE_BONDED )
  {
    if ( status == SUCCESS )
    {
      HalLcdWriteString( "Bonding success", HAL_LCD_LINE_1 );
    }
  }
}

//******************************************************************************          
//name:             NpiSerialCallback          
//introduce:        串口回调函数        
//parameter:        port event         
//return:           none
//changetime:       2017.09.15
//author:           wy
//****************************************************************************** 
static void NpiSerialCallback( uint8 port, uint8 events )  
{  
    (void)port;//加个 (void)，是未了避免编译告警，明确告诉缓冲区不用理会这个变量  
  
    if (events & (HAL_UART_RX_TIMEOUT | HAL_UART_RX_FULL))   //串口有数据  
    {  
       Delay_us(2000);
       osal_start_timerEx( simpleBLEPeripheral_TaskID, NpiSerial_EVT, 0 );
    }  
}

/*********************************************************************
*********************************************************************/
