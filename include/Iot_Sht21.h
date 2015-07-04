/**
******************************************************************************
* @file     Iot_Sht21.h
* @authors  zhangwen
* @version  V1.0.0
* @date     12-Feb-2015
* @brief    
******************************************************************************
*/
#ifndef IOT_SHT21_H
#define IOT_SHT21_H



//*****************************************************************************
//
// macro
//
//*****************************************************************************
void  Sht21Init(void);
void  ProcessSht21Data();
void  SetSht21MeasreOn();
u16 GetIndoorTempData();
u16 GetRhData();
void SetSht21MeasOn();
#endif

