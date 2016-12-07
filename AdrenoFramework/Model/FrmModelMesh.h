//-----------------------------------------------------------------------------
// 
//  Mesh
//	
//  Copyright (c) 2011 QUALCOMM Incorporated.
//  All Rights Reserved. QUALCOMM Proprietary/GTDR
//	
//-----------------------------------------------------------------------------

#ifndef ADRENO_MESH_H
#define ADRENO_MESH_H

#include "FrmIndexBuffer.h"
#include "FrmMeshSurfaceArray.h"
#include "FrmVertexBuffer.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct Mesh
    {
        VertexBuffer     Vertices;
        IndexBuffer      Indices;
        MeshSurfaceArray Surfaces;
        INT32            JointIndex;
        
        Mesh();
    };
}

//-----------------------------------------------------------------------------

#endif