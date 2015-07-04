/****************************************Copyright (c)**************************************************
**                                 Co.,LTD.
**                                     
**                                
**
**--------------File Info-------------------------------------------------------------------------------
** File Name:               Uart0.c
** Last modified Date:      2014.10.03
** Last Version:            v1.0
** Description:             串口驱动源文件
********************************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <ac_common.h>
#include "Iot_Uart.h"
#include "Iot_Timer.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/interrupt.h"
#include "Iot_Lcd.h"
#include <stdio.h>

//*****************************************************************************
//
// macro
//
//*****************************************************************************
#define RCTRL_MSG_FLAG		       0x02030405
#define RCTRL_MSG_PREFIX		"\2\3\4\5"
#define AT_CMD_PREFIX			"AT#"
#define ARM_ZIGBEE_MSG_FLAG		"ARM#"
#define ZIGBEE_ARM_MSG_FLAG		"ZIGB"
#define PRINT_FLAG                      "\1\2\3\4"
#define    UART1RX_RING_LEN    1024   
#define    UART3RX_RING_LEN    1024  
#define    AC_PAYLOADLENOFFSET 9

#define    UART_BAUTRATE       115200

#define    UART0_INTPRIO       6

#define    UART1_INTPRIO       6

#define    UART3_INTPRIO       6


// ================================================================
//				Golbal	Variable Definition
// ================================================================
u8 UART1RxBuf[UART1RX_RING_LEN];
u8 UART3RxBuf[UART3RX_RING_LEN];
char ATCmdPrefix[] = AT_CMD_PREFIX;
char ZADataPrefix[] = ZIGBEE_ARM_MSG_FLAG;
char PrintCmdPrefix[] = PRINT_FLAG;
char PureDataPrefix[] = RCTRL_MSG_PREFIX;//ARM DATA
//#pragma location = 0x4000
UARTStruct    UART1Port;
UARTStruct    UART3Port;
u8 pCmdWifiBuf[UART1RX_RING_LEN];
u8 pCmdZigBuf[UART3RX_RING_LEN];
u8 g_queryrspmsg_index = 0;
u8 g_struqueryrspmsg[64] ={0};
u8  g_u8_machineType = MAINNODE_COORDINATOR;
u64 g_u64_air_machine_sn = 0;
//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
s8 ConfigureUART0(u32  BaudRate, u8  Prio)
{
    if (BaudRate > 115200) {                                            /*  波特率太高，错误返回       */
        return(AC_RET_ERROR);
    }
    
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    
    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    
    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    
    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    
    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, BaudRate, 16000000);
    return AC_RET_OK;
}
//*****************************************************************************
//
// Configure the UART and pb0,pb1.  This is used for communicating with 7681.
//
//*****************************************************************************
s8 ConfigureUART1(u32  BaudRate, u8  Prio)
{
    
    if (BaudRate > 115200) {                                            /*  波特率太高，错误返回       */
        return(AC_RET_ERROR);
    }
    
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    
    //
    // Enable UART1
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    
    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PB0_U1RX);
    ROM_GPIOPinConfigure(GPIO_PB1_U1TX);
    ROM_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    
    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART1_BASE, UART_CLOCK_PIOSC);
    
    //
    // Initialize the UART for console I/O.
    //
    UARTConfigSetExpClk(UART1_BASE, 16000000, BaudRate,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));
    IntPrioritySet(INT_UART1, Prio<<5);                                 /*  设置中断优先级             */
    UARTFIFOLevelSet(UART1_BASE, UART_FIFO_TX1_8, UART_FIFO_RX4_8);
    IntEnable(INT_UART1);                                               /*  使能串口0系统中断          */
    UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);               /*  使能串口0接收中断和接收超时*/
    /*  中断                       */
    UARTEnable(UART1_BASE); 
    return AC_RET_OK;
}
//*****************************************************************************
//
// Configure the UART and pc6,pc7.  This is used for communicating with zigbee.p0.2 p0.3
//
// RX: PC6<====> TX p0.3
// TX: PC7<====> TX p0.2
//*****************************************************************************
s8 ConfigureUART3(u32  BaudRate, u8  Prio)
{
    if (BaudRate > 115200) {                                            /*  波特率太高，错误返回       */
        return(AC_RET_ERROR);
    }
    
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    
    //
    // Enable UART3
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART3);
    
    //
    // Configure GPIO Pins for UART mode.
    //
    GPIOPinConfigure(GPIO_PC6_U3RX);
    GPIOPinConfigure(GPIO_PC7_U3TX);
    GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_6 | GPIO_PIN_7);
    
    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART3_BASE, UART_CLOCK_PIOSC);
    
    //
    // 
    //
    UARTConfigSetExpClk(UART3_BASE, 16000000, BaudRate,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));
    IntPrioritySet(INT_UART3, Prio<<5);                                 /*  设置中断优先级             */
    UARTFIFOLevelSet(UART3_BASE, UART_FIFO_TX1_8, UART_FIFO_RX4_8);
    IntEnable(INT_UART3);                                               /*  使能串口0系统中断          */
    UARTIntEnable(UART3_BASE, UART_INT_RX | UART_INT_RT);               /*  使能串口0接收中断和接收超时*/
    /*  中断                       */
    UARTEnable(UART3_BASE);       
    return AC_RET_OK;
}

