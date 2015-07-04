/******************************************************************************
 * MODULE NAME:    types.h
 * PROJECT CODE:    _
 * DESCRIPTION:     This file is intends for ring-buffer API.
 * DESIGNER:        Th3 Huang
 * DATE:            04 23 2012
 *
 * SOURCE CONTROL:
 *
 * LICENSE:
 *     This source code is copyright (c) 2012 MediaTek. Inc.
 *     All rights reserved.
 *
 * REVISION     HISTORY:
 *   V1.0.0     04 23 2012    - Initial Version V1.0
 *
 *
 * SOURCE:
 * ISSUES:
 *    First Implementation.
 * NOTES TO USERS:
 *
 ******************************************************************************/
#ifndef TYPES_H
#define TYPES_H

//*****************************************************************************
//
// type
//
//*****************************************************************************

typedef	unsigned char       u8;
typedef	unsigned short      u16;
typedef	unsigned int        u32;
typedef	unsigned long  long     u64;

typedef	 char	    s8;
typedef	 short      s16;
typedef	 int        s32;
typedef	 long  long     s64;

//*****************************************************************************
//
// MACROS
//
//*****************************************************************************
#define FALSE -1
#define TRUE 0

#define RT_DEBUG_OFF        0
#define RT_DEBUG_ERROR      1
#define RT_DEBUG_TRACE      2
#define RT_DEBUG_INFO       3

#ifdef RT_DEBUG_SUPPORT
extern u8 RTDebugLevel;
#define DBGPRINT(Level, Fmt)    	\
{                                   \
    if ((Level) <= RTDebugLevel)    \
    {                               \
        UARTprintf Fmt;			    \
    }                               \
}
#else
#define DBGPRINT(Level, Fmt)
#endif
#endif

/******************************************************************************
 * MODULE NAME:    types.h
 * PROJECT CODE:    _
 * DESCRIPTION:     This file is intends for ring-buffer API.
 * DESIGNER:        Th3 Huang
 * DATE:            04 23 2012
 *
 * SOURCE CONTROL:
 *
 * LICENSE:
 *     This source code is copyright (c) 2012 MediaTek. Inc.
 *     All rights reserved.
 *
 * REVISION     HISTORY:
 *   V1.0.0     04 23 2012    - Initial Version V1.0
 *
 *
 * SOURCE:
 * ISSUES:
 *    First Implementation.
 * NOTES TO USERS:
 *
 ******************************************************************************/
#ifndef TYPES_H
#define TYPES_H

//*****************************************************************************
//
// type
//
//*****************************************************************************

typedef	unsigned char       u8;
typedef	unsigned short      u16;
typedef	unsigned int        u32;
typedef	unsigned long  long     u64;

typedef	 char	    s8;
typedef	 short      s16;
typedef	 int        s32;
typedef	 long  long     s64;

#define FALSE -1
#define TRUE 0
#endif
