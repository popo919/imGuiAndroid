//--------------------------------------------------------------------------------------
// File: FrmPackedResourceD3D.cpp
// Desc: 
//
// Author:                 QUALCOMM, Adreno SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include "FrmPlatform.h"
#include "Direct3D/FrmPackedResourceD3D.h"
#include "FrmFile.h"
#include "FrmStdLib.h"
#include "FrmUtils.h"


struct FRAMEWORK_TEXTURE_HEADER
{
    UINT32 nWidth;
    UINT32 nHeight;
    UINT32 nNumLevels;
    UINT32 nPixelFormat;
    UINT32 nPixelType;
    UINT32 nPitch;
    UINT32 nBaseSize;
    UINT32 nMipChainSize;
    UINT32 nMinMipSize;
    UINT32 nMagFilter;
    UINT32 nMinFilter;
    UINT32 nWrapMode;
};

struct FRAMEWORK_TEXTURE3D_HEADER
{
    UINT32 nWidth;
    UINT32 nHeight;
    UINT32 nDepth;
    UINT32 nNumLevels;
	UINT32 nPixelFormat;
	UINT32 nPixelType;
    UINT32 nRowPitch;
    UINT32 nSlicePitch;
    UINT32 nBaseSize;
    UINT32 nMagFilter;
    UINT32 nMinFilter;
    UINT32 nWrapMode;
};

struct FRAMEWORK_CUBEMAP_HEADER
{
    UINT32 nWidth;
    UINT32 nHeight;
    UINT32 nNumLevels;
    UINT32 nPixelFormat;
    UINT32 nPixelType;
    UINT32 nPitch;
    UINT32 nBaseSize;
    UINT32 nMipChainSize;
    UINT32 nMinMipSize;
    UINT32 nMagFilter;
    UINT32 nMinFilter;
    UINT32 nWrapMode;
};

DXGI_FORMAT FrmGLTextureFormatToD3D( UINT32 nPixelFormat, UINT32 nPixelType );
VOID FrmGLToD3DSamplerDesc( UINT32 nMagFilter, UINT32 nMinFilter, UINT32 nWrapMode, D3D11_SAMPLER_DESC *pSamplerDesc );


//--------------------------------------------------------------------------------------
// Name: GetTexture()
// Desc: 
//--------------------------------------------------------------------------------------
CFrmTexture* CFrmPackedResourceD3D::GetTexture( const CHAR* strResource )
{
    // Find the requested resource tag
    FRM_RESOURCE_TAG* pResourceTag = GetResource( strResource );
    if( NULL == pResourceTag )
        return NULL;

    // If the texture already exists, return a reference to it
    if( pResourceTag->pObject )
    {
        CFrmResource* pTexture = (CFrmResource*)pResourceTag->pObject;
        pTexture->AddRef();
        return (CFrmTexture*)pTexture;
    }

    // Otherwise, create the texture using it's data stored in the packed resource

    FRAMEWORK_TEXTURE_HEADER* pTextureHeader = (FRAMEWORK_TEXTURE_HEADER*)pResourceTag->pSystemData;
    VOID* pTextureImageData = pResourceTag->pBufferData;

    UINT32 nNumLevels   = pTextureHeader->nNumLevels;
    UINT32 nBorder      = 0;
    UINT32 nPixelFormat = pTextureHeader->nPixelFormat;
    UINT32 nPixelType   = pTextureHeader->nPixelType;


    // Create the texture

    CFrmTexture* pTexture;

    if( FALSE == FrmCreateTexture( pTextureHeader->nWidth, pTextureHeader->nHeight,
                                   nNumLevels, FrmGLTextureFormatToD3D(nPixelFormat, nPixelType),
                                   nBorder, pTextureImageData,
                                   pTextureHeader->nBaseSize, &pTexture ) )
        return NULL;

    D3D11_SAMPLER_DESC samplerDesc;    
    FrmGLToD3DSamplerDesc( pTextureHeader->nMagFilter, pTextureHeader->nMinFilter, pTextureHeader->nWrapMode, &samplerDesc );
    if ( FAILED( D3DDevice()->CreateSamplerState( &samplerDesc, &pTexture->m_pSamplerState ) ) )
    {
        return FALSE;
    }

    // Return it
    pResourceTag->pObject = pTexture;
    return pTexture;
}


