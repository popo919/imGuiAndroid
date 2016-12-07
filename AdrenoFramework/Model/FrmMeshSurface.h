//-----------------------------------------------------------------------------
// 
//  MeshSurface
//	
//  Copyright (c) 2011 QUALCOMM Incorporated.
//  All Rights Reserved. QUALCOMM Proprietary/GTDR
//	
//-----------------------------------------------------------------------------

#ifndef ADRENO_MESH_SURFACE_H
#define ADRENO_MESH_SURFACE_H

#include "FrmPlatform.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct MeshSurface
    {
        UINT32 StartIndex;
        UINT32 NumTriangles;
        UINT32 MaterialId;
    };
}

//-----------------------------------------------------------------------------

#endif