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
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/i2c.h"
#include "driverlib/interrupt.h" 
#include "ac_common.h"
#include "Iot_Timer.h"

#define TE 0
#define RH 1
#define SLAVE_ADDRESS 0x40

/*********************************************************************************************************
** Function name:           ConfigureI2C0Master
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

void ConfigureI2C0Master(void)
{
    // The I2C0 peripheral must be enabled before use.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    
    //
    // For this example I2C0 is used with PortB[3:2].  The actual port and
    // pins used may be different on your part, consult the data sheet for
    // more information.  GPIO port B needs to be enabled so these pins can
    // be used.
    // TODO: change this to whichever GPIO port you are using.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    
    //
    // Configure the pin muxing for I2C0 functions on port B2 and B3.
    // This step is not necessary if your part does not support pin muxing.
    // TODO: change this to select the port/pin you are using.
    //
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);
    
    //
    // Select the I2C function for these pins.  This function will also
    // configure the GPIO pins pins for I2C operation, setting them to
    // open-drain operation with weak pull-ups.  Consult the data sheet
    // to see which functions are allocated per pin.
    // TODO: change this to select the port/pin you are using.
    //
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);
    //
    // Enable and initialize the I2C0 master module.  Use the system clock for
    // the I2C0 module.  The last parameter sets the I2C data transfer rate.
    // If false the data rate is set to 100kbps and if true the data rate will
    // be set to 400kbps.  For this example we will use a data rate of 100kbps.
    //
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), true);
    I2CMasterEnable(I2C0_BASE);
}

/*********************************************************************************************************
** Function name:           I2CWReadSHT21
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
u32 I2CWReadSHT21(u32 MODE)
{
    unsigned int  RetData = 0;
    unsigned short int u16data = 0;
    unsigned short int u16Command = 0; 
    unsigned char pdata[3] = {0};
    if(MODE==TE)
    {
        u16Command = 0xf3;
    }
    else
    {
        u16Command = 0xf5;
    }
    //
    // Tell the master module what address it will place on the bus when
    // communicating with the slave.  Set the address to SLAVE_ADDRESS
    // (as set in the slave module).  The receive parameter is set to false
    // which indicates the I2C Master is initiating a writes to the slave.  If
    // true, that would indicate that the I2C Master is initiating reads from
    // the slave.
    //
    
    // data = address<<8|white_direction
    I2CMasterSlaveAddrSet(I2C0_BASE, SLAVE_ADDRESS, false);
    //slave RH Measure
    I2CMasterDataPut(I2C0_BASE, u16Command);
    //
    // Initiate send of data from the master.  Since the loopback
    // mode is enabled, the master and slave units are connected
    // allowing us to receive the same data that we sent out.
    //
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while(I2CMasterBusy(I2C0_BASE))
    {
    }
    SysCtlDelay(SysCtlClockGet() / 3/10);//100ms
    // Wait until the slave has received and acknowledged the data.
    //
    I2CMasterSlaveAddrSet(I2C0_BASE, SLAVE_ADDRESS, true);
    //Read RH Measure			
    //
    // Initiate send of data from the master.  Since the loopback
    // mode is enabled, the master and slave units are connected
    // allowing us to receive the same data that we sent out.
    //		
    while(I2CMasterBusy(I2C0_BASE))
    {
    }
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
    //Read RH Measure			
    // Read the data from the slave.
    //
    while(I2CMasterBusy(I2C0_BASE))
    {
    }
    //SysCtlDelay(SysCtlClockGet() /3/100); //100ms     
    //  I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
    
    pdata[0] = I2CMasterDataGet(I2C0_BASE);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
    while(I2CMasterBusy(I2C0_BASE))
    {
    }
    pdata[1] = I2CMasterDataGet(I2C0_BASE);   
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
    while(I2CMasterBusy(I2C0_BASE))
    {
    }
    pdata[2] = I2CMasterDataGet(I2C0_BASE);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    while(I2CMasterBusy(I2C0_BASE))
    {
    }
    //  I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);			
    u16data = (pdata[0]<<8|(pdata[1]&0xfc));
    if(MODE==TE)	
    {
        RetData = (u32)((float)u16data*0.0026812-46.85);
    }
    else
    {
        RetData = (u32)(0.0019073*(float)u16data-6);
    }
    //
    // Wait until master module is done transferring.
    //
    return RetData;
}

/*********************************************************************************************************
** Function name:           ReadTemp
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
u16 ReadIndoorTemp()
{
    return I2CWReadSHT21(TE);
}

/*********************************************************************************************************
** Function name:           ReadRH
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
u16 ReadRH()
{
    return I2CWReadSHT21(RH);
}