//--------------------------------------------------------------------------------------
// Name: GetTexture3D()
// Desc: 
//--------------------------------------------------------------------------------------
CFrmTexture3D* CFrmPackedResourceD3D::GetTexture3D( const CHAR* strResource )
{
    // Find the requested resource tag
    FRM_RESOURCE_TAG* pResourceTag = GetResource( strResource );
    if( NULL == pResourceTag )
        return NULL;

    // If the texture already exists, return a reference to it
    if( pResourceTag->pObject )
    {
        CFrmResource* pTexture = (CFrmResource*)pResourceTag->pObject;
        pTexture->AddRef();
        return (CFrmTexture3D*)pTexture;
    }

    // Otherwise, create the texture using it's data stored in the packed resource

    FRAMEWORK_TEXTURE3D_HEADER* pTextureHeader = (FRAMEWORK_TEXTURE3D_HEADER*)pResourceTag->pSystemData;
    VOID* pTextureImageData = pResourceTag->pBufferData;

    UINT32 nNumLevels   = pTextureHeader->nNumLevels;
    UINT32 nBorder      = 0;
    UINT32 nPixelFormat = pTextureHeader->nPixelFormat;
    UINT32 nPixelType   = pTextureHeader->nPixelType;

    // Create the texture
    CFrmTexture3D* pTexture;
    if( FALSE == FrmCreateTexture3D( pTextureHeader->nWidth, pTextureHeader->nHeight, pTextureHeader->nDepth,
                                     nNumLevels, FrmGLTextureFormatToD3D(nPixelFormat, nPixelType),
									 nBorder, pTextureImageData,
                                     pTextureHeader->nBaseSize * pTextureHeader->nDepth, &pTexture ) )
        return NULL;

    D3D11_SAMPLER_DESC samplerDesc;    
    FrmGLToD3DSamplerDesc( pTextureHeader->nMagFilter, pTextureHeader->nMinFilter, pTextureHeader->nWrapMode, &samplerDesc );
    if ( FAILED( D3DDevice()->CreateSamplerState( &samplerDesc, &pTexture->m_pSamplerState ) ) )
    {
        return FALSE;
    }

    // Return it
    pResourceTag->pObject = pTexture;
    return pTexture;
}


//--------------------------------------------------------------------------------------
// Name: GetCubeMap()
// Desc: 
//--------------------------------------------------------------------------------------
CFrmCubeMap* CFrmPackedResourceD3D::GetCubeMap( const CHAR* strResource )
{
    // Find the requested resource tag
    FRM_RESOURCE_TAG* pResourceTag = GetResource( strResource );
    if( NULL == pResourceTag )
        return NULL;

    // If the cubemap already exists, return a reference to it
    if( pResourceTag->pObject )
    {
        CFrmCubeMap* pCubeMap = (CFrmCubeMap*)pResourceTag->pObject;
        pCubeMap->AddRef();
        return pCubeMap;
    }

    // Otherwise, create the cubemap using it's data stored in the packed resource

    FRAMEWORK_CUBEMAP_HEADER* pTextureHeader = (FRAMEWORK_CUBEMAP_HEADER*)pResourceTag->pSystemData;
    VOID* pCubeMapImageData = pResourceTag->pBufferData;

    UINT32 nNumLevels   = pTextureHeader->nNumLevels;
    UINT32 nBorder      = 0;
    UINT32 nPixelFormat = pTextureHeader->nPixelFormat;
    UINT32 nPixelType   = pTextureHeader->nPixelType;

    // Create the cubemap
    CFrmCubeMap* pCubeMap;
    if( FALSE == FrmCreateCubeMap( pTextureHeader->nWidth, pTextureHeader->nHeight,
                                   nNumLevels, FrmGLTextureFormatToD3D(nPixelFormat, nPixelType), 
                                   nBorder, pCubeMapImageData,
                                   pTextureHeader->nMipChainSize, &pCubeMap ) )
        return NULL;

    // Set the texture filter and wrap modes
    D3D11_SAMPLER_DESC samplerDesc;    
    FrmGLToD3DSamplerDesc( pTextureHeader->nMagFilter, pTextureHeader->nMinFilter, pTextureHeader->nWrapMode, &samplerDesc );
    if ( FAILED( D3DDevice()->CreateSamplerState( &samplerDesc, &pCubeMap->m_pSamplerState ) ) )
    {
        return FALSE;
    }

    // Return it
    pResourceTag->pObject = pCubeMap;
    return pCubeMap;
}

