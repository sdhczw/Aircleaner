/**
******************************************************************************
* @file     ac_app_main.c
* @authors  zw
* @version  V1.0.0
* @date     10-Sep-2014
* @brief   
******************************************************************************
*/
#include <stdlib.h>
#include <ac_common.h>
#include <ac_protocol_interface.h>
#include <ac_api.h>
#include <ac_hal.h>
#include <ac_cfg.h>
#include "inc/hw_nvic.h"
#include "utils/uartstdio.h"
#include "driverlib/eeprom.h"
#include "Iot_Uart.h"
#include "Iot_Timer.h"
#include "Iot_Fan.h"
#include "Iot_Sht21.h"
#include "Iot_Pm25.h"
#include "Iot_Pwm.h"
#include "Iot_T6603.h"
#include "Iot_Key.h"
#include "Iot_Lcd.h"
#include "inc/hw_gpio.h"

#define  DEFAULTSMARTVERSION (0)  // 风扇控制2分钟
#define  DEFAULTFANSMARTPEARIOD (2*10)  // 风扇控制2分钟
#define  DEFAULTREPORTPEARIOD (10*60)//report devecie status interval 10min

#define  DEFAULTPM25SMARTDELAYONPEARIOD (3*60) //pm2.5 智能延迟判断
#define  DEFAULTPM25SMARTDELAYOFFPEARIOD (2*60) //pm2.5 智能延迟判断
#define  DEFAULTPM25SMARTMEASPEARIOD (10*60) //pm2.5 智能延迟判断
#define  DEFAULTPM25SMARTWORKTIME (40) //测量40次
#define  DEFAULTPM25SMARTLEVEL 75 // 75ppm

#define  DEFAULTCO2SMARTONDELAYPEARIOD (3*10) //智能 延迟开 3分钟
#define  DEFAULTCO2SMARTOFFDELAYPEARIOD (2*10) //智能 延迟关 3分钟
#define  DEFAULTCO2SMARTLEVEL 800 // 800ppm

u8  g_u8EqVersion[AC_EQVERSION_LEN]={0,0,0,1};      
u8  g_u8ModuleKey[AC_MODULE_KEY_LEN] = DEFAULT_IOT_PRIVATE_KEY;
u64  g_u64Domain = ((((u64)MAJOR_DOMAIN_ID)<<16) + (u64)SUB_DOMAIN_ID) ;
u8  g_u8DeviceId[AC_HS_DEVICE_ID_LEN] = DEVICE_ID;
u32 g_u32CloudStatus = CLOUDDISCONNECT;
u8 g_OkRspBuffer[] = "OK";
u8 g_ErrorRspBuffer[] = "ERROR";

u32 g_u32WifiPowerStatus = WIFIPOWEROFF;
u32 g_u32WifiDisConnectTimes = 0;

extern u8 g_u8MsgBuildBuffer[AC_MESSAGE_MAX_LEN];

