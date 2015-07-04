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
#include "driverlib/timer.h"
#include "driverlib/interrupt.h" 
#include "utils/uartstdio.h"
#include "ac_common.h"
#include "ac_hal.h"
#include "Iot_Timer.h"
#include "Iot_Uart.h"
#include "Iot_Sht21.h"
#include "Iot_Fan.h"
#include "Iot_T6603.h"
#include "Iot_Pm25.h"
#include "Iot_Lcd.h"
//*****************************************************************************
//
// Counter to count the number of interrupts that have been called.
//
//*****************************************************************************
#define  PM25MEASEPEARIOD ((u64)10*60) //pm2.5 10min测量一次

#define  CO2MEASEPEARIOD (1) //co2 1s测量一次

#define  FANFBMEASEPEARIOD 30 //温湿度5s测量一次

#define  SHT21MEASEPEARIOD 5 //温湿度5s测量一次

#define  NTCMEASEPEARIOD 5 //温湿度5s测量一次

//#define  FANSMARTPEARIOD (2*60)  // 风扇控制2分钟


#define  DEFAULTREPORTTIME (1*60)//report devecie status interval 10min

#define  ACTIMERMAXNUM 10

#define SMARTDELAYTIMERINTPRIORITY 5

static volatile u32 g_u32Timer0AReportCounter = 0;
static volatile u8 g_u8Timer0ASHT21Counter = 0;
static volatile u8 g_u8Timer0AFanCounter = 0;
static volatile u8 g_u8Timer0ANtcCounter = 0;
static volatile u8 g_u8Timer0ACo2Counter = 0;
static volatile u16 g_u16Timer0APm25Counter = 0;

static volatile u8 g_u8Timer0AFanSmartFlag = 0;
static volatile u8 g_u8Timer0APm25PeroidFlag = 0;
static volatile u8  g_u8WifiPowerQueryFlag = 1;
static volatile u8  g_u8Timer0ADevReportFlag = 0;
AC_Timer g_struBspTimer[ACTIMERMAXNUM];
u16 g_struBspTimerCount[ACTIMERMAXNUM];

u32 g_u32PeriodicReportTimer = DEFAULTREPORTTIME;
u32 g_u32PeriodicSHT21 = SHT21MEASEPEARIOD;//温湿度5s测量一次
u32 g_u32PeriodicNtc = NTCMEASEPEARIOD;//温湿度5s测量一次
u32 g_u32PeriodicFanSmart = 0;//温湿度5s测量一次
u32 g_u32PeriodicPm25 = PM25MEASEPEARIOD;//pm2.5 10min测量一次
u32 g_u32PeriodicCo2 = CO2MEASEPEARIOD;//pm2.5 10min测量一次
//*****************************************************************************
//	Timer 1B中断处理函数
//*****************************************************************************


