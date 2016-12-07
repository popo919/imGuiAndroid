//--------------------------------------------------------------------------------------
// File: FrmUtilsD3D.cpp
// Desc: 
//
// Author:                 QUALCOMM, Adreno SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include "FrmPlatform.h"
#include "Direct3D/FrmUtilsD3D.h"
#include "Direct3D/FrmShader.h"
#include "FrmFile.h"

using namespace Microsoft::WRL;

//--------------------------------------------------------------------------------------
// Name: CFrmRenderTextureToScreenD3D()
// Desc: Constructor
//--------------------------------------------------------------------------------------
CFrmRenderTextureToScreenD3D::CFrmRenderTextureToScreenD3D()
{
    m_pVertexBuffer = NULL;
    m_pIndexBuffer = NULL;
}

//--------------------------------------------------------------------------------------
// Name: ~CFrmRenderTextureToScreenD3D()
// Desc: Destructor
//--------------------------------------------------------------------------------------
CFrmRenderTextureToScreenD3D::~CFrmRenderTextureToScreenD3D()
{
    if ( m_pVertexBuffer ) m_pVertexBuffer->Release();
    if ( m_pIndexBuffer )  m_pIndexBuffer->Release();
}


//--------------------------------------------------------------------------------------
// Name: AllocateD3DResources()
// Desc: Helper function to allocate D3D resources required for rendering texture to
// screen
//--------------------------------------------------------------------------------------
BOOL CFrmRenderTextureToScreenD3D::AllocateD3DResources()
{
    // Quad index buffer
    UINT32 indices[6] = { 0, 1, 2, 0, 2, 3 };
    if( FALSE == FrmCreateIndexBuffer( 6, sizeof(UINT32), &indices, &m_pIndexBuffer ) )
        return FALSE;

    // Create vertex buffer
    if( FALSE == FrmCreateVertexBuffer( 4, sizeof(FRMVECTOR4), NULL, &m_pVertexBuffer ) )
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
// Name: RenderTextureToScreen()
// Desc: Helper function to render a texture in screenspace.
//--------------------------------------------------------------------------------------
VOID CFrmRenderTextureToScreenD3D::RenderTextureToScreen( FLOAT32 sx, FLOAT32 sy, FLOAT32 fScale, 
                                                          CFrmTexture* pTexture,
                                                          CFrmShaderProgramD3D* pShaderProgram, 
                                                          CFrmConstantBuffer* pConstantBuffer,
                                                          void *pConstantBufferData,
                                                          UINT32 nScreenSizeOffset )
{
    RenderTextureToScreen( sx, sy, 
                           fScale * pTexture->m_nWidth, 
                           fScale * pTexture->m_nHeight, 
                           pTexture, 
                           pShaderProgram,
                           pConstantBuffer,
                           pConstantBufferData,
                           nScreenSizeOffset );
}


VOID CFrmRenderTextureToScreenD3D::RenderTextureToScreen( FLOAT32 sx, FLOAT32 sy, FLOAT32 w, FLOAT32 h,
                                                          CFrmTexture* pTexture,
                                                          CFrmShaderProgramD3D* pShaderProgram, 
                                                          CFrmConstantBuffer* pConstantBuffer,
                                                          void *pConstantBufferData,
                                                          UINT32 nScreenSizeOffset )
{
    // Allocate on first use
    if (m_pVertexBuffer == NULL)
        AllocateD3DResources();

    UINT32 nNumViewpots = 1;
    D3D11_VIEWPORT viewport;

    // Query for the current viewport
    D3DDeviceContext()->RSGetViewports(&nNumViewpots, &viewport);

    FLOAT32 fTextureWidth  = w;
    FLOAT32 fTextureHeight = h;
    FLOAT32 fScreenWidth   = (FLOAT32)viewport.Width;
    FLOAT32 fScreenHeight  = (FLOAT32)viewport.Height;

    if( sx < 0.0f ) sx += fScreenWidth  - fTextureWidth;
    if( sy < 0.0f ) sy += fScreenHeight - fTextureHeight;

    sx = FrmFloor( sx );
    sy = FrmFloor( sy );

    // Set the geoemtry
    FRMVECTOR4 vQuad[] = 
    { 
        //          Screenspace x     Screenspace y      tu    tv
        FRMVECTOR4( sx,               sy,                0.0f, 1.0f ),
        FRMVECTOR4( sx,               sy+fTextureHeight, 0.0f, 0.0f ),
        FRMVECTOR4( sx+fTextureWidth, sy+fTextureHeight, 1.0f, 0.0f ),
        FRMVECTOR4( sx+fTextureWidth, sy,                1.0f, 1.0f ),
    };

    // Copy the geometry
    VOID* pData = m_pVertexBuffer->Map( D3D11_MAP_WRITE_DISCARD );
    memcpy( pData, vQuad, sizeof(vQuad) );
    m_pVertexBuffer->Unmap();

    // Bind the VB/IB
    m_pIndexBuffer->Bind( 0 );
    m_pVertexBuffer->Bind( 0 );

    // Set the texture
    if (pTexture != NULL )
        pTexture->Bind( 0 );

    // Save off the existing state before changing it so it can be restored
    D3DDeviceContext()->OMGetBlendState( &m_savedBlendState, NULL, NULL );
    D3DDeviceContext()->OMGetDepthStencilState( &m_savedDepthStencilState, NULL );
        
    // Disable depth
    D3DDeviceContext()->OMSetDepthStencilState( m_depthStencilState.Get(), 1 );
        

    // Enable blending
    float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    UINT32 sampleMask = 0xffffffff;
    D3DDeviceContext()->OMSetBlendState( m_blendState.Get(), blendFactor, sampleMask );
        
    // Set the shader program
    pShaderProgram->Bind();

    // Update and bind the constant buffer
    FRMVECTOR2* pScreenSize = (FRMVECTOR2*)((BYTE*)pConstantBufferData + nScreenSizeOffset);
    pScreenSize->x = fScreenWidth;
    pScreenSize->y = fScreenHeight;
    pConstantBuffer->Update(pConstantBufferData);
    pConstantBuffer->Bind ( CFrmConstantBuffer::BindFlagPS |  CFrmConstantBuffer::BindFlagVS );

    // Draw the quad
    FrmDrawIndexedVertices( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, 6, sizeof(UINT32), 0 );


    // Restore the state
    D3DDeviceContext()->OMSetDepthStencilState( m_savedDepthStencilState.Get(), 1 );
       
    // Enable blending
    D3DDeviceContext()->OMSetBlendState( m_savedBlendState.Get(), blendFactor, sampleMask );        
}