//*****************************************************************************
//
// Configure the UART and pc6,pc7.  This is used for communicating with zigbee.
//
//*****************************************************************************
s8 ConfigureUART5(u32  BaudRate, u8  Prio)
{
    if (BaudRate > 115200) {                                            /*  波特率太高，错误返回       */
        return(AC_RET_ERROR);
    }

  //
  // Enable the GPIO Peripheral used by the UART.
  //
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
  
  //
  // Enable UART3
  //
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5);
  
  //
  // Configure GPIO Pins for UART mode.
  //
  GPIOPinConfigure(GPIO_PE4_U5RX);
  GPIOPinConfigure(GPIO_PE5_U5TX);
  GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);
  
  //
  // Use the internal 16MHz oscillator as the UART clock source.
  //
  UARTClockSourceSet(UART5_BASE, UART_CLOCK_PIOSC);
  
  //
  // 
  //
  UARTConfigSetExpClk(UART5_BASE, 16000000, BaudRate,
                      (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                       UART_CONFIG_PAR_NONE));
  IntPrioritySet(INT_UART5, Prio<<5);                                 /*  设置中断优先级             */
  UARTFIFOLevelSet(UART5_BASE, UART_FIFO_TX2_8, UART_FIFO_RX4_8);
  IntEnable(INT_UART5);                                               /*  使能串口0系统中断          */
  UARTIntEnable(UART5_BASE, UART_INT_RX | UART_INT_RT);               /*  使能串口0接收中断和接收超时*/
  /*  中断                       */
  UARTEnable(UART5_BASE);       
  return AC_RET_OK;
}

//*****************************************************************************
//
// Configure the UART and pc6,pc7.  This is used for communicating with zigbee.
//
//*****************************************************************************
s8 ConfigureUART7(u32  BaudRate, u8  Prio)
{
    if (BaudRate > 115200) {                                            /*  波特率太高，错误返回       */
        return(AC_RET_ERROR);
    }
    
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    
    //
    // Enable UART3
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART7);
    
    //
    // Configure GPIO Pins for UART mode.
    //
    GPIOPinConfigure(GPIO_PE0_U7RX);
    GPIOPinConfigure(GPIO_PE1_U7TX);
    GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    
    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART7_BASE, UART_CLOCK_PIOSC);
    
    //
    // 
    //
    UARTConfigSetExpClk(UART7_BASE, 16000000, BaudRate,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));
    IntPrioritySet(INT_UART7, Prio<<5);                                 /*  设置中断优先级             */
    UARTFIFOLevelSet(UART7_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);
    IntEnable(INT_UART7);                                               /*  使能串口0系统中断          */
    UARTIntEnable(UART7_BASE, UART_INT_RX);               /*  使能串口0接收中断和接收超时*/
    /*  中断                       */
    UARTEnable(UART7_BASE);       
    return AC_RET_OK;
}

