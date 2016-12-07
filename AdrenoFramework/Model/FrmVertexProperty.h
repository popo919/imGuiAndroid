//-----------------------------------------------------------------------------
// 
//  VertexProperty
//	
//  Copyright (c) 2011 QUALCOMM Incorporated.
//  All Rights Reserved. QUALCOMM Proprietary/GTDR
//	
//-----------------------------------------------------------------------------

#ifndef ADRENO_VERTEX_PROPERTY_H
#define ADRENO_VERTEX_PROPERTY_H

#include "FrmNamedId.h"
#include "FrmPlatform.h"
#include "FrmVertexPropertyType.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct VertexProperty
    {
        NamedId            Usage;
        VertexPropertyType Type;
        UINT32             Offset;
        
        VertexProperty();

        UINT32  Size         () const;
        UINT32  NumValues    () const;
        bool    IsNormalized () const;
    };
}

//-----------------------------------------------------------------------------

#endif