//-----------------------------------------------------------------------------
// 
//  VertexBuffer
//	
//  Copyright (c) 2011 QUALCOMM Incorporated.
//  All Rights Reserved. QUALCOMM Proprietary/GTDR
//	
//-----------------------------------------------------------------------------

#include "FrmVertexBuffer.h"

//-----------------------------------------------------------------------------

Adreno::VertexBuffer::VertexBuffer()
: Format()
, NumVerts( 0 )
, BufferSize( 0 )
, Buffer( NULL )
{

}

//-----------------------------------------------------------------------------

Adreno::VertexBuffer::~VertexBuffer()
{
    if( Buffer )
    {
        delete[] Buffer;
        Buffer = NULL;
    }
}

//-----------------------------------------------------------------------------

void Adreno::VertexBuffer::IntializeBuffer( UINT32 num_verts )
{
    if( Buffer )
    {
        delete[] Buffer;
        Buffer = NULL;
    }

    NumVerts = num_verts;
    
    INT32 vertex_stride = Format.Stride;
    BufferSize          = num_verts * vertex_stride;

    if( BufferSize > 0 )
    {
        Buffer = new UINT8[ BufferSize ];
    }
}
