//--------------------------------------------------------------------------------------
// File: FrmComputeGraphicsApplication_Win32.cpp
// Desc: Win32 implementation of the Compute/Graphics Framework application
//
// Author:      QUALCOMM, Advanced Content Group - Snapdragon SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include "FrmPlatform.h"
#include "FrmComputeGraphicsApplication.h"
#include "FrmUtils.h"

#include <X11/Xlib.h>

#include <stdio.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>


//--------------------------------------------------------------------------------------
// Name: class CFrmAppContainer()
// Desc: Application class container
//--------------------------------------------------------------------------------------
class CFrmAppContainer
{
public:
    NativeWindowType  m_hWindow;
    NativeDisplayType m_hDisplay;
    CFrmComputeGraphicsApplication*  m_pApplication;

    BOOL CreateNativeWindow( NativeWindowType* pWindow, NativeDisplayType* pDisplay );
    
public:
    CFrmAppContainer();
    ~CFrmAppContainer();

    BOOL Run( cl_device_type devicetype );
    BOOL HandleEvents();
    VOID ToggleOrientation();
};


//--------------------------------------------------------------------------------------
// Name: main()
// Desc: Application entry-point on the Linux platform
//--------------------------------------------------------------------------------------
INT32 main(INT32 argc, CHAR* argv[])
{
    cl_device_type deviceType = CL_DEVICE_TYPE_ALL;
    CFrmAppContainer appContainer;
    return (INT32)appContainer.Run(deviceType);
}


//--------------------------------------------------------------------------------------
// Name: Run()
// Desc: Create the framework, initialize the application, and render frames.
//--------------------------------------------------------------------------------------
BOOL CFrmAppContainer::Run( cl_device_type deviceType )
{
    // Use the executable's directory so that relative media paths work correctly
    {
	char processDir[PATH_MAX];
	char currentDir[PATH_MAX];

	struct stat dirInfo;
	pid_t pid = getpid();

	sprintf(processDir, "/proc/%d/exe", (int)pid);
	if (readlink(processDir, currentDir, PATH_MAX) != -1)
	{
	    strcpy(currentDir, dirname(currentDir));
            chdir(currentDir);
	}
        else
        {
            printf("Error finding executable directory.\n");
            return FALSE;
        }
    }

    // Create the Application
    m_pApplication = FrmCreateComputeGraphicsApplicationInstance();

    if( NULL == m_pApplication )
        return FALSE;

    m_pApplication->m_bRunTests = TRUE;

    // Create the application window
    if( FALSE == CreateNativeWindow( &m_hWindow, &m_hDisplay ) )
        return FALSE;

    // Create the render context
    if (FALSE == m_pApplication->CreateRenderContext(m_hWindow, m_hDisplay))
    {
        // Display the message log, which might indicate what failed
        const CHAR* strErrorMessage = FrmGetMessageLog();
        if (strErrorMessage && strErrorMessage[0])
        {
            printf("Error %s: %s\n", m_pApplication->m_strName, strErrorMessage);
        }
        else
        {
            printf("Error %s: Application failed to create render context!\n", m_pApplication->m_strName);
        }

        return FALSE;
    }

    eglBindAPI(EGL_OPENGL_ES_API);
    eglMakeCurrent(m_pApplication->m_eglDisplay, m_pApplication->m_eglSurface, m_pApplication->m_eglSurface, m_pApplication->m_eglContextGL);

    if( FALSE == m_pApplication->CreateOpenCLContext( m_hWindow, deviceType ) )
    {
        return FALSE;
    }

    if ((FALSE == m_pApplication->Initialize()) || (FALSE == m_pApplication->Resize()))
    {
        return FALSE;
    }
    
    // Run the main loop
    while (1)
    {
	// event handling goes here
	XNextEvent(display, &xEvent);
	if (xEvent.type == Expose)
	{
	}

        // Update and render the application
        m_pApplication->Update();
        m_pApplication->Render();

        // Present the scene
        m_pApplication->SwapDrawBuffers();
    }

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: ToggleOrientation()
// Desc: Toggle the window orientation between portrait and landscape mode
//--------------------------------------------------------------------------------------
VOID CFrmAppContainer::ToggleOrientation()
{
    // Switch orientation by swapping the width and height
    INT32 nOldWidth  = m_pApplication->m_nWidth;
    INT32 nOldHeight = m_pApplication->m_nHeight;
    INT32 nNewWidth  = nOldHeight;
    INT32 nNewHeight = nOldWidth;

   

    // Update the app variables
    m_pApplication->m_nWidth  = nNewWidth;
    m_pApplication->m_nHeight = nNewHeight;

    // Update the OpenGL viewport
    // Note: The derived app class should do this during the Render() function!
    m_pApplication->Resize();
}


//--------------------------------------------------------------------------------------
// Name: HandleEvents()
// Desc: Pumps the application window's message queue
//--------------------------------------------------------------------------------------
BOOL CFrmAppContainer::HandleEvents()
{
    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: CreateNativeWindow()
// Desc: Creates a window for the application
//--------------------------------------------------------------------------------------
BOOL CFrmAppContainer::CreateNativeWindow( NativeWindowType* pWindow,
                                           NativeDisplayType* pDisplay )
{
    Display *display = XOpenDisplay(NULL);
    
    if (display == NULL)
    {
        FrmLogMessage("ERROR: Unable to open a connection to the X server.\n");
        return FALSE;
    }

    int screen = DefaultScreen(display);
    int depth = DefaultDepth(display, screen);
    Visual *visual = DefaultVisual(display, screen);

    XSetWindowAttributes attributes;
    attributes.border_pixel = 0;
    attributes.event_mask = StructureNotifyMask;
    attributes.colormap = XCreateColormap(display, XRootWindow(display, screen), visual, AllocNone);

    Window window = XCreateWindow(display, XRootWindow(display, screen), 50, 50,
				  m_pApplication->m_nWidth, m_pApplication->m_nHeight,
				  0, depth, InputOutput, visual, 
				  CWColormap | CWBorderPixel | CWEventMask,
				  &attributes);
    XMapWindow(display, window);

    // Note: We delay showing the window until after Initialization() succeeds
    // Otherwise, an unsightly, empty window briefly appears during initialization

    // Return
    (*pWindow)  = (NativeWindowType)window;
    (*pDisplay) = (NativeDisplayType)display;
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

    XDestroyWindow(m_hDisplay, m_hWindow);
}

