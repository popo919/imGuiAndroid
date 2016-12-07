//--------------------------------------------------------------------------------------
// File: FrmPlatform.h
// Desc: 
//
// Author:                 QUALCOMM, Adreno SDK
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
#include <wrl.h>
#include <d3d11_1.h>

//--------------------------------------------------------------------------------------
// Global Reference to D3DDevice Pointer
//--------------------------------------------------------------------------------------
Microsoft::WRL::ComPtr<ID3D11Device> D3DDevice();
Microsoft::WRL::ComPtr<ID3D11DeviceContext1> D3DDeviceContext();

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
