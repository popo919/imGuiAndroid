//--------------------------------------------------------------------------------------
// File: FrmShader.cpp
// Desc: 
//
// Author:                 QUALCOMM, Adreno SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include "FrmPlatform.h"
#include "FrmShader.h"
#include "FrmResource.h"
#include "FrmUtilsD3D.h"


//--------------------------------------------------------------------------------------
// Name: CFrmShaderProgramD3D()
// Desc: Constructor
//--------------------------------------------------------------------------------------
CFrmShaderProgramD3D::CFrmShaderProgramD3D(): 
    m_vertexShader(nullptr),
    m_pixelShader(nullptr),
    m_inputLayout(nullptr),
    m_vertexShaderByteCode(NULL),
    m_vertexShaderByteCodeSize(0),
    m_pCachedVertexElements(NULL),
    m_nCachedVertexSize(0)
{
}

//--------------------------------------------------------------------------------------
// Name: ~CFrmShaderProgramD3D()
// Desc: Destructor
//--------------------------------------------------------------------------------------
CFrmShaderProgramD3D::~CFrmShaderProgramD3D()
{
    if (m_vertexShaderByteCode)
    {
        delete [] m_vertexShaderByteCode;
        m_vertexShaderByteCode = NULL;
    }
}

//--------------------------------------------------------------------------------------
// Name: Compile
// Desc: Given vertex and pixel shader byte code (from fxc), compiles the VS/PS shaders.
// Also creates IAVertexLayout based on FRM_SHADER_ATTRIBUTE list
//--------------------------------------------------------------------------------------
BOOL CFrmShaderProgramD3D::Compile( const CHAR* strVertexShaderFileName,
                                 const CHAR* strFragmentShaderFileName,
                                 const FRM_SHADER_ATTRIBUTE* pAttributes,
                                 UINT32 nNumAttributes )
{

    // Load vertex shader byte code
    CHAR* strVertexShaderData;
    UINT32 vsSize;
    if ( FALSE == FrmLoadFile( strVertexShaderFileName, (VOID**)&strVertexShaderData, &vsSize ) )
    {
        FrmLogMessage("ERROR: Could not load shader file '", strVertexShaderFileName, "\n" );
        return FALSE;
    }

    // Load pixel shader byte code
    CHAR* strFragmentShaderData;
    UINT32 fsSize;
    if ( FALSE == FrmLoadFile( strFragmentShaderFileName, (VOID**)&strFragmentShaderData, &fsSize) )
    {
        FrmLogMessage("ERROR: Could not load shader file '", strFragmentShaderFileName, "\n" );
        return FALSE;
    }

    // Create the vertex shader from bytecode
    if (FAILED(D3DDevice()->CreateVertexShader(
            strVertexShaderData,
            vsSize,
            nullptr,
            &m_vertexShader) ) )
    {
        delete [] strVertexShaderData;
        delete [] strFragmentShaderData;
        FrmLogMessage("ERROR: Failed to create vertex shader from '", strVertexShaderFileName, "\n" );
        return FALSE;
    }
     
    // Create the pixel shader from bytecode
    if (FAILED(D3DDevice()->CreatePixelShader(
            strFragmentShaderData,
            fsSize,
            nullptr,
            &m_pixelShader) ) )
    {
        delete [] strVertexShaderData;
        delete [] strFragmentShaderData;
        FrmLogMessage("ERROR: Failed to create pixel shader from '", strFragmentShaderFileName, "\n" );
        return FALSE;
    }

    // Store the vertex shader byte code since it is needed for binding attributes
    m_vertexShaderByteCode = (BYTE*) strVertexShaderData;
    m_vertexShaderByteCodeSize = vsSize;
    delete [] strFragmentShaderData;

    // Setup the IAVertexLayout
    if (FALSE == BindShaderAttributes(pAttributes, nNumAttributes))
    {
        delete [] m_vertexShaderByteCode;
        FrmLogMessage("ERROR: Failed to bind shader attributes for '", strFragmentShaderFileName, "\n" );
        return FALSE;
    }

    return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: Bind
// Desc: Bind the Vertex Shader/Pixel Shader and input layout
//--------------------------------------------------------------------------------------
BOOL CFrmShaderProgramD3D::Bind()
{
    if (m_vertexShader == nullptr || m_pixelShader == nullptr || m_inputLayout == nullptr)
    {
        return FALSE;
    }

    D3DDeviceContext()->IASetInputLayout(m_inputLayout.Get());

    D3DDeviceContext()->VSSetShader(
        m_vertexShader.Get(),
        nullptr,
        0);

    D3DDeviceContext()->PSSetShader(
        m_pixelShader.Get(),
        nullptr,
        0);

    return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: BindShaderAttributes
// Desc: Use the FRM_SHADER_ATTRIBUTE list to create a D3D11InputLayout
//--------------------------------------------------------------------------------------
BOOL CFrmShaderProgramD3D::BindShaderAttributes( const FRM_SHADER_ATTRIBUTE* pAttributes,
                                              UINT32 nNumAttributes )
{
    static UINT32 semanticIndices[] =
    {
        0,   //  FRM_VERTEX_POSITION,          // 0
        0,   //  FRM_VERTEX_BONEINDICES,       // 1
        0,   //  FRM_VERTEX_BONEWEIGHTS,       // 2
        0,   //  FRM_VERTEX_NORMAL,            // 3
        0,   //  FRM_VERTEX_TANGENT,           // 4
        0,   //  FRM_VERTEX_BINORMAL,          // 5
        0,   //  FRM_VERTEX_TEXCOORD0,         // 6
        1,   //  FRM_VERTEX_TEXCOORD1,         // 7
        2,   //  FRM_VERTEX_TEXCOORD2,         // 8
        3,   //  FRM_VERTEX_TEXCOORD3,         // 9
        4,   //  FRM_VERTEX_TEXCOORD4,         // 10
        5,   //  FRM_VERTEX_TEXCOORD5,         // 11
        0,   //  FRM_VERTEX_COLOR0,            // 12
        1,   //  FRM_VERTEX_COLOR1,            // 13
        2,   //  FRM_VERTEX_COLOR2,            // 14
        3    //  FRM_VERTEX_COLOR3,            // 15
    };

    BOOL result;
    // If specific attributes were supplied, use those
    if (nNumAttributes)
    {
        D3D11_INPUT_ELEMENT_DESC* vertexLayoutDesc = new D3D11_INPUT_ELEMENT_DESC[nNumAttributes];
        for ( UINT32 i = 0; i < nNumAttributes; i++)
        {
            ZeroMemory(&vertexLayoutDesc[i], sizeof(D3D11_INPUT_ELEMENT_DESC));            
            vertexLayoutDesc[i].SemanticName = pAttributes[i].strName;
            vertexLayoutDesc[i].SemanticIndex = semanticIndices[pAttributes[i].nLocation];
            vertexLayoutDesc[i].Format = pAttributes[i].format;
            vertexLayoutDesc[i].InputSlot = 0;
            vertexLayoutDesc[i].AlignedByteOffset = (i == 0) ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
            vertexLayoutDesc[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
            vertexLayoutDesc[i].InstanceDataStepRate = 0;
        }

        result = !FAILED(
            D3DDevice()->CreateInputLayout(
                vertexLayoutDesc,
                nNumAttributes,
                m_vertexShaderByteCode,
                m_vertexShaderByteCodeSize,
                &m_inputLayout) );
        
        delete [] vertexLayoutDesc;
    }
    else
    {                
        // Otherwise, do not create the input layout and assume it will be done later (e.g., with use of FrmMesh).
        // This differs from the GL implementation because we can't create a default input layout without knowing
        // about the declarations in the shader.
        result = TRUE;
    }

    return result;
}

//--------------------------------------------------------------------------------------
// Name: GLVertexFormatToD3D()
// Desc: Convert the Size/Type GL layout information stored in FRM_VERTEX_ELEMENT to
// the corresponding D3D format.
//--------------------------------------------------------------------------------------
DXGI_FORMAT GLVertexFormatToD3D( UINT8 nSize, UINT16 nType )
{
// TODO: Should this be here? 
#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_FIXED                          0x140C

    switch ( nType )
    {
    case GL_BYTE:
        if (nSize == 1)
            return DXGI_FORMAT_R8_SNORM;
        else if (nSize == 2)
            return DXGI_FORMAT_R8G8_SNORM;
        else if (nSize == 4)
            return DXGI_FORMAT_R8G8B8A8_SNORM;
        break;
    case GL_UNSIGNED_BYTE:
        if (nSize == 1)
            return DXGI_FORMAT_R8_UNORM;
        else if (nSize == 2)
            return DXGI_FORMAT_R8G8_UNORM;
        else if (nSize == 4)
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        break;
    case GL_SHORT:
        if (nSize == 1)
            return DXGI_FORMAT_R16_SNORM;
        else if (nSize == 2)
            return DXGI_FORMAT_R16G16_SNORM;
        else if (nSize == 4)
            return DXGI_FORMAT_R16G16B16A16_SNORM;
        break;
    case GL_UNSIGNED_SHORT:
        if (nSize == 1)
            return DXGI_FORMAT_R16_UNORM;
        else if (nSize == 2)
            return DXGI_FORMAT_R16G16_UNORM;
        else if (nSize == 4)
            return DXGI_FORMAT_R16G16B16A16_UNORM;
        break;
    case GL_INT:
        if (nSize == 1)
            return DXGI_FORMAT_R32_SINT;
        else if (nSize == 2)
            return DXGI_FORMAT_R32G32_SINT;
        else if (nSize == 3)
            return DXGI_FORMAT_R32G32B32_SINT;
        else if (nSize == 4)
            return DXGI_FORMAT_R32G32B32A32_SINT;
        break;
    case GL_UNSIGNED_INT:
        if (nSize == 1)
            return DXGI_FORMAT_R32_UINT;
        else if (nSize == 2)
            return DXGI_FORMAT_R32G32_UINT;
        else if (nSize == 3)
            return DXGI_FORMAT_R32G32B32_UINT;
        else if (nSize == 4)
            return DXGI_FORMAT_R32G32B32A32_UINT;
        break;
    case GL_FLOAT:
        if (nSize == 1)
            return DXGI_FORMAT_R32_FLOAT;
        else if (nSize == 2)
            return DXGI_FORMAT_R32G32_FLOAT;
        else if (nSize == 3)
            return DXGI_FORMAT_R32G32B32_FLOAT;
        else if (nSize == 4)
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        break;
    }

    FrmAssert( 0 ); // Unsupported D3D format
    return DXGI_FORMAT_UNKNOWN;
}
   
//--------------------------------------------------------------------------------------
// Name: SetVertexLayout()
// Desc: Use the FRM_VERTEX_ELEMENT description (rather than FRM_SHADER_ATTRIBUTE) to
// create the input layout
//--------------------------------------------------------------------------------------
VOID CFrmShaderProgramD3D::SetVertexLayout( FRM_VERTEX_ELEMENT* pVertexElements,
                                         UINT32 nVertexSize, VOID* pBaseOffset )
{
    // Early-out if nothing has changed
    if ( pVertexElements == m_pCachedVertexElements &&
         nVertexSize == m_nCachedVertexSize )
    {
         return;
    }

    m_nCachedVertexSize = nVertexSize;
    m_pCachedVertexElements = pVertexElements;

    static const CHAR* semanticNames[] =
    {    
        "POSITION",     //  FRM_VERTEX_POSITION,          // 0
        "BONEINICES",   //  FRM_VERTEX_BONEINDICES,       // 1
        "BONEWEIGHTS",  //  FRM_VERTEX_BONEWEIGHTS,       // 2
        "NORMAL",       //  FRM_VERTEX_NORMAL,            // 3
        "TANGENT",      //  FRM_VERTEX_TANGENT,           // 4
        "BINORMAL",     //  FRM_VERTEX_BINORMAL,          // 5
        "TEXCOORD",     //  FRM_VERTEX_TEXCOORD0,         // 6
        "TEXCOORD",     //  FRM_VERTEX_TEXCOORD1,         // 7
        "TEXCOORD",     //  FRM_VERTEX_TEXCOORD2,         // 8
        "TEXCOORD",     //  FRM_VERTEX_TEXCOORD3,         // 9
        "TEXCOORD",     //  FRM_VERTEX_TEXCOORD4,         // 10
        "TEXCOORD",     //  FRM_VERTEX_TEXCOORD5,         // 11
        "COLOR",        //  FRM_VERTEX_COLOR0,            // 12
        "COLOR",        //  FRM_VERTEX_COLOR1,            // 13
        "COLOR",        //  FRM_VERTEX_COLOR2,            // 14
        "COLOR"         //  FRM_VERTEX_COLOR3,            // 15
    };

    static UINT32 semanticIndices[] =
    {
        0,   //  FRM_VERTEX_POSITION,          // 0
        0,   //  FRM_VERTEX_BONEINDICES,       // 1
        0,   //  FRM_VERTEX_BONEWEIGHTS,       // 2
        0,   //  FRM_VERTEX_NORMAL,            // 3
        0,   //  FRM_VERTEX_TANGENT,           // 4
        0,   //  FRM_VERTEX_BINORMAL,          // 5
        0,   //  FRM_VERTEX_TEXCOORD0,         // 6
        1,   //  FRM_VERTEX_TEXCOORD1,         // 7
        2,   //  FRM_VERTEX_TEXCOORD2,         // 8
        3,   //  FRM_VERTEX_TEXCOORD3,         // 9
        4,   //  FRM_VERTEX_TEXCOORD4,         // 10
        5,   //  FRM_VERTEX_TEXCOORD5,         // 11
        0,   //  FRM_VERTEX_COLOR0,            // 12
        1,   //  FRM_VERTEX_COLOR1,            // 13
        2,   //  FRM_VERTEX_COLOR2,            // 14
        3    //  FRM_VERTEX_COLOR3,            // 15
    };


    D3D11_INPUT_ELEMENT_DESC* vertexLayoutDesc = new D3D11_INPUT_ELEMENT_DESC[nVertexSize];
    UINT32 nOffset = (UINT32)pBaseOffset;

    UINT32 i = 0;
    while( pVertexElements->nSize )
    {
        ZeroMemory(&vertexLayoutDesc[i], sizeof(D3D11_INPUT_ELEMENT_DESC));            
        vertexLayoutDesc[i].SemanticName = semanticNames[ pVertexElements->nUsage ];
        vertexLayoutDesc[i].SemanticIndex = semanticIndices[ pVertexElements->nUsage ];
        vertexLayoutDesc[i].Format = GLVertexFormatToD3D( pVertexElements->nSize, pVertexElements->nType );
        vertexLayoutDesc[i].InputSlot = 0;
        vertexLayoutDesc[i].AlignedByteOffset = nOffset;
        vertexLayoutDesc[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        vertexLayoutDesc[i].InstanceDataStepRate = 0;

        i++;
        nOffset+= pVertexElements->nStride;
        pVertexElements++;
    }

    D3DDevice()->CreateInputLayout(
                vertexLayoutDesc,
                i,
                m_vertexShaderByteCode,
                m_vertexShaderByteCodeSize,
                &m_inputLayout);

    delete [] vertexLayoutDesc;
}