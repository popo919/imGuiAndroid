//--------------------------------------------------------------------------------------
// File: FrmAviCapture_Platform.h
// Desc: 
//
// Author: QUALCOMM, Advanced Content Group - Snapdragon SDK
//
// Copyright (c) 2009 QUALCOMM Incorporated.
// All Rights Reserved. 
// QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Name: class CFrmAviCapture
// Desc: 
//--------------------------------------------------------------------------------------
class CFrmAviCapture  
{
public:
    BOOL Create( const CHAR* strFileName, UINT32 nFrameRate, 
                 UINT32 nWidth, UINT32 nHeight, UINT32 nBPP = 32 );
    BOOL AddFrame( BYTE* pBits );
    VOID Finish();

	CFrmAviCapture()  {}
	~CFrmAviCapture() { Finish(); }
};
