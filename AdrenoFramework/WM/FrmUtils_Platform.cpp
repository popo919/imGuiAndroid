//--------------------------------------------------------------------------------------
// File: FrmUtils_Win32.cpp
// Desc: Win32 implementation of the Framework utility helper functions
//
// Author:      QUALCOMM, Advanced Content Group - Snapdragon SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include "FrmPlatform.h"
#include "FrmUtils.h"

#ifndef UNDER_CE
#include <assert.h>
#include <string>

std::string g_strMessageLog;
#endif // UNDER_CE


//--------------------------------------------------------------------------------------
// Name: FrmAssert()
// Desc: Platform-dependent assert function.
//--------------------------------------------------------------------------------------
VOID FrmAssert( INT32 nExpression )
{
#ifndef UNDER_CE
    assert( nExpression );
#endif // UNDER_CE
}


//--------------------------------------------------------------------------------------
// Name: FrmLogMessage()
// Desc: Platform-dependent debug spew functions
//--------------------------------------------------------------------------------------
VOID FrmLogMessage( const CHAR* strPrefix, const CHAR* strMessage,
                    const CHAR* strPostFix )
{
#ifndef UNDER_CE
    if( strPrefix )  OutputDebugString( strPrefix );
    if( strMessage ) OutputDebugString( strMessage );
    if( strPostFix ) OutputDebugString( strPostFix );

    if( strPrefix )  g_strMessageLog.append( strPrefix );
    if( strMessage ) g_strMessageLog.append( strMessage );
    if( strPostFix ) g_strMessageLog.append( strPostFix );
#else
	int iLength = 0;
	WCHAR *pwTemp = NULL;

	if(strPrefix != NULL)
	{
		iLength = strlen(strPrefix) + 1;
		pwTemp = (WCHAR *)malloc(iLength);
	    mbstowcs(pwTemp, (CHAR *)strPrefix, iLength);
		OutputDebugString(pwTemp);
		free(pwTemp);
	}

	if(strMessage != NULL)
	{
		iLength = strlen(strMessage) + 1;
		pwTemp = (WCHAR *)malloc(iLength);
	    mbstowcs(pwTemp, (CHAR *)strPrefix, iLength);
		OutputDebugString(pwTemp);
		free(pwTemp);
	}

	if(strPostFix != NULL)
	{
		iLength = strlen(strPostFix) + 1;
		pwTemp = (WCHAR *)malloc(iLength);
	    mbstowcs(pwTemp, (CHAR *)strPrefix, iLength);
		OutputDebugString(pwTemp);
		free(pwTemp);
	}
#endif // UNDER_CE
}


//--------------------------------------------------------------------------------------
// Name: FrmLogMessage()
// Desc: Platform-dependent debug spew functions
//--------------------------------------------------------------------------------------
const CHAR* FrmGetMessageLog()
{
#ifndef UNDER_CE
    return g_strMessageLog.c_str();
#else
	return NULL;
#endif // UNDER_CE
}


//--------------------------------------------------------------------------------------
// Name: FrmGetTime()
// Desc: Platform-dependent function to get the current time (in seconds).
//--------------------------------------------------------------------------------------
FLOAT32 FrmGetTime()
{
    static BOOL     bInitialized = FALSE;
    static LONGLONG m_llQPFTicksPerSec;
    static LONGLONG m_llBaseTime;
    if( FALSE == bInitialized )
    {
        LARGE_INTEGER qwTicksPerSec;
        QueryPerformanceFrequency( &qwTicksPerSec );
        m_llQPFTicksPerSec = qwTicksPerSec.QuadPart;

        LARGE_INTEGER qwTime;
        QueryPerformanceCounter( &qwTime );
        m_llBaseTime = qwTime.QuadPart;

        bInitialized = TRUE;
        return 0.0f;
    }

    // Get the current time
    LARGE_INTEGER qwTime;
    QueryPerformanceCounter( &qwTime );
    FLOAT64 fAppTime = (FLOAT64)( qwTime.QuadPart - m_llBaseTime ) / (FLOAT64) m_llQPFTicksPerSec;
    return (FLOAT32)fAppTime;
}

