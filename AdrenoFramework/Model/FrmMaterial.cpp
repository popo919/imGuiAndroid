//-----------------------------------------------------------------------------
// 
//  Material
//	
//  Copyright (c) 2011 QUALCOMM Incorporated.
//  All Rights Reserved. QUALCOMM Proprietary/GTDR
//	
//-----------------------------------------------------------------------------

#include "FrmMaterial.h"

//-----------------------------------------------------------------------------

Adreno::Material::Material()
: Id()
{

}

//-----------------------------------------------------------------------------

void Adreno::Material::SetName( const char* name )
{
    Id.SetName( name );
}
