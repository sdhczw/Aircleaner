/**
******************************************************************************
* @file     Iot_Key.c
* @authors  cxy
* @version  V1.0.0
* @date     10-Sep-2014
* @brief   
******************************************************************************
*/

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "utils/uartstdio.h"
#include "inc/hw_gpio.h"
#include "ac_common.h"
#include "ac_hal.h"



/*********************************************************************************************************
** Function name:           KeyIntHandle
** Descriptions:            
** input parameters:        
**                          
** Output parameters::      无
** Returned value:          
**                          
** Created by:              
** Created Date:            
**--------------------------------------------------------------------------------------------------------
** Modified by:            
** Modified date:          
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
void KeyIntHandle(void)
{
    
    static vu32  g_vu32Key = 0;
    u32 ulStatus;
    ulStatus = GPIOIntStatus(GPIO_PORTE_BASE, true); 
    GPIOIntClear(GPIO_PORTE_BASE, ulStatus); 
    if (ulStatus & GPIO_PIN_2) //
    {
        SysCtlDelay(SysCtlClockGet() / 100 / 3);//10ms      
        g_vu32Key = g_vu32Key^0x1;

        
        SysCtlDelay(SysCtlClockGet()); // 延时约10ms，消除松键抖动
        if (GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_2) == 0x00); 
        {
            AC_SendRestMsg(UART1_BASE,NULL);
        }
        return;
        
    } 
}



/*********************************************************************************************************
** Function name:           KeyInit
** Descriptions:           
** input parameters:        
**                          
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
void KeyInit(void)
{
    //
    // Reset the error indicator.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);                   /*  使能GPIO PE口  */
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE,GPIO_PIN_2);
    GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_2, GPIO_FALLING_EDGE);

    ROM_GPIODirModeSet(GPIO_PORTE_BASE, GPIO_PIN_2, GPIO_DIR_MODE_IN);
    GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_2,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
    GPIOIntEnable(GPIO_PORTE_BASE, GPIO_PIN_2);
    IntEnable(INT_GPIOE); // 使能GPIOF端口中断
}
