//--------------------------------------------------------------------------------------
// File: FrmUserInterfaceD3D.h
// Desc: 
//
// Author:                 QUALCOMM, Adreno SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#ifndef _FRM_USER_INTERFACE_D3D_H_
#define _FRM_USER_INTERFACE_D3D_H_

#include "FrmUserInterface.h"
#include "Direct3D/FrmShader.h"
#include "Direct3D/FrmUtilsD3D.h"

//--------------------------------------------------------------------------------------
// Name: class CFrmUserInterfaceD3D
// Desc: Represents a sample's user interface components.
//--------------------------------------------------------------------------------------
class CFrmUserInterfaceD3D : public CFrmUserInterface
{
public:
    CFrmUserInterfaceD3D();
    virtual ~CFrmUserInterfaceD3D();

    // Initialize the user interface
    virtual BOOL Initialize( CFrmFont* pFont, const CHAR* strHeading );

protected:
    // Render functions
    virtual VOID RenderFadedBackground();
    virtual VOID RenderOverlays();
    
    struct UIOverlayConstantBuffer
    {
        FRMVECTOR4 vScreenSize;
    };
    CFrmShaderProgramD3D m_OverlayShader;
    CFrmConstantBuffer* m_pOverlayConstantBuffer;
    UIOverlayConstantBuffer m_OverlayConstantBufferData;

    struct UIBackgroundConstantBuffer
    {
        FRMVECTOR4 vBackground;
    };    
    CFrmShaderProgramD3D m_BackgroundShader;
    CFrmConstantBuffer* m_pBackgroundConstantBuffer;
    UIBackgroundConstantBuffer m_BackgroundConstantBufferData;
    CFrmVertexBuffer* m_pBackgroundVertexBuffer;
    CFrmIndexBuffer* m_pBackgroundIndexBuffer;
    CFrmRenderTextureToScreenD3D m_ScreenRenderer;

    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_savedDepthStencilState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_savedBlendState;

    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;

};


#endif // _FRM_USER_INTERFACE_D3D_H_

