//--------------------------------------------------------------------------------------
// File: FrmComputeApplication_Win32.cpp
// Desc: Win32 implementation of the Compute Framework application
//
// Author:      QUALCOMM, Advanced Content Group - Snapdragon SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include "FrmPlatform.h"
#include "FrmComputeApplication.h"
#include "FrmUtils.h"
#include <stdio.h>
#include <crtdbg.h>
#include <conio.h>

//--------------------------------------------------------------------------------------
// Name: class CFrmAppContainer()
// Desc: Application class container
//--------------------------------------------------------------------------------------
class CFrmAppContainer
{
public:
    CFrmComputeApplication*  m_pApplication;
    
public:
    CFrmAppContainer();
    ~CFrmAppContainer();

    BOOL Run(cl_device_type deviceType);
};


//--------------------------------------------------------------------------------------
// Name: WinMain()
// Desc: Application entry-point on the Windows platform
//--------------------------------------------------------------------------------------
INT32 main(INT32 argc, CHAR* argv[])
{
    // TODO: parse comand-line options for CPU vs. GPU devices
    cl_device_type deviceType = CL_DEVICE_TYPE_ALL;
	CFrmAppContainer appContainer;
    return (INT32)appContainer.Run(deviceType);
}

//--------------------------------------------------------------------------------------
// Name: Run()
// Desc: Create the framework, initialize the application, and render frames.
//--------------------------------------------------------------------------------------
BOOL CFrmAppContainer::Run(cl_device_type deviceType)
{
    // Use the executable's directory so that relative media paths work correctly
    {
        CHAR strFilePath[256];
        GetModuleFileName( NULL, strFilePath, 256 );
        *( strrchr( strFilePath, '\\' ) ) = '\0';
        SetCurrentDirectory( strFilePath );
    }

 	// Create the Application
	m_pApplication = FrmCreateComputeApplicationInstance();

    m_pApplication->m_bRunTests = TRUE;

	if( NULL == m_pApplication )
        return FALSE;

    if( FALSE == m_pApplication->CreateOpenCLContext( deviceType ) )
    {
        return FALSE;
    }

    if( FALSE == m_pApplication->Initialize() )
    {
        printf( "Application failed initialization!\nPlease debug accordingly.");
        return FALSE;
    }

    // Run the computation
    BOOL result = m_pApplication->Compute();

    if ( m_pApplication->RunTests() )
    {
        char msg[256];
		FrmSprintf( msg, sizeof(msg), "RunTests: %s\n", result ? "PASSED" : "FAILED" );
		FrmLogMessage( msg );
    }

    // Display the message log
    const CHAR* strErrorMessage = FrmGetMessageLog();
    printf("%s", strErrorMessage);

	printf("Waiting for a character to be pressed from the keyboard to exit.\n");
	getch();
	
    return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: CFrmAppContainer()
// Desc: Constructor
//--------------------------------------------------------------------------------------
CFrmAppContainer::CFrmAppContainer()
{
    m_pApplication   = NULL;
}


//--------------------------------------------------------------------------------------
// Name: ~CFrmAppContainer()
// Desc: Destroys the application's window, etc.
//--------------------------------------------------------------------------------------
CFrmAppContainer::~CFrmAppContainer()
{
    if( m_pApplication )
    {
        m_pApplication->Destroy();
        delete m_pApplication;
    }
}

