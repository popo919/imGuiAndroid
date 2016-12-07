//--------------------------------------------------------------------------------------
// File: FrmComputeGraphicsApplication.h
// Desc: Application that uses both OpenCL and OpenGL ES
//
// Author:      QUALCOMM, Advanced Content Group - Snapdragon SDK
//                        
//               Copyright (c) 2013 QUALCOMM Technologies, Inc.
//                         All Rights Reserved.
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#ifndef _FRM_COMPUTE_GRAPHICS_APPLICATION_H_
#define _FRM_COMPUTE_GRAPHICS_APPLICATION_H_

#include "FrmPlatform.h"
#include "FrmApplication.h"
#include <CL/cl.h>


//--------------------------------------------------------------------------------------
// Name: class CFrmComputeGraphicsApplication()
// Desc: OpenCL/OpenGL ES Compute/Graphics Application class
//--------------------------------------------------------------------------------------
class CFrmComputeGraphicsApplication : public CFrmApplication
{
public:

    BOOL RunTests() const { return m_bRunTests; }

    CFrmComputeGraphicsApplication( const CHAR* strName );
    virtual ~CFrmComputeGraphicsApplication();

    BOOL CreateOpenCLContext( EGLNativeWindowType window, cl_device_type deviceType);
    VOID DestroyOpenCLContext();


public:

    // OpenCL platform
    cl_platform_id m_platform;

    // OpenCL context
    cl_context    m_context;

    // OpenCL devices
    cl_device_id *m_devices;

    // Number of devices
    INT32         m_deviceCount;
    
    // Run tests against gold reference to
    // verify output
    BOOL m_bRunTests;    
};

//--------------------------------------------------------------------------------------
// Name: FrmCreateComputeGraphicsApplicationInstance()
// Desc: Global function to create the compute/graphics application instance
//--------------------------------------------------------------------------------------
extern CFrmComputeGraphicsApplication* FrmCreateComputeGraphicsApplicationInstance();


#endif // _FRM_COMPUTE_GRAPHICS_APPLICATION_H_
