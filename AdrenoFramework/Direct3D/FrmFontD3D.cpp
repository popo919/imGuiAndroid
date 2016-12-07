//--------------------------------------------------------------------------------------
// File: FrmFontD3D.cpp
// Desc: 
//
// Author:                 QUALCOMM, Adreno SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include "FrmPlatform.h"
#include "Direct3D/FrmFontD3D.h"
#include "Direct3D/FrmShader.h"


//--------------------------------------------------------------------------------------
// Name: CFrmFontD3D()
// Desc: 
//--------------------------------------------------------------------------------------
CFrmFontD3D::CFrmFontD3D() : 
    CFrmFont()
{
    m_pGlyphsGLES       = NULL;
    m_pTexture          = NULL;
    m_pConstantBuffer   = NULL;
    m_pVertexBuffer     = NULL;
    m_pIndexBuffer      = NULL;

    m_fScaleX           = 1.0f;
    m_fScaleY           = 1.0f;

    m_nNestedBeginCount = 0;
}


//--------------------------------------------------------------------------------------
// Name: ~CFrmFontD3D()
// Desc: 
//--------------------------------------------------------------------------------------
CFrmFontD3D::~CFrmFontD3D()
{
    Destroy();
}


//--------------------------------------------------------------------------------------
// Name: Create()
// Desc: 
//--------------------------------------------------------------------------------------
BOOL CFrmFontD3D::Create( const CHAR* strFontFileName )
{
    // Load the packed resources
    if( FALSE == m_ResourceD3D.LoadFromFile( strFontFileName ) )
        return FALSE;

    // Access the font data
    FRM_FONT_FILE_HEADER_GLES* pFontData;
    pFontData = (FRM_FONT_FILE_HEADER_GLES*)m_ResourceD3D.GetData( "FontData" );
    if( NULL == pFontData )
        return FALSE;

    // Verify the font data is valid
    if( pFontData->nMagicID != FRM_FONT_FILE_MAGIC_ID )
        return FALSE;

    // Extract the glyphs
    m_nGlyphHeight = pFontData->nGlyphHeight;
    m_pGlyphsGLES  = pFontData->pGlyphs;

    // Create the font texture
    m_pTexture = m_ResourceD3D.GetTexture( "FontTexture" );
    if( NULL == m_pTexture )
        return FALSE;

    // Compile the font shaders
    FRM_SHADER_ATTRIBUTE pShaderAttributes[] =
    {
        { "POSITION", FRM_VERTEX_POSITION, DXGI_FORMAT_R32G32_FLOAT },
        { "COLOR", FRM_VERTEX_COLOR0, DXGI_FORMAT_R8G8B8A8_UNORM },
        { "TEXCOORD", FRM_VERTEX_TEXCOORD0, DXGI_FORMAT_R32G32_FLOAT }
    };
    const INT32 nNumShaderAttributes = sizeof(pShaderAttributes)/sizeof(pShaderAttributes[0]);

    if( FALSE == m_shader.Compile( "FrmFontVS.cso", "FrmFontPS.cso",
                                    pShaderAttributes, nNumShaderAttributes ) )
        return FALSE;


    if( FALSE == FrmCreateConstantBuffer( sizeof(FontConstantBuffer), 
                                          &m_constantBufferData, &m_pConstantBuffer ) )
        return FALSE;

    if( FALSE == FrmCreateVertexBuffer( 4, sizeof(FRM_FONT_VERTEX), NULL, &m_pVertexBuffer ) )
        return FALSE;

    // Quad
    UINT32 indices[6] = { 0, 1, 2, 0, 2, 3  };
    if( FALSE == FrmCreateIndexBuffer( 6, sizeof(UINT32), &indices, &m_pIndexBuffer ) )
        return FALSE;

    D3D11_DEPTH_STENCIL_DESC dsdesc = CD3D11_DEPTH_STENCIL_DESC(D3D11_DEFAULT);
    D3DDevice()->CreateDepthStencilState(&dsdesc, &m_savedDepthStencilState);
    dsdesc.DepthEnable = false;
    D3DDevice()->CreateDepthStencilState(&dsdesc, &m_depthStencilState);
    
    D3D11_BLEND_DESC bdesc = CD3D11_BLEND_DESC(D3D11_DEFAULT);
    D3DDevice()->CreateBlendState(&bdesc, &m_savedBlendState);
    bdesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bdesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bdesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bdesc.RenderTarget[0].BlendEnable = TRUE;
    D3DDevice()->CreateBlendState(&bdesc, &m_blendState);
   
    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: Destroy()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CFrmFontD3D::Destroy()
{
    if( m_pTexture ) m_pTexture->Release();
    m_pTexture = NULL;

    if( m_pConstantBuffer ) m_pConstantBuffer->Release();
    m_pConstantBuffer = NULL;

    if( m_pVertexBuffer ) m_pVertexBuffer->Release();
    m_pVertexBuffer = NULL;

    if( m_pIndexBuffer ) m_pIndexBuffer->Release();
    m_pIndexBuffer = NULL;

    m_ResourceD3D.Destroy();
}


//--------------------------------------------------------------------------------------
// Name: GetTextExtent()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CFrmFontD3D::GetTextExtent( const CHAR* strText, FLOAT32* pSX, FLOAT32* pSY )
{
    INT32 sx = 0;
    INT32 sy = m_nGlyphHeight;

    while( *strText )
    {
        BYTE ch = *((BYTE*)strText);
        strText++;

        if( ch == '\n' )
        {
            sx  = 0;
            sy += m_nGlyphHeight;
            continue;
        }

        FRM_FONT_GLYPH_GLES* pGlyph = &m_pGlyphsGLES[ch];
        sx += pGlyph->nOffset;
        sx += pGlyph->nAdvance;
    }

    if(pSX) (*pSX) = m_fScaleX * sx;
    if(pSY) (*pSY) = m_fScaleY * sy;
}


//--------------------------------------------------------------------------------------
// Name: GetTextWidth()
// Desc: 
//--------------------------------------------------------------------------------------
FLOAT32 CFrmFontD3D::GetTextWidth( const CHAR* strText )
{
    INT32 sx = 0;

    while( *strText )
    {
        BYTE ch = *((BYTE*)strText);
        strText++;

        if( ch == '\n' )
            break;

        FRM_FONT_GLYPH_GLES* pGlyph = &m_pGlyphsGLES[ch];
        sx += pGlyph->nOffset;
        sx += pGlyph->nAdvance;
    }

    return m_fScaleX * sx;
}


//--------------------------------------------------------------------------------------
// Name: GetTextHeight()
// Desc: 
//--------------------------------------------------------------------------------------
FLOAT32 CFrmFontD3D::GetTextHeight()
{
    return m_fScaleY * m_nGlyphHeight;
}


//--------------------------------------------------------------------------------------
// Name: SetScaleFactors()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CFrmFontD3D::SetScaleFactors( FLOAT32 fScaleX, FLOAT32 fScaleY )
{
    m_fScaleX = fScaleX;
    m_fScaleY = fScaleY;
}


//--------------------------------------------------------------------------------------
// Name: Begin()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CFrmFontD3D::Begin()
{
    if( 0 == m_nNestedBeginCount++ )
    {
        UINT32 nNumViewpots = 1;
        D3D11_VIEWPORT viewport;

        // Query for the current viewport
        D3DDeviceContext()->RSGetViewports(&nNumViewpots, &viewport);

        m_nViewportWidth  = (UINT32) viewport.Width;
        m_nViewportHeight = (UINT32) viewport.Height;

        FLOAT32 fTextureWidth  = (FLOAT32)m_pTexture->m_nWidth;
        FLOAT32 fTextureHeight = (FLOAT32)m_pTexture->m_nHeight;

        // Set the texture
        m_pTexture->Bind(0);

        // Save off the existing state before changing it so it can be restored
        // after font rendering
        D3DDeviceContext()->OMGetBlendState( &m_savedBlendState, NULL, NULL );
        D3DDeviceContext()->OMGetDepthStencilState( &m_savedDepthStencilState, NULL );
            
        // Disable depth
        D3DDeviceContext()->OMSetDepthStencilState( m_depthStencilState.Get(), 1 );
        

        // Enable blending
        float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        UINT32 sampleMask = 0xffffffff;
        D3DDeviceContext()->OMSetBlendState( m_blendState.Get(), blendFactor, sampleMask );
            
        // Disable depth
        D3DDeviceContext()->OMSetDepthStencilState( m_depthStencilState.Get(), 1 );
        
        // Bind the shader
        m_shader.Bind();
        
        // Set the constants
        m_constantBufferData.vScale = FRMVECTOR4( +2.0f / m_nViewportWidth, -2.0f / m_nViewportHeight, 
                                                  1.0f/fTextureWidth, -1.0f/fTextureHeight );
        m_constantBufferData.vOffset = FRMVECTOR4( -1.0f, +1.0f, 0.0f, 1.0f );
        m_pConstantBuffer->Update(&m_constantBufferData);
        m_pConstantBuffer->Bind(CFrmConstantBuffer::BindFlagVS | CFrmConstantBuffer::BindFlagPS);
    }
}


//--------------------------------------------------------------------------------------
// Name: DrawText()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CFrmFontD3D::DrawText( FLOAT32 sx, FLOAT32 sy, FRMCOLOR nColor, const CHAR* strText,
                         UINT32 nFlags )
{
    Begin();

    sx = FrmFloor( sx );
    sy = FrmFloor( sy );

    if( sx < 0.0f ) sx += (FLOAT32)m_nViewportWidth;
    if( sy < 0.0f ) sy += (FLOAT32)m_nViewportHeight - (FLOAT32)m_nGlyphHeight;

    FLOAT32 fOriginX = sx;
    FLOAT32 fOriginY = sy;

    FRMCOLOR nWhiteColor = 0x00ffffff | ( 0xff000000 & nColor.v );

    
    if( nFlags & FRM_FONT_RIGHT )
        sx -= GetTextWidth( strText );
    if( nFlags & FRM_FONT_CENTER )
        sx -= GetTextWidth( strText ) / 2;

    while( *strText )
    {
        BYTE ch = *((BYTE*)strText);
        strText++;

        if( ch == '\n' )
        {
            sx  = fOriginX;
            sy += m_fScaleX * m_nGlyphHeight;

            if( nFlags & FRM_FONT_RIGHT )
                sx -= (FLOAT32)GetTextWidth( strText );

            if( nFlags & FRM_FONT_CENTER )
                sx -= (FLOAT32)GetTextWidth( strText ) / 2;
            
            continue;
        }

        FRM_FONT_GLYPH_GLES* pGlyph = &m_pGlyphsGLES[ch];
        sx += m_fScaleX * pGlyph->nOffset;

        FLOAT32 w = m_fScaleX * ( pGlyph->u1 - pGlyph->u0 );
        FLOAT32 h = m_fScaleY * ( pGlyph->v1 - pGlyph->v0 );


        UINT32 nGlyphColor = ( ch >= 128 && ch < 160 ) ? nWhiteColor.v : nColor.v;

        FRM_FONT_VERTEX vQuad[4] =
        {
            { sx + 0, sy + 0, nGlyphColor, pGlyph->u0, pGlyph->v0 }, 
            { sx + 0, sy + h, nGlyphColor, pGlyph->u0, pGlyph->v1 }, 
            { sx + w, sy + h, nGlyphColor, pGlyph->u1, pGlyph->v1 }, 
            { sx + w, sy + 0, nGlyphColor, pGlyph->u1, pGlyph->v0 },
        };

        // Update the vertex buffer
        VOID* data = m_pVertexBuffer->Map( D3D11_MAP_WRITE_DISCARD );
        if (data == NULL)
            return;
        memcpy( data, &vQuad[0], sizeof(vQuad) );
        m_pVertexBuffer->Unmap();

        m_pVertexBuffer->Bind( 0 );
        
        
        // Bind the vertex/index buffer
        m_pVertexBuffer->Bind( 0 );
        m_pIndexBuffer->Bind( 0 );
        
        FrmDrawIndexedVertices( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, 6, sizeof(UINT32), 0);
        
        sx += m_fScaleX * pGlyph->nAdvance;
    }

		
    m_fCursorX = sx;
    m_fCursorY = sy;
        
    End();
}


//--------------------------------------------------------------------------------------
// Name: End()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CFrmFontD3D::End()
{
    if( --m_nNestedBeginCount == 0 )
    {
        // Restore state
        D3DDeviceContext()->OMSetDepthStencilState( m_savedDepthStencilState.Get(), 1 );
       
        float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        UINT32 sampleMask = 0xffffffff;
        D3DDeviceContext()->OMSetBlendState( m_savedBlendState.Get(), blendFactor, sampleMask );        
    }
}

