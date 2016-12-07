//--------------------------------------------------------------------------------------
// File: FrmResourceD3D.h
// Desc: Wrappers for Direct3D resources to ease their use
//
// Author:                 QUALCOMM, Adreno SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#ifndef _FRM_RESOURCE_GLES_H_
#define _FRM_RESOURCE_GLES_H_

#include "FrmPlatform.h"
#include "FrmResource.h"

const UINT32 FRM_RESOURCE_TYPE_TEXTURE_2D = 0;
const UINT32 FRM_RESOURCE_TYPE_TEXTURE_3D = 1;
const UINT32 FRM_RESOURCE_TYPE_TEXTURE_CUBE_MAP = 2;
const UINT32 FRM_RESOURCE_TYPE_VERTEX_BUFFER = 3;
const UINT32 FRM_RESOURCE_TYPE_INDEX_BUFFER = 3;

//--------------------------------------------------------------------------------------
// Name: class CFrmTexture
// Desc: Resource wrapper for 2D textures
//--------------------------------------------------------------------------------------
class CFrmTexture : public CFrmResource
{
public:
    CFrmTexture() : CFrmResource( FRM_RESOURCE_TYPE_TEXTURE_2D )
    {
        m_pTexture = nullptr;
        m_pTextureView = nullptr;
        m_pSamplerState = nullptr;
    }

    ~CFrmTexture()
    { 
    }

	VOID Bind( UINT32 nTextureUnit )
	{
        D3DDeviceContext()->PSSetShaderResources(
            nTextureUnit,
            1,
            m_pTextureView.GetAddressOf());

        D3DDeviceContext()->PSSetSamplers(
            nTextureUnit,
            1,
            m_pSamplerState.GetAddressOf());
	}

    UINT32 m_nWidth;
    UINT32 m_nHeight;
    UINT32 m_nNumLevels;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pTextureView;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSamplerState;
};


//--------------------------------------------------------------------------------------
// Name: class CFrmTexture3D
// Desc: Resource wrapper for 3D textures
//--------------------------------------------------------------------------------------
class CFrmTexture3D : public CFrmResource
{
public:
    CFrmTexture3D() : CFrmResource( FRM_RESOURCE_TYPE_TEXTURE_3D )
    {
        m_pTexture = nullptr;
        m_pTextureView = nullptr;
        m_pSamplerState = nullptr;
    }

    ~CFrmTexture3D()
    { 
    }

	VOID Bind( UINT32 nTextureUnit )
	{
        D3DDeviceContext()->PSSetShaderResources(
            nTextureUnit,
            1,
            m_pTextureView.GetAddressOf());

        D3DDeviceContext()->PSSetSamplers(
            nTextureUnit,
            1,
            m_pSamplerState.GetAddressOf());
	}

    UINT32 m_nWidth;
    UINT32 m_nHeight;
    UINT32 m_nDepth;
    Microsoft::WRL::ComPtr<ID3D11Texture3D> m_pTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pTextureView;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSamplerState;
};


//--------------------------------------------------------------------------------------
// Name: class CFrmCubeMap
// Desc: Resource wrapper for cubemaps
//--------------------------------------------------------------------------------------
class CFrmCubeMap : public CFrmResource
{
public:
    CFrmCubeMap() : CFrmResource( FRM_RESOURCE_TYPE_TEXTURE_CUBE_MAP )
    {
        m_pTexture = nullptr;
        m_pTextureView = nullptr;
        m_pSamplerState = nullptr;
    }

    ~CFrmCubeMap()
    { 
    }

	VOID Bind( UINT32 nTextureUnit )
	{
        D3DDeviceContext()->PSSetShaderResources(
            nTextureUnit,
            1,
            m_pTextureView.GetAddressOf());

        D3DDeviceContext()->PSSetSamplers(
            nTextureUnit,
            1,
            m_pSamplerState.GetAddressOf());
	}

    UINT32 m_nWidth;
    UINT32 m_nHeight;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pTextureView;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSamplerState;
};


BOOL FrmCreateTexture( UINT32 nWidth, UINT32 nHeight, UINT32 nNumLevels,
                       DXGI_FORMAT nInternalPixelFormat, UINT32 nBorder,
                       VOID* pInitialData, UINT32 nBaseSize, CFrmTexture** ppTexture );

BOOL FrmCreateTexture( UINT32 nWidth, UINT32 nHeight, UINT32 nNumLevels,
                       DXGI_FORMAT nInternalPixelFormat, UINT32 nBorder,
                       VOID* pInitialData, UINT32 nBaseSize, Microsoft::WRL::ComPtr<ID3D11Texture2D>* ppTexture,
                       Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>* ppTextureView,
                       Microsoft::WRL::ComPtr<ID3D11SamplerState>* ppSamplerState);

