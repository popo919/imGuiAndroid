//-----------------------------------------------------------------------------
// 
//  MeshSurfaceArray
//	
//  Copyright (c) 2011 QUALCOMM Incorporated.
//  All Rights Reserved. QUALCOMM Proprietary/GTDR
//	
//-----------------------------------------------------------------------------

#include "FrmMeshSurfaceArray.h"

//-----------------------------------------------------------------------------

Adreno::MeshSurfaceArray::MeshSurfaceArray()
: NumSurfaces( 0 )
, Surfaces( NULL )
{

}

//-----------------------------------------------------------------------------

Adreno::MeshSurfaceArray::~MeshSurfaceArray()
{
    if( Surfaces )
    {
        delete[] Surfaces;
        Surfaces = NULL;
   }
}

//-----------------------------------------------------------------------------

void Adreno::MeshSurfaceArray::Resize( UINT32 num_surfaces )
{
    if( Surfaces )
    {
        delete[] Surfaces;
        Surfaces = NULL;
   }

    NumSurfaces = num_surfaces;

    if( num_surfaces > 0 )
    {
        Surfaces = new Adreno::MeshSurface[ num_surfaces ];
    }
}
