//--------------------------------------------------------------------------------------
// File: FrmComputeApplication.h
// Desc:
//
// Author:      QUALCOMM, Advanced Content Group - Snapdragon SDK
//
//               Copyright (c) 2013-2014 QUALCOMM Technologies, Inc.
//                         All Rights Reserved.
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#ifndef _FRM_COMPUTE_APPLICATION_H_
#define _FRM_COMPUTE_APPLICATION_H_

#include "FrmPlatform.h"
#include <CL/cl.h>


//--------------------------------------------------------------------------------------
// Name: class CFrmApplication()
// Desc: OpenCL Compute Application class
//--------------------------------------------------------------------------------------
class CFrmComputeApplication
{
public:
    virtual BOOL Initialize() = 0;
    virtual VOID Destroy() = 0;
    virtual BOOL Compute() = 0;

    BOOL RunTests() const { return m_bRunTests; }

    CFrmComputeApplication( const CHAR* strName );
    virtual ~CFrmComputeApplication();

    BOOL CreateOpenCLContext(cl_device_type deviceType);
    VOID DestroyOpenCLContext();


protected:
    void Select2DWGSize(size_t maxWorkGroupSize, size_t defGlobalWG_X, size_t defGlobalWG_Y, size_t minWorkGroupSize=1);

public:

    // OpenCL platform
    cl_platform_id m_platform;

    // OpenCL context
    cl_context    m_context;

    // OpenCL devices
    cl_device_id *m_devices;

    // Number of devices
    INT32         m_deviceCount;

    // Application name
    const CHAR* m_strName;

    // Run tests against gold reference to
    // verify output
    BOOL m_bRunTests;

    int m_WGSize; // a percentage value of the max WG size

    // A value in 0...100 range.
    // It represents the width:height ratio of 2D local workgroup.
    // m_WG2DWHRatio:(100-m_WG2DWHRatio) = width:height
    // 0 indicates the driver should select the size
    int m_WG2DWHRatio;

    bool m_useLocal; // indicate to use local memory if possible

protected:
    size_t m_globalWorkSize[2];
    size_t m_localWorkSize[2];
    size_t m_numWorkgroup[2];


};

//--------------------------------------------------------------------------------------
// Name: FrmCreateComputeApplicationInstance()
// Desc: Global function to create the compute application instance
//--------------------------------------------------------------------------------------
extern CFrmComputeApplication* FrmCreateComputeApplicationInstance();


#endif // _FRM_COMPUTE_APPLICATION_H_
