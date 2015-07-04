//*****************************************************************************
//
// Iot_LCD.h - 
//
// Copyright (c) 2012-2014 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.0.12573 of the EK-TM4C123GXL Firmware Package.
//
//*****************************************************************************
#ifndef IOT_LCD_H
#define IOT_LCD_H


void LCDInit(void);
void LCDFanStopShow();
void LCDFanRunShow();
void LCDShowAll();
void LCDWifiShow();
void LCDPtcShow(u8 OnOff);
void LCDIndoorTempShow(u16 Temp);
void LCDOutdoorTempShow(u16 Temp);
void LCDRhShow(u16 Temp);
void LCDPm25Show(u16 Temp);
void LCDCo2Show(u16 Temp);
void LCDFanSpeedShow(u16 Temp);
void LCDAirVolShow(u16 Temp);
void LCDAMErrorShow(u16 OnOff);
void LCDFanRunShow();
void LCDFanStopShow();
#endif

