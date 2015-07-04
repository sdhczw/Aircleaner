/**
******************************************************************************
* @file     ac_hal.h
* @authors  cxy
* @version  V1.0.0
* @date     10-Sep-2014
* @brief    
******************************************************************************
*/

#ifndef  __AC_HAL_H__ 
#define  __AC_HAL_H__

#include <ac_common.h>
#include <ac_protocol_interface.h>
#include <ac_api.h>
#include <Iot_Uart.h>
/*     ��ѯ�ࣺ����Χ��151~199��       */
#define MSG_SERVER_CLIENT_DEVSTATUS_REQ  (151)
#define MSG_SERVER_CLIENT_DEVSTATUS_RSP  (152)

/*           �����ࣺ����Χ��65~127��*/
#define MSG_SERVER_CLIENT_SMARTSTRATEGY_REQ  (65)
#define MSG_SERVER_CLIENT_SET_FAN_ONOFF_REQ  (68)

/*       �����ϱ��ࣺ(��Χ��200~255)*/
#define MSG_CLIENT_SERVER_GET_DEV_STATUS_RSP (201)

/*    ״̬����Ӧ������Χ��128~150��*/
#define CLIENT_SERVER_OK  (128)  
#define CLIENT_SERVER_ERROR  (129)  

#define CLOUDSTATUS 0
#define WIFIPOWERSTATUS 1

#define SMARTVERIONDATA 2
#define PM25SMARTDELAYONPEARIODDATA 3
#define PM25SMARTDELAYOFFPEARIODDATA 4
#define PM25SMARTLEVELDATA 5
#define PM25SMARTWORKTIMEDATA 6
#define PM25SMARTMEASPEARIODDATA 7
#define FANSMARTPEARIODDATA 8
#define REPORTPEARIODDATA 9
#define CO2SMARTDELAYONPEARIODDATA 10
#define CO2SMARTDELAYOFFPEARIODDATA 11
#define CO2SMARTLEVELDATA 12


#define CLOUDCONNECT 1 
#define CLOUDDISCONNECT 0

#define AC_Printf UARTprintf
#define WIFIPOWERON 1
#define WIFIPOWEROFF 0
#define INVALID_VERSION_FLAG (0xFFFFFFFF)

typedef struct tag_STRU_FAN_ONOFF
{		
    u8	     u8FanOnOff ; // 0:�أ�1������2����ȡ��ǰ����״̬
    u8	     u8Pad[3];		 
}STRU_FAN_ONOFF;

typedef struct tag_STRU_DEV_STATUS
{
    u32		u32SmartVerison;	  // pm2.5�����ж�ֵ
    u16		u16FanPwm;	  // ����ռ�ձ� ��λ 1% ���58
    u16		u16FanSpeed;	  // ����ת��
    u16	    u16PM25;        //PM2.5Ũ�Ȳ������ ppm,
    u16		u16CO2;		  //CO2Ũ�Ȳ������ ppm
    u16     u16IndoorTemp;        //�����¶ȵ�λ 1��
    u16     u16OutdoorTemp;       //�����¶ȵ�λ 1��
    u16     u16Humidity;    //ʪ�� ��λ 1%
} STRU_DEV_STATUS;

typedef struct tag_STRU_SMART_STRATEGY
{
    u32		u32SmartVerison;	  // ���԰汾��
    u16		u16PM25SmartLevel;	  // pm2.5�����ж�ֵ
    u16	    u16PM25SmartDelayOnTime;        //pm2.5���ܿ��ӳ�
    u16		u16PM25SmartDelayOffTime;		  //pm2.5���ܹ��ӳ�
    u16		u16PM25SmartPeriodMeasTime;		  //pm2.5���ڲ��Լ��ʱ��
    u16		u16PM25SmartWorkTime;     //pm2.5���ڲ����Ĵ���
    u16		u16Co2SmartLevel;	  // CO2�����ж�ֵ
    u16     u16Co2SmartDelayOnTime;        //CO2���ܿ��ӳ�
    u16     u16Co2SmartDelayOffTime;       //CO2���ܹ��ӳ�
    u16     u16FanPwmSmartTimeInterval;    //���pwm�ź�����ʱ����
    u16     u16DevReportTime;    //�豸��ʱ�ϱ�����
} STRU_SMART_STRATEGY;

#ifdef __cplusplus
extern "C" {
#endif
void AC_HwInit();
void AC_Init();
void AC_SendMessage(u32 UartBase, u8 *pu8Msg, u16 u16DataLen);
void AC_DealNotifyMessage(u32 UartBase, AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u8 *pu8Playload);
void AC_DealOtaMessage(u32 UartBase, AC_MessageHead *pstruMsg, AC_OptList *pstruOptList,  u8 *pu8Playload);
void AC_DealEvent(u32 UartBase, AC_MessageHead *pstruMsg, AC_OptList *pstruOptList,u8 *pu8Playload);
u32  AC_GetStoreStatus(u32 u32Type);
unsigned short crc16_ccitt(const unsigned char *buf, unsigned int len);	
void AC_StoreStatus(u32 u32Type , u32 u32Data);
void AC_SendDevStatus2Server();
void AC_SendLedStatus2Server();
void AC_DealFan(u32 u32UartBase, AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u8 *pu8Playload);
u32 AC_GetStoreStatus(u32 u32Type);
void AC_ReadEEROM();
#ifdef __cplusplus
}
#endif
#endif
/******************************* FILE END ***********************************/



