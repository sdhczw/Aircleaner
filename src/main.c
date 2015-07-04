//*****************************************************************************
//
// hello.c - Simple hello world example.
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

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
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
#include "ac_common.h"
#include "ac_hal.h"
#include "Iot_Uart.h"
#include "Iot_Timer.h"
#include "Iot_Fan.h"
#include "Iot_Sht21.h"
#include "Iot_Pm25.h"
#include "Iot_T6603.h"
#include "Iot_Key.h"
#include "Iot_Lcd.h"
u8 RTDebugLevel =RT_DEBUG_TRACE;
u8 g_u8PtcFlag = 0;
/*********************************************************************************************************
** Function name:           ClockInit
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
void ClockInit()
{
    // Enable lazy stacking for interrupt handlers.  This allows floating-point
  // instructions to be used within interrupt handlers, but at the expense of
  // extra stack usage.
  //
  ROM_FPULazyStackingEnable();
  
  //
  // Set the clocking to run directly from the crystal.
  //
  ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                     SYSCTL_OSC_MAIN);
  

}

/*********************************************************************************************************
** Function name:           Sleep
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
void Sleep(unsigned char milisecond)
{
    SysCtlDelay(SysCtlClockGet() / 1000 / 3*milisecond);//100ms
}

/*********************************************************************************************************
** Function name:           PtcOn
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
void PtcOn()
{
    if(!g_u8PtcFlag)
    {
        GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3, 0); 
        g_u8PtcFlag = 1;
    }
}

/*********************************************************************************************************
** Function name:           PtcOn
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
void PtcOff()
{
    if(g_u8PtcFlag)
    {
        GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3, GPIO_PIN_3); 
        g_u8PtcFlag = 0;
    }
}
/*********************************************************************************************************
** Function name:           PtcInit
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
void PtcInit()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    //
    // Reset the error indicator.
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE,GPIO_PIN_3);
    // init ptc module, 0:on,1:off
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3, GPIO_PIN_3); 
    g_u8PtcFlag = 0;
}



/*********************************************************************************************************
** Function name:           ProcessHwData
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
void ProcessPeriodData()
{
    u16 u16IndoorTemp = 0;
    u16 u16OutdoorTemp = 0;
    u8 u8FanStatus = 0;
#if I2C_DEVICE_SUPPORT
    ProcessSht21Data();
#endif
    ProcessNtcData();
    ProcessCo2Data();
    ProcessPeriodReportData();
    u16OutdoorTemp = GetOutdoorTempData();
    u16IndoorTemp = GetIndoorTempData();
    u8FanStatus = GetFanStatus();
    if((u8FanStatus!=0)&&(u16IndoorTemp<19)&&(u16OutdoorTemp<6))//室内温度小于19度，室外温度小于6度启动ptc模块
    {
        PtcOn();
    }
    else
    {
       PtcOff();
    }
}
/*********************************************************************************************************
** Function name:           PeriodicTimerInit
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
int main(void)
{  
    AC_SwInit();
    AC_HwInit();
    AC_ResetWifi();
    AC_ReadEEROM();
    DBGPRINT(RT_DEBUG_TRACE,("Air Machine Power On!\n"));
    while(1)
    {
       ProcessWifiMsg();
       ProcessPeriodData();
    }    
}


