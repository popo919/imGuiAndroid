//-----------------------------------------------------------------------------
// 
//  NamedId
//	
//  Copyright (c) 2011 QUALCOMM Incorporated.
//  All Rights Reserved. QUALCOMM Proprietary/GTDR
//	
//-----------------------------------------------------------------------------

#ifndef ADRENO_VERTEX_SEMANTIC_H
#define ADRENO_VERTEX_SEMANTIC_H

#include "FrmPlatform.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct NamedId
    {
        static const int MAX_NAME_LENGTH = 32;

        char  Name[ MAX_NAME_LENGTH ];
        INT32 Id;
        
        NamedId();
        NamedId( const char* name );
        NamedId& operator = ( const char* name );
        
        void SetName( const char* name );
        
        bool operator == ( const NamedId& other );
        bool operator != ( const NamedId& other );
    };
}

//-----------------------------------------------------------------------------

#endif