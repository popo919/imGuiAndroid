//--------------------------------------------------------------------------------------
// File: FrmUtilsD3D.h
// Desc: 
//
// Author:                 QUALCOMM, Adreno SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#ifndef _FRM_UTILS_D3D_H_
#define _FRM_UTILS_D3D_H_

#include "FrmPlatform.h"
#include "FrmUtils.h"
#include "FrmMath.h"
#include "Direct3D/FrmResourceD3D.h"
#include "Direct3D/FrmShader.h"
#include "FrmStdLib.h"


//--------------------------------------------------------------------------------------
// Helper function to render a texture in screenspace
//--------------------------------------------------------------------------------------
class CFrmRenderTextureToScreenD3D
{
public:
    CFrmRenderTextureToScreenD3D();
    virtual ~CFrmRenderTextureToScreenD3D();

    VOID RenderTextureToScreen( FLOAT32 sx, FLOAT32 sy, FLOAT32 w, FLOAT32 h,
                                CFrmTexture* pTexture,
                                CFrmShaderProgramD3D* pShaderProgram, 
                                CFrmConstantBuffer* pConstantBuffer,
                                void *pConstantBufferData,
                                UINT32 nScreenSizeOffset );

    VOID RenderTextureToScreen( FLOAT32 sx, FLOAT32 sy, FLOAT32 fScale, 
                                CFrmTexture* pTexture,
                                CFrmShaderProgramD3D* pShaderProgram, 
                                CFrmConstantBuffer* pConstantBuffer,
                                void *pConstantBufferData,
                                UINT32 nScreenSizeOffset );


private:

    BOOL AllocateD3DResources();
    
    CFrmVertexBuffer* m_pVertexBuffer;
    CFrmIndexBuffer* m_pIndexBuffer;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_savedDepthStencilState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_savedBlendState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;

};

#endif // _FRM_UTILS_D3D_H_
