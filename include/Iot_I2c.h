/**
******************************************************************************
* @file     Iot_I2c.h
* @authors  zhangwen
* @version  V1.0.0
* @date     12-Feb-2015
* @brief    
******************************************************************************
*/
#ifndef IOT_I2C_H
#define IOT_I2C_H



//*****************************************************************************
//
// macro
//
//*****************************************************************************
void ConfigureI2C0Master(void);
u16 ReadIndoorTemp();
u16 ReadRH();
#endif

