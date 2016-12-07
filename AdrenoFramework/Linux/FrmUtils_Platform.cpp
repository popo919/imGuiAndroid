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
#include <stdio.h>
#include <assert.h>
#include <string>

#if LINUX
#include <time.h>
#elif OSX
#include <mach/clock.h>
#include <mach/mach.h>
#endif


std::string g_strMessageLog;


//--------------------------------------------------------------------------------------
// Name: FrmAssert()
// Desc: Platform-dependent assert function.
//--------------------------------------------------------------------------------------
VOID FrmAssert( INT32 nExpression )
{
    assert( nExpression );
}


//--------------------------------------------------------------------------------------
// Name: FrmLogMessage()
// Desc: Platform-dependent debug spew functions
//--------------------------------------------------------------------------------------
VOID FrmLogMessage( const CHAR* strPrefix, const CHAR* strMessage,
                    const CHAR* strPostFix )
{
    if( strPrefix )  printf("%s", strPrefix);
    if( strMessage ) printf("%s", strMessage);
    if( strPostFix ) printf("%s", strPostFix);

    if( strPrefix )  g_strMessageLog.append( strPrefix );
    if( strMessage ) g_strMessageLog.append( strMessage );
    if( strPostFix ) g_strMessageLog.append( strPostFix );
}


//--------------------------------------------------------------------------------------
// Name: FrmLogMessage()
// Desc: Platform-dependent debug spew functions
//--------------------------------------------------------------------------------------
const CHAR* FrmGetMessageLog()
{
    return g_strMessageLog.c_str();
}


//--------------------------------------------------------------------------------------
// Name: FrmGetTime()
// Desc: Platform-dependent function to get the current time (in seconds).
//--------------------------------------------------------------------------------------
FLOAT32 FrmGetTime()
{
    static BOOL     bInitialized = FALSE;
    static struct timespec startTime;

    if (FALSE == bInitialized)
    {
	// these need #include time.h and librt (-lrt)
	#if LINUX
    	clock_gettime(CLOCK_REALTIME, &startTime);
	#elif OSX
		clock_serv_t cclock;
		mach_timespec_t mts;
		host_get_clock_service(mach_host_self(),CALENDAR_CLOCK,&cclock);
		clock_get_time(cclock,&mts);
		mach_port_deallocate(mach_task_self(),cclock);
		startTime.tv_sec = mts.tv_sec;
		startTime.tv_nsec = mts.tv_nsec;
	#endif

        bInitialized = TRUE;
        return 0.0f;
    }

	static struct timespec nowTime;    	
    // Get the current time
    #if LINUX
    	clock_gettime(CLOCK_REALTIME, &nowTime);
	#elif OSX
		clock_serv_t cclock;
		mach_timespec_t mts;
		host_get_clock_service(mach_host_self(),CALENDAR_CLOCK,&cclock);
		clock_get_time(cclock,&mts);
		mach_port_deallocate(mach_task_self(),cclock);
		nowTime.tv_sec = mts.tv_sec;
		nowTime.tv_nsec = mts.tv_nsec;
	#endif
	


    FLOAT64 fAppTime = (FLOAT64)(nowTime.tv_sec - startTime.tv_sec) + (FLOAT64)(nowTime.tv_nsec - startTime.tv_nsec) / 1.0e9f;
    return (FLOAT32)fAppTime;
}

//--------------------------------------------------------------------------------------
// Name: FrmOpenConsole()
// Desc: Platform-dependent function to open a console
//--------------------------------------------------------------------------------------
int FrmOpenConsole()
{
	return 1;
}