//--------------------------------------------------------------------------------------
// Name: FrmGLTextureFormatToD3D()
// Desc: The ResourcePacker tool stores texture pixel types in GL enumerants.  This
// function converts to the appropriate D3D enumerant
//--------------------------------------------------------------------------------------
DXGI_FORMAT FrmGLTextureFormatToD3D( UINT32 nPixelFormat, UINT32 nPixelType )
{
// Need to define these somewhere...is there a better place?  Should I include GL headers?  
#define GL_DEPTH_COMPONENT                0x1902
#define GL_ALPHA                          0x1906
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_LUMINANCE                      0x1909
#define GL_LUMINANCE_ALPHA                0x190A
#define GL_SRGB8_ALPHA8                   0x8C43


#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_FIXED                          0x140C
#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1         0x8034
#define GL_UNSIGNED_SHORT_5_6_5           0x8363

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT         0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT        0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT        0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT        0x83F3
#define GL_COMPRESSED_RGB_S3TC_DXT1_SRGB_EXT    0x93F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_SRGB_EXT   0x93F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_SRGB_EXT   0x93F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_SRGB_EXT   0x93F3


    switch ( nPixelFormat )
    {
    case GL_ALPHA:
    case GL_LUMINANCE:
        {
            switch (nPixelType)
            {
            case GL_UNSIGNED_BYTE:
                return DXGI_FORMAT_R8_UNORM;
            case GL_FLOAT:
                return DXGI_FORMAT_R32_FLOAT;
            }
        }
        break;
    case GL_LUMINANCE_ALPHA:
        {
            switch (nPixelType)
            {
            case GL_UNSIGNED_BYTE:
                return DXGI_FORMAT_R8G8_UNORM;
            case GL_FLOAT:
                return DXGI_FORMAT_R32G32_FLOAT;
            }
        }        
    case GL_RGB:
        {
            switch (nPixelType)
            {
            case GL_UNSIGNED_SHORT_5_6_5:
                return DXGI_FORMAT_B5G6R5_UNORM;
            case GL_FLOAT:
                return DXGI_FORMAT_R32G32B32_FLOAT;
            }
        }
    case GL_RGBA:
        {
            switch (nPixelType)
            {
            case GL_UNSIGNED_BYTE:
                return DXGI_FORMAT_R8G8B8A8_UNORM;
            case GL_UNSIGNED_SHORT_4_4_4_4:
                return DXGI_FORMAT_B4G4R4A4_UNORM;
            case GL_UNSIGNED_SHORT_5_5_5_1:
                return DXGI_FORMAT_B5G5R5A1_UNORM;
            case GL_FLOAT:
                return DXGI_FORMAT_R32G32B32A32_FLOAT;
            }
        }
        break;
    case GL_SRGB8_ALPHA8:
        {
            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        }
        break;
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        return DXGI_FORMAT_BC1_UNORM;
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        return DXGI_FORMAT_BC2_UNORM;
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        return DXGI_FORMAT_BC3_UNORM;
    case GL_COMPRESSED_RGB_S3TC_DXT1_SRGB_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT1_SRGB_EXT:
        return DXGI_FORMAT_BC1_UNORM_SRGB;
    case GL_COMPRESSED_RGBA_S3TC_DXT3_SRGB_EXT:
        return DXGI_FORMAT_BC2_UNORM_SRGB;
    case GL_COMPRESSED_RGBA_S3TC_DXT5_SRGB_EXT:
        return DXGI_FORMAT_BC3_UNORM_SRGB;
    }

    FrmAssert( 0 ); // Unknown format/pixel type
    return DXGI_FORMAT_UNKNOWN;
}

