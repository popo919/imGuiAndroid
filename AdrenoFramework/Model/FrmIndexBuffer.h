//-----------------------------------------------------------------------------
// 
//  IndexBuffer
//	
//  Copyright (c) 2011 QUALCOMM Incorporated.
//  All Rights Reserved. QUALCOMM Proprietary/GTDR
//	
//-----------------------------------------------------------------------------

#ifndef ADRENO_INDEX_BUFFER_H
#define ADRENO_INDEX_BUFFER_H

#include "FrmPlatform.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct IndexBuffer
    {
        UINT32  NumIndices;
        UINT32* Indices;

         IndexBuffer();
        ~IndexBuffer();

        void Resize( UINT32 num_indices );
    };
}

//-----------------------------------------------------------------------------

#endif