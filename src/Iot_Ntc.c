#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "utils/uartstdio.h"
#include "types.h"
#include "driverlib/adc.h"
#include "Iot_Lcd.h"
u8 g_u8Timer0BNtcFlag = 1;
u16 g_u16OutdoorTEData= 450;
u16 g_u16OutdoorTETab[] = {
    1536 ,1455 ,1378 ,1306 ,1237 ,1172 ,1111 ,1053 ,998 ,946 ,
    897  ,851  ,807  ,766  ,726  ,689  ,654  ,621  ,589 ,560 ,
    531  ,505  ,480  ,456  ,433  ,412  ,392  ,372  ,354 ,337 ,
    321  ,305  ,291  ,277  ,263  ,251  ,239  ,228  ,217 ,207 ,
    197  ,188  ,180  ,172  ,164  ,156  ,149  ,143  ,136 ,130 ,
    124  ,119  ,115  ,110  ,105  ,101  ,96   ,92   ,88  ,85  ,
    81   ,78   ,74   ,71   ,68   ,66   ,63   ,60   ,58  ,56  ,
};
/*********************************************************************************************************
** Function name:           ConfigureADC0
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
void ConfigureADC0(void)
{
  //
  
     // ROM_FPULazyStackingEnable();
    
    //
    // Set the clocking to run directly from the crystal.
    //
  //  ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       //SYSCTL_OSC_MAIN);
    // The ADC0 peripheral must be enabled for use.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
 
    //500ksps
  //  ADCClockConfigSet(ADC0_BASE,ADC_CLOCK_SRC_PIOSC | ADC_CLOCK_RATE_HALF,1); 
    //
    // For this example ADC0 is used with AIN0 on port E7.
    // The actual port and pins used may be different on your part, consult
    // the data sheet for more information.  GPIO port E needs to be enabled
    // so these pins can be used.
    // TODO: change this to whichever GPIO port you are using.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    //
    // Select the analog ADC function for these pins.
    // Consult the data sheet to see which functions are allocated per pin.
    // TODO: change this to select the port/pin you are using.
    //
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

    //
    // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    // will do a single sample when the processor sends a signal to start the
    // conversion.  Each ADC module has 4 programmable sequences, sequence 0
    // to sequence 3.  This example is arbitrarily using sequence 3.
    //
    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);

    //
    // Configure step 0 on sequence 3.  Sample channel 0 (ADC_CTL_CH0) in
    // single-ended mode (default) and configure the interrupt flag
    // (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
    // that this is the last conversion on sequence 3 (ADC_CTL_END).  Sequence
    // 3 has only one programmable step.  Sequence 1 and 2 have 4 steps, and
    // sequence 0 has 8 programmable steps.  Since we are only doing a single
    // conversion using sequence 3 we will only configure step 0.  For more
    // information on the ADC sequences and steps, reference the datasheet.
    //
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH0);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_CH0);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 2, ADC_CTL_CH0);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 3, ADC_CTL_CH0);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 4, ADC_CTL_CH0);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 5, ADC_CTL_CH0);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 6, ADC_CTL_CH0);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 7, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);

    //
    // Since sample sequence 3 is now configured, it must be enabled.
    //
    ADCSequenceEnable(ADC0_BASE, 0);

    //
    // Clear the interrupt status flag.  This is done to make sure the
    // interrupt flag is cleared before we sample.
    //
    //ADCIntClear(ADC0_BASE, 0);
}


/*********************************************************************************************************
** Function name:           bsearch
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
u16 bsearch(u16 array[],u16 low,u16 high,u16 target)
{
    if(target>array[low])
    {
        return (low-30);
    }
    while(low<=high)
    {
        u16 mid = (low+high)/2;
        if(array[mid]<target)
            high = mid-1;
        else if(array[mid]>target)
            low  = mid+1;
        else//findthetarget
            return (mid-30);
    }
    //thearraydoesnotcontainthetarget
    return (low-30);
}

/*********************************************************************************************************
** Function name:           ReadOutdoorTemp
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
u16 ReadOutdoorTemp()
{
	u32 temvol[8];
    u32 i=0;
    u32 vol;
    u32 avg = 0;
    u32 sum=0;   
    u32 Rt = 0;
    ADCProcessorTrigger(ADC0_BASE, 0);  
    while(!ADCIntStatus(ADC0_BASE,0, false))
    {
    }
    ADCIntClear(ADC0_BASE, 0);
    ADCSequenceDataGet(ADC0_BASE, 0, temvol);
    
    for(i=0;i<8;i++)
    {
        sum  = temvol[i] +sum;
    }
    avg = sum / 8;
    vol=avg*3000/4095; 
    Rt = vol*33*10/(3300-vol);
    return bsearch(g_u16OutdoorTETab,0,sizeof(g_u16OutdoorTETab)/sizeof(u16)-1,Rt);
}

/*********************************************************************************************************
** Function name:           NtcInit
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
void NtcInit()
{
    ConfigureADC0();
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
void  ProcessNtcData()
{
    if(1==g_u8Timer0BNtcFlag)
    {
        g_u16OutdoorTEData = ReadOutdoorTemp();
        LCDOutdoorTempShow(g_u16OutdoorTEData);
        g_u8Timer0BNtcFlag = 0;
    }
    return;
}

/*********************************************************************************************************
** Function name:           GetOutdoorTempData
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
u16 GetOutdoorTempData()
{
    return g_u16OutdoorTEData;
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
void SetNtcMeasOn()
{
     g_u8Timer0BNtcFlag = 1;
}
