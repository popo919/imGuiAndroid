//--------------------------------------------------------------------------------------
// File: FrmFontD3D.h
// Desc: 
//
// Author:                 QUALCOMM, Adreno SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#ifndef _FRM_FONT_D3D_H
#define _FRM_FONT_D3D_H

#include "FrmFont.h"
#include "Direct3D/FrmPackedResourceD3D.h"
#include "Direct3D/FrmShader.h"


//--------------------------------------------------------------------------------------
// Name: struct FRM_FONT_GLYPH_GLES
// Desc: 
//--------------------------------------------------------------------------------------
struct FRM_FONT_GLYPH_GLES
{
    UINT16 u0, v0;
    UINT16 u1, v1;
    INT16  nOffset;
    INT16  nWidth;
    INT16  nAdvance;
};


//--------------------------------------------------------------------------------------
// Name: struct FRM_FONT_FILE_HEADER_GLES
// Desc: 
//--------------------------------------------------------------------------------------
struct FRM_FONT_FILE_HEADER_GLES
{
    UINT32 nMagicID;
    UINT32 nGlyphHeight;
    FRM_FONT_GLYPH_GLES pGlyphs[1];
};


//--------------------------------------------------------------------------------------
// Name: class CFrmFont
// Desc: 
//--------------------------------------------------------------------------------------
class CFrmFontD3D : public CFrmFont
{
public:
    CFrmFontD3D();
    virtual ~CFrmFontD3D();

    // Create/Destroy functions
    virtual BOOL    Create( const CHAR* strFontFileName );
    virtual VOID    Destroy();

    // Text helper functions
    virtual VOID    GetTextExtent( const CHAR* strText, FLOAT32* pSX, FLOAT32* pSY );
    virtual FLOAT32 GetTextWidth( const CHAR* strText );
    virtual FLOAT32 GetTextHeight();
    virtual VOID    SetScaleFactors( FLOAT32 fScaleX, FLOAT32 fScaleY );

    // Text drawing functions
    virtual VOID    Begin();
    virtual VOID    DrawText( FLOAT32 sx, FLOAT32 sy, FRMCOLOR nColor, 
                              const CHAR* strText, UINT32 nFlags = 0 );
    virtual VOID    End();

protected:

    struct FontConstantBuffer
    {
        FRMVECTOR4 vScale;
        FRMVECTOR4 vOffset;
    };

    struct FRM_FONT_VERTEX
    {
        FLOAT32 sx, sy;
        UINT32  nColor;
        FLOAT32 tu, tv;
    };


    FRM_FONT_GLYPH_GLES*    m_pGlyphsGLES;

    CFrmPackedResourceD3D   m_ResourceD3D;
    CFrmTexture*            m_pTexture;
    CFrmShaderProgramD3D       m_shader; 
    CFrmConstantBuffer*     m_pConstantBuffer;
    CFrmVertexBuffer*       m_pVertexBuffer;
    CFrmIndexBuffer*        m_pIndexBuffer;
    FontConstantBuffer      m_constantBufferData;    

    FLOAT32                 m_fScaleX;
    FLOAT32                 m_fScaleY;

    UINT32                  m_nNestedBeginCount;

    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_savedDepthStencilState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_savedBlendState;

    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;

};


#endif // _FRM_FONT_GLES_H