//*****************************************************************************
//
// The interrupt handler for the Timer0B interrupt.
//
//*****************************************************************************
void Timer0AIntHandler(void)
{
    u32 wifipowerstatus = WIFIPOWEROFF;
    u32 fanspeed = 55;  
    u32 fanstatus = 0;  
    //
    // Clear the timer interrupt flag.
    //
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    // Co2 1s测量一次
    
    SendCo2Command();
    
    // PM25 10min测量一次   
    if((1==g_u8Timer0APm25PeroidFlag)&&(g_u16Timer0APm25Counter++>=AC_GetStoreStatus(PM25SMARTMEASPEARIODDATA)))
    {
        PM25SetOn();     
        g_u16Timer0APm25Counter = 0;
        DBGPRINT(RT_DEBUG_TRACE,("PM2.5 Peroid Meas On\n"));
    }    
    
    if(g_u8Timer0ACo2Counter++>=g_u32PeriodicCo2)
    {
        g_u8Timer0ACo2Counter = 0;
        SetCo2ProcessOn();
    }
    
    if(WIFIPOWEROFF==AC_GetStoreStatus(WIFIPOWERSTATUS))
    {
        g_u8WifiPowerQueryFlag = 1;
    }
    
    if((g_u32Timer0AReportCounter++>=AC_GetStoreStatus(REPORTPEARIODDATA))&&(CLOUDCONNECT==AC_GetStoreStatus(CLOUDSTATUS)))
    {
       g_u8Timer0ADevReportFlag = 1;
           
        //Report data
        g_u32Timer0AReportCounter = 0;

    }
    if(g_u8Timer0ASHT21Counter++>=g_u32PeriodicSHT21)
    {
        g_u8Timer0ASHT21Counter = 0;
        SetSht21MeasOn();
    }
    if(g_u8Timer0ANtcCounter++>=g_u32PeriodicNtc)
    {
        g_u8Timer0ANtcCounter = 0;
        SetNtcMeasOn();
    }
    if((1==g_u8Timer0AFanSmartFlag)&&(g_u8Timer0AFanCounter++==g_u32PeriodicFanSmart))
    {
        fanspeed = GetFanSpeed();
        fanstatus = GetFanStatus();
        fanspeed++;
        if((fanspeed<101)&&(fanstatus!=0))
        {
            LCDFanSpeedShow(fanspeed);
            SetFanSpeed(fanspeed);
        }
        g_u8Timer0AFanCounter = 0; 
    }
}

 /*********************************************************************************************************
** Function name:           Timer0BIntHandler
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
void FanCountIntHandler(void)
{
    
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    
    
    
    IntMasterDisable();
    
    // AC_SendLedStatus2Server();
    //Report data
    DBGPRINT(RT_DEBUG_TRACE,("Fan Speed = %d\n",g_u32FanFbSpeed*2));
    g_u32FanFbSpeed = 0;
    IntMasterEnable();
    
}
 
 /*********************************************************************************************************
** Function name:           FanSmartStart
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
void FanSmartStart(u16 u16period)
{
    if(0==g_u8Timer0AFanSmartFlag)
    {
        g_u8Timer0AFanSmartFlag = 1;
        g_u8Timer0AFanCounter = 0;
        g_u32PeriodicFanSmart = u16period/3;
    }
}

 /*********************************************************************************************************
** Function name:           FanSmartStop
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
void FanSmartStop()
{
    if((0==g_u16Co2SmartFlag)&&(0==g_u16PM25SmartFlag))
   {
        g_u8Timer0AFanSmartFlag = 0;
        g_u8Timer0AFanCounter = 0;
        SetFanSpeed(1);
        LCDFanSpeedShow(1);
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
void PeriodicTimerInit()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    //
    // Configure Timer0B as a 16-bit periodic timer.
    //
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    
    //
    // Set the Timer0B load value to 1s.
    //
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet());
    
    //
    // Enable processor interrupts.
    //
    IntMasterEnable();
    //
    // Configure the Timer0B interrupt for timer timeout.
    //
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    
    //
    // Enable the Timer0B interrupt on the processor (NVIC).
    //
    IntEnable(INT_TIMER0A);
    
    //
    // Enable Timer0B.
    //
    TimerEnable(TIMER0_BASE, TIMER_A);
    
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
void FanCountTimerInit()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    //
    // Set the Timer0B load value to 30s.
    //
    TimerLoadSet(TIMER1_BASE, TIMER_A, SysCtlClockGet()*FANFBMEASEPEARIOD);
    
    //
    // Enable processor interrupts.
    //
    IntMasterEnable();
    //
    // Configure the Timer0B interrupt for timer timeout.
    //
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    
    //
    // Enable the Timer0B interrupt on the processor (NVIC).
    //
    IntEnable(INT_TIMER1A);
    
    //
    // Enable Timer0B.
    //
    TimerEnable(TIMER1_BASE, TIMER_A);
    
}

/*********************************************************************************************************
** Function name:           Co2TimerInit
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
void Co2SmartDelayTimerInit()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER1);
    //
    // Configure Timer0B as a 16-bit periodic timer.
    //
    TimerConfigure(WTIMER1_BASE, TIMER_CFG_ONE_SHOT);
    
    
    //
    // Set the Timer0B load value to 1s.
    //
    TimerLoadSet64(WTIMER1_BASE, SysCtlClockGet()*CO2SMARTONDELAYPEARIOD);
    //
    // Enable processor interrupts.
    //
    IntMasterEnable();
    //
    // Configure the Timer0B interrupt for timer timeout.
    //
    TimerIntEnable(WTIMER1_BASE, TIMER_TIMA_TIMEOUT);
    
    //
    // Enable the Timer0B interrupt on the processor (NVIC).
    //
    IntPrioritySet(INT_WTIMER1A, SMARTDELAYTIMERINTPRIORITY<<5); 
    
    IntEnable(INT_WTIMER1A); 
    //
    // Enable Timer0B.
    //
   // TimerEnable(WTIMER1_BASE, TIMER_A);
    
}

/*********************************************************************************************************
** Function name:           Co2TimerStop
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
void Co2SmartDelayTimerStop()
{
    TimerDisable(WTIMER1_BASE, TIMER_A); 
}

/*********************************************************************************************************
** Function name:           Co2SmartDelayTimerStart
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
void Co2SmartDelayTimerStart(u64 u64second)
{
    if(u64second)
    {
        TimerLoadSet(WTIMER1_BASE, TIMER_A, SysCtlClockGet()*u64second);
        TimerEnable(WTIMER1_BASE, TIMER_A);   
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
void Pm25TimerInit()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER3);
    //
    // Configure Timer0B as a 16-bit periodic timer.
    //
    TimerConfigure(WTIMER3_BASE, TIMER_CFG_ONE_SHOT);
    
    
    //
    // Set the Timer0B load value to 1s.
    //
    TimerLoadSet64(WTIMER3_BASE, SysCtlClockGet()*PM25SMARTDELAYONPEARIOD);
    //
    // Enable processor interrupts.
    //
    IntMasterEnable();
    //
    // Configure the Timer0B interrupt for timer timeout.
    //
    TimerIntEnable(WTIMER3_BASE, TIMER_TIMA_TIMEOUT);
    
    //
    // Enable the Timer0B interrupt on the processor (NVIC).
    //
    IntPrioritySet(INT_WTIMER3A, SMARTDELAYTIMERINTPRIORITY<<5); 
    
    IntEnable(INT_WTIMER3A); 

    
}

/*********************************************************************************************************
** Function name:           Pm25SmartDelayTimerStop
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
void Pm25SmartDelayTimerStop()
{
    TimerDisable(WTIMER3_BASE, TIMER_A); 
}

/*********************************************************************************************************
** Function name:           Pm25SmartDelayTimerStart
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
void Pm25SmartDelayTimerStart(u64 u64seccond)
{
    if(u64seccond)
    {
        TimerLoadSet(WTIMER3_BASE, TIMER_A, SysCtlClockGet()*u64seccond);
        TimerEnable(WTIMER3_BASE, TIMER_A);   
    }
}

/*********************************************************************************************************
** Function name:           Pm25PeroidMeasStart
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
void Pm25PeroidMeasStart(u16 u16second)
{
    if(u16second)
    {
        g_u32PeriodicPm25 = u16second;
        g_u8Timer0APm25PeroidFlag = 1;
        g_u16Timer0APm25Counter = 0;
    }
}

/*********************************************************************************************************
** Function name:           Pm25PeroidMeasStop
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
void Pm25PeroidMeasStop()
{
    g_u8Timer0APm25PeroidFlag = 0;
    g_u16Timer0APm25Counter = 0;
}

/*********************************************************************************************************
** Function name:           SetReportPeriodic
** Descriptions:            设置数据上报周期 
** input parameters:        TimerInterval:   时间间隔 s
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
void SetReportPeriodic(u32 TimerInterval)
{
    g_u32Timer0AReportCounter = 0;
    g_u32PeriodicReportTimer =TimerInterval;
}
 /*********************************************************************************************************
** Function name:           Timer_Create
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
actimer_handle_t Timer_Create(u32 u32Interval, u8 u8TimerIndex,ac_timer_callback p_callback)
{
    g_struBspTimer[u8TimerIndex].p_callback = p_callback;
    g_struBspTimer[u8TimerIndex].u32Interval = u32Interval;
    g_struBspTimer[u8TimerIndex].u8ValidFlag = 0;
    g_struBspTimerCount[u8TimerIndex]=0;
    return &g_struBspTimer[u8TimerIndex];
}
 
 /*********************************************************************************************************
** Function name:           Timer_Init
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
void Timer_Init()
{
    u8 i = 0;
    for(i=0;i<ACTIMERMAXNUM;i++)
    {
        g_struBspTimer[i].u8ValidFlag = 0;
    }
}

 /*********************************************************************************************************
** Function name:           Timer_Start
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
void Timer_Start(actimer_handle_t TimerHandle)
{
    ((AC_Timer*)TimerHandle)->u8ValidFlag = 1;
}

 /*********************************************************************************************************
** Function name:           Timer_Stop
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
void Timer_Stop(actimer_handle_t TimerHandle)
{
    ((AC_Timer*)TimerHandle)->u8ValidFlag = 0;
}
/*********************************************************************************************************
** Function name:           SetReportPeriodic
** Descriptions:            设置数据上报周期 
** input parameters:        TimerInterval:   时间间隔 s
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
void EdgeWTimer0Init()
{
  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
  //
  //	使能WTimer 0
  //
  SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER0);
  
  //
  //	配置PC4类型
  //
  //GPIOPinTypeQEI(GPIO_PORTC_BASE, GPIO_PIN_5);
  GPIOPinTypeTimer(GPIO_PORTC_BASE, GPIO_PIN_4);  
  //
  //	配置PF4为WTCCP0
  GPIOPinConfigure(GPIO_PC4_WT0CCP0);
  GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4,
                   GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  // Enable processor interrupts.
  //
  IntMasterEnable();
  
  /****************************以下是定时器的配置***********************************/
  
  
  //
  //	配置Timer 2B为16位capture
  //;
  TimerConfigure(WTIMER0_BASE, (TIMER_CFG_SPLIT_PAIR |
                                TIMER_CFG_A_CAP_TIME));
  
  
  //
  // 	配置Capture为沿触发
  //
  TimerControlEvent(WTIMER0_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);
  //
  //	给定时器装载初TimerLoadSet值
  //			 
  TimerLoadSet(WTIMER0_BASE,TIMER_A,0xFFFFFFFF);
  //TimerPrescaleSet(TIMER3_BASE, TIMER_A, 0xFFFF);
  
  //
  //	装载匹配寄存器的值 2
  //
  TimerMatchSet(WTIMER0_BASE,TIMER_A, 0);
  //
  //	设置中断
  //
  IntEnable(INT_WTIMER0A);
  //
  //	设置中断类型
  //
  TimerIntEnable(WTIMER0_BASE,  TIMER_CAPA_EVENT);
  //
  //	使能定时器
  //
  TimerEnable(WTIMER0_BASE, TIMER_A); 
}
/*********************************************************************************************************
** Function name:           SetReportPeriodic
** Descriptions:            设置数据上报周期 
** input parameters:        TimerInterval:   时间间隔 s
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
void TimerInit()
{
 
  PeriodicTimerInit();
  Timer_Init();
  FanCountTimerInit();
  Co2SmartDelayTimerInit();
  Pm25TimerInit();
}

/*************************************************
* Function: TIMER_TimeoutAction
* Description:
* Author: cxy
* Returns:
* Parameter:
* History:
*************************************************/
void TIMER_TimeoutAction(u8 u8TimerIndex)
{
//    switch (u8TimerIndex)
//    {
//        case :
//
//            break;
//        case :
//
//            break;
//        case :            
//
//            break;
//        case :            
//
//            break;
//        case : 
//
//            break;
//            
//    }
}

/*********************************************************************************************************
** Function name:           ProcessNtcData
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
void  ProcessPeriodReportData()
{
    if(1==g_u8WifiPowerQueryFlag)
    {
        AC_Init();
        g_u8WifiPowerQueryFlag = 0;
    }
    if(1==g_u8Timer0ADevReportFlag)
        
    {
        AC_SendDevStatus2Server();
        g_u8Timer0ADevReportFlag = 0;
    }
    return;
}