BOOL FrmCreateTexture3D( UINT32 nWidth, UINT32 nHeight, UINT32 nDepth, UINT32 nLevel,
                         DXGI_FORMAT nInternalPixelFormat, UINT32 nBorder,
                         VOID* pInitialData, UINT32 nSize, CFrmTexture3D** ppTexture );

BOOL FrmCreateTexture3D( UINT32 nWidth, UINT32 nHeight, UINT32 nDepth, UINT32 nLevel,
                         DXGI_FORMAT nInternalPixelFormat, UINT32 nBorder,
                         VOID* pInitialData, UINT32 nSize, Microsoft::WRL::ComPtr<ID3D11Texture3D>* ppTexture,
                         Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>* ppTextureView,
                         Microsoft::WRL::ComPtr<ID3D11SamplerState>* ppSamplerState);

BOOL FrmCreateCubeMap( UINT32 nWidth, UINT32 nHeight, UINT32 nNumLevels,
                       DXGI_FORMAT nInternalPixelFormat, UINT32 nBorder,
                       VOID* pInitialData, UINT32 nFaceSize, CFrmCubeMap** ppTexture );

BOOL FrmCreateCubeMap( UINT32 nWidth, UINT32 nHeight, UINT32 nNumLevels,
                       DXGI_FORMAT nInternalPixelFormat, UINT32 nBorder,
                       VOID* pInitialData, UINT32 nFaceSize, Microsoft::WRL::ComPtr<ID3D11Texture2D>* ppTexture,
                       Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>* ppTextureView,
                       Microsoft::WRL::ComPtr<ID3D11SamplerState>* ppSamplerState);


//--------------------------------------------------------------------------------------
// Name: class CFrmVertexBuffer
// Desc: Resource wrapper for vertex buffer objects
//--------------------------------------------------------------------------------------
class CFrmVertexBuffer : public CFrmResource
{
public:
    CFrmVertexBuffer() : CFrmResource( FRM_RESOURCE_TYPE_VERTEX_BUFFER )
    {
        m_pBuffer = nullptr;
    }

    ~CFrmVertexBuffer()
    { 
    }

	VOID Bind( UINT32 nUnused ) 
	{
        UINT32 offset = 0;
        D3DDeviceContext()->IASetVertexBuffers(
            0,                          // StartSlot
            1,                          // NumBuffers
            m_pBuffer.GetAddressOf(),   // VertexBuffers
            &m_nVertexSize,             // Strides
            &offset);                   // Offsets
	}

    VOID* Map( D3D11_MAP mapType )
    {
        D3D11_MAPPED_SUBRESOURCE ms;
        D3DDeviceContext()->Map( m_pBuffer.Get(), 0, mapType, 0, &ms);
        return ms.pData;
    }

    VOID Unmap( )
    {
        D3DDeviceContext()->Unmap( m_pBuffer.Get(), NULL );
    }

    UINT32 m_nNumVertices;
    UINT32 m_nVertexSize;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_pBuffer;
};


BOOL FrmCreateVertexBuffer( UINT32 nNumVertices, UINT32 nVertexSize, VOID* pSrcVertices,
                            CFrmVertexBuffer** ppVertexBuffer );
BOOL FrmCreateVertexBuffer( UINT32 nNumVertices, UINT32 nVertexSize, VOID* pSrcVertices,
                            Microsoft::WRL::ComPtr<ID3D11Buffer>* ppBuffer );


//--------------------------------------------------------------------------------------
// Name: class CFrmIndexBuffer
// Desc: Resource wrapper for index buffer (vertex array) objects
//--------------------------------------------------------------------------------------
class CFrmIndexBuffer : public CFrmResource
{
public:
    CFrmIndexBuffer() : CFrmResource(FRM_RESOURCE_TYPE_INDEX_BUFFER )
    {
        m_pBuffer = nullptr;
    };
    
    ~CFrmIndexBuffer()
    { 
    }