/*********************************************************************************************************
** Function name:           Uart0Init
** Descriptions:            初始化Uart0 
** input parameters:        BaudRate:   波特率
**                          Prio:       中断优先级
** Output parameters::      无
** Returned value:          TRUE :      成功
**                          FALSE:      失败
** Created by:              
** Created Date:            
**--------------------------------------------------------------------------------------------------------
** Modified by:            
** Modified date:          
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/

void UARTRx_Buf_Init(UARTStruct *qp, u8 *rxbuf, u16 len)
{
    PKT_DESC     *rx_desc = &(qp->Rx_desc);
    BUFFER_INFO  *rx_ring = &(qp->Rx_Buffer);
    
    rx_desc->pkt_num = 0;
    rx_desc->cur_num = 0;
    rx_desc->cur_type = PKT_UNKNOWN;
    Buf_init(rx_ring,(rxbuf),(u16)len);
}


/*********************************************************************************************************
** Function name:           Uart0Init
** Descriptions:            初始化Uart0 
** input parameters:        BaudRate:   波特率
**                          Prio:       中断优先级
** Output parameters::      无
** Returned value:          TRUE :      成功
**                          FALSE:      失败
** Created by:              
** Created Date:            2006.11.18
**--------------------------------------------------------------------------------------------------------
** Modified by:            
** Modified date:           2007.10.20 
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
void UartInit()
{
    ConfigureUART0(UART_BAUTRATE,UART0_INTPRIO);
    ConfigureUART1(UART_BAUTRATE,UART1_INTPRIO);
    ConfigureUART3(UART_BAUTRATE,UART3_INTPRIO);
    UARTRx_Buf_Init((UARTStruct*)(&UART1Port),(u8 *)(UART1RxBuf),UART1RX_RING_LEN);                             
    UARTRx_Buf_Init((UARTStruct*)(&UART3Port),(u8 *)(UART3RxBuf),UART3RX_RING_LEN); 
    return;
}

/*********************************************************************************************************
** Function name:           Uart0Send
** Descriptions:            发送多个字节数据
** input parameters:        Buffer:发送数据存储位置
**                          NByte:发送数据个数
** Output parameters::      无
** Returned value:          无
** Created by:             
** Created Date:             2014.10.03
**--------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           2014.10.03
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
void UartSend (u32 Base, u8  *Buffer, u16  NByte)
{
    IntMasterDisable();
    while (NByte) {
        if ( UARTSpaceAvail(Base) ) {
            UARTCharPutNonBlocking(Base, *Buffer++);
            NByte--;
        }
    }
    while (UARTBusy(Base) ) {
        ;
    }
    IntMasterEnable();
}

void SendMsg2Wifi( u8  *Buffer, u16  NByte)
{
    UartSend(UART1_BASE,Buffer,NByte);
}

void SendMsg2Zigbee( u8  *Buffer, u16  NByte)
{
    UartSend(UART3_BASE,Buffer,NByte);
    
}

/*********************************************************************************************************
** Function name:           UART1_ISR
**  Descriptions:           串口1中断服务函数
** input parameters:        无
** Output parameters::      无
** Returned value:          无
** Created by:              
** Created Date:            2014.10.03
**--------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           2014.10.03
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
void UART1_ISR(void)
{
    u16   roomleft = 0;
    PKT_FIFO     *infor;
    PKT_FIFO     *temp_info;
    u8        ch = 0;
    u8 i= 0;
    
    PKT_DESC     *rx_desc = &(UART1Port.Rx_desc);
    BUFFER_INFO  *rx_ring = &(UART1Port.Rx_Buffer); 
    static u16 AMHeadLen = sizeof(RCTRL_STRU_MSGHEAD);
    static u16 AMBodyLen =0;
    static u8  ATMatchNum = 0;
    static u8  PDMatchNum = 0;
    static u8  PrintMatchNum = 0;
    unsigned long ulStatus; 
    Buf_GetRoomLeft(rx_ring,roomleft);
    
    ulStatus = UARTIntStatus(UART1_BASE, true);                         /*  读取已使能的串口0中断状态   */
    UARTIntClear(UART1_BASE, ulStatus);                                 /*  清除当前的串口0中断         */
    if((ulStatus & UART_INT_RT)||(ulStatus & UART_INT_RX)) {            /*  接收中断                    */
        while(UARTCharsAvail(UART1_BASE))
        {
            ch = UARTCharGetNonBlocking(UART1_BASE);
            switch (rx_desc->cur_type)
            {
                case PKT_UNKNOWN:
                {  
                    /**************** detect packet type ***************/
                    if (ATCmdPrefix[ATMatchNum] == ch)
                    {         
                        ATMatchNum++;
                    }
                    else
                    {         
                        ATMatchNum = 0;
                    }
                    if (PureDataPrefix[PDMatchNum] == ch)
                    {         
                        PDMatchNum++;
                    }
                    else
                    {         
                        PDMatchNum = 0;
                    } 
                    if (PrintCmdPrefix[PrintMatchNum] == ch)
                    {         
                        PrintMatchNum++;
                    }
                    else
                    {         
                        PrintMatchNum = 0;
                    }         
                    if ((ATMatchNum == sizeof(ATCmdPrefix)-1) ||   //match case 1: AT#
                        (PDMatchNum == sizeof(PureDataPrefix)-1) ||  
                            (PrintMatchNum == sizeof(PrintCmdPrefix)-1))   //match case 3:arm  data
                    {   
                        
                        rx_desc->cur_num = rx_desc->pkt_num;                  
                        infor = &(rx_desc->infor[rx_desc->cur_num]);
                        infor->pkt_len = 0;
                        
                        if(ATMatchNum == sizeof(ATCmdPrefix)-1)
                        {            
                            rx_desc->cur_type = PKT_ATCMD;           //match case 1: AT#
                        }
                        else if (PrintMatchNum == sizeof(PrintCmdPrefix)-1)
                        {            
                            rx_desc->cur_type = PKT_PRINTCMD;           //match case 2:iwpriv ra0
                        }
                        else if (PDMatchNum == sizeof(PureDataPrefix)-1)
                        {            
                            rx_desc->cur_type = PKT_PUREDATA;           //match case 2:iwpriv ra0
                            if(roomleft<AMHeadLen)
                            {
                                rx_desc->cur_type= PKT_UNKNOWN;
                            }
                            else
                            {
                                for(i = 0;i < sizeof(PureDataPrefix)-1;i++)
                                {
                                    Buf_Push(rx_ring,PureDataPrefix[i]);
                                }
                                roomleft= roomleft-sizeof(PureDataPrefix)+1;
                                infor = &(rx_desc->infor[rx_desc->cur_num]);
                                infor->pkt_len = infor->pkt_len + i;
                            }                                       
                        }
                        
                        ATMatchNum = 0;
                        PrintMatchNum = 0;
                        PDMatchNum = 0;
                        continue;
                    }           
                }
                break;
                case PKT_ATCMD:
                {   
                    infor = &(rx_desc->infor[rx_desc->cur_num]);
                    
                    /*
                    * received one complete packet
                    */
                    if(ch == '\n' || ch == '\r')
                    {   
                        //if task has consumed some packets
                        if (rx_desc->cur_num != rx_desc->pkt_num)
                        {   
                            temp_info = infor;
                            infor     = &(rx_desc->infor[rx_desc->pkt_num]);
                            infor->pkt_len = temp_info->pkt_len;
                            temp_info->pkt_len = 0;
                            temp_info->pkt_type = PKT_UNKNOWN;
                        }
                        
                        infor->pkt_type = rx_desc->cur_type;  // PKT_ATCMD / PKT_IWCMD;
                        rx_desc->pkt_num++;
                        rx_desc->cur_type = PKT_UNKNOWN;
                    }
                    /*
                    * continue to receiving packet
                    */                
                    else
                    {
                        Buf_Push(rx_ring,ch);
                        roomleft--;
                        infor->pkt_len++;
                    }
                    
                    
                    /*
                    * if overflow,we discard the current packet
                    * example1:packet length > ring size
                    * example2:rx ring buff can no be freed by task as quickly as rx interrupt coming
                    */    
                    if ((!roomleft) || (rx_desc->pkt_num >= NUM_DESCS))
                    {   
                        //rollback
                        Buff_RollBack(rx_ring,infor->pkt_len);
                        
                        roomleft += infor->pkt_len;
                        
                        infor->pkt_type = PKT_UNKNOWN;
                        infor->pkt_len = 0;
                        rx_desc->cur_type = PKT_UNKNOWN;
                        
                        if (rx_desc->pkt_num >= NUM_DESCS)
                        {
                            rx_desc->pkt_num--;
                        }
                        
                    }
                }  
                break;
                case PKT_PRINTCMD:
                {          
                    /*
                    * received one complete packet
                    */
                    if(ch == '\0'||ch == '\n' || ch == '\r')
                    {   
                        rx_desc->cur_type = PKT_UNKNOWN;
                        UARTprintf("%c",ch);
                        return;
                    }
                    /*
                    * continue to receiving packet
                    */                
                    UARTprintf("%c",ch);
                }
                break;
                
                case PKT_PUREDATA:
                {   
                    infor = &(rx_desc->infor[rx_desc->cur_num]);
                    Buf_Push(rx_ring,ch);
                    roomleft--;
                    infor->pkt_len++;
                    if(infor->pkt_len==AC_PAYLOADLENOFFSET)
                    {
                        AMBodyLen = ch;
                    }
                    else if(infor->pkt_len==(AC_PAYLOADLENOFFSET +1))
                    {
                        AMBodyLen = (AMBodyLen<<8) + ch;
                    }   
                    /*
                    * if overflow,we discard the current packet
                    * example1:packet length > ring size
                    * example2:rx ring buff can no be freed by task as quickly as rx interrupt coming
                    */    
                    if ((!roomleft) || (rx_desc->pkt_num >= NUM_DESCS))
                    {   
                        //rollback
                        Buff_RollBack(rx_ring,infor->pkt_len);
                        
                        roomleft += infor->pkt_len;
                        
                        infor->pkt_type = PKT_UNKNOWN;
                        infor->pkt_len = 0;
                        rx_desc->cur_type = PKT_UNKNOWN;
                        
                        if (rx_desc->pkt_num >= NUM_DESCS)
                        {
                            rx_desc->pkt_num--;
                        }
                        
                    }      
                    /*
                    * received one complete packet
                    */
                    if(AMHeadLen+AMBodyLen==infor->pkt_len)
                    {   
                        //if task has consumed some packets
                        if (rx_desc->cur_num != rx_desc->pkt_num)
                        {   
                            temp_info = infor;
                            infor     = &(rx_desc->infor[rx_desc->pkt_num]);
                            infor->pkt_len = temp_info->pkt_len;
                            temp_info->pkt_len = 0;
                            temp_info->pkt_type = PKT_UNKNOWN;
                        }
                        
                        infor->pkt_type = rx_desc->cur_type;  // PKT_ATCMD / PKT_IWCMD;
                        rx_desc->pkt_num++;
                        rx_desc->cur_type = PKT_UNKNOWN;
                        AMBodyLen =0;
                    }
                                   
                    

                }
                break;
                default:
                break;
                
            }
            
        }    
    }
    return;
}
/*********************************************************************************************************
** Function name:           UART3_ISR
**  Descriptions:           串口1中断服务函数
** input parameters:        无
** Output parameters::      无
** Returned value:          无
** Created by:              
** Created Date:            2014.10.03
**--------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           2014.10.03
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
void UART3_ISR (void)
{
    u16   roomleft = 0;
    PKT_FIFO     *infor;
    PKT_FIFO     *temp_info;
    u8        ch = 0;
    
    
    PKT_DESC     *rx_desc = &(UART3Port.Rx_desc);
    BUFFER_INFO  *rx_ring = &(UART3Port.Rx_Buffer); 
    RCTRL_STRU_MSGHEAD  StruMsgHead;
    
    
    static u16 AMHeadLen = sizeof(StruMsgHead);
    static u16  MsgFlagLen = sizeof(StruMsgHead.MsgFlag);
    static u16 ZAHeadLen= sizeof(ARM_ZIGB_MSG_HEAD);
    static u16 AMBodyLen =0; 
    static u16 ZADataLen = 0;
    static u8  ATMatchNum = 0;
    static u8  PDMatchNum = 0;
    static u8  ZAMatchNum = 0;//arm d
    static u8  PrintMatchNum = 0;//arm d
    unsigned long ulStatus;   
    
    Buf_GetRoomLeft(rx_ring,roomleft);
    
    ulStatus = UARTIntStatus(UART3_BASE, true);                         /*  读取已使能的串口0中断状态   */
    UARTIntClear(UART3_BASE, ulStatus);                                 /*  清除当前的串口0中断         */
    if((ulStatus & UART_INT_RT)||(ulStatus & UART_INT_RX)) {            /*  接收中断                    */
        while(UARTCharsAvail(UART3_BASE))
        {
            ch = UARTCharGetNonBlocking(UART3_BASE);
            switch (rx_desc->cur_type)
            {
                
                case PKT_UNKNOWN:
                {  
                    
                    /**************** detect packet type ***************/
                    //support more ATcmd prefix analysis
                    /*case 1:AT#*/
                    if (ATCmdPrefix[ATMatchNum] == ch)
                    {         
                        ATMatchNum++;
                    }
                    else
                    {         
                        ATMatchNum = 0;
                    }
                    
                    
                    if (ZADataPrefix[ZAMatchNum] == ch)
                    {         
                        ZAMatchNum++;
                    }
                    else
                    {         
                        ZAMatchNum = 0;
                    }
                    if (PureDataPrefix[PDMatchNum] == ch)
                    {         
                        PDMatchNum++;
                    }
                    else
                    {         
                        PDMatchNum = 0;
                    } 
                    if (PrintCmdPrefix[PrintMatchNum] == ch)
                    {         
                        PrintMatchNum++;
                    }
                    else
                    {         
                        PrintMatchNum = 0;
                    }         
                    if ((ATMatchNum == sizeof(ATCmdPrefix)-1) ||   //match case 1: AT#
                        (PDMatchNum == sizeof(PureDataPrefix)-1) ||  
                            (PrintMatchNum == sizeof(PrintCmdPrefix)-1)|| 
                                (ZAMatchNum == sizeof(ZADataPrefix)-1) )   //match case 3:arm  data
                    {   
                        
                        rx_desc->cur_num = rx_desc->pkt_num;                  
                        infor = &(rx_desc->infor[rx_desc->cur_num]);
                        infor->pkt_len = 0;
                        
                        if(ATMatchNum == sizeof(ATCmdPrefix)-1)
                        {            
                            rx_desc->cur_type = PKT_ATCMD;           //match case 1: AT#
                        }
                        else if (PrintMatchNum == sizeof(PrintCmdPrefix)-1)
                        {            
                            rx_desc->cur_type = PKT_PRINTCMD;           //match case 2:iwpriv ra0
                        }
                        else if (ZAMatchNum == sizeof(ZADataPrefix)-1)
                        {            
                            rx_desc->cur_type = PKT_ZADATA;           //match case 2:iwpriv ra0
                        }
                        
                        else if (PDMatchNum == sizeof(PureDataPrefix)-1)
                        {            
                            rx_desc->cur_type = PKT_PUREDATA;           //match case 2:iwpriv ra0
                            if(roomleft<AMHeadLen)
                            {
                                rx_desc->cur_type= PKT_UNKNOWN;
                            }
                            else
                            {
                                Put32toBuf(rx_ring,((u32*)PureDataPrefix));
                                infor = &(rx_desc->infor[rx_desc->cur_num]);
                                infor->pkt_len = infor->pkt_len + sizeof(StruMsgHead.MsgFlag);
                            }                                       
                        }
                        
                        ATMatchNum = 0;
                        PrintMatchNum = 0;
                        PDMatchNum = 0;
                        ZAMatchNum = 0;
                        continue;
                    }           
                }
                break;
                                              
                case PKT_ATCMD:
                {   
                    infor = &(rx_desc->infor[rx_desc->cur_num]);
                    
                    /*
                    * received one complete packet
                    */
                    if(ch == '\n' || ch == '\r')
                    {   
                        //if task has consumed some packets
                        if (rx_desc->cur_num != rx_desc->pkt_num)
                        {   
                            temp_info = infor;
                            infor     = &(rx_desc->infor[rx_desc->pkt_num]);
                            infor->pkt_len = temp_info->pkt_len;
                            temp_info->pkt_len = 0;
                            temp_info->pkt_type = PKT_UNKNOWN;
                        }
                        
                        infor->pkt_type = rx_desc->cur_type;  // PKT_ATCMD / PKT_IWCMD;
                        rx_desc->pkt_num++;
                        rx_desc->cur_type = PKT_UNKNOWN;
                    }
                    /*
                    * continue to receiving packet
                    */                
                    else
                    {
                        Buf_Push(rx_ring,ch);
                        roomleft--;
                        infor->pkt_len++;
                    }
                    
                    
                    /*
                    * if overflow,we discard the current packet
                    * example1:packet length > ring size
                    * example2:rx ring buff can no be freed by task as quickly as rx interrupt coming
                    */    
                    if ((!roomleft) || (rx_desc->pkt_num >= NUM_DESCS))
                    {   
                        //rollback
                        Buff_RollBack(rx_ring,infor->pkt_len);
                        
                        roomleft += infor->pkt_len;
                        
                        infor->pkt_type = PKT_UNKNOWN;
                        infor->pkt_len = 0;
                        rx_desc->cur_type = PKT_UNKNOWN;
                        
                        if (rx_desc->pkt_num >= NUM_DESCS)
                        {
                            rx_desc->pkt_num--;
                        }
                        
                    }
                }  
                break;
                case PKT_ZADATA:
                {                     
                    infor = &(rx_desc->infor[rx_desc->cur_num]);
                    
                    /*
                    * received one complete packet
                    */
                    if((MsgFlagLen+AMHeadLen+AMBodyLen)==infor->pkt_len)
                    {   
                        //if task has consumed some packets
                        if (rx_desc->cur_num != rx_desc->pkt_num)
                        {   
                            temp_info = infor;
                            infor     = &(rx_desc->infor[rx_desc->pkt_num]);
                            infor->pkt_len = temp_info->pkt_len;
                            temp_info->pkt_len = 0;
                            temp_info->pkt_type = PKT_UNKNOWN;
                        }
                        
                        infor->pkt_type = rx_desc->cur_type;  // PKT_ATCMD / PKT_IWCMD;
                        rx_desc->pkt_num++;
                        rx_desc->cur_type = PKT_UNKNOWN;
                    }
                    
                    /*
                    * continue to receiving packet
                    */                
                    else
                    {
                        Buf_Push(rx_ring,ch);
                        roomleft--;
                        infor->pkt_len++;
                        if(infor->pkt_len==AC_PAYLOADLENOFFSET)
                        {
                            AMBodyLen = ch;
                        }
                        else if(infor->pkt_len==(AC_PAYLOADLENOFFSET +1))
                        {
                            AMBodyLen = AMBodyLen<<8 + ch;
                        }
                    }
                    
                    
                    /*
                    * if overflow,we discard the current packet
                    * example1:packet length > ring size
                    * example2:rx ring buff can no be freed by task as quickly as rx interrupt coming
                    */    
                    if ((!roomleft) || (rx_desc->pkt_num >= NUM_DESCS))
                    {   
                        //rollback
                        Buff_RollBack(rx_ring,infor->pkt_len);
                        
                        roomleft += infor->pkt_len;
                        
                        infor->pkt_type = PKT_UNKNOWN;
                        infor->pkt_len = 0;
                        rx_desc->cur_type = PKT_UNKNOWN;
                        
                        if (rx_desc->pkt_num >= NUM_DESCS)
                        {
                            rx_desc->pkt_num--;
                        }
                        
                    }
                }
                break;
                case PKT_PRINTCMD:
                {          
                    /*
                    * received one complete packet
                    */
                    if(ch == '\n' || ch == '\r')
                    {   
                        rx_desc->cur_type = PKT_UNKNOWN;
                    }
                    /*
                    * continue to receiving packet
                    */                
                    UARTprintf("%c",ch);
                }
                break;
                
                case PKT_PUREDATA:
                {   
                    infor = &(rx_desc->infor[rx_desc->cur_num]);
                    
                    /*
                    * received one complete packet
                    */
                    if((MsgFlagLen+AMHeadLen+AMBodyLen)==infor->pkt_len)
                    {   
                        //if task has consumed some packets
                        if (rx_desc->cur_num != rx_desc->pkt_num)
                        {   
                            temp_info = infor;
                            infor     = &(rx_desc->infor[rx_desc->pkt_num]);
                            infor->pkt_len = temp_info->pkt_len;
                            temp_info->pkt_len = 0;
                            temp_info->pkt_type = PKT_UNKNOWN;
                        }
                        
                        infor->pkt_type = rx_desc->cur_type;  // PKT_ATCMD / PKT_IWCMD;
                        rx_desc->pkt_num++;
                        rx_desc->cur_type = PKT_UNKNOWN;
                    }
                    
                    /*
                    * continue to receiving packet
                    */                
                    else
                    {
                        Buf_Push(rx_ring,ch);
                        roomleft--;
                        infor->pkt_len++;
                        if(infor->pkt_len==AC_PAYLOADLENOFFSET)
                        {
                            AMBodyLen = ch;
                        }
                        else if(infor->pkt_len==(AC_PAYLOADLENOFFSET +1))
                        {
                            AMBodyLen = AMBodyLen<<8 + ch;
                        }
                    }
                    
                    
                    /*
                    * if overflow,we discard the current packet
                    * example1:packet length > ring size
                    * example2:rx ring buff can no be freed by task as quickly as rx interrupt coming
                    */    
                    if ((!roomleft) || (rx_desc->pkt_num >= NUM_DESCS))
                    {   
                        //rollback
                        Buff_RollBack(rx_ring,infor->pkt_len);
                        
                        roomleft += infor->pkt_len;
                        
                        infor->pkt_type = PKT_UNKNOWN;
                        infor->pkt_len = 0;
                        rx_desc->cur_type = PKT_UNKNOWN;
                        
                        if (rx_desc->pkt_num >= NUM_DESCS)
                        {
                            rx_desc->pkt_num--;
                        }
                        
                    }
                }
                break;
                default:
                break;
                
            }
            
        }    
    }
    return;
    
}
/*
* task context
* fucntion: dispatch and handle packet from rx ring buffer
*/
void ProcessZigbeeMsg(void)
{
    
    PKT_FIFO     *infor;
    PKT_DESC     *rx_desc = &(UART3Port.Rx_desc);
    BUFFER_INFO  *rx_ring = &(UART3Port.Rx_Buffer); 
    
    
    PKT_TYPE rxpkt_type;
    u16   rxpkt_len;
    u16 i;
    //PCHAR pCmdBuf = NULL;
	
    while (rx_desc->pkt_num)
    {   
        //simulate FIFO,1st in,1st out
        infor = &(rx_desc->infor[0]);
        rxpkt_type = infor->pkt_type;
        rxpkt_len  = infor->pkt_len;
        
        memset(pCmdZigBuf, 0, UART3RX_RING_LEN);
        //pCmdBuf = (PCHAR)malloc(rxpkt_len);
        
        //copy from uart rx ring
        for(i = 0;i < rxpkt_len;i++)       //O(n)
        {
            Buf_Pop(rx_ring,pCmdZigBuf[i]);
            //Printf_High("Buf_Pop=%x \n",pCmdBuf[i]);
        }
        
        //reset value
        infor->pkt_type = PKT_UNKNOWN;
        infor->pkt_len = 0;
        
        //shift FIFO
        for (i=0; i<(rx_desc->pkt_num)-1; i++)  //O(n)
        {
            rx_desc->infor[i]= rx_desc->infor[i+1];
        }  
        rx_desc->pkt_num--;
        
        //handle previous packet
        switch (rxpkt_type)
        {
            case PKT_ATCMD:
            //IoT_parse_ATcommand(pCmdBuf, rxpkt_len);
            break;
            case PKT_ZADATA:
            //IoT_parse_IWCmd(pCmdBuf, rxpkt_len);
            break;
            // <----custom defined packet dispatch
            case PKT_PUREDATA:
            AC_RecvMessage(UART3_BASE,(AC_MessageHead*)(pCmdWifiBuf + 4));
            break;
            default:
            break;
        }    
        
        //free(pCmdBuf); 
    }   
    
}

