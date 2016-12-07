//-----------------------------------------------------------------------------
// 
//  Animation
//	
//  Copyright (c) 2011 QUALCOMM Incorporated.
//  All Rights Reserved. QUALCOMM Proprietary/GTDR
//	
//-----------------------------------------------------------------------------

#include "FrmAnimation.h"

//-----------------------------------------------------------------------------

Adreno::Animation::Animation()
: NumFrames( 0 )
, NumTracks( 0 )
, Tracks( NULL )
{

}

//-----------------------------------------------------------------------------

Adreno::Animation::~Animation()
{
    DeleteTracks();
}

//-----------------------------------------------------------------------------

void Adreno::Animation::ResizeTracks( int num_tracks )
{
    DeleteTracks();
    
    NumTracks = num_tracks;
    
    if( num_tracks > 0 )
    {
        Tracks = new Adreno::AnimationTrack[ num_tracks ];
    }
}

//-----------------------------------------------------------------------------

void Adreno::Animation::DeleteTracks()
{
    if( Tracks )
    {
        delete[] Tracks;
        Tracks = NULL;
    }
}
