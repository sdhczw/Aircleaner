/**
******************************************************************************
* @file     ac_api.c
* @authors  cxy
* @version  V1.0.0
* @date     10-Sep-2014
* @brief   
******************************************************************************
*/

#include <ac_common.h>
#include <ac_protocol_interface.h>
#include <ac_hal.h>

u8 g_u8MsgBuildBuffer[AC_MESSAGE_MAX_LEN];

/*************************************************
* Function: AC_BuildOption
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_BuildOption(AC_OptList *pstruOptList, u8 *pu8OptNum, u8 *pu8Buffer, u16 *pu16Len)
{
    //可选字段组包函数
    AC_MessageOptHead *pstruOpt;
    u8 u8OptNum = 0;
    u8 u16OptLen = 0;
    
    *pu16Len = u16OptLen;
    *pu8OptNum = u8OptNum;
    
    
    if (NULL == pstruOptList)
    {
        return;
    }
    
    pstruOpt = (AC_MessageOptHead *)pu8Buffer;

    /*add opt, if it exist*/
    if (NULL != pstruOptList->pstruTransportInfo)
    {
        pstruOpt->OptCode = AC_HTONS(AC_OPT_TRANSPORT);
        pstruOpt->OptLen = AC_HTONS(sizeof(AC_TransportInfo));
        memcpy((u8*)(pstruOpt + 1), (u8*)pstruOptList->pstruTransportInfo, sizeof(AC_TransportInfo));

        u8OptNum++;
        u16OptLen += sizeof(AC_MessageOptHead) + sizeof(AC_TransportInfo);
        pstruOpt = (AC_MessageOptHead *)(pu8Buffer + u16OptLen);        
    }
    

    if (NULL != pstruOptList->pstruSsession)
    {
        pstruOpt = (AC_MessageOptHead *)pu8Buffer;
        pstruOpt->OptCode = AC_HTONS(AC_OPT_SSESSION);
        pstruOpt->OptLen = AC_HTONS(sizeof(AC_SsessionInfo));
        memcpy((u8*)(pstruOpt + 1), (u8*)pstruOptList->pstruSsession, sizeof(AC_SsessionInfo));

        u8OptNum++;
        u16OptLen += sizeof(AC_MessageOptHead) + sizeof(AC_SsessionInfo);
        pstruOpt = (AC_MessageOptHead *)(pu8Buffer + u16OptLen);        
    }    

    
    *pu16Len = u16OptLen;
    *pu8OptNum = u8OptNum;
    return;
}

/*************************************************
* Function: AC_BuildMessage
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_BuildMessage(u8 u8MsgCode, u8 u8MsgId, 
    u8 *pu8Payload, u16 u16PayloadLen,
    AC_OptList *pstruOptList,
    u8 *pu8Msg, u16 *pu16Len)
{
    //协议组包函数
    AC_MessageHead *pstruMsg = NULL;
    u16 u16OptLen = 0;
    u16 crc = 0;
    
    pstruMsg = (AC_MessageHead *)pu8Msg;
    pstruMsg->MsgCode = u8MsgCode;
    pstruMsg->MsgId = u8MsgId;  

    pstruMsg->Version = 0;

    AC_BuildOption(pstruOptList, &pstruMsg->OptNum, (pu8Msg + sizeof(AC_MessageHead)), &u16OptLen);
    memcpy((pu8Msg + sizeof(AC_MessageHead) + u16OptLen), pu8Payload, u16PayloadLen);

    /*updata len*/
    pstruMsg->Payloadlen = AC_HTONS(u16PayloadLen + u16OptLen);

    /*calc crc*/
    crc = crc16_ccitt((pu8Msg + sizeof(AC_MessageHead)), (u16PayloadLen + u16OptLen));
    pstruMsg->TotalMsgCrc[0]=(crc&0xff00)>>8;
    pstruMsg->TotalMsgCrc[1]=(crc&0xff);


    *pu16Len = (u16)sizeof(AC_MessageHead) + u16PayloadLen + u16OptLen;
}



