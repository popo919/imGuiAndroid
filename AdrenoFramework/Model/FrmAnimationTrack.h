//-----------------------------------------------------------------------------
// 
//  AnimationTrack
//	
//  Copyright (c) 2011 QUALCOMM Incorporated.
//  All Rights Reserved. QUALCOMM Proprietary/GTDR
//	
//-----------------------------------------------------------------------------

#ifndef ADRENO_ANIMATION_TRACK_H
#define ADRENO_ANIMATION_TRACK_H

#include "FrmNamedId.h"
#include "FrmPlatform.h"
#include "FrmTransform.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct AnimationTrack
    {
        NamedId    Id;
        INT32      NumKeyframes;
        Transform* Keyframes;

         AnimationTrack();
        ~AnimationTrack();

        void    SetName         ( const char* name );
        void    ResizeKeyframes ( int num_keyframes );

        private:
            void DeleteKeyframes();
    };
}

//-----------------------------------------------------------------------------

#endif