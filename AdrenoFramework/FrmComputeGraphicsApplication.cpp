//--------------------------------------------------------------------------------------
// File: FrmComputeGraphicsApplication.cpp
// Desc: Application that uses both OpenCL and OpenGL ES
//
// Author:                 QUALCOMM, Advanced Content Group - Adreno SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc.
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include "FrmPlatform.h"
#include <CL/cl_gl.h>
#include "FrmComputeGraphicsApplication.h"
#include "FrmInput.h"
#include "FrmUtils.h"

//--------------------------------------------------------------------------------------
// Name: CFrmComputeGraphicsApplication()
// Desc: Construction the application base class. The default values can be overrided
//       by the derived class.
//--------------------------------------------------------------------------------------
CFrmComputeGraphicsApplication::CFrmComputeGraphicsApplication( const CHAR* strName ) :
    CFrmApplication( strName )
{ 
    m_platform = 0;
    m_context = 0;
    m_devices = 0;
    m_deviceCount = 0;
    m_bRunTests = FALSE;
}


//--------------------------------------------------------------------------------------
// Name: ~CFrmComputeGraphicsApplication()
// Desc: Destructor
//--------------------------------------------------------------------------------------
CFrmComputeGraphicsApplication::~CFrmComputeGraphicsApplication()
{
    DestroyOpenCLContext();
}

//--------------------------------------------------------------------------------------
// Name: CreateOpenCLContext()
// Desc: Create the OpenCL context based on the command-line options for creation
//--------------------------------------------------------------------------------------
BOOL CFrmComputeGraphicsApplication::CreateOpenCLContext( EGLNativeWindowType window, cl_device_type deviceType )
{
    cl_int errNum;
    cl_uint numPlatforms = 0;
    cl_platform_id platformId[2];

    // Get the first platform ID
    errNum = clGetPlatformIDs( 2, platformId, &numPlatforms );
    if (errNum != CL_SUCCESS || numPlatforms <= 0)
    {
        FrmLogMessage("No OpenCL platforms found.");
        return FALSE;
    }

    m_platform = platformId[0];

	// Get the number of devices for the requested device type (CPU, GPU, ALL)
    cl_uint numDevices = 0;
    errNum = clGetDeviceIDs( m_platform, deviceType, 0, NULL, &numDevices );
    if (errNum != CL_SUCCESS || numDevices <= 0)
    {
        FrmLogMessage("No matching OpenCL devices found.");
        return FALSE;
    }

    char platformInfo[1024];
    char logMessage[2048];
    errNum = clGetPlatformInfo( m_platform, CL_PLATFORM_VENDOR, sizeof(platformInfo), platformInfo, NULL );
    if (errNum != CL_SUCCESS)
    {
        FrmLogMessage("ERROR: getting platform info."); 
        return FALSE;
    }
    FrmSprintf( logMessage, sizeof(logMessage), "OpenCL Platform: %s\n", platformInfo );
    FrmLogMessage( logMessage );

    // Get the devices
    m_devices = new cl_device_id[numDevices];
    m_deviceCount = numDevices; 
    errNum = clGetDeviceIDs( m_platform, deviceType, numDevices, m_devices, NULL );
    

    if (errNum != CL_SUCCESS)
    {
        FrmLogMessage("Erorr getting OpenCL device(s).");
        return FALSE;
    }    

    switch (deviceType)
    {
    case CL_DEVICE_TYPE_GPU:
        FrmLogMessage("Selected device: GPU\n");
        break;
    case CL_DEVICE_TYPE_CPU:
        FrmLogMessage("Selected device: CPU\n");
        break;
    case CL_DEVICE_TYPE_DEFAULT:
    default:
        FrmLogMessage("Selected device: DEFAULT\n");
        break;   
    }

    for (int i = 0; i < m_deviceCount; i++)
    {
        char deviceInfo[1024];
        errNum = clGetDeviceInfo( m_devices[i], CL_DEVICE_NAME, sizeof(deviceInfo), deviceInfo, NULL );
        if (errNum == CL_SUCCESS )
        {
            FrmSprintf( logMessage, sizeof(logMessage), "OpenCL Device Name (%d) : %s\n", i , deviceInfo );
            FrmLogMessage( logMessage );
        }
    }

    // Finally, create the context
    cl_context_properties contextProperties[] =
    {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)m_platform,
        CL_GL_CONTEXT_KHR,
        (cl_context_properties)m_eglContextGL,
#ifdef ANDROID_CL // For Android using the EGL Display
        CL_EGL_DISPLAY_KHR,
        (cl_context_properties)m_eglDisplay,
#else // For Windows, use the HDC
        CL_WGL_HDC_KHR,
        (cl_context_properties)window,
#endif
        0
    };

    m_context = clCreateContext( contextProperties, m_deviceCount, m_devices, NULL, NULL, &errNum );
    if (errNum != CL_SUCCESS)
    {
        FrmLogMessage("Could not create OpenCL context.");
        return FALSE;
    }

    return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: DestroyOpenCLContext()
// Desc: Destroy the OpenCL context
//--------------------------------------------------------------------------------------
VOID CFrmComputeGraphicsApplication::DestroyOpenCLContext()
{
    if ( m_context != 0 )
    {
        clReleaseContext( m_context );
        m_context = 0;
    }

    if ( m_devices )
    {
        delete [] m_devices;
        m_devices = NULL;
    }
}
