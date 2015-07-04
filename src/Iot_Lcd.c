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
#ifdef I2C_DEVICE_SUPPORT
#define LCD_SUPPORT 1
#endif
#define LCD_SLAVE_ADDRESS 0x3E
u8 LCDDisplayShow[22];

/*********************************************************************************************************
** Function name:           LCDWriteCmd
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
void LCDWriteCmd(u8 addr,u8 cmd)
{
#ifdef LCD_SUPPORT
    IntMasterDisable();
    I2CMasterSlaveAddrSet(I2C1_BASE, LCD_SLAVE_ADDRESS, false);
    //slave RH Measure
    I2CMasterDataPut(I2C1_BASE, addr);
    //
    // Initiate send of data from the master.  Since the loopback
    // mode is enabled, the master and slave units are connected
    // allowing us to receive the same data that we sent out.
    //
    //    if(I2CMasterErr(I2C1_BASE))
    //    {
    //                return;
    //    }
    I2CMasterControl(I2C1_BASE,  I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(I2C1_BASE))
    {
    }
    I2CMasterDataPut(I2C1_BASE, cmd);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(I2CMasterBusy(I2C1_BASE))
    {
    }
    IntMasterEnable();
#endif
}
/*********************************************************************************************************
** Function name:           LCDReadOneByte
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

void LCDReadOneByte(u8 addr,u8 *pdata)
{
#ifdef LCD_SUPPORT
    IntMasterDisable();
    // slave address
    I2CMasterSlaveAddrSet(I2C1_BASE, LCD_SLAVE_ADDRESS, false);
    // commond type
    I2CMasterDataPut(I2C1_BASE, 0x80);
    //
    // Initiate send of data from the master.  Since the loopback
    // mode is enabled, the master and slave units are connected
    // allowing us to receive the same data that we sent out.
    //
    //    if(I2CMasterErr(I2C1_BASE))
    //    {
    //                return;
    //    }
    I2CMasterControl(I2C1_BASE,  I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(I2C1_BASE))
    {
    }
    
    I2CMasterDataPut(I2C1_BASE, addr);
    // 主模块开始发送数据
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(I2CMasterBusy(I2C1_BASE))
    {
    }
    
    
    I2CMasterSlaveAddrSet(I2C1_BASE, LCD_SLAVE_ADDRESS, true);      
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
    while(I2CMasterBusy(I2C1_BASE))
    {
    }
    *pdata= I2CMasterDataGet(I2C1_BASE); 
    IntMasterEnable();
#endif
}
/*********************************************************************************************************
** Function name:           ConfigureLCD
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
void ConfigureLCD(void)
{
  // The I2C0 peripheral must be enabled before use.
  //
  SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
  
  //
  // For this example I2C0 is used with PortB[3:2].  The actual port and
  // pins used may be different on your part, consult the data sheet for
  // more information.  GPIO port B needs to be enabled so these pins can
  // be used.
  // TODO: change this to whichever GPIO port you are using.
  //
      SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  //
  // Configure the pin muxing for I2C0 functions on port B2 and B3.
  // This step is not necessary if your part does not support pin muxing.
  // TODO: change this to select the port/pin you are using.
  //
  GPIOPinConfigure(GPIO_PA6_I2C1SCL);
  GPIOPinConfigure(GPIO_PA7_I2C1SDA);
  
  //
  // Select the I2C function for these pins.  This function will also
  // configure the GPIO pins pins for I2C operation, setting them to
  // open-drain operation with weak pull-ups.  Consult the data sheet
  // to see which functions are allocated per pin.
  // TODO: change this to select the port/pin you are using.
  //
  GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);
  GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);
  //
  // Enable and initialize the I2C0 master module.  Use the system clock for
  // the I2C0 module.  The last parameter sets the I2C data transfer rate.
  // If false the data rate is set to 100kbps and if true the data rate will
  // be set to 400kbps.  For this example we will use a data rate of 100kbps.
  //
  I2CMasterInitExpClk(I2C1_BASE, SysCtlClockGet(), false);
  
  LCDWriteCmd(0x82,3);
  LCDWriteCmd(0x84,3);
  LCDWriteCmd(0x86,0);
  LCDWriteCmd(0x88,0);
  LCDWriteCmd(0x8A,0x10);
}
/*********************************************************************************************************
** Function name:           LCDWriteOneByte
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
void LCDWriteOneByte(u8 addr,u8 data)
{
#ifdef LCD_SUPPORT
     IntMasterDisable();

    I2CMasterSlaveAddrSet(I2C1_BASE, LCD_SLAVE_ADDRESS, false);
    //slave RH Measure
    I2CMasterDataPut(I2C1_BASE, 0x80);
    //
    // Initiate send of data from the master.  Since the loopback
    // mode is enabled, the master and slave units are connected
    // allowing us to receive the same data that we sent out.
    //
//    if(I2CMasterErr(I2C1_BASE))
//    {
//                return;
//    }
    I2CMasterControl(I2C1_BASE,  I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(I2C1_BASE))
    {
    }
    
    I2CMasterDataPut(I2C1_BASE, addr);
    // 主模块开始发送数据
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
    while(I2CMasterBusy(I2C1_BASE))
    {
    }
    I2CMasterDataPut(I2C1_BASE, data);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(I2CMasterBusy(I2C1_BASE))
    {
    }
     IntMasterEnable();
#endif
}

/*********************************************************************************************************
** Function name:           LCDShowAll
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
void LCDShowAll()
{
#ifdef LCD_SUPPORT
    u8    i;
    //
    // Tell the master module what address it will place on the bus when
    // communicating with the slave.  Set the address to SLAVE_ADDRESS
    // (as set in the slave module).  The receive parameter is set to false
    // which indicates the I2C Master is initiating a writes to the slave.  If
    // true, that would indicate that the I2C Master is initiating reads from
    // the slave.
    //
    
    // LCDDisplayShow = address<<8|white_direction
    memset(LCDDisplayShow,0,sizeof(LCDDisplayShow));
    LCDDisplayShow[0]=0xf0;//s1 t1 t2 t3
    LCDDisplayShow[8]=0x80;//t4
    LCDDisplayShow[11]=0x01;//t5
    
    LCDDisplayShow[12]=0x10;//s0
    LCDDisplayShow[15]=0x18;//t6 t10
    LCDDisplayShow[16]=0x40;//t13
    LCDDisplayShow[17]=0x08;//p5
    I2CMasterSlaveAddrSet(I2C1_BASE, LCD_SLAVE_ADDRESS, false);
    //slave RH Measure
    I2CMasterDataPut(I2C1_BASE, 0x80);
    //
    // Initiate send of LCDDisplayShow from the master.  Since the loopback
    // mode is enabled, the master and slave units are connected
    // allowing us to receive the same LCDDisplayShow that we sent out.
    //
    I2CMasterControl(I2C1_BASE,  I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(I2C1_BASE))
    {
    }
    
    I2CMasterDataPut(I2C1_BASE, 0);
    // 主模块开始发送数据
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
    while(I2CMasterBusy(I2C1_BASE))
    {
    }
    // 主模块开始发送寄存器地址
    for(i=0; i<22; i++)
    {
        I2CMasterDataPut(I2C1_BASE, LCDDisplayShow[i]);
        // 主模块开始发送数据
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
        while(I2CMasterBusy(I2C1_BASE))
        {
        }
    }
    I2CMasterDataPut(I2C1_BASE, LCDDisplayShow[20]);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(I2CMasterBusy(I2C1_BASE))
    {
    }
    return;
#endif   
}
   

/*********************************************************************************************************
** Function name:           LCDWifiShow
** Descriptions:        pass     
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
void LCDWifiShow(u8 OnOff)
{
    LCDReadOneByte(14,&LCDDisplayShow[14]);
    if(OnOff)
    {
        LCDDisplayShow[14] = 0x0f|LCDDisplayShow[14];
    }
    else
    {
        LCDDisplayShow[14] = 0xf0&LCDDisplayShow[14]; 
    }
    LCDWriteOneByte(14,LCDDisplayShow[14]);
}

/*********************************************************************************************************
** Function name:           LCDPtcShow
** Descriptions:          pass  
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
void LCDPtcShow(u8 OnOff)
{
    LCDReadOneByte(12,&LCDDisplayShow[12]);
    if(OnOff)
    {
        LCDDisplayShow[12] |= 0x60;
    }
    else
    {
        LCDDisplayShow[12] &= 0x9f; 
    }
    LCDWriteOneByte(12,LCDDisplayShow[12]);
}

/*********************************************************************************************************
** Function name:           LCDIndoorShow
** Descriptions:            pass
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
void LCDIndoorTempShow(u16 Temp)
{
    u8 i = 0;
    while(i<2)
    {
        LCDReadOneByte(10-2*i,&LCDDisplayShow[10-2*i]);
        LCDReadOneByte(11-2*i,&LCDDisplayShow[11-2*i]);
        LCDDisplayShow[10-2*i] &= 0xe0;
        LCDDisplayShow[11-2*i] &= 0xf1;
        switch(Temp%10)
        {
            case 0:
            if(i==1)
            {
                break; 
            }
            LCDDisplayShow[10-2*i] |= 0xf;
            LCDDisplayShow[11-2*i] |= 0xa;
            break;
            case 1:
            LCDDisplayShow[11-2*i] |= 0xa;
            break;
            case 2:
            LCDDisplayShow[10-2*i] |= 0xd;
            LCDDisplayShow[11-2*i] |= 0x6;
            
            break;
            case 3:
            LCDDisplayShow[10-2*i] |= 0x9;
            LCDDisplayShow[11-2*i] |= 0xe;
            break;
            case 4:
            LCDDisplayShow[10-2*i] |= 0x2;
            LCDDisplayShow[11-2*i] |= 0xe;
            break;
            case 5:
            LCDDisplayShow[10-2*i] |= 0xb;
            LCDDisplayShow[11-2*i] |= 0xc;    
            break;
            case 6:      
            LCDDisplayShow[10-2*i] |= 0xf;
            LCDDisplayShow[11-2*i] |= 0xc;
            break;
            case 7:
            LCDDisplayShow[10-2*i] |= 0x1;
            LCDDisplayShow[11-2*i] |= 0xa;
            break;
            case 8:
            LCDDisplayShow[10-2*i] |= 0xf;
            LCDDisplayShow[11-2*i] |= 0xe;
            break;
            case 9:
            LCDDisplayShow[10-2*i] |= 0xb;
            LCDDisplayShow[11-2*i] |= 0xe;
            break;
            
        }
        LCDWriteOneByte(10-2*i,LCDDisplayShow[10-2*i]);
        LCDWriteOneByte(11-2*i,LCDDisplayShow[11-2*i]);
        Temp/=10;
        i++;
    }
    
}

/*********************************************************************************************************
** Function name:           LCDOutdoorTempShow
** Descriptions:         pass   
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
void LCDOutdoorTempShow(u16 Temp)
{        
    LCDReadOneByte(14,&LCDDisplayShow[14]);
    LCDReadOneByte(15,&LCDDisplayShow[15]);
    LCDDisplayShow[14] &= 0x0f;
    LCDDisplayShow[15] &= 0x1f;
    switch(Temp%10)
    {
        case 0:
        
        LCDDisplayShow[14] |= 0xf0;
        LCDDisplayShow[15] |= 0xa0;
        break;
        case 1:
        LCDDisplayShow[15] |= 0xa0;
        break;
        case 2:
        LCDDisplayShow[14] |= 0xd0;
        LCDDisplayShow[15] |= 0x60;
        
        break;
        case 3:
        LCDDisplayShow[14] |= 0x90;
        LCDDisplayShow[15] |= 0xe0;
        break;
        case 4:
        LCDDisplayShow[14] |= 0x20;
        LCDDisplayShow[15] |= 0xe0;
        break;
        case 5:
        LCDDisplayShow[14] |= 0xb0;
        LCDDisplayShow[15] |= 0xc0;    
        break;
        case 6:      
        LCDDisplayShow[14] |= 0xf0;
        LCDDisplayShow[15] |= 0xc0;
        break;
        case 7:
        LCDDisplayShow[14] |= 0x10;
        LCDDisplayShow[15] |= 0xa0;
        break;
        case 8:
        LCDDisplayShow[14] |= 0xf0;
        LCDDisplayShow[15] |= 0xe0;
        break;
        case 9:
        LCDDisplayShow[14] |= 0xb0;
        LCDDisplayShow[15] |= 0xe0;
        break;
        
    }
    LCDWriteOneByte(14,LCDDisplayShow[14]);
    LCDWriteOneByte(15,LCDDisplayShow[15]);
    Temp/=10;
    
    LCDReadOneByte(12,&LCDDisplayShow[12]);
    LCDReadOneByte(13,&LCDDisplayShow[13]);
    LCDDisplayShow[12] &= 0xf0;
    LCDDisplayShow[13] &= 0xf1;
    switch(Temp%10)
    {
        case 0:
        //LCDDisplayShow[12] |= 0xf;
        //LCDDisplayShow[13] |= 0xa;
        break;
        case 1:
        LCDDisplayShow[13] |= 0xa;
        break;
        case 2:
        LCDDisplayShow[12] |= 0xd;
        LCDDisplayShow[13] |= 0x6;
        
        break;
        case 3:
        LCDDisplayShow[12] |= 0x9;
        LCDDisplayShow[13] |= 0xe;
        break;
        case 4:
        LCDDisplayShow[12] |= 0x2;
        LCDDisplayShow[13] |= 0xe;
        break;
        case 5:
        LCDDisplayShow[12] |= 0xb;
        LCDDisplayShow[13] |= 0xc;    
        break;
        case 6:      
        LCDDisplayShow[12] |= 0xf;
        LCDDisplayShow[13] |= 0xc;
        break;
        case 7:
        LCDDisplayShow[12] |= 0x1;
        LCDDisplayShow[13] |= 0xa;
        break;
        case 8:
        LCDDisplayShow[12] |= 0xf;
        LCDDisplayShow[13] |= 0xe;
        break;
        case 9:
        LCDDisplayShow[12] |= 0xb;
        LCDDisplayShow[13] |= 0xe;
        break;
        
    }
    LCDWriteOneByte(12,LCDDisplayShow[12]);
    LCDWriteOneByte(13,LCDDisplayShow[13]);
    
}

/*********************************************************************************************************
** Function name:           LCDRhShow
** Descriptions:            pass
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
void LCDRhShow(u16 Temp)
{ 
    u8 i = 0;
    while(i<2)
    {
        LCDReadOneByte(8+2*i,&LCDDisplayShow[8+2*i]);
        LCDReadOneByte(9+2*i,&LCDDisplayShow[9+2*i]);
        LCDDisplayShow[8+2*i] &= 0x8f;
        LCDDisplayShow[9+2*i] &= 0x0f;
        switch(Temp%10)
        {
            case 0:
            if(i==1)
            {
                break; 
            }
            LCDDisplayShow[8+2*i] |= 0x50;
            LCDDisplayShow[9+2*i] |= 0xf0;
            break;
            case 1:
            LCDDisplayShow[8+2*i] |= 0x50;
            break;
            case 2:
            LCDDisplayShow[8+2*i] |= 0x60;
            LCDDisplayShow[9+2*i] |= 0xb0;
            
            break;
            case 3:
            LCDDisplayShow[8+2*i] |= 0x70;
            LCDDisplayShow[9+2*i] |= 0x90;
            break;
            case 4:
            LCDDisplayShow[8+2*i] |= 0x70;
            LCDDisplayShow[9+2*i] |= 0x40;
            break;
            case 5:
            LCDDisplayShow[8+2*i] |= 0x30;
            LCDDisplayShow[9+2*i] |= 0xd0;   
            break;
            case 6:      ;
            LCDDisplayShow[8+2*i] |= 0x30;
            LCDDisplayShow[9+2*i] |= 0xf0;
            break;
            case 7:
            LCDDisplayShow[8+2*i] |= 0x50;
            LCDDisplayShow[9+2*i] |= 0x80;
            break;
            case 8:
            LCDDisplayShow[8+2*i] |= 0x70;
            LCDDisplayShow[9+2*i] |= 0xf0;
            break;
            case 9:
            LCDDisplayShow[8+2*i] |= 0x70;
            LCDDisplayShow[9+2*i] |= 0xd0;
            break;
            
        }
        LCDWriteOneByte(8+2*i,LCDDisplayShow[8+2*i]);
        LCDWriteOneByte(9+2*i,LCDDisplayShow[9+2*i]);
        Temp/=10;
        i++;
    }
  
}

/*********************************************************************************************************
** Function name:           LCDPm25Show
** Descriptions:        pass    
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
void LCDPm25Show(u16 Temp)
{
    u8 i = 0;
    u8 decflag;
    u8 hunflag;
    if(Temp>999)
    {
      Temp = 999;
    }
    decflag = (Temp/10)%10;
    hunflag = (Temp/100)%10;
    while(i<3)
    {
        LCDReadOneByte(2+2*i,&LCDDisplayShow[2+2*i]);
        LCDReadOneByte(3+2*i,&LCDDisplayShow[3+2*i]);
        LCDDisplayShow[2+2*i] &= 0xf8;
        LCDDisplayShow[3+2*i] &= 0xf0;
        switch(Temp%10)
        {
            case 0:
            if(i>1)
            {
                break;
            }
            LCDDisplayShow[2+2*i] |= 0x5;
            LCDDisplayShow[3+2*i] |= 0xf;
            break;
            case 1:
            LCDDisplayShow[2+2*i] |= 0x5;
            break;
            case 2:
            LCDDisplayShow[2+2*i] |= 0x6;
            LCDDisplayShow[3+2*i] |= 0xb;
            
            break;
            case 3:
            LCDDisplayShow[2+2*i] |= 0x7;
            LCDDisplayShow[3+2*i] |= 0x9;
            break;
            case 4:
            LCDDisplayShow[2+2*i] |= 0x7;
            LCDDisplayShow[3+2*i] |= 0x4;
            break;
            case 5:
            LCDDisplayShow[2+2*i] |= 0x3;
            LCDDisplayShow[3+2*i] |= 0xd;   
            break;
            case 6:      ;
            LCDDisplayShow[2+2*i] |= 0x3;
            LCDDisplayShow[3+2*i] |= 0xf;
            break;
            case 7:
            LCDDisplayShow[2+2*i] |= 0x5;
            LCDDisplayShow[3+2*i] |= 0x8;
            break;
            case 8:
            LCDDisplayShow[2+2*i] |= 0x7;
            LCDDisplayShow[3+2*i] |= 0xf;
            break;
            case 9:
            LCDDisplayShow[2+2*i] |= 0x7;
            LCDDisplayShow[3+2*i] |= 0xd;
            break;
            
        }
        LCDWriteOneByte(2+2*i,LCDDisplayShow[2+2*i]);
        LCDWriteOneByte(3+2*i,LCDDisplayShow[3+2*i]);
        Temp/=10;
        i++;
    }
    if(0==(decflag+hunflag))//百位和十位都是0,十位0点灭
    {
        LCDDisplayShow[4] &= 0xf8;
        LCDDisplayShow[5] &= 0xf0;
        LCDWriteOneByte(4,LCDDisplayShow[4]);
        LCDWriteOneByte(5,LCDDisplayShow[5]);
    }
                                         
}
                                         
/*********************************************************************************************************
** Function name:           LCDCo2Show
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
void LCDCo2Show(u16 Temp)
{
    u8 i = 0;
    u8 decflag = (Temp/10)%10;
    u8 hunflag = (Temp/100)%10;
    u8 thouflag = (Temp/1000)%10;
    LCDReadOneByte(0,&LCDDisplayShow[0]);
    LCDReadOneByte(1,&LCDDisplayShow[1]);
    LCDDisplayShow[0] &= 0xf1;
    LCDDisplayShow[1] &= 0xf0;
    switch(Temp%10)
    {
        case 0:
        LCDDisplayShow[0] |= 0xa;
        LCDDisplayShow[1] |= 0xf;
        break;
        case 1:
        LCDDisplayShow[0] |= 0xa;
        break;
        case 2:
        LCDDisplayShow[0] |= 0x6;
        LCDDisplayShow[1] |= 0xd;
        
        break;
        case 3:
        LCDDisplayShow[0] |= 0xe;
        LCDDisplayShow[1] |= 0x9;
        break;
        case 4:
        LCDDisplayShow[0] |= 0xe;
        LCDDisplayShow[1] |= 0x2;
        break;
        case 5:
        LCDDisplayShow[0] |= 0xc;
        LCDDisplayShow[1] |= 0xb;    
        break;
        case 6:      
        LCDDisplayShow[0] |= 0xc;
        LCDDisplayShow[1] |= 0xf;
        break;
        case 7:
        LCDDisplayShow[0] |= 0xa;
        LCDDisplayShow[1] |= 0x1;
        break;
        case 8:
        LCDDisplayShow[0] |= 0xe;
        LCDDisplayShow[1] |= 0xf;
        break;
        case 9:
        LCDDisplayShow[0] |= 0xe;
        LCDDisplayShow[1] |= 0xb;
        break;
        
    }
    LCDWriteOneByte(0,LCDDisplayShow[0]);
    LCDWriteOneByte(1,LCDDisplayShow[1]);
    Temp/=10;
    
    while(i<3)
    {
        LCDReadOneByte(2+2*i,&LCDDisplayShow[2+2*i]);
        LCDReadOneByte(3+2*i,&LCDDisplayShow[3+2*i]);
        LCDDisplayShow[2+2*i] &= 0x8f;
        LCDDisplayShow[3+2*i] &= 0x0f;
        switch(Temp%10)
        {
            case 0:
            if(i==2)
            {
                break;
            }
            LCDDisplayShow[2+2*i] |= 0x50;
            LCDDisplayShow[3+2*i] |= 0xf0;
            break;
            case 1:
            LCDDisplayShow[2+2*i] |= 0x50;
            break;
            case 2:
            LCDDisplayShow[2+2*i] |= 0x60;
            LCDDisplayShow[3+2*i] |= 0xb0;
            
            break;
            case 3:
            LCDDisplayShow[2+2*i] |= 0x70;
            LCDDisplayShow[3+2*i] |= 0x90;
            break;
            case 4:
            LCDDisplayShow[2+2*i] |= 0x70;
            LCDDisplayShow[3+2*i] |= 0x40;
            break;
            case 5:
            LCDDisplayShow[2+2*i] |= 0x30;
            LCDDisplayShow[3+2*i] |= 0xd0;   
            break;
            case 6:      ;
            LCDDisplayShow[2+2*i] |= 0x30;
            LCDDisplayShow[3+2*i] |= 0xf0;
            break;
            case 7:
            LCDDisplayShow[2+2*i] |= 0x50;
            LCDDisplayShow[3+2*i] |= 0x80;
            break;
            case 8:
            LCDDisplayShow[2+2*i] |= 0x70;
            LCDDisplayShow[3+2*i] |= 0xf0;
            break;
            case 9:
            LCDDisplayShow[2+2*i] |= 0x70;
            LCDDisplayShow[3+2*i] |= 0xd0;
            break;
            
        }
        LCDWriteOneByte(2+2*i,LCDDisplayShow[2+2*i]);
        LCDWriteOneByte(3+2*i,LCDDisplayShow[3+2*i]);
        Temp/=10;
        i++;
    }
    if(0==(thouflag+hunflag))
    {
        LCDDisplayShow[4] &= 0x8f;
        LCDDisplayShow[5] &= 0x0f;
        LCDWriteOneByte(4,LCDDisplayShow[4]);
        LCDWriteOneByte(5,LCDDisplayShow[5]);
    }
    if(0==(decflag+hunflag+thouflag))//百位和十位都是0,十位0点灭
    {
        LCDDisplayShow[2] &= 0x8f;
        LCDDisplayShow[3] &= 0x0f;
        LCDWriteOneByte(2,LCDDisplayShow[2]);
        LCDWriteOneByte(3,LCDDisplayShow[3]);
    }
                                         
}
                                         
/*********************************************************************************************************
** Function name:           LCDFanSpeedShow
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
void LCDFanSpeedShow(u16 Temp)
{
    u8 i = 0;
    if(Temp>100)
    {
        return;
    }
    u8 decflag = (Temp/10)%10;
    u8 hunflag = (Temp/100)%10;
    while(i<3)
    {
        LCDReadOneByte(17+2*i,&LCDDisplayShow[17+2*i]);
        LCDReadOneByte(18+2*i,&LCDDisplayShow[18+2*i]);
        LCDDisplayShow[17+2*i] &= 0x8f;
        LCDDisplayShow[18+2*i] &= 0x0f;
        switch(Temp%10)
        {
            case 0:
            if(i==2)
            {
                break;
            }
          
            LCDDisplayShow[17+2*i] |= 0x50;
            LCDDisplayShow[18+2*i] |= 0xf0;
            break;
            case 1:
            LCDDisplayShow[17+2*i] |= 0x50;
            break;
            case 2:
            LCDDisplayShow[17+2*i] |= 0x60;
            LCDDisplayShow[18+2*i] |= 0xb0;
            
            break;
            case 3:
            LCDDisplayShow[17+2*i] |= 0x70;
            LCDDisplayShow[18+2*i] |= 0x90;
            break;
            case 4:
            LCDDisplayShow[17+2*i] |= 0x70;
            LCDDisplayShow[18+2*i] |= 0x40;
            break;
            case 5:
            LCDDisplayShow[17+2*i] |= 0x30;
            LCDDisplayShow[18+2*i] |= 0xd0;   
            break;
            case 6:      ;
            LCDDisplayShow[17+2*i] |= 0x30;
            LCDDisplayShow[18+2*i] |= 0xf0;
            break;
            case 7:
            LCDDisplayShow[17+2*i] |= 0x50;
            LCDDisplayShow[18+2*i] |= 0x80;
            break;
            case 8:
            LCDDisplayShow[17+2*i] |= 0x70;
            LCDDisplayShow[18+2*i] |= 0xf0;
            break;
            case 9:
            LCDDisplayShow[17+2*i] |= 0x70;
            LCDDisplayShow[18+2*i] |= 0xd0;
            
        }
        LCDWriteOneByte(17+2*i,LCDDisplayShow[17+2*i]);
        LCDWriteOneByte(18+2*i,LCDDisplayShow[18+2*i]);
        Temp/=10;
        i++;
    } 
    if(0==(decflag+hunflag))//百位和十位都是0,十位0点灭
    {
        LCDDisplayShow[19] &= 0x8f;
        LCDDisplayShow[20] &= 0x0f;
        LCDWriteOneByte(19,LCDDisplayShow[19]);
        LCDWriteOneByte(20,LCDDisplayShow[20]);
    }
}                                         
/*********************************************************************************************************
** Function name:           LCDAirVolShow
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
void LCDAirVolShow(u16 Temp)
{
    u8 i = 0;
    u8 decflag = (Temp/10)%10;
    u8 hunflag = (Temp/100)%10;
    u8 thouflag = (Temp/1000)%10;
    while(i<4)
    {
        LCDReadOneByte(15+2*i,&LCDDisplayShow[15+2*i]);
        LCDReadOneByte(16+2*i,&LCDDisplayShow[16+2*i]);
        LCDDisplayShow[15+2*i] &= 0x1f;
        LCDDisplayShow[16+2*i] &= 0x0f;
        switch(Temp%10)
        {
            case 0:
            if(i==3)
            {
                break;
            }
            LCDDisplayShow[15+2*i] |= 0x5;
            LCDDisplayShow[16+2*i] |= 0xf;
            break;
            case 1:
            LCDDisplayShow[15+2*i] |= 0x5;
            break;
            case 2:
            LCDDisplayShow[15+2*i] |= 0x6;
            LCDDisplayShow[16+2*i] |= 0xb;
            
            break;
            case 3:
            LCDDisplayShow[15+2*i] |= 0x7;
            LCDDisplayShow[16+2*i] |= 0x9;
            break;
            case 4:
            LCDDisplayShow[15+2*i] |= 0x7;
            LCDDisplayShow[16+2*i] |= 0x4;
            break;
            case 5:
            LCDDisplayShow[15+2*i] |= 0x3;
            LCDDisplayShow[16+2*i] |= 0xd;    
            break;
            case 6:      
            LCDDisplayShow[15+2*i] |= 0x3;
            LCDDisplayShow[16+2*i] |= 0xf;
            break;
            case 7:
            LCDDisplayShow[15+2*i] |= 0x5;
            LCDDisplayShow[16+2*i] |= 0x8;
            break;
            case 8:
            LCDDisplayShow[15+2*i] |= 0x7;
            LCDDisplayShow[16+2*i] |= 0xf;
            break;
            case 9:
            LCDDisplayShow[15+2*i] |= 0x7;
            LCDDisplayShow[16+2*i] |= 0xd;
            break;
            
        }
        LCDWriteOneByte(15+2*i,LCDDisplayShow[15+2*i]);
        LCDWriteOneByte(16+2*i,LCDDisplayShow[16+2*i]);
        Temp/=10;
        i++;
    }
    if(0==(hunflag+thouflag))//百位和十位都是0,百位0点灭
    {
        LCDDisplayShow[19] &= 0xf8;
        LCDDisplayShow[20] &= 0xf0;
        LCDWriteOneByte(19,LCDDisplayShow[19]);
        LCDWriteOneByte(20,LCDDisplayShow[20]);
    }
    if(0==(decflag+hunflag+thouflag))//百位，十位，千位都是0,十位0点灭
    {
        LCDDisplayShow[17] &= 0xf8;
        LCDDisplayShow[18] &= 0xf0;
        LCDWriteOneByte(17,LCDDisplayShow[17]);
        LCDWriteOneByte(18,LCDDisplayShow[18]);
    }
                                                                              
} 

/*********************************************************************************************************
** Function name:           LCDAMError
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
void LCDAMErrorShow(u16 OnOff)
{
    LCDReadOneByte(13,&LCDDisplayShow[13]);
    if(OnOff)
    {
        LCDDisplayShow[13] |= 0x01;
    }
    else
    {
        LCDDisplayShow[13] &= 0xfe; 
    }
    LCDWriteOneByte(13,LCDDisplayShow[13]);
                                                                              
}
/*********************************************************************************************************
** Function name:           LCDFanRunShow
** Descriptions:            pass
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
void LCDFanRunShow()
{
    LCDReadOneByte(16,&LCDDisplayShow[16]);
    LCDReadOneByte(17,&LCDDisplayShow[17]);
    LCDReadOneByte(19,&LCDDisplayShow[19]);
    LCDReadOneByte(21,&LCDDisplayShow[21]);
    
    
    LCDDisplayShow[16] |= 0x80;
    LCDDisplayShow[19] |= 0x80;
    
    LCDDisplayShow[17] |= 0x80;
    LCDDisplayShow[21] |= 0x80;
    
    
    LCDWriteOneByte(16,LCDDisplayShow[16]);
    LCDWriteOneByte(17,LCDDisplayShow[17]);
    LCDWriteOneByte(19,LCDDisplayShow[19]);                                                                           
    LCDWriteOneByte(21,LCDDisplayShow[21]);  
}

/*********************************************************************************************************
** Function name:           LCDFanStopShow
** Descriptions:            pass
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
void LCDFanStopShow()
{
    
    LCDReadOneByte(16,&LCDDisplayShow[16]);
    LCDReadOneByte(17,&LCDDisplayShow[17]);
    LCDReadOneByte(19,&LCDDisplayShow[19]);
    LCDReadOneByte(21,&LCDDisplayShow[21]);
    
    LCDDisplayShow[16] &= ~0x80;
    LCDDisplayShow[19] &= ~0x80;
    
    LCDDisplayShow[17] &= ~0x80;
    LCDDisplayShow[21] &= ~0x80;
    LCDWriteOneByte(16,LCDDisplayShow[16]);
    LCDWriteOneByte(17,LCDDisplayShow[17]);
    LCDWriteOneByte(19,LCDDisplayShow[19]);                                                                           
    LCDWriteOneByte(21,LCDDisplayShow[21]); 
    
}

/*********************************************************************************************************
** Function name:           LCDBackgroundOn
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
void LCDBackgroundOn()
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
}

/*********************************************************************************************************
** Function name:           LCDInit
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
void LCDInit(void)
{
    LCDBackgroundOn();
    ConfigureLCD();
    LCDShowAll();
}