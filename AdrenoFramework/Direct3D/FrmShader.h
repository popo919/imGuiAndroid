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
#ifndef _FRM_SHADER_H_
#define _FRM_SHADER_H_

#include "FrmPlatform.h"
#include "FrmResourceD3D.h"

// Shader attribute type
struct FRM_SHADER_ATTRIBUTE
{
    const CHAR* strName;
    UINT32      nLocation;
    DXGI_FORMAT format;    
};


class CFrmShaderProgramD3D
{
public:
    CFrmShaderProgramD3D();
    virtual ~CFrmShaderProgramD3D();

    
    // Given vertex and pixel shader byte code (from fxc), compiles the VS/PS shaders.
    // Also creates IAVertexLayout based on FRM_SHADER_ATTRIBUTE list
    BOOL Compile( const CHAR* strVertexShaderFileName,
                  const CHAR* strFragmentShaderFileName, 
                  const FRM_SHADER_ATTRIBUTE* pAttributes = NULL,
                  UINT32 nNumAttributes = 0 );

    // Bind the Vertex Shader/Pixel Shader and input layout
    BOOL Bind();

    // Use the FRM_SHADER_ATTRIBUTE list to create a D3D11InputLayout
    BOOL BindShaderAttributes( const FRM_SHADER_ATTRIBUTE* pAttributes,
                               UINT32 nNumAttributes );

    // Use the FRM_VERTEX_ELEMENT description (rather than FRM_SHADER_ATTRIBUTE) to
    // create the input layout
    VOID SetVertexLayout( FRM_VERTEX_ELEMENT* pVertexElements,
                          UINT32 nVertexSize, VOID* pBaseOffset = NULL );
private:
    // Vertex Shader
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;

    // Pixel Shader
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;

    // Input Layout (for Input Assembler)
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

    // Retain the bytecode to the vertex shader for InputLayout creation
    BYTE* m_vertexShaderByteCode;

    // Size of vertex shader byte code
    UINT32 m_vertexShaderByteCodeSize;

    // Keep a pointer to the last call to SetVertexLayout so that we don't
    // create new layouts when nothing has changed
    FRM_VERTEX_ELEMENT *m_pCachedVertexElements;
    UINT32              m_nCachedVertexSize;

};

#endif // _FRM_SHADER_H_
