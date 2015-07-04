#include <stdint.h>
#include <stdbool.h>
#include "utils/uartstdio.h"
#include "types.h"
#include "Iot_I2c.h"
#include "Iot_Lcd.h"
u16 g_u16RhData = 10;
u16 g_u16IndoorTEData= 25;
u8 g_u8Timer0BSHT21Flag = 1;
/*********************************************************************************************************
** Function name:           ProcessI2cData
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
void  ProcessSht21Data()
{
    if(1==g_u8Timer0BSHT21Flag)
    {
        g_u16IndoorTEData = ReadIndoorTemp();
        g_u16RhData = ReadRH(); 
        LCDIndoorTempShow(g_u16IndoorTEData);
        LCDRhShow(g_u16RhData);
        g_u8Timer0BSHT21Flag = 0;
    }
    return;
}

/*********************************************************************************************************
** Function name:           GetIndoorTempData
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
u16 GetIndoorTempData()
{
    return g_u16IndoorTEData;
}

/*********************************************************************************************************
** Function name:           GetRHData
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
u16 GetRhData()
{
    return g_u16RhData;
}
/*********************************************************************************************************
** Function name:           SetSht21Flag
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
void SetSht21MeasOn()
{
     g_u8Timer0BSHT21Flag = 1;
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
void Sht21Init()
{   
    ConfigureI2C0Master();
    ProcessSht21Data();
}
