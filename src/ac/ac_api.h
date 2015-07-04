/**
******************************************************************************
* @file     ac_api.h
* @authors  cxy
* @version  V1.0.0
* @date     10-Sep-2014
* @brief    
******************************************************************************
*/

#ifndef  __AC_API_H__ 
#define  __AC_API_H__

#include <ac_common.h>
#include <ac_protocol_interface.h>

typedef struct 
{
    AC_TransportInfo *pstruTransportInfo;
    AC_SsessionInfo *pstruSsession;
    
}AC_OptList;


#ifdef __cplusplus
extern "C" {
#endif
void AC_BuildOption(AC_OptList *pstruOptList, u8 *pu8OptNum, u8 *pu8Buffer, u16 *pu16Len);
void AC_BuildMessage(u8 u8MsgCode, u8 u8MsgId, 
    u8 *pu8Payload, u16 u16PayloadLen,
    AC_OptList *pstruOptList,
    u8 *pu8Msg, u16 *pu16Len);
void AC_SendDeviceStart(u32 u32UartBase, AC_OptList *pstruOptList);
void AC_SendDeviceRegsiter(u32 u32UartBase, AC_OptList *pstruOptList, u8 *pu8EqVersion, u8 *pu8ModuleKey, u64 u64Domain, u8 *pu8DeviceId);
void AC_SendAckMsg(u32 u32UartBase, AC_OptList *pstruOptList, u8 u8MsgId);
void AC_SendErrMsg(u32 u32UartBase, AC_OptList *pstruOptList, u8 u8MsgId, u8 *pu8ErrorMsg, u16 u16DataLen);
void AC_SendRestMsg(u32 u32UartBase, AC_OptList *pstruOptList);
void AC_RecvMessage(u32 u32UartBase, AC_MessageHead *pstruMsg);
void AC_ParseOption(AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u16 *pu16OptLen);

#ifdef __cplusplus
}
#endif
#endif
/******************************* FILE END ***********************************/



