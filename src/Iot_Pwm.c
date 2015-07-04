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
#include "driverlib/timer.h"
#include "utils/uartstdio.h"
#include "inc/hw_gpio.h"
#include "ac_common.h"
#include "ac_hal.h"

/*********************************************************************************************************
** Function name:           ConfigureFanPWM
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
void ConfigureFanPWM(u8 u8DefaulPwm)
{
    
  //
    // The Timer1 peripheral must be enabled for use.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER2);



    //
    // Configure the GPIO pin muxing for the Timer/CCP function.
    // This is only necessary if your part supports GPIO pin function muxing.
    // Study the data sheet to see which functions are allocated per pin.
    // TODO: change this to select the port/pin you are using
    //
    GPIOPinConfigure(GPIO_PD1_WT2CCP1);

    //
 

    //
    // Configure the ccp settings for CCP pin.  This function also gives
    // control of these pins to the SSI hardware.  Consult the data sheet to
    // see which functions are allocated per pin.
    // TODO: change this to select the port/pin you are using.
    //
    GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_1);


    //
    // Configure Timer1B as a 16-bit periodic timer.
    //
    TimerConfigure(WTIMER2_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PWM);

    //
    // Set the Timer1B load value to 50000.  For this example a 66% duty cycle
    // PWM signal will be generated.  From the load value (i.e. 50000) down to
    // match value (set below) the signal will be high.  From the match value
    // to 0 the timer will be low.
    //
    TimerLoadSet(WTIMER2_BASE, TIMER_B, 50000);

    //
    // Set the Timer1B match value to load value / 3.
    //
    TimerMatchSet(WTIMER2_BASE, TIMER_B,
                  TimerLoadGet(WTIMER2_BASE, TIMER_B) *u8DefaulPwm/100);

    //
    // Enable Timer1B.
    //
    TimerEnable(WTIMER2_BASE, TIMER_B);

    //
    // Loop forever while the Timer1B PWM runs.
}

/*********************************************************************************************************
** Function name:           PWMInit
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
void PWMInit(u8 u8DefaulPwm)
{
  ConfigureFanPWM( u8DefaulPwm);

}

/*********************************************************************************************************
** Function name:           SetFanPwm
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
void SetFanPwmOut(u8 u8PwmValue)
{
     // Set the Timer1B match value to load value / 3.
    //
    TimerMatchSet(WTIMER2_BASE, TIMER_B,
                  TimerLoadGet(WTIMER2_BASE, TIMER_B) *u8PwmValue/100);

}
/*********************************************************************************************************
** Function name:           SetFanPwm
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
u8 GetFanPwm()
{
     // Set the Timer1B match value to load value / 3.
    //
    return (TimerMatchGet(WTIMER2_BASE, TIMER_B)*100)/TimerLoadGet(WTIMER2_BASE, TIMER_B);
   
}

