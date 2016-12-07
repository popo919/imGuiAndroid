//-----------------------------------------------------------------------------
// 
//  MeshSurfaceArray
//	
//  Copyright (c) 2011 QUALCOMM Incorporated.
//  All Rights Reserved. QUALCOMM Proprietary/GTDR
//	
//-----------------------------------------------------------------------------

#ifndef ADRENO_MESH_SURFACE_ARRAY_H
#define ADRENO_MESH_SURFACE_ARRAY_H

#include "FrmPlatform.h"
#include "FrmMeshSurface.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct MeshSurfaceArray
    {
        UINT32       NumSurfaces;
        MeshSurface* Surfaces;

         MeshSurfaceArray();
        ~MeshSurfaceArray();

        void Resize( UINT32 num_surfaces );
    };
}

//-----------------------------------------------------------------------------

#endif