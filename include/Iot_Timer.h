#ifndef IOT_TIMER_H
#define IOT_TIMER_H

extern void TimerInit();
extern void SetReportPeriodic(u32 TimerInterval);
extern void SetThreshold(u32 Type,u32 Threshold);
extern u32 ReadCO2();
void FanSmartStart(u16 u16period);
void FanSmartStop();
void Co2SmartDelayTimerStart(u64 u64seccond);
void Co2SmartDelayTimerStop();
void Pm25SmartDelayTimerStart(u64 u64seccond);
void Pm25SmartDelayTimerStop();
void Pm25PeroidMeasStart(u16 u16second);
void FanSmartStart(u16 u16period);
void Pm25PeroidMeasStop();
void  ProcessPeriodReportData();
void  EdgeWTimer0Init();
typedef void * actimer_handle_t;
typedef void (*ac_timer_callback)( u8 u8TimerIndex );
typedef struct 
{
    u32 u32Interval;  
    ac_timer_callback p_callback;
    u8 u8TimerIndex;
    u8 u8ValidFlag;
}AC_Timer;

#define REPORTTIMERINDEX 0
#define SHT21TIMERINDEX 1
#endif

