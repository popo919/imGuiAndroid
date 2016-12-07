//-----------------------------------------------------------------------------
// 
//  Joint
//	
//  Copyright (c) 2011 QUALCOMM Incorporated.
//  All Rights Reserved. QUALCOMM Proprietary/GTDR
//	
//-----------------------------------------------------------------------------

#ifndef ADRENO_JOINT_H
#define ADRENO_JOINT_H

#include "FrmNamedId.h"
#include "FrmTransform.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct Joint
    {
        static const INT32 NULL_NODE_INDEX = -1;

        Adreno::NamedId   Id;
        INT32             ParentIndex;
        Adreno::Transform Transform;
        Adreno::Transform InverseBindPose;

        Joint();

        void SetName( const char* name );
    };
}

//-----------------------------------------------------------------------------

#endif