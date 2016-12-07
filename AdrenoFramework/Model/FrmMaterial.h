//-----------------------------------------------------------------------------
// 
//  Material
//	
//  Copyright (c) 2011 QUALCOMM Incorporated.
//  All Rights Reserved. QUALCOMM Proprietary/GTDR
//	
//-----------------------------------------------------------------------------

#ifndef ADRENO_MATERIAL_H
#define ADRENO_MATERIAL_H

#include "FrmNamedId.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct Material
    {
        Adreno::NamedId Id;

        Material();

        void SetName( const char* name );
    };
}

//-----------------------------------------------------------------------------

#endif