//-----------------------------------------------------------------------------
// 
//  VertexBuffer
//	
//  Copyright (c) 2011 QUALCOMM Incorporated.
//  All Rights Reserved. QUALCOMM Proprietary/GTDR
//	
//-----------------------------------------------------------------------------

#ifndef ADRENO_VERTEX_BUFFER_H
#define ADRENO_VERTEX_BUFFER_H

#include "FrmPlatform.h"
#include "FrmVertexFormat.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct VertexBuffer
    {
        VertexFormat Format;
        UINT32       NumVerts;
        UINT32       BufferSize;
        UINT8*       Buffer;
        
         VertexBuffer();
        ~VertexBuffer();

        void IntializeBuffer( UINT32 num_verts );
    };
}

//-----------------------------------------------------------------------------

#endif