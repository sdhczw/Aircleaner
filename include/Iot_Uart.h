//*****************************************************************************
//
// Beta_Arm.h - Simple hello world example.
//
// Copyright (c) 2012-2014 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.0.12573 of the EK-TM4C123GXL Firmware Package.
//
//*****************************************************************************
#ifndef IOT_UART_H
#define IOT_UART_H

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"

#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/interrupt.h"
#include "bmd.h"
#include "ac_api.h"
#include <ac_common.h>
#include <ac_protocol_interface.h>
//*****************************************************************************
//
// macro
//
//*****************************************************************************
//#define RCTRL_MSG_FLAG		       0x02030405
//#define RCTRL_MSG_PREFIX		"\2\3\4\5"
//#define AT_CMD_PREFIX			"AT#"
//#define ARM_ZIGBEE_MSG_FLAG		"ARM#"
//#define ZIGBEE_ARM_MSG_FLAG		"ZIGB"
//#define PRINT_FLAG                      "\1\2\3\4"



#define NUM_SSI_DATA 3
#define FLASH_SN_ADDR ((u32)&(g_u64_air_machine_sn))

//how many packets can be buffered in rxring ,(each packet size need < UARTRX_RING_LEN - 1)
#define    NUM_DESCS           30

//buffer length for uart rx buffer whose data is moved from uart UART HW RX FIFO
#define    UART1RX_RING_LEN    1024   
#define    AC_PAYLOADLENOFFSET 9

#define    MAINNODE_COORDINATOR 0

#define    SUBNODE_ROUTER  1

#define    SUBNODE_ENDDEVICE  2
// ================================================================
//					Message Definition
// ================================================================

//*****************************************************************************
//
// message define
//
//*****************************************************************************
typedef enum
{
    OPT_TRANSPORT = 0
}RCTRL_OPT_TYPE;


typedef enum
{
    ARM_ZIGB_QUERY_MAINNODE_ADDR_REQ = 0,
    ARM_ZIGB_SET_NET_ACCESS_REQ = 1,
    ARM_ZIGB_SET_CLIENT_LIST_REQ = 2,
    ZIGB_ARM_QUERY_MAINNODE_ADDR_RSP = 0x1000,
}ARM_ZIGBEE_MSG_TYPE;

typedef enum {
    PKT_UNKNOWN,
    PKT_ATCMD,
    PKT_PUREDATA,
    PKT_ZADATA,
    PKT_PRINTCMD,
    PKT_HR01DATA,
    PKT_T6603DATA
} PKT_TYPE;

typedef struct tag_RCTRL_STRU_MSGHEAD
{
    u32 MsgFlag;         //消息标示 byte0:0X76, byte1:0x81, byte2:0x43, byte3:0x50 0x76814350
    AC_MessageHead  struMessageHead;
}RCTRL_STRU_MSGHEAD;    //消息头定义

typedef struct tag_ARM_ZIGB_MSG_HEAD
{
    u16 MsgType;        //消息类型
    u16 MsgLen;
}ARM_ZIGB_MSG_HEAD;    //消息头定义

typedef struct tag_ARM_ZIGB_MSG_HEADER
{
    s8 MsgFlag[4];         //消息标示 byte0:0X76, byte1:0x81, byte2:0x43, byte3:0x50 0x76814350
    ARM_ZIGB_MSG_HEAD MsgHead;
}ARM_ZIGB_MSG_HEADER;    //消息头定义

typedef struct tag_ZIGB_ARM_QUERY_MSG_RSP
{
   ARM_ZIGB_MSG_HEAD MsgHead;
    u8  MachineType;        //主子机状态0：子机，1：主机
    u8 sn[8];             //sn
}ZIGB_ARM_QUERY_MSG_RSP;    //消息头定义

typedef struct tag_SET_NET_ACCESS_REQ
{        //消息标示 byte0:0X76, byte1:0x81, byte2:0x43, byte3:0x50 0x76814350
    ARM_ZIGB_MSG_HEADER MsgHead;
    u8 u8AccessTime; //单位：s 0，关闭，ff打开，中间值是时间
}ARM_ZIGB_SET_ACCESS_TIMER_REQ;    //消息头定义

typedef struct tag_SET_CLIENT_LIST
{
    ARM_ZIGB_MSG_HEADER MsgHeader;
    u8 u8BlackClientNum;
    u8 u8WhiteClientNum;    
    u8 u8Pad[2];
    u8 DeviceId[0];     //用户ID，定长AC_HS_DEVICE_ID_LEN（8字节），依次下排
}ARM_ZIGB_CLIENT_LIST;    //消息头定义




typedef struct tag_STRU_SERVER_CLIENT_SET_REPROT_PERIOD_REQ
{
    u32      u32Period;        // 单位：秒
}STRU_SERVER_CLIENT_SET_REPROT_PERIOD_REQ;

typedef struct tag_STRU_SERVER_CLIENT_SET_TH_REQ
{	
    u32      u32Type;          //上报数据类型
    u32      u32Threshold;    //上报阈值
} STRU_SERVER_CLIENT_SET_TH_REQ;


typedef struct {
    PKT_TYPE pkt_type;
    u16   pkt_len;
}PKT_FIFO;//packet infor is in sequence with index[0,num_desc-1] which mapping the sequence in rx


/*
* 2014/05/13,terrence
* struct to descrypt the packet in ring buffer
* we just need to know the packet number and their lengh in sequence
*/
typedef struct {
    PKT_TYPE  cur_type;              //receiving packet:which packet type is receiving current? 
    u8     cur_num;               //receiving packet:current index of receiving packet
    u8     pkt_num;               //completely packets:packet number in ring buffer
    PKT_FIFO  infor[NUM_DESCS];      //completely packets:FIFO,packet infor in ring
} PKT_DESC; 


typedef struct
{
    BUFFER_INFO                    Rx_Buffer;  //receive buffer
    PKT_DESC                       Rx_desc;    //description       
    
    BUFFER_INFO                    Tx_Buffer;  //transmit buffer
    
} UARTStruct;

// ================================================================
//				Golbal	Variable Definition
// ================================================================
extern u64 g_u64_air_machine_sn;

extern u8 g_u8_wifi_powstatus;

extern u8 g_u8_machineType;
//char Zigbee_Arm_Flag[] = ZIGBEE_ARM_MSG_FLAG;
//*****************************************************************************
//
//! fuction
//
//*****************************************************************************

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

void UartSend (u32 Base, u8  *Buffer, u16  NByte);
void UartInit();
void SendMsg2Zigbee( u8  *Buffer, u16  NByte);
void SendMsg2Wifi( u8  *Buffer, u16  NByte);
u32 get_register_status();
void ProcessWifiMsg();
void ProcessZigbeeMsg();
void Send2ServerCommonMsg(u32 Base,u8 Version,u8 MsgCode,u8 MsgId);
u8 get_cloud_connect_status();
void SendDevStatus2Server(u32 Base, u8 Version,u8 MsgId);
s8 ConfigureUART5(u32  BaudRate, u8  Prio);
s8 ConfigureUART7(u32  BaudRate, u8  Prio);
#endif

