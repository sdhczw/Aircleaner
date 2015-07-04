#include <stdint.h>
#include <stdbool.h>
#include "utils/uartstdio.h"
#include "ac_common.h"
#include "ac_hal.h"
#include "Iot_Uart.h"
#include "Iot_T6603.h"
#include "Iot_Timer.h"
#include "driverlib/timer.h"
#include "Iot_Lcd.h"
#define     UART5_INTPRIO       5
#define    UART5_BAUTRATE       9600
#define     T6603DATALEN 7
#define  FANSMARTPEARIOD (2*10)  // 风扇控制2分钟

char T6603DataPrefix[] = {0x16};
UARTStruct    UART5Port;
u16 g_u16Co2Data = 0;

#define CO2RECORDDATAMAXLEN 600
u16 g_u16Co2RecordIndex = 0;
u16 g_u16Co2RecordData[CO2RECORDDATAMAXLEN] = {0};
u16 g_u16Co2SmartFlag = 0;
u32 WTimer0EdgeValue = 0;
u8 temp = 0xFF;
u32 StartTime = 0;
u32 EndTime = 0;

u8 g_u8TimerCo2Flag = 1;
 /*********************************************************************************************************
** Function name:           ConvertWTimer0EdgeValue
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
void ConvertWTimer0EdgeValue(unsigned long sTime,unsigned long eTime)
{
  WTimer0EdgeValue = sTime >= eTime ?
    sTime - eTime :
    0xFFFFFFFF - eTime + sTime ;
}

/*********************************************************************************************************
** Function name:           Co2SmartDelayIntHandler
** Descriptions:    CO2阈值测试结果,启动智能         
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
void Co2SmartDelayIntHandler(void)
{
    u32 i =0;
    u32 sum =0;
    u32 avg =0;
    //
    // Clear the timer interrupt flag.
    //
    TimerIntClear(WTIMER1_BASE, TIMER_TIMA_TIMEOUT);
    for(i = 0;i<g_u16Co2RecordIndex;i++)
    {
        sum += g_u16Co2RecordData[i];
    }
    avg = sum/g_u16Co2RecordIndex;
    g_u16Co2RecordIndex = 0;
    DBGPRINT(RT_DEBUG_TRACE,("CO2 SmartDelay avg=%d\n",avg));
    //当二氧化碳超过800ppm后3分钟，设置为二氧化碳报警阈值点，启动智能延时判断
    //每2分钟增加1%电机转速,启动电机转速定时器
    if(avg >AC_GetStoreStatus(CO2SMARTLEVELDATA))//enter 
    {
        FanSmartStart(AC_GetStoreStatus(FANSMARTPEARIODDATA));
        g_u16Co2SmartFlag = 1;
        DBGPRINT(RT_DEBUG_TRACE,("CO2 Fan Smart On\n"));
    }
    else //当二氧化碳浓度下降到800ppm以下2分钟后，自动关闭智能延时判断。新风机转速即可恢复到1%
    {
        Co2SmartDelayTimerStop();
        g_u16Co2SmartFlag = 0;
        FanSmartStop();
        DBGPRINT(RT_DEBUG_TRACE,("CO2 Fan Smart Off\n"));
    }
}
/*********************************************************************************************************
** Function name:           WTimer0AHandler
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
void WTimer0AHandler(void){
  //
  //	清除定时器1的中断
  //
  IntMasterDisable();
  TimerIntClear(WTIMER0_BASE, TIMER_CAPA_EVENT);
  
  //
  //	设置为捕捉下降沿
  //
  
  
  if(temp == 0xFF){
    StartTime = TimerValueGet(WTIMER0_BASE,TIMER_A);
    temp = 0x00;
    TimerControlEvent(WTIMER0_BASE, TIMER_A, TIMER_EVENT_NEG_EDGE);
  }
  else{
    EndTime = TimerValueGet(WTIMER0_BASE,TIMER_A);
    //TimerDisable(TIMER2_BASE,TIMER_BOTH); 
    //		TimerDisable(TIMER0_BASE,TIMER_A);
    ConvertWTimer0EdgeValue(StartTime,EndTime);
    g_u16Co2Data =WTimer0EdgeValue/10000-10;
    temp = 0xFF;
    TimerControlEvent(WTIMER0_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);
 
  }
  IntMasterEnable();
}
/*********************************************************************************************************
** Function name:           T6603IntHandle
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
void T6603IntHandle(void)
{
    u8        ch = 0;
    static u8  T6603MatchNum = 0;
    static u8  T6603Data[T6603DATALEN] ={0};
    static u8  T6603Dataindex = 0;
    PKT_DESC     *rx_desc = &(UART5Port.Rx_desc);
    unsigned long ulStatus;
    ulStatus = UARTIntStatus(UART5_BASE, true);                         /*  读取已使能的串口0中断状态   */
    UARTIntClear(UART5_BASE, ulStatus);                                 /*  清除当前的串口0中断         */
    if((ulStatus & UART_INT_RT)||(ulStatus & UART_INT_RX))             /*  接收中断                    */
    {           
        while(UARTCharsAvail(UART5_BASE))
        {
            ch = UARTCharGetNonBlocking(UART5_BASE); 
            switch (rx_desc->cur_type)
            {
                case PKT_UNKNOWN:
                {  
                    
                    /**************** detect packet type ***************/
                    //support more ATcmd prefix analysis
                    /*case 1:AT#*/
                    if (T6603DataPrefix[T6603MatchNum] == ch)
                    {         
                        T6603MatchNum++;
                    }
                    else
                    {         
                        T6603MatchNum = 0;
                    }
                    
                    if (T6603MatchNum == sizeof(T6603DataPrefix))   //match case 3:arm  data
                    {   
                        
                        rx_desc->cur_type = PKT_T6603DATA;           //match case 1: AT#                         
                        T6603MatchNum = 0;
                        continue;
                    }           
                }
                break;
                case PKT_T6603DATA:
                {  
                    T6603Data[T6603Dataindex++]= ch;
                    if(T6603Dataindex==T6603DATALEN)
                   {
                       T6603Dataindex = 0; 
                       rx_desc->cur_type = PKT_UNKNOWN;
                      g_u16Co2Data =  (T6603Data[2]<<8)+T6603Data[3];
//                      if((0==g_u16Co2SmartFlag)&&(g_u16Co2Data>AC_GetStoreStatus(CO2SMARTLEVELDATA)))
//                      {
//                          g_u16Co2SmartFlag = 2;
//                          Co2SmartDelayTimerStart(AC_GetStoreStatus(CO2SMARTDELAYONPEARIODDATA));  
//                          DBGPRINT(RT_DEBUG_TRACE,("CO2 SmartDelay On\n"));
//                      }
//                      //当Co2连续实时检测数据低于800ppm每立方米以下2分钟后，
//                      //   自动关闭智能延时判断
//                      if((1==g_u16Co2SmartFlag)&&(g_u16Co2Data<AC_GetStoreStatus(CO2SMARTLEVELDATA)))
//                      {
//                          g_u16Co2SmartFlag = 2;
//                          Co2SmartDelayTimerStart(AC_GetStoreStatus(CO2SMARTDELAYOFFPEARIODDATA));
//                          DBGPRINT(RT_DEBUG_TRACE,("CO2 SmartDelay Off\n"));
//                      }
//                      if((2==g_u16Co2SmartFlag)&&(g_u16Co2RecordIndex<CO2RECORDDATAMAXLEN))
//                      {
//                          
//                          g_u16Co2RecordData[g_u16Co2RecordIndex++] = g_u16Co2Data;
//                          
//                      }
//                      DBGPRINT(RT_DEBUG_INFO,("CO2=%d\n",g_u16Co2Data));
//                      LCDCo2Show(g_u16Co2Data);   
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
** Function name:           ProcessCo2Data
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
void  ProcessCo2Data()
{
    if(1==g_u8TimerCo2Flag)
    {
        if((0==g_u16Co2SmartFlag)&&(g_u16Co2Data>AC_GetStoreStatus(CO2SMARTLEVELDATA)))
        {
            g_u16Co2SmartFlag = 2;
            Co2SmartDelayTimerStart(AC_GetStoreStatus(CO2SMARTDELAYONPEARIODDATA));  
            DBGPRINT(RT_DEBUG_TRACE,("CO2 SmartDelay On\n"));
        }
        //当Co2连续实时检测数据低于800ppm每立方米以下2分钟后，
        //   自动关闭智能延时判断
        if((1==g_u16Co2SmartFlag)&&(g_u16Co2Data<AC_GetStoreStatus(CO2SMARTLEVELDATA)))
        {
            g_u16Co2SmartFlag = 2;
            Co2SmartDelayTimerStart(AC_GetStoreStatus(CO2SMARTDELAYOFFPEARIODDATA));
            DBGPRINT(RT_DEBUG_TRACE,("CO2 SmartDelay Off\n"));
        }
        if((2==g_u16Co2SmartFlag)&&(g_u16Co2RecordIndex<CO2RECORDDATAMAXLEN))
        {
            
            g_u16Co2RecordData[g_u16Co2RecordIndex++] = g_u16Co2Data;
            
        }
        DBGPRINT(RT_DEBUG_INFO,("CO2=%d\n",g_u16Co2Data));
        LCDCo2Show(g_u16Co2Data);
        
        g_u8TimerCo2Flag = 0;
    }
    return;
}
/*********************************************************************************************************
** Function name:           SendCo2Command
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
void SendCo2Command()
{
    u8 senddata[4] = {0x11,0x01,0x01,0xED};
    UartSend(UART5_BASE,senddata,4);
}

/*********************************************************************************************************
** Function name:           GetCo2Data
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
u16 GetCo2()
{
    return g_u16Co2Data;
}
/*********************************************************************************************************
** Function name:           Co2Init
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
void Co2Init()
{
    ConfigureUART5(UART5_BAUTRATE, UART5_INTPRIO);
    EdgeWTimer0Init();
}

/*********************************************************************************************************
** Function name:           SetNtcFlag
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
void SetCo2ProcessOn()
{
     g_u8TimerCo2Flag = 1;
}
