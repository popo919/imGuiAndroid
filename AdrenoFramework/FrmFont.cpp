//--------------------------------------------------------------------------------------
// File: FrmFont.cpp
// Desc: 
//
// Author:      QUALCOMM, Advanced Content Group - Snapdragon SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include "FrmPlatform.h"
#include "FrmFont.h"


//--------------------------------------------------------------------------------------
// Name: CFrmFont()
// Desc: 
//--------------------------------------------------------------------------------------
CFrmFont::CFrmFont()
{
    m_nGlyphHeight    = 0;

    m_nViewportWidth  = 0;
    m_nViewportHeight = 0;

    m_fCursorX        = 0.0f;
    m_fCursorY        = 0.0f;
}


//--------------------------------------------------------------------------------------
// Name: ~CFrmFont()
// Desc: 
//--------------------------------------------------------------------------------------
CFrmFont::~CFrmFont()
{
}


//--------------------------------------------------------------------------------------
// Name: DrawText()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CFrmFont::DrawText( FRMCOLOR nColor, const CHAR* strText, UINT32 nFlags )
{
    return DrawText( m_fCursorX, m_fCursorY, nColor, strText, nFlags );
}