void ProcessWifiMsg()
{
    PKT_FIFO     *infor;
    PKT_DESC     *rx_desc = &(UART1Port.Rx_desc);
    BUFFER_INFO  *rx_ring = &(UART1Port.Rx_Buffer); 
    
    PKT_TYPE rxpkt_type;
    u16   rxpkt_len;
    u16 i;
    
    while (rx_desc->pkt_num)
    {   
        //simulate FIFO,1st in,1st out
        infor = &(rx_desc->infor[0]);
        rxpkt_type = infor->pkt_type;
        rxpkt_len  = infor->pkt_len;
        
        memset(pCmdWifiBuf, 0, UART1RX_RING_LEN);
        
        //copy from uart rx ring
        for(i = 0;i < rxpkt_len;i++)       //O(n)
        {
            Buf_Pop(rx_ring,pCmdWifiBuf[i]);
        }
        
        //reset value
        infor->pkt_type = PKT_UNKNOWN;
        infor->pkt_len = 0;
        
        //shift FIFO
        for (i=0; i<(rx_desc->pkt_num)-1; i++)  //O(n)
        {
            rx_desc->infor[i]= rx_desc->infor[i+1];
        }  
        rx_desc->pkt_num--;
        
        //handle previous packet
        switch (rxpkt_type)
        {
            case PKT_PUREDATA:
                AC_RecvMessage(UART1_BASE,(AC_MessageHead*)(pCmdWifiBuf + 4));
                break;
            default:
            break;
        }    
        
    }   
}
/*********************************************************************************************************
END FILE
*********************************************************************************************************/