//--------------------------------------------------------------------------------------
// Name: FrmGLTextureFormatToD3D()
// Desc: The ResourcePacker tool stores filter/wrap modes in GL enumerants.  This
// function converts to the appropriate D3D sampler description
//--------------------------------------------------------------------------------------
VOID FrmGLToD3DSamplerDesc( UINT32 nMagFilter, UINT32 nMinFilter, UINT32 nWrapMode, D3D11_SAMPLER_DESC *pSamplerDesc )
{
// Need to define these somewhere...is there a better place?  Should I include GL headers?  
#define GL_NEAREST                                       0x2600
#define GL_LINEAR                                        0x2601
#define GL_NEAREST_MIPMAP_NEAREST                        0x2700
#define GL_LINEAR_MIPMAP_NEAREST                         0x2701
#define GL_NEAREST_MIPMAP_LINEAR                         0x2702
#define GL_LINEAR_MIPMAP_LINEAR                          0x2703

#define GL_REPEAT                                        0x2901
#define GL_CLAMP_TO_EDGE                                 0x812F
#define GL_MIRRORED_REPEAT                               0x8370

    
    ZeroMemory(pSamplerDesc, sizeof(D3D11_SAMPLER_DESC));    
    pSamplerDesc->MaxAnisotropy = 0;
    pSamplerDesc->MipLODBias = 0.0f;
    pSamplerDesc->MinLOD = 0;
    pSamplerDesc->MaxLOD = D3D11_FLOAT32_MAX;
    pSamplerDesc->ComparisonFunc = D3D11_COMPARISON_NEVER;
    pSamplerDesc->BorderColor[0] = 0.0f;
    pSamplerDesc->BorderColor[1] = 0.0f;
    pSamplerDesc->BorderColor[2] = 0.0f;
    pSamplerDesc->BorderColor[3] = 0.0f;



    switch (nWrapMode)
    {
    case GL_REPEAT:
        // Repeat wrap mode
        pSamplerDesc->AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        pSamplerDesc->AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        pSamplerDesc->AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        break;
    case GL_CLAMP_TO_EDGE:
        // Clamp wrap mode
        pSamplerDesc->AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        pSamplerDesc->AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        pSamplerDesc->AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        break;
    case GL_MIRRORED_REPEAT:
        // Clamp wrap mode
        pSamplerDesc->AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
        pSamplerDesc->AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
        pSamplerDesc->AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
        break;
    default:
        // Default
        pSamplerDesc->AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        pSamplerDesc->AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        pSamplerDesc->AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        break;
    }
    
    switch (nMinFilter)
    {
    case GL_NEAREST:
        if (nMagFilter == GL_NEAREST)
        {
            pSamplerDesc->Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        }
        else
        {
            pSamplerDesc->Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        }        
        break;
    case GL_LINEAR:
        if (nMagFilter == GL_NEAREST)
        {
            pSamplerDesc->Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        }
        else
        {
            pSamplerDesc->Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        }        
        break;
    case GL_NEAREST_MIPMAP_NEAREST:
        if (nMagFilter == GL_NEAREST)
        {
            pSamplerDesc->Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        }
        else
        {
            pSamplerDesc->Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        }        
        break;
    case GL_LINEAR_MIPMAP_NEAREST:
        if (nMagFilter == GL_NEAREST)
        {
            pSamplerDesc->Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        }
        else
        {
            pSamplerDesc->Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        }        
        break;
    case GL_NEAREST_MIPMAP_LINEAR:
        if (nMagFilter == GL_NEAREST)
        {
            pSamplerDesc->Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        }
        else
        {
            pSamplerDesc->Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
        }        
        break;
    case GL_LINEAR_MIPMAP_LINEAR:
        if (nMagFilter == GL_NEAREST)
        {
            pSamplerDesc->Filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        }
        else
        {
            pSamplerDesc->Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        }        
        break;
    default:
        // Default
        pSamplerDesc->Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;       
        break;
    }
}
