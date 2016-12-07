//--------------------------------------------------------------------------------------
// File: FrmResourceD3D.cpp
// Desc: Wrappers for Direct3D resources to ease their use
//
// Author:                 QUALCOMM, Adreno SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include "FrmPlatform.h"
#include "Direct3D/FrmResourceD3D.h"

using namespace Microsoft::WRL;

//--------------------------------------------------------------------------------------
// Name: FrmCreateTexture()
// Desc: Create a texture from the supplied initial data.
//--------------------------------------------------------------------------------------
BOOL FrmCreateTexture( UINT32 nWidth, UINT32 nHeight, UINT32 nNumLevels,
                       DXGI_FORMAT nInternalPixelFormat, 
                       UINT32 nBorder, VOID* pInitialData, UINT32 nBaseSize,
                       CFrmTexture** ppTexture )
{
    if( (!pInitialData) || (!ppTexture) )
        return FALSE;

    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11ShaderResourceView> textureView;
    ComPtr<ID3D11SamplerState> samplerState;
    if( FALSE == FrmCreateTexture( nWidth, nHeight, nNumLevels,
                                   nInternalPixelFormat, nBorder,
                                   pInitialData, nBaseSize, &texture,
                                   &textureView, &samplerState ) )
        return FALSE;

    CFrmTexture* pTexture = new CFrmTexture();
    pTexture->m_nWidth         = nWidth;
    pTexture->m_nHeight        = nHeight;
    pTexture->m_nNumLevels     = nNumLevels;
    pTexture->m_pTexture       = texture;
    pTexture->m_pTextureView   = textureView;
    pTexture->m_pSamplerState  = samplerState;
    (*ppTexture) = pTexture;


    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: FrmCreateTexture()
// Desc: Create a texture from the supplied initial data.
//--------------------------------------------------------------------------------------
BOOL FrmCreateTexture( UINT32 nWidth, UINT32 nHeight, UINT32 nNumLevels,
                       DXGI_FORMAT nInternalPixelFormat,
                       UINT32 nBorder, VOID* pInitialData, UINT32 nBaseSize,
                       ComPtr<ID3D11Texture2D>* ppTexture,
                       ComPtr<ID3D11ShaderResourceView>* ppTextureView,
                       ComPtr<ID3D11SamplerState>* ppSamplerState )
{
    if( (!pInitialData) || (!ppTexture) || (!ppTextureView) || (!ppSamplerState) )
        return FALSE;

    // Initialize the texture description
    D3D11_TEXTURE2D_DESC textureDesc = {0};
    textureDesc.Width = nWidth;
    textureDesc.Height = nHeight;
    textureDesc.Format = nInternalPixelFormat;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;
    textureDesc.MipLevels = nNumLevels;
    textureDesc.ArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    
    
    UINT32 nMipWidth  = nWidth;
    UINT32 nMipHeight = nHeight;
    UINT32 nBPP       = 8 * nBaseSize / ( nWidth * nHeight );
    UINT32 nMipSize   = 16 * ((nMipWidth+3)/4) * ((nMipHeight+3)/4) * nBPP / 8;
    
    // Create an array of subresources for all of the mipmap levels 
    D3D11_SUBRESOURCE_DATA* textureSubresourceData = new D3D11_SUBRESOURCE_DATA[nNumLevels];
    
    // Populate the mip-levels
    for( UINT32 nLevel=0; nLevel<nNumLevels; nLevel++ )
    {
        ZeroMemory(&textureSubresourceData[nLevel], sizeof(D3D11_SUBRESOURCE_DATA));
        textureSubresourceData[nLevel].pSysMem = pInitialData;

        switch (textureDesc.Format)
        {
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
            // For BC1, each 4x4 block is 64-bits (8 bytes)
            textureSubresourceData[nLevel].SysMemPitch = 8 * ((nMipWidth+3)/4);
            break;
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
            // For BC2/3, each 4x4 block is 128-bits (16 bytes)
            textureSubresourceData[nLevel].SysMemPitch = 16 * ((nMipWidth+3)/4);
            break;
        default:
            textureSubresourceData[nLevel].SysMemPitch = nMipWidth * nBPP / 8;
            break;
        }
        
        textureSubresourceData[nLevel].SysMemSlicePitch = 0;

        // Advance to the next level
        pInitialData = (VOID*)( (BYTE*)pInitialData + nMipSize );
        if( nMipWidth > 1 )  nMipWidth  /= 2;
        if( nMipHeight > 1 ) nMipHeight /= 2;
        nMipSize = 16 * ((nMipWidth+3)/4) * ((nMipHeight+3)/4) * nBPP / 8;
    }

    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11ShaderResourceView> textureView;
    ComPtr<ID3D11SamplerState> sampler;

    if ( FAILED(
        D3DDevice()->CreateTexture2D(
            &textureDesc,
            textureSubresourceData,
            &texture ) ) )
    {
        delete [] textureSubresourceData;
        return FALSE;
    }
    delete [] textureSubresourceData;
    
    // Create the shader resource view for the texture
    D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc;
    ZeroMemory(&textureViewDesc, sizeof(textureViewDesc));
    textureViewDesc.Format = textureDesc.Format;
    textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    textureViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
    textureViewDesc.Texture2D.MostDetailedMip = 0;

    if ( FAILED( D3DDevice()->CreateShaderResourceView(
                texture.Get(),
                &textureViewDesc,
                &textureView
                ) ) )
    {
        return FALSE;
    }

    // Create a sampler
    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(samplerDesc));

    // Trilinear filtering
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

    // No anisotropic filtering, ignored
    samplerDesc.MaxAnisotropy = 0;

    // Repeat wrap mode
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

    // No MIP clamping or LOD bias
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    
    // Don't use a comparison function.
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    // Border address mode is not used, so this parameter is ignored.
    samplerDesc.BorderColor[0] = 0.0f;
    samplerDesc.BorderColor[1] = 0.0f;
    samplerDesc.BorderColor[2] = 0.0f;
    samplerDesc.BorderColor[3] = 0.0f;

    if ( FAILED( D3DDevice()->CreateSamplerState(
                    &samplerDesc,
                    &sampler ) ) )
    {
        return FALSE;
    }

    *ppTexture = texture;
    *ppTextureView = textureView;
    *ppSamplerState = sampler;

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: FrmCreateTexture()
// Desc: Create a 3D texture from the supplied initial data.
//--------------------------------------------------------------------------------------
BOOL FrmCreateTexture3D( UINT32 nWidth, UINT32 nHeight, UINT32 nDepth, UINT32 nNumLevels,
                         DXGI_FORMAT nInternalPixelFormat, UINT32 nBorder,
                         VOID* pInitialData, UINT32 nBaseSize, CFrmTexture3D** ppTexture )
{
    if( (!pInitialData) || (!ppTexture) )
        return FALSE;

    ComPtr<ID3D11Texture3D> texture;
    ComPtr<ID3D11ShaderResourceView> textureView;
    ComPtr<ID3D11SamplerState> samplerState;
    if( FALSE == FrmCreateTexture3D( nWidth, nHeight, nDepth, nNumLevels,
                                     nInternalPixelFormat, nBorder,
                                     pInitialData, nBaseSize, &texture,
                                     &textureView, &samplerState ) )    
        return FALSE;

    CFrmTexture3D* pTexture = new CFrmTexture3D();
    pTexture->m_nWidth         = nWidth;
    pTexture->m_nHeight        = nHeight;
    pTexture->m_nDepth         = nDepth;
    pTexture->m_pTexture       = texture;
    pTexture->m_pTextureView   = textureView;
    pTexture->m_pSamplerState  = samplerState;
    (*ppTexture) = pTexture;
    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: FrmCreateTexture()
// Desc: Create a 3D texture from the supplied initial data.
//--------------------------------------------------------------------------------------
BOOL FrmCreateTexture3D( UINT32 nWidth, UINT32 nHeight, UINT32 nDepth, UINT32 nNumLevels,
                         DXGI_FORMAT nInternalPixelFormat, UINT32 nBorder,
                         VOID* pInitialData, UINT32 nBaseSize, Microsoft::WRL::ComPtr<ID3D11Texture3D>* ppTexture,
                         Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>* ppTextureView,
                         Microsoft::WRL::ComPtr<ID3D11SamplerState>* ppSamplerState)
{
    if( (!pInitialData) || (!ppTexture) || (!ppTextureView) || (!ppSamplerState) )
        return FALSE;

    // Initialize the texture description
    D3D11_TEXTURE3D_DESC textureDesc = {0};
    textureDesc.Width = nWidth;
    textureDesc.Height = nHeight;
    textureDesc.Depth = nDepth;
    textureDesc.Format = nInternalPixelFormat;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;
    textureDesc.MipLevels = nNumLevels;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    UINT32 nMipWidth  = nWidth;
    UINT32 nMipHeight = nHeight;
    UINT32 nBPP       = 8 * nBaseSize / ( nWidth * nHeight );
    UINT32 nMipSize   = 16 * ((nMipWidth+3)/4) * ((nMipHeight+3)/4) * nBPP / 8;

    // Create an array of subresources for all of the mipmap levels 
    D3D11_SUBRESOURCE_DATA* textureSubresourceData = new D3D11_SUBRESOURCE_DATA[nNumLevels];

    // Populate the mip-levels
    for( UINT32 nLevel=0; nLevel<nNumLevels; nLevel++ )
    {
        ZeroMemory(&textureSubresourceData[nLevel], sizeof(D3D11_SUBRESOURCE_DATA));
        textureSubresourceData[nLevel].pSysMem = pInitialData;
        switch (textureDesc.Format)
        {
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
            // For BC1, each 4x4 block is 64-bits (8 bytes)
            textureSubresourceData[nLevel].SysMemPitch = 8 * ((nMipWidth+3)/4);
            break;
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
            // For BC2/3, each 4x4 block is 128-bits (16 bytes)
            textureSubresourceData[nLevel].SysMemPitch = 16 * ((nMipWidth+3)/4);
            break;
        default:
            textureSubresourceData[nLevel].SysMemPitch = nMipWidth * nBPP / 8;
            break;
        }
        textureSubresourceData[nLevel].SysMemSlicePitch = nMipSize;

        // Advance to the next level
        pInitialData = (VOID*)( (BYTE*)pInitialData + nMipSize );
        if( nMipWidth > 1 )  nMipWidth  /= 2;
        if( nMipHeight > 1 ) nMipHeight /= 2;
        nMipSize = 16 * ((nMipWidth+3)/4) * ((nMipHeight+3)/4) * nBPP / 8;
    }

    ComPtr<ID3D11Texture3D> texture;
    ComPtr<ID3D11ShaderResourceView> textureView;
    ComPtr<ID3D11SamplerState> sampler;

    if ( FAILED( D3DDevice()->CreateTexture3D( &textureDesc, textureSubresourceData, &texture ) ) )
    {
        delete [] textureSubresourceData;
        return FALSE;
    }
    delete [] textureSubresourceData;
    
    // Create the shader resource view for the texture
    D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc;
    ZeroMemory(&textureViewDesc, sizeof(textureViewDesc));
    textureViewDesc.Format = textureDesc.Format;
    textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
    textureViewDesc.Texture3D.MipLevels = nNumLevels;
    textureViewDesc.Texture3D.MostDetailedMip = 0;

    if ( FAILED( D3DDevice()->CreateShaderResourceView( texture.Get(), &textureViewDesc, &textureView ) ) )
    {
        return FALSE;
    }

    // Create a sampler
    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(samplerDesc));

    // Trilinear filtering
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

    // No anisotropic filtering, ignored
    samplerDesc.MaxAnisotropy = 0;

    // Repeat wrap mode
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

    // No MIP clamping or LOD bias
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    
    // Don't use a comparison function.
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    // Border address mode is not used, so this parameter is ignored.
    samplerDesc.BorderColor[0] = 0.0f;
    samplerDesc.BorderColor[1] = 0.0f;
    samplerDesc.BorderColor[2] = 0.0f;
    samplerDesc.BorderColor[3] = 0.0f;

    if ( FAILED( D3DDevice()->CreateSamplerState( &samplerDesc, &sampler ) ) )
    {
        return FALSE;
    }

    *ppTexture = texture;
    *ppTextureView = textureView;
    *ppSamplerState = sampler;

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: FrmCreateCubeMap()
// Desc: Create a cubemap from the supplied initial data.
//--------------------------------------------------------------------------------------
BOOL FrmCreateCubeMap( UINT32 nWidth, UINT32 nHeight, UINT32 nNumLevels,
                       DXGI_FORMAT nInternalPixelFormat,
                       UINT32 nBorder, VOID* pInitialData, UINT32 nBaseSize,
                       CFrmCubeMap** ppCubeMap )
{
    if( (!pInitialData) || (!ppCubeMap) )
        return FALSE;

    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11ShaderResourceView> textureView;
    ComPtr<ID3D11SamplerState> samplerState;

    if( FALSE == FrmCreateCubeMap( nWidth, nHeight, nNumLevels,
                                   nInternalPixelFormat, nBorder,
                                   pInitialData, nBaseSize, &texture,
                                   &textureView, &samplerState ) )
        return FALSE;

    CFrmCubeMap* pCubeMap = new CFrmCubeMap();
    pCubeMap->m_nWidth         = nWidth;
    pCubeMap->m_nHeight        = nHeight;
    pCubeMap->m_pTexture       = texture;
    pCubeMap->m_pTextureView   = textureView;
    pCubeMap->m_pSamplerState  = samplerState;
    (*ppCubeMap) = pCubeMap;

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: FrmCreateCubeMap()
// Desc: Create a cubemap from the supplied initial data.
//--------------------------------------------------------------------------------------
BOOL FrmCreateCubeMap( UINT32 nWidth, UINT32 nHeight, UINT32 nNumLevels,
                       DXGI_FORMAT nInternalPixelFormat, UINT32 nBorder,
                       VOID* pInitialData, UINT32 nFaceSize, Microsoft::WRL::ComPtr<ID3D11Texture2D>* ppTexture,
                       Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>* ppTextureView,
                       Microsoft::WRL::ComPtr<ID3D11SamplerState>* ppSamplerState)
{
    if( (!pInitialData) || (!ppTexture) || (!ppTextureView) || (!ppSamplerState) )
        return FALSE;

    // Initialize the texture description
    D3D11_TEXTURE2D_DESC textureDesc = {0};
    textureDesc.Width = nWidth;
    textureDesc.Height = nHeight;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Format = nInternalPixelFormat;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.ArraySize = 6; // 6 cube faces
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    textureDesc.MipLevels = nNumLevels;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    // Create an array of subresources for all of the mipmap levels 
    D3D11_SUBRESOURCE_DATA* textureSubresourceData = new D3D11_SUBRESOURCE_DATA[nNumLevels * 6];       

    // Create the mip-chain for each face
    for( UINT32 nFace=0; nFace<6; nFace++ )
    {
        UINT32 nMipWidth  = nWidth;
        UINT32 nMipHeight = nHeight;
        UINT32 nBPP       = 8 * nFaceSize / ( nWidth * nHeight );
        UINT32 nMipSize   = 16 * ((nMipWidth+3)/4) * ((nMipHeight+3)/4) * nBPP / 8;

        // Populate the mip-levels
        for( UINT32 nLevel=0; nLevel<nNumLevels; nLevel++ )        
        {
            UINT32 index = nFace * nNumLevels;

            ZeroMemory(&textureSubresourceData[index], sizeof(D3D11_SUBRESOURCE_DATA));
            textureSubresourceData[index].pSysMem = pInitialData;

            switch (textureDesc.Format)
            {
            case DXGI_FORMAT_BC1_UNORM:
            case DXGI_FORMAT_BC1_UNORM_SRGB:
                // For BC1, each 4x4 block is 64-bits (8 bytes)
                textureSubresourceData[index].SysMemPitch = 8 * ((nMipWidth+3)/4);
                break;
            case DXGI_FORMAT_BC2_UNORM:
            case DXGI_FORMAT_BC2_UNORM_SRGB:
            case DXGI_FORMAT_BC3_UNORM:
            case DXGI_FORMAT_BC3_UNORM_SRGB:
                // For BC2/3, each 4x4 block is 128-bits (16 bytes)
                textureSubresourceData[index].SysMemPitch = 16 * ((nMipWidth+3)/4);
                break;
            default:
                textureSubresourceData[index].SysMemPitch = nMipWidth * nBPP / 8;
                break;
            }
            textureSubresourceData[index].SysMemSlicePitch = 0;

            // Advance to the next level
            pInitialData = (VOID*)( (BYTE*)pInitialData + nMipSize );
            if( nMipWidth > 1 )  nMipWidth  /= 2;
            if( nMipHeight > 1 ) nMipHeight /= 2;
            nMipSize = 16 * ((nMipWidth+3)/4) * ((nMipHeight+3)/4) * nBPP / 8;
        }
    }

    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11ShaderResourceView> textureView;
    ComPtr<ID3D11SamplerState> sampler;

    if ( FAILED( D3DDevice()->CreateTexture2D( &textureDesc, textureSubresourceData, &texture ) ) )
    {
        delete [] textureSubresourceData;
        return FALSE;
    }
    delete [] textureSubresourceData;

    // Create the shader resource view for the texture
    D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc;
    ZeroMemory(&textureViewDesc, sizeof(textureViewDesc));
    textureViewDesc.Format = textureDesc.Format;
    textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    textureViewDesc.TextureCube.MipLevels = nNumLevels;
    textureViewDesc.TextureCube.MostDetailedMip = 0;

    if ( FAILED( D3DDevice()->CreateShaderResourceView( texture.Get(), &textureViewDesc, &textureView ) ) )
    {
        return FALSE;
    }

    // Create a sampler
    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(samplerDesc));

    // Trilinear filtering
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

    // No anisotropic filtering, ignored
    samplerDesc.MaxAnisotropy = 0;

    // Repeat wrap mode
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

    // No MIP clamping or LOD bias
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    
    // Don't use a comparison function.
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    // Border address mode is not used, so this parameter is ignored.
    samplerDesc.BorderColor[0] = 0.0f;
    samplerDesc.BorderColor[1] = 0.0f;
    samplerDesc.BorderColor[2] = 0.0f;
    samplerDesc.BorderColor[3] = 0.0f;

    if ( FAILED( D3DDevice()->CreateSamplerState( &samplerDesc, &sampler ) ) )
    {
        return FALSE;
    }

    *ppTexture = texture;
    *ppTextureView = textureView;
    *ppSamplerState = sampler;

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: FrmCreateVertexBuffer()
// Desc: Create a vertex buffer from the supplied initial data.
//--------------------------------------------------------------------------------------
BOOL FrmCreateVertexBuffer( UINT32 nNumVertices, UINT32 nVertexSize, VOID* pSrcVertices,
                            CFrmVertexBuffer** ppVertexBuffer )
{
    if( (!ppVertexBuffer) )
        return FALSE;

    ComPtr<ID3D11Buffer> pBuffer = nullptr;
    if( FALSE == FrmCreateVertexBuffer( nNumVertices, nVertexSize, pSrcVertices,
                                        &pBuffer ) )
        return FALSE;

    CFrmVertexBuffer* pVertexBuffer = new CFrmVertexBuffer();
    pVertexBuffer->m_nNumVertices   = nNumVertices;
    pVertexBuffer->m_nVertexSize    = nVertexSize;
    pVertexBuffer->m_pBuffer        = pBuffer;
    (*ppVertexBuffer) = pVertexBuffer;
    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: FrmCreateVertexBuffer()
// Desc: Create a vertex buffer from the supplied initial data.
//--------------------------------------------------------------------------------------
BOOL FrmCreateVertexBuffer( UINT32 nNumVertices, UINT32 nVertexSize, VOID* pSrcVertices,
                            ComPtr<ID3D11Buffer>* ppBuffer )
{
    if( (!ppBuffer) )
        return FALSE;

    D3D11_SUBRESOURCE_DATA *pVertexBufferData = NULL;    
    D3D11_SUBRESOURCE_DATA vertexBufferData;
    D3D11_BUFFER_DESC bdesc = CD3D11_BUFFER_DESC( nVertexSize * nNumVertices, D3D11_BIND_VERTEX_BUFFER);
        
    if (pSrcVertices)
    {
        pVertexBufferData = &vertexBufferData;
        vertexBufferData.pSysMem = pSrcVertices;
        vertexBufferData.SysMemPitch = 0;
        vertexBufferData.SysMemSlicePitch = 0;
    }
    else
    {
        // Make available for mapping
        bdesc.Usage = D3D11_USAGE_DYNAMIC;
        bdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    }

    ComPtr<ID3D11Buffer> buffer;
    if ( FAILED( 
        D3DDevice()->CreateBuffer(
            &bdesc,
            pVertexBufferData,
            &buffer) ) )
    {
        return FALSE;
    }    
    (*ppBuffer) = buffer;
    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: FrmCreateIndexBuffer()
// Desc: Create an index buffer from the supplied initial data.
//--------------------------------------------------------------------------------------
BOOL FrmCreateIndexBuffer( UINT32 nNumIndices, UINT32 nIndexSize, VOID* pSrcIndices,
                           CFrmIndexBuffer** ppIndexBuffer )
{
    if( (!pSrcIndices) || (!ppIndexBuffer) )
        return FALSE;

    ComPtr<ID3D11Buffer> pBuffer = nullptr;
    if( FALSE == FrmCreateIndexBuffer( nNumIndices, nIndexSize, pSrcIndices,
                                       &pBuffer ) )
        return FALSE;

    CFrmIndexBuffer* pIndexBuffer  = new CFrmIndexBuffer();
    pIndexBuffer->m_nNumIndices    = nNumIndices;
    pIndexBuffer->m_nIndexSize     = nIndexSize;
    pIndexBuffer->m_pBuffer        = pBuffer;
    (*ppIndexBuffer) = pIndexBuffer;
    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: FrmCreateIndexBuffer()
// Desc: Create an index buffer from the supplied initial data.
//--------------------------------------------------------------------------------------
BOOL FrmCreateIndexBuffer( UINT32 nNumIndices, UINT32 nIndexSize, VOID* pSrcIndices,
                           ComPtr<ID3D11Buffer>* ppBuffer )
{
    if( (!pSrcIndices) || (!ppBuffer) )
        return FALSE;

    D3D11_SUBRESOURCE_DATA indexBufferData;
    indexBufferData.pSysMem = pSrcIndices;
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;

    ComPtr<ID3D11Buffer> buffer;
    if ( FAILED( 
            D3DDevice()->CreateBuffer(
            &CD3D11_BUFFER_DESC(nIndexSize * nNumIndices, D3D11_BIND_INDEX_BUFFER),
            &indexBufferData,
            &buffer) ) )
    {
        return FALSE;
    }
    
    (*ppBuffer) = buffer;
    return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: FrmCreateConstantBuffer()
// Desc: Create a constant buffer from the supplied initial data.
//--------------------------------------------------------------------------------------
BOOL FrmCreateConstantBuffer( UINT32 nBufferSize, VOID* pSrcConstants,
                              CFrmConstantBuffer** ppConstantBuffer )
{
    if( (!pSrcConstants) || (!ppConstantBuffer) )
        return FALSE;

    ComPtr<ID3D11Buffer> pBuffer = nullptr;
    if( FALSE == FrmCreateConstantBuffer( nBufferSize, pSrcConstants, &pBuffer ) )
        return FALSE;

    CFrmConstantBuffer* pConstantBuffer  = new CFrmConstantBuffer();
    pConstantBuffer->m_nBufferSize    = nBufferSize;
    pConstantBuffer->m_pBuffer        = pBuffer;
    (*ppConstantBuffer) = pConstantBuffer;
    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: FrmCreateConstantBuffer()
// Desc: Create a D3D11 constant buffer from the supplied initial data.
//--------------------------------------------------------------------------------------
BOOL FrmCreateConstantBuffer( UINT32 nBufferSize, VOID* pSrcConstants,
                              ComPtr<ID3D11Buffer>* ppBuffer )
{
    if( (!pSrcConstants) || (!ppBuffer) )
        return FALSE;

    D3D11_SUBRESOURCE_DATA constantBufferData;
    constantBufferData.pSysMem = pSrcConstants;
    constantBufferData.SysMemPitch = 0;
    constantBufferData.SysMemSlicePitch = 0;

    ComPtr<ID3D11Buffer> buffer;
    if ( FAILED( 
            D3DDevice()->CreateBuffer(
            &CD3D11_BUFFER_DESC(nBufferSize, D3D11_BIND_CONSTANT_BUFFER),
            &constantBufferData,
            &buffer) ) )
    {
        return FALSE;
    }
    
    (*ppBuffer) = buffer;
    return TRUE;
}


