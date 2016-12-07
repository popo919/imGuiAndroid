//--------------------------------------------------------------------------------------
// File: FrmPlatform.h
// Desc: 
//
// Author:      QUALCOMM, Advanced Content Group - Snapdragon SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#ifndef _FRM_PLATFORM_H_
#define _FRM_PLATFORM_H_


//--------------------------------------------------------------------------------------
// Platform
//--------------------------------------------------------------------------------------
#ifdef _WIN32
#define _WIN32_WINNT 0x0502
#include <windows.h>
#endif


//--------------------------------------------------------------------------------------
// Common types
//--------------------------------------------------------------------------------------
typedef unsigned char      BYTE;

typedef char               CHAR;
typedef char               CHAR8;
typedef unsigned char      UCHAR;
typedef unsigned char      UCHAR8;

typedef wchar_t            WCHAR;
typedef unsigned short     UCHAR16;

typedef signed char        INT8;
typedef signed short       INT16;
typedef signed int         INT32;
typedef unsigned char      UINT8;
typedef unsigned short     UINT16;
typedef unsigned int       UINT32;

typedef float              FLOAT32;
typedef double             FLOAT64;

typedef int                BOOL;

#ifndef VOID
    #define VOID void
#endif

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

#ifndef NULL
    #define NULL 0
#endif


#endif // _FRM_PLATFORM_H_
