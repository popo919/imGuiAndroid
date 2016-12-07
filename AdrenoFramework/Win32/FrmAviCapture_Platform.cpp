//--------------------------------------------------------------------------------------
// File: FrmAviCapture_Platform.cpp
// Desc: 
//
// Author: QUALCOMM, Advanced Content Group - Snapdragon SDK
//
// Copyright (c) 2009 QUALCOMM Incorporated.
// All Rights Reserved. 
// QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include "FrmPlatform.h"
#include <EGL/egl.h>
#include <GLES2/GL2.h>
#include <vfw.h>
#include "FrmAviCapture_Platform.h"
#pragma comment ( lib, "vfw32.lib")

PAVIFILE   m_pAVIFile;
PAVISTREAM m_pAviStream;		
PAVISTREAM m_pAviStreamCompressed; 
UINT32     m_nFrame;

UINT32* g_pPixels = NULL;
UINT32  g_nWidth;
UINT32  g_nHeight;

//--------------------------------------------------------------------------------------
// Name: 
// Desc: 
//--------------------------------------------------------------------------------------
BOOL CFrmAviCapture::Create( const CHAR* strFileName, UINT32 nFrameRate,
                             UINT32 nWidth, UINT32 nHeight, UINT32 nBPP )
{
    HRESULT hr;

    AVIFileInit();

    hr = AVIFileOpen( &m_pAVIFile, strFileName, OF_WRITE | OF_CREATE, NULL );
    if( FAILED(hr) )
        return FALSE;


    AVISTREAMINFO AviStreamInfo = {0};
    AviStreamInfo.fccType                = streamtypeVIDEO;
    AviStreamInfo.fccHandler             = 0;
    AviStreamInfo.dwScale                = 1;
    AviStreamInfo.dwRate                 = nFrameRate;
    AviStreamInfo.dwSuggestedBufferSize  = 4 * nWidth * nHeight;
    AviStreamInfo.rcFrame.left           = 0;
    AviStreamInfo.rcFrame.top            = 0;
    AviStreamInfo.rcFrame.right          = nWidth;
    AviStreamInfo.rcFrame.bottom         = nHeight;
    SetRect( &AviStreamInfo.rcFrame, 0, 0, nWidth, nHeight );

    hr = AVIFileCreateStream( m_pAVIFile, &m_pAviStream, &AviStreamInfo );
    if( FAILED(hr) )
        return FALSE;

    AVICOMPRESSOPTIONS  AviCompressOptions = {0};
    AVICOMPRESSOPTIONS* _AviCompressOptions[1] = {&AviCompressOptions};

#define USE_HARCODED_CODEC
#ifdef USE_HARCODED_CODEC
	// This mumbo jumbo is for the Intel codec version 4.5 on my machine
	BYTE pParams[] = 
	{
		0x78, 0x56, 0x34, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	AviCompressOptions.fccHandler = 0x31347669;
	AviCompressOptions.dwQuality  = 0x00002134;
	AviCompressOptions.dwFlags    = 0x00000008;
	AviCompressOptions.lpParms    = pParams;
	AviCompressOptions.cbParms	  = 0x00000038;
#else
	if( NULL == AVISaveOptions( NULL, 0, 1, &m_pAviStream, (AVICOMPRESSOPTIONS**)&_AviCompressOptions ) )
		return FALSE;
#endif // USE_HARCODED_CODEC

    hr = AVIMakeCompressedStream( &m_pAviStreamCompressed, m_pAviStream, 
                                  &AviCompressOptions, NULL );
    if( FAILED(hr) )
        return FALSE;

#ifdef USE_HARCODED_CODEC
	AviCompressOptions.lpParms    = NULL;
#endif // USE_HARCODED_CODEC
    hr = AVISaveOptionsFree( 1, (AVICOMPRESSOPTIONS**)&_AviCompressOptions );
    if( FAILED(hr) )
        return FALSE;

    BITMAPINFOHEADER BitmapInfoHeader = {0};
    BitmapInfoHeader.biSize        = sizeof(BITMAPINFOHEADER);
    BitmapInfoHeader.biWidth       = nWidth;
    BitmapInfoHeader.biHeight      = nHeight;
    BitmapInfoHeader.biPlanes      = 1;
    BitmapInfoHeader.biBitCount    = 32;
    BitmapInfoHeader.biCompression = BI_RGB;
    BitmapInfoHeader.biSizeImage   = nWidth * nHeight * 4; 

    hr = AVIStreamSetFormat( m_pAviStreamCompressed, 0, &BitmapInfoHeader,
                             BitmapInfoHeader.biSize + BitmapInfoHeader.biClrUsed * sizeof(RGBQUAD) );
    if( FAILED(hr) )
        return FALSE;

    // Allocate temporary space for the screen captured pixels
    g_pPixels = (UINT32*)malloc( sizeof(UINT32)*nWidth*nHeight );
    g_nWidth  = nWidth;
    g_nHeight = nHeight;
    m_nFrame  = 0;

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: 
// Desc: 
//- ------------------------------------------------
BOOL CFrmAviCapture::AddFrame( BYTE* pBits )
{
    // Grab the pixels from the backbuffer
    glReadPixels( 0, 0, g_nWidth, g_nHeight, GL_RGBA, GL_UNSIGNED_BYTE, g_pPixels );

    // Lacking GL_BGRA_EXT, manually (uggh) swap RGBA to BGRA
    BYTE* p = (BYTE*)g_pPixels;
    for( UINT32 i=0; i<g_nWidth*g_nHeight; i++ )
    {
        BYTE r = p[0];
        BYTE b = p[2];
        p[0] = b;
        p[2] = r;
        p += 4;
    }

    return SUCCEEDED( AVIStreamWrite( m_pAviStreamCompressed, m_nFrame++, 1, g_pPixels, 
                                      g_nWidth*g_nHeight*4, AVIIF_KEYFRAME, NULL, NULL ) );
}

//--------------------------------------------------------------------------------------
// Name: 
// Desc: 
//--------------------------------------------------------------------------------------
VOID CFrmAviCapture::Finish()
{
    free( g_pPixels );

    if( m_pAviStream )		     AVIStreamRelease( m_pAviStream );
    if( m_pAviStreamCompressed ) AVIStreamRelease( m_pAviStreamCompressed );
    if( m_pAVIFile )             AVIFileRelease( m_pAVIFile );
    
    m_pAviStream           = NULL;
    m_pAviStreamCompressed = NULL;
    m_pAVIFile             = NULL;

    AVIFileExit();
}

