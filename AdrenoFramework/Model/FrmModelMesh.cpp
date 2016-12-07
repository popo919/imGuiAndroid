//-----------------------------------------------------------------------------
// 
//  Mesh
//	
//  Copyright (c) 2011 QUALCOMM Incorporated.
//  All Rights Reserved. QUALCOMM Proprietary/GTDR
//	
//-----------------------------------------------------------------------------

#include "FrmModelMesh.h"
#include "FrmJoint.h"

//-----------------------------------------------------------------------------

Adreno::Mesh::Mesh()
: Vertices()
, Indices()
, Surfaces()
, JointIndex( Adreno::Joint::NULL_NODE_INDEX )
{

}