STRU_SMART_STRATEGY g_struSmartStrategy;
/*************************************************
* Function: AC_SendDevStatus2Server
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_SendDevStatus2Server()
{
    STRU_DEV_STATUS struRsp = {0};
    u16 u16DataLen;
    struRsp.u32SmartVerison = AC_NtoHl(g_struSmartStrategy.u32SmartVerison);
    struRsp.u16FanPwm = AC_HtoNs(GetFanPwm());
    struRsp.u16FanSpeed = AC_HtoNs(GetFanSpeed());
    struRsp.u16PM25 = AC_HtoNs(GetPM25());
    struRsp.u16CO2 = AC_HtoNs(GetCo2());
    struRsp.u16IndoorTemp = AC_HtoNs(GetIndoorTempData());
    struRsp.u16OutdoorTemp = AC_HtoNs(GetOutdoorTempData());
    struRsp.u16Humidity = AC_HtoNs(GetRhData());

    AC_BuildMessage(MSG_CLIENT_SERVER_GET_DEV_STATUS_RSP,0,
                    (u8*)&struRsp, sizeof(STRU_DEV_STATUS),
                    NULL, 
                    g_u8MsgBuildBuffer, &u16DataLen);
    AC_SendMessage(UART1_BASE,g_u8MsgBuildBuffer, u16DataLen);
}

/*************************************************
* Function: AC_ResetWifi
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_ResetWifi()
{
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;                                    //UNLOCK CR REGISTER
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;   //
    
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0,GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);                       //SET UP GPIOF weak pull-up   WITH 2MA);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
    IntMasterDisable();   
    Sleep(1);
    IntMasterEnable();     
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
}
/*************************************************
* Function: AC_DealDevStatusReq
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_DealDevStatusReq(u32 u32UartBase, AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u8 *pu8Playload)
{
    STRU_DEV_STATUS struRsp = {0};
    u16 u16DataLen;
    
    struRsp.u16FanPwm = AC_HtoNs(GetFanPwm());
    struRsp.u16FanSpeed = AC_HtoNs(GetFanSpeed());
    struRsp.u16PM25 = AC_HtoNs(GetPM25());
    struRsp.u16CO2 = AC_HtoNs(GetCo2());
    struRsp.u16IndoorTemp = AC_HtoNs(GetIndoorTempData());
    struRsp.u16OutdoorTemp = AC_HtoNs(GetOutdoorTempData());
    struRsp.u16Humidity = AC_HtoNs(GetRhData());
    AC_BuildMessage(MSG_SERVER_CLIENT_DEVSTATUS_RSP,pstruMsg->MsgId,
                    (u8*)&struRsp, sizeof(STRU_DEV_STATUS),
                    pstruOptList, 
                    g_u8MsgBuildBuffer, &u16DataLen);
    AC_SendMessage(u32UartBase, g_u8MsgBuildBuffer, u16DataLen);
}
/*************************************************
* Function: AC_UpdateSmartStrategyReq
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_UpdateSmartStrategyReq(u32 u32UartBase, AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u8 *pu8Playload)
{
    u16 u16DataLen;    

    g_struSmartStrategy.u32SmartVerison = AC_NtoHl(((STRU_SMART_STRATEGY*)pu8Playload)->u32SmartVerison);
    g_struSmartStrategy.u16PM25SmartLevel = AC_NtoHs(((STRU_SMART_STRATEGY*)pu8Playload)->u16PM25SmartLevel); 
    g_struSmartStrategy.u16PM25SmartDelayOnTime = AC_NtoHs(((STRU_SMART_STRATEGY*)pu8Playload)->u16PM25SmartDelayOnTime); 
    g_struSmartStrategy.u16PM25SmartDelayOffTime = AC_NtoHs(((STRU_SMART_STRATEGY*)pu8Playload)->u16PM25SmartDelayOffTime); 
    g_struSmartStrategy.u16PM25SmartPeriodMeasTime = AC_NtoHs(((STRU_SMART_STRATEGY*)pu8Playload)->u16PM25SmartPeriodMeasTime); 
    g_struSmartStrategy.u16PM25SmartWorkTime = AC_NtoHs(((STRU_SMART_STRATEGY*)pu8Playload)->u16PM25SmartWorkTime);
    g_struSmartStrategy.u16FanPwmSmartTimeInterval = AC_NtoHs(((STRU_SMART_STRATEGY*)pu8Playload)->u16FanPwmSmartTimeInterval);
    g_struSmartStrategy.u16DevReportTime = AC_NtoHs(((STRU_SMART_STRATEGY*)pu8Playload)->u16DevReportTime);
    g_struSmartStrategy.u16Co2SmartDelayOnTime = AC_NtoHs(((STRU_SMART_STRATEGY*)pu8Playload)->u16Co2SmartDelayOnTime);
    g_struSmartStrategy.u16Co2SmartDelayOffTime = AC_NtoHs(((STRU_SMART_STRATEGY*)pu8Playload)->u16Co2SmartDelayOffTime);
    g_struSmartStrategy.u16Co2SmartLevel = AC_NtoHs(((STRU_SMART_STRATEGY*)pu8Playload)->u16Co2SmartLevel);

    AC_BuildMessage(CLIENT_SERVER_OK,pstruMsg->MsgId,
                    (u8*)g_OkRspBuffer, sizeof(g_OkRspBuffer)-1,
                    pstruOptList, 
                    g_u8MsgBuildBuffer, &u16DataLen);
    AC_SendMessage(u32UartBase, g_u8MsgBuildBuffer, u16DataLen);
    EEPROMProgram((u32 *)&g_struSmartStrategy, 0x0, ((sizeof(STRU_SMART_STRATEGY)+3)&~3));
}
/*************************************************
* Function: AC_ReadEEROM
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/    
void AC_ReadEEROM()
{
    u32 size = 1;
    u32 version = INVALID_VERSION_FLAG;
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    EEPROMInit(); 
   // size = EEPROMSizeGet();
    EEPROMProgram(&version, 0x0, 4);
    EEPROMRead(&version, 0x0, 4);
    if(version==INVALID_VERSION_FLAG)
    {
        DBGPRINT(RT_DEBUG_INFO,("No Para, Use Default Smart "));
    }
    else
    {
        EEPROMRead((u32 *)&g_struSmartStrategy, 0x0, ((sizeof(STRU_SMART_STRATEGY)+3)&~3));
    }
}
/*************************************************
* Function: AC_Init
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_Init()
{
    AC_SendDeviceStart(UART1_BASE,NULL);
}
/*************************************************
* Function: AC_ConfigWifi
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_ConfigWifi(u32 u32UartBase)
{
    static AC_Configuration struConfig;
    u16 u16DateLen;
    static u8 u8CloudKey[AC_CLOUD_KEY_MAX_LEN] = DEFAULT_IOT_CLOUD_KEY;
    struConfig.u32TraceSwitch = AC_HTONL(0);       //Sec data switch, 1:open, 0:close, 2:close RSA, default 1
    struConfig.u32SecSwitch = AC_HTONL(0);       //Sec data switch, 1:open, 0:close, 2:close RSA, default 1
    struConfig.u32WifiConfig =  AC_HTONL(0);      //Use Config SSID,password,1:open, 0:close, default 0
    struConfig.u32TestAddrConfig = AC_HTONL(0);      //connect with test url,1:open, 0:close, default 0
    memcpy(struConfig.u8Ssid, "HW_test", 7);
    memcpy(struConfig.u8Password, "12345678", 8);
    struConfig.u32IpAddr = AC_HTONL(0xc0a8c772);  //local test ip
//    struConfig.u16Port = AC_HTONS(9000); 
    struConfig.u16Port = AC_HTONS(9100); 
    memcpy(struConfig.u8CloudAddr, "test.ablecloud.cn", AC_CLOUD_ADDR_MAX_LEN);
    memcpy(struConfig.u8CloudKey, u8CloudKey, AC_CLOUD_KEY_MAX_LEN);

       //设备参数配置


    AC_BuildMessage(AC_CODE_CONFIG, 0, 
        (u8*)&struConfig, sizeof(AC_Configuration),        /*payload+payload len*/
        NULL,
        g_u8MsgBuildBuffer, &u16DateLen);
    
    AC_SendMessage(u32UartBase,g_u8MsgBuildBuffer, u16DateLen);
}
/*************************************************
* Function: AC_SendMessage
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_SendMessage(u32 u32Base,u8 *pu8Msg, u16 u16DataLen)
{
    u8 u8MagicHead[4] = {0x02,0x03,0x04,0x05};
    UartSend (u32Base, u8MagicHead, sizeof(u8MagicHead));
    UartSend (u32Base, pu8Msg,  u16DataLen);
}
/*************************************************
* Function: AC_DealNotifyMessage
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_DealNotifyMessage(u32 u32UartBase, AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u8 *pu8Playload)
{
    //处理wifi模块的通知类消息
    switch(pstruMsg->MsgCode)
    {
        case AC_CODE_EQ_DONE://wifi模块启动通知
        AC_StoreStatus(WIFIPOWERSTATUS , WIFIPOWERON);
        UARTprintf("Wifi Power On!\n");
        AC_ConfigWifi(u32UartBase);
        break;
        case AC_CODE_WIFI_CONNECTED://wifi连接成功通知
        AC_StoreStatus(WIFIPOWERSTATUS , WIFIPOWERON);
        AC_SendDeviceRegsiter(u32UartBase, NULL, g_u8EqVersion,g_u8ModuleKey,g_u64Domain,g_u8DeviceId);
        AC_StoreStatus(WIFIPOWERSTATUS , WIFIPOWERON);
        UARTprintf("Wifi Connect!\n");
       // AC_ConfigWifi(u32UartBase);
        break;
        case AC_CODE_WIFI_DISCONNECTED://wifi连接成功通知
        LCDWifiShow(0);
        UARTprintf("Wifi DisConnect!\n");
        if(g_u32WifiDisConnectTimes++>10)
        {       
            AC_ResetWifi();
            UARTprintf("Wifi Reset!\n");
            g_u32WifiDisConnectTimes=0;           
        }
        // AC_ConfigWifi(u32UartBase);
        break;
        case AC_CODE_CLOUD_CONNECTED://云端连接通知
        AC_StoreStatus(WIFIPOWERSTATUS , WIFIPOWERON);
        AC_StoreStatus(CLOUDSTATUS,CLOUDCONNECT);
        LCDWifiShow(1);
        UARTprintf("Cloud Connect!\n");
        break;
        case AC_CODE_CLOUD_DISCONNECTED://云端断链通知
        AC_StoreStatus(CLOUDSTATUS,CLOUDDISCONNECT);
        LCDWifiShow(0);
        UARTprintf("Cloud DisConnect!\n");
        break;
    }
}


/*************************************************
* Function: AC_JumpToBootLoader
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_JumpToBootLoader(void)
{
    //
    // Disable all processor interrupts.  Instead of disabling them
    // one at a time, a direct write to NVIC is done to disable all
    // peripheral interrupts.
    //
    HWREG(NVIC_DIS0) = 0xffffffff;
    HWREG(NVIC_DIS1) = 0xffffffff;

    //
    // Return control to the boot loader.  This is a call to the SVC
    // handler in the boot loader.
    //
    (*((void (*)(void))(*(uint32_t *)0x2c)))();
}


/*************************************************
* Function: AC_DealOtaMessage
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_DealOtaMessage(u32 u32UartBase, AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u8 *pu8Playload)
{
    AC_JumpToBootLoader();
    //处理wifi模块的OTA类消息，本Demo没有实现该功能
    AC_SendAckMsg(u32UartBase, NULL, pstruMsg->MsgId);
}

/*************************************************
* Function: AC_DealEvent
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_DealEvent(u32 u32UartBase, AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u8 *pu8Playload)
{   
    //处理设备自定义控制消息
    switch(pstruMsg->MsgCode)
    {
        case MSG_SERVER_CLIENT_SET_FAN_ONOFF_REQ:
        {
            AC_DealFan(u32UartBase, pstruMsg, pstruOptList, pu8Playload);

        }
        break;
        case MSG_SERVER_CLIENT_DEVSTATUS_REQ:
        {
            AC_DealDevStatusReq(u32UartBase, pstruMsg, pstruOptList, pu8Playload);
                break;
        }
        case MSG_SERVER_CLIENT_SMARTSTRATEGY_REQ:
        {
            AC_UpdateSmartStrategyReq(u32UartBase, pstruMsg, pstruOptList, pu8Playload);
                break;
        }
    }
    
}

/*************************************************
* Function: AC_DealEvent
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_StoreStatus(u32 u32Type , u32 u32Data)
{
    
    switch(u32Type)
    {
        case CLOUDSTATUS:
        g_u32CloudStatus = u32Data;
        break;
        case WIFIPOWERSTATUS:
        g_u32WifiPowerStatus = u32Data;
        break;
    }
}
/*************************************************
* Function: AC_DealFan
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_DealFan(u32 u32UartBase, AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u8 *pu8Playload)
{
    
    u16 u16DataLen;
    
    if(((STRU_FAN_ONOFF *)pu8Playload)->u8FanOnOff<2)
    {
        SetFanOnOff(((STRU_FAN_ONOFF *)pu8Playload)->u8FanOnOff);
        
        
        AC_BuildMessage(CLIENT_SERVER_OK,pstruMsg->MsgId,
                        (u8*)g_OkRspBuffer, sizeof(g_OkRspBuffer)-1,
                        pstruOptList, 
                        g_u8MsgBuildBuffer, &u16DataLen);
        AC_SendMessage(u32UartBase, g_u8MsgBuildBuffer, u16DataLen);
        
        LCDFanSpeedShow(((STRU_FAN_ONOFF *)pu8Playload)->u8FanOnOff);
    }
    else
    {
        AC_BuildMessage(CLIENT_SERVER_ERROR,pstruMsg->MsgId,
                        (u8*)g_ErrorRspBuffer, sizeof(g_ErrorRspBuffer)-1,
                        pstruOptList, 
                        g_u8MsgBuildBuffer, &u16DataLen);
        AC_SendMessage(u32UartBase, g_u8MsgBuildBuffer, u16DataLen);   
    }
}
/*************************************************
* Function: AC_DealEvent
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
u32 AC_GetStoreStatus(u32 u32Type)
{
    switch(u32Type)
    {
        case CLOUDSTATUS:
        return g_u32CloudStatus;
        break;
        case WIFIPOWERSTATUS:
        return g_u32WifiPowerStatus;
        break;
        case SMARTVERIONDATA:
        return g_struSmartStrategy.u32SmartVerison;
        break;
        case PM25SMARTDELAYONPEARIODDATA:
        return g_struSmartStrategy.u16PM25SmartDelayOnTime;
        break;
        case PM25SMARTDELAYOFFPEARIODDATA:
        return g_struSmartStrategy.u16PM25SmartDelayOffTime;
        break;
        case PM25SMARTLEVELDATA:
        return g_struSmartStrategy.u16PM25SmartLevel;
        break;
        case PM25SMARTWORKTIMEDATA:
        return g_struSmartStrategy.u16PM25SmartWorkTime;
        break;
        case PM25SMARTMEASPEARIODDATA:
        return g_struSmartStrategy.u16PM25SmartPeriodMeasTime;
        break;
        case FANSMARTPEARIODDATA:
        return g_struSmartStrategy.u16FanPwmSmartTimeInterval;
        break;
        case REPORTPEARIODDATA:
        return g_struSmartStrategy.u16DevReportTime;
        break;
        case CO2SMARTLEVELDATA:
        return g_struSmartStrategy.u16Co2SmartLevel;
        break;
        case CO2SMARTDELAYONPEARIODDATA:
        return g_struSmartStrategy.u16Co2SmartDelayOnTime;
        break;
        case CO2SMARTDELAYOFFPEARIODDATA:
        return g_struSmartStrategy.u16Co2SmartDelayOffTime;
        break;
    }
   return AC_RET_ERROR;
}

/*********************************************************************************************************
** Function name:           HwInit
** Descriptions:            
** input parameters:        
**                          
** Output parameters::      无
** Returned value:          
**                          
** Created by:     zhangwen          
** Created Date:            
**--------------------------------------------------------------------------------------------------------
** Modified by:            
** Modified date:          
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
void AC_HwInit()
{
    ClockInit();
    // u8 u8DefaulPwm = FAN_DEFAULT_PWM;
    UartInit();
    PM25Init();
    Co2Init();

    LCDInit();
    FanInit();
    
    KeyInit();
#if I2C_DEVICE_SUPPORT
    Sht21Init();
#endif
    NtcInit();
    PtcInit();
    TimerInit();
    
}
/*********************************************************************************************************
** Function name:           HwInit
** Descriptions:            
** input parameters:        
**                          
** Output parameters::      无
** Returned value:          
**                          
** Created by:     zhangwen          
** Created Date:            
**--------------------------------------------------------------------------------------------------------
** Modified by:            
** Modified date:          
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
void AC_SwInit()
{
    g_struSmartStrategy.u32SmartVerison = DEFAULTSMARTVERSION;
    g_struSmartStrategy.u16PM25SmartDelayOnTime = DEFAULTPM25SMARTDELAYONPEARIOD;
    g_struSmartStrategy.u16PM25SmartDelayOffTime = DEFAULTPM25SMARTDELAYOFFPEARIOD;
    g_struSmartStrategy.u16PM25SmartLevel = DEFAULTPM25SMARTLEVEL;
    g_struSmartStrategy.u16PM25SmartPeriodMeasTime = DEFAULTPM25SMARTMEASPEARIOD; 
    g_struSmartStrategy.u16PM25SmartWorkTime =DEFAULTPM25SMARTWORKTIME;
    g_struSmartStrategy.u16Co2SmartDelayOnTime = DEFAULTCO2SMARTONDELAYPEARIOD;
    g_struSmartStrategy.u16Co2SmartDelayOffTime = DEFAULTCO2SMARTOFFDELAYPEARIOD;
    g_struSmartStrategy.u16Co2SmartLevel = DEFAULTCO2SMARTLEVEL;
    g_struSmartStrategy.u16FanPwmSmartTimeInterval = DEFAULTFANSMARTPEARIOD;
    g_struSmartStrategy.u16DevReportTime = DEFAULTREPORTPEARIOD;
    
}
