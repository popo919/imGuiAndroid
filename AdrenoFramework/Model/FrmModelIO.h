//-----------------------------------------------------------------------------
// 
//  ModelIO
//	
//  Copyright (c) 2011 QUALCOMM Incorporated.
//  All Rights Reserved. QUALCOMM Proprietary/GTDR
//	
//-----------------------------------------------------------------------------

#ifndef ADRENO_MODEL_IO_H
#define ADRENO_MODEL_IO_H

#include "FrmPlatform.h"
#include "FrmAnimation.h"
#include "FrmModel.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    BOOL        FrmWriteModelToFile         ( const Model& model, const CHAR* filename );
    Model*      FrmLoadModelFromFile        ( const CHAR* filename );
    void        FrmDestroyLoadedModel       ( Adreno::Model*& model );

    BOOL        FrmWriteAnimationToFile     ( const Adreno::Animation& animation, const CHAR* filename );
    Animation*  FrmLoadAnimationFromFile    ( const CHAR* filename );
    void        FrmDestroyLoadedAnimation   ( Adreno::Animation*& animation );
}

//-----------------------------------------------------------------------------

#endif