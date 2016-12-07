//-----------------------------------------------------------------------------
// 
//  Joint
//	
//  Copyright (c) 2011 QUALCOMM Incorporated.
//  All Rights Reserved. QUALCOMM Proprietary/GTDR
//	
//-----------------------------------------------------------------------------

#include "FrmJoint.h"
#include "FrmStdLib.h"

//-----------------------------------------------------------------------------

Adreno::Joint::Joint()
: Id()
, ParentIndex( NULL_NODE_INDEX )
, Transform()
, InverseBindPose()
{

}

//-----------------------------------------------------------------------------

void Adreno::Joint::SetName( const char* name )
{
    Id.SetName( name );
}