/*************************************************
* Function: AC_SendDeviceStart
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_SendDeviceStart(u32 UartBase, AC_OptList *pstruOptList)
{
    //设备启动通知
    u16 u16DateLen;
    
    AC_BuildMessage(AC_CODE_EQ_BEGIN, 0, 
        NULL, 0,        /*payload+payload len*/
        pstruOptList,
        g_u8MsgBuildBuffer, &u16DateLen);
    
    AC_SendMessage(UartBase, g_u8MsgBuildBuffer, u16DateLen);
}

/*************************************************
* Function: AC_SendDeviceRegsiter
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/

void AC_HexToString(u8* HexBuf,u8 *StringBuf,u8 len)
{
  u8 i;
  u8 *xad;

  // Display the extended address.
  xad = HexBuf + len - 1;

  for (i = 0; i < len*2; xad--)
  {
    u8 ch;
    ch = (*xad >> 4) & 0x0F;
    StringBuf[i++] = ch + (( ch < 10 ) ? '0' : '7');
    ch = *xad & 0x0F;
    StringBuf[i++] = ch + (( ch < 10 ) ? '0' : '7');
  }
 // buf[len*2] = '\0';
}

/*************************************************
* Function: AC_SendDeviceRegsiter
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_SendDeviceRegsiter(u32 UartBase, AC_OptList *pstruOptList, u8 *pu8EqVersion, u8 *pu8ModuleKey, u64 u64Domain, u8 *pu8DeviceId)
{
    //设备注册请求
    static AC_RegisterReq struReg;
    u16 u16DateLen;
    u64Domain =     AC_HTONLU64(u64Domain); 
    memcpy(struReg.u8EqVersion, pu8EqVersion, AC_EQVERSION_LEN);
    memcpy(struReg.u8ModuleKey, pu8ModuleKey, AC_MODULE_KEY_LEN); 
    memcpy(struReg.u8Domain, &u64Domain, AC_DOMAIN_LEN); 
    memcpy(struReg.u8DeviceId, pu8DeviceId, AC_HS_DEVICE_ID_LEN); 

    AC_BuildMessage(AC_CODE_REGSITER, 0, 
        (u8*)&struReg, sizeof(AC_RegisterReq),   /*payload+payload len*/
        pstruOptList,
        g_u8MsgBuildBuffer, &u16DateLen);
    
    AC_SendMessage(UartBase, g_u8MsgBuildBuffer, u16DateLen);
}

/*************************************************
* Function: AC_SendAckMsg
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_SendAckMsg(u32 UartBase, AC_OptList *pstruOptList, u8 u8MsgId)
{
    //OTA正确响应
    u16 u16DateLen;
    AC_BuildMessage(AC_CODE_ACK, u8MsgId, 
        NULL, 0,        /*payload+payload len*/
        pstruOptList,
        g_u8MsgBuildBuffer, &u16DateLen);
    
    AC_SendMessage(UartBase, g_u8MsgBuildBuffer, u16DateLen);
}

/*************************************************
* Function: AC_SendErrMsg
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_SendErrMsg(u32 UartBase, AC_OptList *pstruOptList, u8 u8MsgId, u8 *pu8ErrorMsg, u16 u16DataLen)
{
    //OTA错误响应
    u16 u16DateLen;
    AC_BuildMessage(AC_CODE_ERR, u8MsgId, 
        pu8ErrorMsg, u16DataLen,        /*payload+payload len*/
        pstruOptList,
        g_u8MsgBuildBuffer, &u16DateLen);
    
    AC_SendMessage(UartBase, g_u8MsgBuildBuffer, u16DateLen);
}


/*************************************************
* Function: AC_SendRestMsg
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_SendRestMsg(u32 UartBase, AC_OptList *pstruOptList)
{
    //wifi密码重置
    u16 u16DateLen;
    AC_BuildMessage(AC_CODE_REST, 0, 
        NULL, 0,        /*payload+payload len*/
        pstruOptList,
        g_u8MsgBuildBuffer, &u16DateLen);
    
    AC_SendMessage(UartBase, g_u8MsgBuildBuffer, u16DateLen);
}

