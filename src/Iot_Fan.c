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
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"
#include "types.h"
#include "Iot_Fan.h"
#include "Iot_Pwm.h"
#include "Iot_Lcd.h"
u32 g_u32FanFbSpeed = 0;
u8 g_u8FanSpeed = 0;
u8 g_u8FanStatus = 0;
/*********************************************************************************************************
** Function name:           GPIOD
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

void FanFbHandler(void)
{
    unsigned long ulStatus;
    ulStatus = GPIOIntStatus(GPIO_PORTD_BASE, true); 
    GPIOIntClear(GPIO_PORTD_BASE, ulStatus);
    switch(ulStatus)
    {
        case GPIO_PIN_2: //
        g_u32FanFbSpeed++;
        break;
        case GPIO_PIN_6:  //
        UARTprintf("HV ERR !\n");
        break;
        case GPIO_PIN_7:  //
        UARTprintf("HV LAHU\n");
        break;
    }
    
}
/*********************************************************************************************************
** Function name:           SetFanSpeed
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
s8 SetFanSpeed(u8 fanspeed)
{
    s8 ret = 0;
    u8 FanPwm = 0;
    if(fanspeed>100)
    {
        fanspeed = 100;
    }
    if(fanspeed==0)
    {
        FanPwm = 0;
    }
    else
    {
        FanPwm = (fanspeed-1)/3+FAN_DEFAULT_PWM;
    }
    g_u8FanSpeed = fanspeed;
    
    DBGPRINT(RT_DEBUG_TRACE,("Set Current Machine fan Speed = %d\r\n",fanspeed));
    SetFanPwmOut(FanPwm);
	return ret;
}
/*********************************************************************************************************
** Function name:           SetFanOnOff
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
s8 SetFanOnOff(u8 fanstatus)
{
    s8 ret = 0;
    u8 FanPwm = 0;
    if(fanstatus==0)
    {
        DBGPRINT(RT_DEBUG_TRACE,("Set Current Machine Fan  Off\r\n"));
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE,("Set Current Machine Fan  On\r\n"));
    }
    g_u8FanStatus = fanstatus;
    SetFanSpeed(g_u8FanStatus);
	return ret;
}
/*********************************************************************************************************
** Function name:           GetFanOnOff
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
u8 GetFanStatus()
{
	return g_u8FanStatus;
}
/*********************************************************************************************************
** Function name:           GetFanSpeed
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
u8 GetFanSpeed()
{
	return g_u8FanSpeed;
}
/*********************************************************************************************************
** Function name:           ConfigureFanFb
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
void ConfigureFanFb()
{      
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    //
    // Reset the error indicator.
    GPIOPinTypeGPIOInput(GPIO_PORTD_BASE,GPIO_PIN_2);
    GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_2, GPIO_RISING_EDGE);

    ROM_GPIODirModeSet(GPIO_PORTD_BASE, GPIO_PIN_2, GPIO_DIR_MODE_IN);
    //GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_2,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_OD);
    GPIOIntEnable(GPIO_PORTD_BASE,GPIO_PIN_2);
    IntEnable(INT_GPIOD); // 使能GPIOD端口中断
}

/*********************************************************************************************************
** Function name:           FanInit
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
void FanInit()
{
///    ConfigureFanFb();

    PWMInit(FAN_DEFAULT_PWM);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
    SetFanOnOff(1);
    //g_u8FanSpeed = (FAN_DEFAULT_PWM-55)*3 + 1;
    LCDFanSpeedShow(g_u8FanSpeed);
}