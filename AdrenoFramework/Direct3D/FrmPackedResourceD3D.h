//--------------------------------------------------------------------------------------
// File: FrmPackedResourceD3D.h
// Desc: 
//
// Author:                 QUALCOMM, Adreno SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#ifndef _FRM_PACKED_RESOURCE_D3D_H_
#define _FRM_PACKED_RESOURCE_D3D_H_

#include "FrmPackedResource.h"
#include "Direct3D/FrmResourceD3D.h"


//--------------------------------------------------------------------------------------
// Name: class CFrmPackedResourceD3D
// Desc: Loads and extracts D3D resources from a packed resource file
//--------------------------------------------------------------------------------------
class CFrmPackedResourceD3D : public CFrmPackedResource
{
public: 
    CFrmTexture*    GetTexture( const CHAR* strResource );
    CFrmTexture3D*  GetTexture3D( const CHAR* strResource );
    CFrmCubeMap*    GetCubeMap( const CHAR* strResource );
};

#endif // _FRM_PACKED_RESOURCE_GLES_H_
