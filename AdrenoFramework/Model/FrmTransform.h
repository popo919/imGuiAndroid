//-----------------------------------------------------------------------------
// 
//  Transform
//	
//  Copyright (c) 2011 QUALCOMM Incorporated.
//  All Rights Reserved. QUALCOMM Proprietary/GTDR
//	
//-----------------------------------------------------------------------------

#ifndef ADRENO_TRANSFORM_H
#define ADRENO_TRANSFORM_H

#include "FrmMath.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct Transform
    {
        FRMVECTOR3 Position;
        FRMVECTOR4 Rotation;
        FRMVECTOR3 Scale;
        
        Transform()
        : Position( 0.0f, 0.0f, 0.0f )
        , Rotation( 0.0f, 0.0f, 0.0f, 1.0f )
        , Scale( 1.0f, 1.0f, 1.0f )
        {
        
        }
    };
}

//-----------------------------------------------------------------------------

#endif