/*************************************************
* Function: AC_ParseOption
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_ParseOption(AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u16 *pu16OptLen)
{
    //解析Option
    u8 u8OptNum;
    AC_MessageOptHead *pstruOptHead;
    u16 u16Offset;

    u16Offset = sizeof(AC_MessageHead);
    pstruOptHead = (AC_MessageOptHead *)((u8*)pstruMsg + u16Offset);
    *pu16OptLen = 0;

    for (u8OptNum = 0; u8OptNum < pstruMsg->OptNum; u8OptNum++)
    {
        switch (AC_HTONS(pstruOptHead->OptCode))
        {
            case AC_OPT_TRANSPORT:
                pstruOptList->pstruTransportInfo = (AC_TransportInfo *)(pstruOptHead + 1);
                break;
            case AC_OPT_SSESSION:
                pstruOptList->pstruSsession = (AC_SsessionInfo *)(pstruOptHead + 1);            
                break;
        }
        *pu16OptLen += sizeof(AC_MessageOptHead) + AC_HTONS(pstruOptHead->OptLen);
        pstruOptHead = (AC_MessageOptHead *)((u8*)pstruOptHead + sizeof(AC_MessageOptHead) + AC_HTONS(pstruOptHead->OptLen));

    }
}

/*************************************************
* Function: AC_CheckCrc
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
u32 AC_CheckCrc(u8 *pu8Crc, u8 *pu8Data, u16 u16Len)
{
    u16 u16Crc;
    u16 u16RecvCrc;
    u16Crc = crc16_ccitt(pu8Data, u16Len);
    u16RecvCrc = (pu8Crc[0] << 8) + pu8Crc[1];
    if (u16Crc == u16RecvCrc)
    {
        return AC_RET_OK;
    }
    else
    {
        AC_Printf("Crc is error, recv = %d, calc = %d\n", u16RecvCrc, u16Crc);    
        
        return AC_RET_ERROR;    
    }
}


/*************************************************
* Function: AC_RecvMessage
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_RecvMessage(u32 UartBase, AC_MessageHead *pstruMsg)
{
    //串口收到消息后，需要调用该接口处理消息。
    AC_OptList struOptList;
    u16 u16OptLen = 0;
    u8 *pu8Playload = NULL;

    struOptList.pstruSsession = NULL;
    struOptList.pstruTransportInfo = NULL;
    if (AC_RET_ERROR == AC_CheckCrc(pstruMsg->TotalMsgCrc, (u8*)(pstruMsg + 1), AC_HTONS(pstruMsg->Payloadlen)))
    {
        return;
    }
    /*Parser Option*/
    AC_ParseOption(pstruMsg, &struOptList, &u16OptLen);
    pu8Playload = (u8*)pstruMsg + u16OptLen + sizeof(AC_MessageHead);
    switch(pstruMsg->MsgCode)
    {
        //事件通知类消息
        case AC_CODE_EQ_DONE:
        case AC_CODE_WIFI_CONNECTED:
        case AC_CODE_WIFI_DISCONNECTED:
        case AC_CODE_CLOUD_CONNECTED:
        case AC_CODE_CLOUD_DISCONNECTED:
            AC_DealNotifyMessage(UartBase, pstruMsg, &struOptList, pu8Playload);
            break;
        //OTA类消息
        case AC_CODE_OTA_BEGIN:
        case AC_CODE_OTA_FILE_BEGIN:
        case AC_CODE_OTA_FILE_CHUNK:
        case AC_CODE_OTA_FILE_END:
        case AC_CODE_OTA_END:
            AC_DealOtaMessage(UartBase, pstruMsg, &struOptList, pu8Playload);
            break;
        //设备事件类消息    
        default:
            if (pstruMsg->MsgCode >= AC_CODE_EVENT_BASE)
            {
                AC_DealEvent(UartBase, pstruMsg, &struOptList, pu8Playload);
            }
            break;            
    }
}

