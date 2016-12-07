//-----------------------------------------------------------------------------
// 
//  PointerTable
//	
//  Copyright (c) 2011 QUALCOMM Incorporated.
//  All Rights Reserved. QUALCOMM Proprietary/GTDR
//	
//-----------------------------------------------------------------------------

#ifndef ADRENO_POINTER_TABLE_H
#define ADRENO_POINTER_TABLE_H

#include "FrmPlatform.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    static const INT32 FRM_NULL_POINTER_OFFSET = -1;

    struct PointerFixup
    {
        INT32 PointerOffset;
        INT32 PointeeOffset;

        PointerFixup()
        : PointerOffset( FRM_NULL_POINTER_OFFSET )
        , PointeeOffset( FRM_NULL_POINTER_OFFSET )
        {
        
        }

        PointerFixup( int pointer_offset, int pointee_offset )
        : PointerOffset( pointer_offset )
        , PointeeOffset( pointee_offset )
        {
        
        }
    };

    //-----------------------------------------------------------------------------

    struct PointerTable
    {
        INT32         NumPointers;
        PointerFixup* PointerFixups;

        PointerTable()
        : NumPointers( 0 )
        , PointerFixups( NULL )
        {
        
        }

        ~PointerTable()
        {
            NumPointers = 0;
            
            if( PointerFixups )
            {
                delete[] PointerFixups;
                PointerFixups = NULL;
            }
        }
    };
}

//-----------------------------------------------------------------------------

#endif