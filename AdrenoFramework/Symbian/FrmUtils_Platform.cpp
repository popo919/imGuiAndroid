//--------------------------------------------------------------------------------------
// File: FrmUtils_Symbian.cpp
// Desc: Win32 implementation of the Framework utility helper functions
//
// Author:      QUALCOMM, Advanced Content Group - Adreno SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include <e32def.h>
#include <ngi/runtime.h>
#include "FrmBasicTypes.h"
#include "FrmUtils.h"


//--------------------------------------------------------------------------------------
// Name: FrmAssert()
// Desc: Platform-dependent assert function.
//--------------------------------------------------------------------------------------
VOID FrmAssert( INT32 nExpression )
{
    if( !nExpression )
    {
        __BREAKPOINT();
    }
}


//--------------------------------------------------------------------------------------
// Name: FrmGetTime()
// Desc: Platform-dependent function to get the current time (in seconds).
//--------------------------------------------------------------------------------------
FLOAT32 FrmGetTime()
{
    ngi::uint64 nTime = ngi::CRuntime::GetElapsedTime();
    return nTime * 0.001f;
}

