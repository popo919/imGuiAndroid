//--------------------------------------------------------------------------------------
// File: FrmComputeApplication.cpp
// Desc:
//
// Author:                 QUALCOMM, Advanced Content Group - Adreno SDK
//
//               Copyright (c) 2013-2014 QUALCOMM Technologies, Inc.
//                         All Rights Reserved.
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include "FrmPlatform.h"
#include "FrmComputeApplication.h"
#include "FrmInput.h"
#include "FrmUtils.h"


//--------------------------------------------------------------------------------------
// Name: CFrmComputeApplication()
// Desc: Construction the application base class. The default values can be overrided
//       by the derived class.
//--------------------------------------------------------------------------------------
CFrmComputeApplication::CFrmComputeApplication( const CHAR* strName )
{
    m_strName = strName;
    m_platform = 0;
    m_context = 0;
    m_devices = 0;
    m_deviceCount = 0;
    m_bRunTests = FALSE;
}


//--------------------------------------------------------------------------------------
// Name: ~CFrmComputeApplication()
// Desc: Destructor
//--------------------------------------------------------------------------------------
CFrmComputeApplication::~CFrmComputeApplication()
{
    DestroyOpenCLContext();
}

//--------------------------------------------------------------------------------------
// Name: CreateOpenCLContext()
// Desc: Create the OpenCL context based on the command-line options for creation
//--------------------------------------------------------------------------------------
BOOL CFrmComputeApplication::CreateOpenCLContext(cl_device_type deviceType)
{
    cl_int errNum;
    cl_uint numPlatforms = 0;
    cl_platform_id platformId;

    // Get the first platform ID
    errNum = clGetPlatformIDs( 1, &platformId, &numPlatforms );
    if (errNum != CL_SUCCESS || numPlatforms <= 0)
    {
        FrmLogMessage("No OpenCL platforms found.");
        return FALSE;
    }

    m_platform = platformId;

    // Get the number of devices for the requested device type (CPU, GPU, ALL)
    cl_uint numDevices = 0;
    errNum = clGetDeviceIDs( platformId, deviceType, 0, NULL, &numDevices );
    if (errNum != CL_SUCCESS || numDevices <= 0)
    {
        FrmLogMessage("No matching OpenCL devices found.");
        return FALSE;
    }

    char platformInfo[1024];
    char logMessage[2048];
    errNum = clGetPlatformInfo( platformId, CL_PLATFORM_VENDOR, sizeof(platformInfo), platformInfo, NULL );
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
    errNum = clGetDeviceIDs( platformId, deviceType, numDevices, m_devices, NULL );
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
        (cl_context_properties)platformId,
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
VOID CFrmComputeApplication::DestroyOpenCLContext()
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

//--------------------------------------------------------------------------------------
// Name: Select2DWGSize()
// Desc: Select 2D local workgroup size based on user input.
//       Final selected dimension is AxB where minWorkGroupSize <= AxB <= maxWorkGroupSize
//.......The dimension of A and B is based on m_WG2DWHRatio where A/B = m_WG2DWHRatio / (100-m_WG2DWHRatio)
//--------------------------------------------------------------------------------------
void CFrmComputeApplication::Select2DWGSize(size_t maxWorkGroupSize, size_t defGlobalWG_X, size_t defGlobalWG_Y, size_t minWorkGroupSize)
{

    char msg[512];

    m_globalWorkSize[0] = defGlobalWG_X;
    m_globalWorkSize[1] = defGlobalWG_Y;

    // m_WGSize is the percentage of maxWorkGroupSize
    float selectedWgSize = (float)m_WGSize / 100.0f * (float)maxWorkGroupSize;
    if ((int)selectedWgSize < minWorkGroupSize) selectedWgSize = (float)minWorkGroupSize;

    FrmSprintf( msg, sizeof(msg), "Max WG Size: %d. Selected WG Size: %d\n", maxWorkGroupSize, (int)selectedWgSize);
    FrmLogMessage( msg );
    FrmSprintf( msg, sizeof(msg), "Global WG Size: %dx%d\n", m_globalWorkSize[0], m_globalWorkSize[1]);
    FrmLogMessage( msg );

    // if ratio = 0, leave local size calculation to the driver
    if (m_WG2DWHRatio == 0) return;

    // Ratio is out of scale 100
    // Adjust local dimension based on width:height ratio
    float whRatio = (float)m_WG2DWHRatio / (float)(100-m_WG2DWHRatio);
    m_localWorkSize[0] = (size_t) FrmSqrt( (FLOAT32) selectedWgSize * whRatio);

    // make sure the size is at lease 1
    if (m_localWorkSize[0] < 1) m_localWorkSize[0] = 1;


    // 50 means 1:1
    if (m_WG2DWHRatio == 50)
    {
        m_localWorkSize[1] = m_localWorkSize[0];
    }
    else
    {
        m_localWorkSize[1] = (size_t) selectedWgSize / m_localWorkSize[0];
        if (m_localWorkSize[1] < 1) m_localWorkSize[1] = 1;
    }

    // Compute the next global size that is a multiple of the local size
    size_t remndr = defGlobalWG_X % m_localWorkSize[0];
    if( remndr )
    {
        m_globalWorkSize[0] = defGlobalWG_X + m_localWorkSize[0] - remndr;
    }

    remndr = defGlobalWG_Y % m_localWorkSize[1];
    if( remndr )
    {
        m_globalWorkSize[1] = defGlobalWG_Y + m_localWorkSize[1] - remndr;
    }
    for (int i = 0; i < 2; i ++)
    {
        remndr = (m_globalWorkSize[i] % m_localWorkSize[i])? 1:0;
        m_numWorkgroup[i] = m_globalWorkSize[i] / m_localWorkSize[i] + remndr;
    }

    FrmSprintf( msg, sizeof(msg), "Local WG Size: %dx%d\n", m_localWorkSize[0], m_localWorkSize[1]);
    FrmLogMessage( msg );
    FrmSprintf( msg, sizeof(msg), "Num of local WG: (%d x %d)\n", m_numWorkgroup[0], m_numWorkgroup[1] );
    FrmLogMessage( msg );

}

