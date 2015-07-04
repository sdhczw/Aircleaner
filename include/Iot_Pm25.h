//*****************************************************************************
//
// Beta_Arm.h - Simple hello world example.
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
#ifndef IOT_PM25_H
#define IOT_PM25_H

#define  PM25SMARTDELAYONPEARIOD ((u64)3*10) //pm2.5 �����ӳ��ж�
#define  PM25SMARTDELAYOFFPEARIOD ((u64)2*10) //pm2.5 �����ӳ��ж�
#define  PM25SMARTLEVEL 75 // measure 20 times

void  PM25Init();
void PM25SetOff();
void PM25SetOn();
u16 GetPM25();
extern u16 g_u16PM25SmartFlag;
#endif