	VOID Bind( UINT32 nUnused ) 
	{
        UINT32 offset = 0;
        
        DXGI_FORMAT format = (m_nIndexSize == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;       
        D3DDeviceContext()->IASetIndexBuffer(
            m_pBuffer.Get(),    // IndexBuffer
            format,             // Format
            0);                 // Offset
	}

    UINT32 m_nNumIndices;
    UINT32 m_nIndexSize;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_pBuffer;
};


BOOL FrmCreateIndexBuffer( UINT32 nNumIndices, UINT32 nIndexSize, VOID* pSrcIndices,
                           CFrmIndexBuffer** ppIndexBuffer );
BOOL FrmCreateIndexBuffer( UINT32 nNumIndices, UINT32 nIndexSize, VOID* pSrcIndices,
                            Microsoft::WRL::ComPtr<ID3D11Buffer>* ppBuffer );

//--------------------------------------------------------------------------------------
// Name: class CFrmConstantBuffer
// Desc: Resource wrapper for a D3D11 constant buffer
//--------------------------------------------------------------------------------------
class CFrmConstantBuffer : public CFrmResource
{
public:
    static const UINT32 BindFlagVS = 0x01;
    static const UINT32 BindFlagPS = 0x02;
    
    CFrmConstantBuffer() : CFrmResource( 0 )
    {
        m_pBuffer = nullptr;
    };
    
    ~CFrmConstantBuffer()
    { 
    }

    VOID Bind( UINT32 bindFlags )
    {
        Bind( bindFlags, 0 );
    }

	VOID Bind( UINT32 bindFlags, UINT32 bufferIndex ) 
	{
        if (m_pBuffer == nullptr)
            return;

        if (bindFlags & BindFlagVS)
        {
            D3DDeviceContext()->VSSetConstantBuffers(
                bufferIndex,
                1,
                m_pBuffer.GetAddressOf());
        }
        if (bindFlags & BindFlagPS)
        {
            D3DDeviceContext()->PSSetConstantBuffers(
                bufferIndex,
                1,
                m_pBuffer.GetAddressOf());
        }
    }

    VOID Update( VOID* pConstantBuffer)
    {
        if (m_pBuffer == nullptr)
            return;

        D3DDeviceContext()->UpdateSubresource(
            m_pBuffer.Get(),
            0,
            NULL,
            pConstantBuffer,
            0,
            0);
    }

    UINT32 m_nBufferSize;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_pBuffer;
};

BOOL FrmCreateConstantBuffer( UINT32 nBufferSize, VOID* pSrcConstants,
                              CFrmConstantBuffer** ppConstantBuffer );
BOOL FrmCreateConstantBuffer( UINT32 nBufferSize, VOID* pSrcConstants,
                              Microsoft::WRL::ComPtr<ID3D11Buffer>* ppBuffer );


//--------------------------------------------------------------------------------------
// Name: enum FRM_VERTEX_USAGE
// Desc: Pre-defined assignments for vertex attribute shader locations
//--------------------------------------------------------------------------------------
enum FRM_VERTEX_USAGE
{
    FRM_VERTEX_POSITION,          // 0
    FRM_VERTEX_BONEINDICES,       // 1
    FRM_VERTEX_BONEWEIGHTS,       // 2
    FRM_VERTEX_NORMAL,            // 3
    FRM_VERTEX_TANGENT,           // 4
    FRM_VERTEX_BINORMAL,          // 5
    FRM_VERTEX_TEXCOORD0,         // 6
    FRM_VERTEX_TEXCOORD1,         // 7
    FRM_VERTEX_TEXCOORD2,         // 8
    FRM_VERTEX_TEXCOORD3,         // 9
    FRM_VERTEX_TEXCOORD4,         // 10
    FRM_VERTEX_TEXCOORD5,         // 11
    FRM_VERTEX_COLOR0,            // 12
    FRM_VERTEX_COLOR1,            // 13
    FRM_VERTEX_COLOR2,            // 14
    FRM_VERTEX_COLOR3,            // 15
};


//--------------------------------------------------------------------------------------
// Name: struct FRM_VERTEX_ELEMENT
// Desc: Description of a single vertex component
//--------------------------------------------------------------------------------------
#pragma pack(push,1)
struct FRM_VERTEX_ELEMENT
{
    UINT16 nUsage;        // FRM_VERTEX_USAGE
    UINT8  nSize;         // Number of components
    UINT16 nType;         // OpenGL data type
    UINT8  bNormalized;   // Whether to normalize the value
    UINT16 nStride;       // Stride of this component
//	BYTE* pStartAddress; //set to the address in the buffer of first index
};
#pragma pack(pop)


//--------------------------------------------------------------------------------------
// Name: FrmDrawIndexedVertices()
// Desc: 
//--------------------------------------------------------------------------------------
inline VOID FrmDrawIndexedVertices( D3D_PRIMITIVE_TOPOLOGY nPrimType, UINT32 nNumIndices,
                                    UINT32 nIndexSize, UINT32 nIndexOffset )
{
    D3DDeviceContext()->IASetPrimitiveTopology(nPrimType);
    D3DDeviceContext()->DrawIndexed(nNumIndices, nIndexOffset, 0);
}




#endif // _FRM_RESOURCE_GLES_H_
