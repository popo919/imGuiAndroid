//--------------------------------------------------------------------------------------
// File: FrmUserInterfaceD3D.cpp
// Desc: 
//
// Author:                 QUALCOMM, Adreno SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include "FrmPlatform.h"
#include "Direct3D/FrmUserInterfaceD3D.h"
#include "Direct3D/FrmShader.h"
#include "Direct3D/FrmUtilsD3D.h"


//--------------------------------------------------------------------------------------
// Name: CFrmUserInterface()
// Desc: CFrmUserInterface constructor
//--------------------------------------------------------------------------------------
CFrmUserInterfaceD3D::CFrmUserInterfaceD3D()
{
    m_pOverlayConstantBuffer = NULL;
    m_pBackgroundConstantBuffer = NULL;
    m_pBackgroundVertexBuffer = NULL;
    m_pBackgroundIndexBuffer = NULL;
}


//--------------------------------------------------------------------------------------
// Name: ~CFrmUserInterface()
// Desc: CFrmUserInterface destructor
//--------------------------------------------------------------------------------------
CFrmUserInterfaceD3D::~CFrmUserInterfaceD3D()
{
    if( m_pOverlayConstantBuffer ) m_pOverlayConstantBuffer->Release();
    m_pOverlayConstantBuffer = NULL;

    if( m_pBackgroundConstantBuffer ) m_pBackgroundConstantBuffer->Release();
    m_pBackgroundConstantBuffer = NULL;

    if( m_pBackgroundVertexBuffer ) m_pBackgroundVertexBuffer->Release();
    m_pBackgroundVertexBuffer = NULL;

    if( m_pBackgroundIndexBuffer ) m_pBackgroundIndexBuffer->Release();
    m_pBackgroundIndexBuffer = NULL;
}


//--------------------------------------------------------------------------------------
// Name: Initialize()
// Desc: Initialize the user interface.
//--------------------------------------------------------------------------------------
BOOL CFrmUserInterfaceD3D::Initialize( CFrmFont* pFont, const CHAR* strHeading )
{
    if( !CFrmUserInterface::Initialize( pFont, strHeading ) )
    {
        return FALSE;
    }

    // Create the background shader
    {
        FRM_SHADER_ATTRIBUTE pShaderAttributes[] =
        {
            { "POSITION", FRM_VERTEX_POSITION, DXGI_FORMAT_R32G32_FLOAT },        
        };
        const INT32 nNumShaderAttributes = sizeof(pShaderAttributes)/sizeof(pShaderAttributes[0]);        
        if( FALSE == m_BackgroundShader.Compile( "FrmUIBackgroundVS.cso", "FrmUIBackgroundPS.cso",
                                                  pShaderAttributes, nNumShaderAttributes ) )
        {
            return FALSE;
        }

        if ( FALSE == FrmCreateConstantBuffer( sizeof(UIBackgroundConstantBuffer), &m_BackgroundConstantBufferData, 
                                               &m_pBackgroundConstantBuffer) )
            return FALSE;
    }

    // Create the overlay shader
    {
        FRM_SHADER_ATTRIBUTE pShaderAttributes[] =
        {
            { "POSITION", FRM_VERTEX_POSITION, DXGI_FORMAT_R32G32B32A32_FLOAT },        
        };
        const INT32 nNumShaderAttributes = sizeof(pShaderAttributes)/sizeof(pShaderAttributes[0]);
        
        if( FALSE == m_OverlayShader.Compile( "FrmUIOverlayVS.cso", "FrmUIOverlayPS.cso",
                                               pShaderAttributes, nNumShaderAttributes ) )
        {
            return FALSE;
        }

        if ( FALSE == FrmCreateConstantBuffer( sizeof(UIOverlayConstantBuffer), &m_OverlayConstantBufferData, 
                                               &m_pOverlayConstantBuffer) )
            return FALSE;
    }

    if( FALSE == FrmCreateVertexBuffer( 4, sizeof(float) * 2, NULL, &m_pBackgroundVertexBuffer ) )
        return FALSE;

    // Quad
    UINT32 indices[6] = { 0, 1, 2, 2, 1, 3 };
    if( FALSE == FrmCreateIndexBuffer( 6, sizeof(UINT32), &indices, &m_pBackgroundIndexBuffer ) )
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
// Name: RenderOverlays()
// Desc: Renders any overlays attached to the user interface
//--------------------------------------------------------------------------------------
VOID CFrmUserInterfaceD3D::RenderOverlays()
{
    for( UINT32 nOverlayIndex = 0; nOverlayIndex < m_nNumOverlays; ++nOverlayIndex )
    {
        FRM_UI_OVERLAY_INFO* pOverlayInfo = m_pOverlays[ nOverlayIndex ];
        m_ScreenRenderer.RenderTextureToScreen( pOverlayInfo->m_fX,  pOverlayInfo->m_fY,
                                                pOverlayInfo->m_fWidth,  pOverlayInfo->m_fHeight,
                                                (CFrmTexture*)pOverlayInfo->m_pTexture,
                                                &m_OverlayShader,
                                                m_pOverlayConstantBuffer,
                                                &m_OverlayConstantBufferData,
                                                offsetof(UIOverlayConstantBuffer, vScreenSize));
    }
}


//--------------------------------------------------------------------------------------
// Name: RenderFadedBackground()
// Desc: Fades the background to make the help text more legible.
//--------------------------------------------------------------------------------------
VOID CFrmUserInterfaceD3D::RenderFadedBackground()
{
    // Update/bind the constant buffer
    m_BackgroundConstantBufferData.vBackground = FRMVECTOR4( 0.0f, 0.0f, 0.0f, 0.5f );
    m_pBackgroundConstantBuffer->Update(&m_BackgroundConstantBufferData);
    m_pBackgroundConstantBuffer->Bind( CFrmConstantBuffer::BindFlagVS | CFrmConstantBuffer::BindFlagPS );
    
    // Bind the shader
    m_BackgroundShader.Bind();

    const FLOAT32 Quad[] =
    {
        -1.0, -1.0f,
        +1.0, -1.0f,
        -1.0, +1.0f,
        +1.0, +1.0f,
    };

    // Copy the vertex data
    VOID* data = m_pBackgroundVertexBuffer->Map( D3D11_MAP_WRITE_DISCARD );
    if (data == NULL)
        return;
    memcpy( data, &Quad[0], sizeof(Quad) );
    m_pBackgroundVertexBuffer->Unmap();

    // Bind the VB/IB
    m_pBackgroundIndexBuffer->Bind( 0 );
    m_pBackgroundVertexBuffer->Bind( 0 );
        
    // Save off the existing state before changing it so it can be restored
    D3DDeviceContext()->OMGetBlendState( &m_savedBlendState, NULL, NULL );
    D3DDeviceContext()->OMGetDepthStencilState( &m_savedDepthStencilState, NULL );

    // Disable depth
    D3DDeviceContext()->OMSetDepthStencilState( m_depthStencilState.Get(), 1 );
        
    // Enable blending
    float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    UINT32 sampleMask = 0xffffffff;
    D3DDeviceContext()->OMSetBlendState( m_blendState.Get(), blendFactor, sampleMask );


    FrmDrawIndexedVertices( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, 6, sizeof(UINT32), 0 );

    // Restore state
    D3DDeviceContext()->OMSetDepthStencilState( m_savedDepthStencilState.Get(), 1 );          
    D3DDeviceContext()->OMSetBlendState( m_savedBlendState.Get(), blendFactor, sampleMask );            
}


