//-----------------------------------------------------------------------------
// 
//  Animation
//	
//  Copyright (c) 2011 QUALCOMM Incorporated.
//  All Rights Reserved. QUALCOMM Proprietary/GTDR
//	
//-----------------------------------------------------------------------------

#ifndef ADRENO_ANIMATION_H
#define ADRENO_ANIMATION_H

#include "FrmAnimationTrack.h"
#include "FrmPlatform.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct Animation
    {
        INT32           NumFrames;
        INT32           NumTracks;
        AnimationTrack* Tracks;

         Animation();
        ~Animation();

        void    ResizeTracks    ( int num_tracks );

        private:
            void DeleteTracks();
    };
}

//-----------------------------------------------------------------------------

#endif