#include <stdint.h>
#include <stdbool.h>
#include "utils/uartstdio.h"
#include "ac_common.h"
#include "ac_hal.h"
#include "Iot_Uart.h"
#include "Iot_Pm25.h"
#include "Iot_Fan.h"
#include "Iot_Timer.h"
#include "inc/hw_gpio.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "Iot_Lcd.h"
#define    UART7_BAUTRATE       9600
#define     HR01DATALEN         32
#define     UART7_INTPRIO       7
#define  PM25MEASEPEARIOD (10*60) //pm2.5 10min测量一次
#define  FANSMARTPEARIOD (2*10)  // 风扇控制2分钟

char HR01DataPrefix[] = {0x42,0x4d};
UARTStruct    UART7Port;
u16 g_u16pm25 = 0;
#define PM25WORKTIME 40 // measure 20 times
#define PM25RECORDDATAMAXLEN 600
u16 g_PM25WorkTime = PM25WORKTIME;
u16 g_PM25WorkCnt = 0;
u16 g_u16PM25RecordData[PM25RECORDDATAMAXLEN] ={0}; 
u16 g_u16PM25RecordIndex = 0;
u16 g_u16PM25SmartFlag = 0;
/*********************************************************************************************************
** Function name:           Pm25SmartDelayIntHandler
** Descriptions:    PM25智能延迟判断         
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
void Pm25SmartDelayIntHandler(void)
{
    u32 i =0;
    u32 sum =0;
    u32 avg =0;
    //
    // Clear the timer interrupt flag.
    //
    TimerIntClear(WTIMER3_BASE, TIMER_TIMA_TIMEOUT);
    for(i = 0;i<g_u16PM25RecordIndex;i++)
    {
        sum += g_u16PM25RecordData[i];
    }
    avg = sum/g_u16PM25RecordIndex;
    g_u16PM25RecordIndex = 0;
    
    DBGPRINT(RT_DEBUG_TRACE,("PM2.5 SmartDelay avg=%d\n",avg));
    
    //在此pm2.5浓度阈值连续检测得到数据在75微克每立方米以上3分钟后
    //每2分钟增加1%电机转速,启动电机转速定时器
    if(avg >AC_GetStoreStatus(PM25SMARTLEVELDATA))//enter 
    {
        FanSmartStart(AC_GetStoreStatus(FANSMARTPEARIODDATA));
        g_u16PM25SmartFlag = 1;
        DBGPRINT(RT_DEBUG_TRACE,("PM2.5 Fan Smart On\n"));
    }
    else 
    {//当pm2.5连续实时检测数据低于75微克每立方米以下2分钟后，自动关闭智能延时判断
        Pm25SmartDelayTimerStop();
        g_u16PM25SmartFlag = 0;
        FanSmartStop();
        DBGPRINT(RT_DEBUG_TRACE,("PM2.5 Fan Smart Off\n"));
        //PM25SetOff();
        //Pm25PeroidMeasStart(PM25MEASEPEARIOD); 
    }

}

/*********************************************************************************************************
** Function name:           CheckSum
**  Descriptions:           
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
u32 CheckSum(const u8 *pui8Data, u32 ui32Size)
{
    u32 ui32CheckSum;

    //
    // Initialize the checksum to zero.
    //
    ui32CheckSum = 0;

    //
    // Add up all the bytes, do not do anything for an overflow.
    //
    while(ui32Size--)
    {
        ui32CheckSum += *pui8Data++;
    }

    //
    // Return the caculated check sum.
    //
    return(ui32CheckSum & 0xffff);
}

/*********************************************************************************************************
** Function name:           PM25IntHandle
**  Descriptions:           
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
void PM25IntHandle(void)
{
    u8        ch = 0;
    static u8  HR01MatchNum = 0;
    static u8  HR01Data[HR01DATALEN] ={0};
    static u8  HR01Dataindex = 0;
    static u8  u8Displayindex = 0;
    PKT_DESC     *rx_desc = &(UART7Port.Rx_desc);
    u32 ulStatus;
    u32 ui32CheckSum = 0;
    ulStatus = UARTIntStatus(UART7_BASE, true);                         /*  读取已使能的串口0中断状态   */
    UARTIntClear(UART7_BASE, ulStatus);                                 /*  清除当前的串口0中断         */
    if((ulStatus & UART_INT_RT)||(ulStatus & UART_INT_RX))             /*  接收中断                    */
    {           
        while(UARTCharsAvail(UART7_BASE))
        {
            ch = UARTCharGetNonBlocking(UART7_BASE); 
            switch (rx_desc->cur_type)
            {
                case PKT_UNKNOWN:
                {  
                    
                    /**************** detect packet type ***************/
                    //support more ATcmd prefix analysis
                    /*case 1:AT#*/
                    if (HR01DataPrefix[HR01MatchNum] == ch)
                    {         
                        HR01MatchNum++;
                    }
                    else
                    {         
                        HR01MatchNum = 0;
                    }
                    
                    if (HR01MatchNum == sizeof(HR01DataPrefix))   //match case 3:arm  data
                    {   
                        memcpy(HR01Data,HR01DataPrefix,sizeof(HR01DataPrefix));
                        HR01Dataindex = HR01Dataindex+2;
                        rx_desc->cur_type = PKT_HR01DATA;           //match case 1: AT#                         
                        HR01MatchNum = 0;
                        continue;
                    }           
                }
                break;
                case PKT_HR01DATA:
                {  
                    HR01Data[HR01Dataindex++]= ch;
                    if(HR01Dataindex==HR01DATALEN)
                    {
                        HR01Dataindex=0; 
                        rx_desc->cur_type = PKT_UNKNOWN;
                        ui32CheckSum = (HR01Data[HR01DATALEN-2]<<8) + HR01Data[HR01DATALEN-1];
                        if(CheckSum(HR01Data, HR01DATALEN-2) != (ui32CheckSum & 0xffff))
                        {
                            break;
                        }
                        //g_u16_pm10 = (HR01Data[8]<<8)+HR01Data[9];
                        g_u16pm25 =  (HR01Data[12]<<8)+HR01Data[13];
                        if((g_PM25WorkCnt++>=AC_GetStoreStatus(PM25SMARTWORKTIMEDATA))||(0!=g_u16PM25SmartFlag))
                        {

                            g_PM25WorkCnt=0;
                            //当pm2.5实时监测超过75微克每立方米后
                            if((0==g_u16PM25SmartFlag)&&(g_u16pm25<AC_GetStoreStatus(PM25SMARTLEVELDATA)))
                            {
                                //PM25SetOff();
                                DBGPRINT(RT_DEBUG_TRACE,("PM2.5 Peroid Meas Off\n"));
                            }
                            else if((1==g_u16PM25SmartFlag)&&(g_u16pm25<AC_GetStoreStatus(PM25SMARTLEVELDATA)))
                            {
                                g_u16PM25SmartFlag = 2;
                                Pm25SmartDelayTimerStart(AC_GetStoreStatus(PM25SMARTDELAYOFFPEARIODDATA));
                                Pm25PeroidMeasStop();
                                DBGPRINT(RT_DEBUG_TRACE,("PM2.5 SmartDelay Off\n"));
                            }
                            else if((0==g_u16PM25SmartFlag)&&(g_u16pm25>AC_GetStoreStatus(PM25SMARTLEVELDATA)))
                            {
                                g_u16PM25SmartFlag = 2;
                                Pm25SmartDelayTimerStart(AC_GetStoreStatus(PM25SMARTDELAYONPEARIODDATA));
                                DBGPRINT(RT_DEBUG_TRACE,("PM2.5 SmartDelay On\n"));
                            }
                            
                            if((2==g_u16PM25SmartFlag)&&(g_u16PM25RecordIndex<PM25RECORDDATAMAXLEN))
                            {
                                g_u16PM25RecordData[g_u16PM25RecordIndex++] = g_u16pm25;
                            }    
                            
                            DBGPRINT(RT_DEBUG_INFO,("pm2.5=%d\n",g_u16pm25));
                        }
                        if(u8Displayindex==3)
                        {
                            LCDPm25Show(g_u16pm25);
                            u8Displayindex = 0;
                        }
                        u8Displayindex++;
                        
                    }
                    
                }
                break; 
                default:
                break;
                
            }
        }
    }
}

/*********************************************************************************************************
** Function name:           PM25SetOn
**  Descriptions:           
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
void PM25SetOn()
{
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
}
/*********************************************************************************************************
** Function name:           PM25SetOff
**  Descriptions:           
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
void PM25SetOff()
{
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
}
/*********************************************************************************************************
** Function name:           PM25SetInit
**  Descriptions:           串口7中断服务函数
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
void PM25SetInit()
{
    //
    // Enable the GPIO port that is used for the on-board LED.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    
    
    
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_2,GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);                       //SET UP GPIOF weak pull-up   WITH 2MA);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
}
/*********************************************************************************************************
** Function name:           PM25Init
**  Descriptions:           
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
void PM25Init()
{   
    ConfigureUART7(UART7_BAUTRATE,UART7_INTPRIO);
    PM25SetInit();
}
/*********************************************************************************************************
** Function name:           PM25Get
**  Descriptions:           
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
u16 GetPM25()
{
    return  g_u16pm25;
}