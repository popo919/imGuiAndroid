//--------------------------------------------------------------------------------------
// File: FrmApplication_Platform.cpp
// Desc: WinRT implementation of the Framework application
//
// Author:                 QUALCOMM, Adreno SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include "FrmPlatform.h"
#include "FrmApplication.h"
#include "FrmUtils.h"
#include <agile.h>
#include <ppltasks.h>
#include <stdio.h>
#include <crtdbg.h>

using namespace Microsoft::WRL;
using namespace Windows::UI::Core;
using namespace Windows::Foundation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::System;
using namespace Windows::UI::Input;
using namespace Windows::UI::Popups;
using namespace Concurrency;

static ComPtr<ID3D11Device1> g_d3dDevice = nullptr;
static ComPtr<ID3D11DeviceContext1> g_d3dDeviceContext = nullptr;

//--------------------------------------------------------------------------------------
// Name: class CFrmAppContainer()
// Desc: Application class container
//--------------------------------------------------------------------------------------
ref class CFrmAppContainer sealed : public IFrameworkView
{
public:

    //--------------------------------------------------------------------------------------
    // Name: CFrmAppContainer()
    // Desc: Constructor
    //--------------------------------------------------------------------------------------
    CFrmAppContainer()
    {
        m_pApplication   = NULL;
    }

    //--------------------------------------------------------------------------------------
    // Name: ~CFrmAppContainer()
    // Desc: Destroys the application's window, etc.
    //--------------------------------------------------------------------------------------
    virtual ~CFrmAppContainer()
    {
        if( m_pApplication )
        {
            delete m_pApplication;
        }
    }
    
    //--------------------------------------------------------------------------------------    
	// Begin Implementation of IFrameworkVIew
    //--------------------------------------------------------------------------------------
    
    //--------------------------------------------------------------------------------------
    // Name: Initialize()
	// Desc: This method is called on application launch.
    //--------------------------------------------------------------------------------------    
    virtual void Initialize( _In_ CoreApplicationView^ applicationView )
    {
        m_pApplication = FrmCreateApplicationInstance();
        if( NULL == m_pApplication )
            return;

        applicationView->Activated +=
            ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &CFrmAppContainer::OnActivated);

    }

    //--------------------------------------------------------------------------------------    
	// Name: SetWindow()
    // Desc: This method is called after Initialize.
    //--------------------------------------------------------------------------------------    
    virtual void SetWindow( _In_ CoreWindow^ window )
    {
        m_window = window;

        // Specify the cursor type as the standard arrow cursor.
        m_window->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
        
        // Allow the application to respond when the window size changes.
        m_window->SizeChanged +=
            ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(
                this,
                &CFrmAppContainer::OnWindowSizeChanged
                );

        m_window->Closed += 
            ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &CFrmAppContainer::OnWindowClosed);

        m_window->KeyDown += 
            ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &CFrmAppContainer::OnKeyDown);
        
        m_window->KeyDown += 
            ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &CFrmAppContainer::OnKeyUp);
        
        m_window->PointerPressed +=
            ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &CFrmAppContainer::OnPointerPressed);

        m_window->PointerMoved +=
            ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &CFrmAppContainer::OnPointerMoved);

        m_window->PointerReleased +=
            ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &CFrmAppContainer::OnPointerReleased);        

    }

    //--------------------------------------------------------------------------------------    
	// Name: Load()
    // Desc: This method is called on load
    //--------------------------------------------------------------------------------------        
    virtual void Load(_In_ Platform::String^ entryPoint)
    {
    }

    //--------------------------------------------------------------------------------------    
    // Name: Run()
    // Desc: This method is called after Load and contains the main loop of the program.
    //--------------------------------------------------------------------------------------    
    virtual void Run()
    {
        // First, create the Direct3D device.

        // This flag is required in order to enable compatibility with Direct2D.
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
        // If the project is in a debug build, enable debugging via SDK Layers with this flag.
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        // This array defines the ordering of feature levels that D3D should attempt to create.
        D3D_FEATURE_LEVEL featureLevels[] = 
        {            
            D3D_FEATURE_LEVEL_9_3
        };

        ComPtr<ID3D11Device> d3dDevice;
        ComPtr<ID3D11DeviceContext> d3dDeviceContext;
        ThrowIfFailed(
            D3D11CreateDevice(
                nullptr,                    // specify nullptr to use the default adapter
                D3D_DRIVER_TYPE_HARDWARE,
                nullptr,                    // leave as nullptr if hardware is used
                creationFlags,              // optionally set debug and Direct2D compatibility flags
                featureLevels,
                ARRAYSIZE(featureLevels),
                D3D11_SDK_VERSION,          // always set this to D3D11_SDK_VERSION
                &d3dDevice,
                nullptr,
                &d3dDeviceContext
                )
            );

        // Retrieve the Direct3D 11.1 interfaces.
        ThrowIfFailed(
            d3dDevice.As(&m_d3dDevice)
            );
            
        
        ThrowIfFailed(
            d3dDeviceContext.As(&m_d3dDeviceContext)
            );

        // Store a global pointer to the D3D Device/DeviceContext
        g_d3dDevice = m_d3dDevice;
        g_d3dDeviceContext = m_d3dDeviceContext;
        

        // After the D3D device is created, create additional application resources.
        CreateWindowSizeDependentResources();

        if( ( FALSE == m_pApplication->Initialize() ) ||
            ( FALSE == m_pApplication->Resize() ) )
        {
            m_bInitialized = false;
            auto msgDlg = 
                ref new MessageDialog("Application failed during scene initialization!  Please debug accordingly.");
            task<IUICommand^>(msgDlg->ShowAsync())
                .then([this](IUICommand^ command)
            {
                m_bRunning = false;
            });
        }
        else
        {
            m_bInitialized = true;
        }

        // Enter the render loop.  Note that Metro style apps should never exit.
        m_bRunning = true;
        while (m_bRunning)
        {
            // Process events incoming to the window.
            m_window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

			// Update and render the application
            if (m_bInitialized == true)
            {
			    m_pApplication->Update();
			    m_pApplication->Render();
            }

            // Present the rendered image to the window.  Because the maximum frame latency is set to 1,
            // the render loop will generally be throttled to the screen refresh rate, typically around
            // 60Hz, by sleeping the application on Present until the screen is refreshed.
            ThrowIfFailed(
                m_swapChain->Present(1, 0)
                );
        }        
    }

    //--------------------------------------------------------------------------------------    
    // Name: Uninitialize()
    // Desc: This method is called before the application exits.
    //--------------------------------------------------------------------------------------    
    virtual void Uninitialize()
    {
        m_pApplication->Destroy();
    }

    //--------------------------------------------------------------------------------------    
    // End Implementation of IFrameworkVIew
    //--------------------------------------------------------------------------------------    
    
    
	
	
public:

    BOOL HandleEvents();
    
    //--------------------------------------------------------------------------------------
    // Name: ToggleOrientation()
    // Desc: Toggle the window orientation between portrait and landscape mode
    //--------------------------------------------------------------------------------------
    VOID ToggleOrientation()
    {
        // Switch orientation by swapping the width and height
        INT32 nOldWidth  = m_pApplication->m_nWidth;
        INT32 nOldHeight = m_pApplication->m_nHeight;
        INT32 nNewWidth  = nOldHeight;
        INT32 nNewHeight = nOldWidth;

        // TODO: Not implemented correctly yet
        
        // Update the app variables
        m_pApplication->m_nWidth  = nNewWidth;
        m_pApplication->m_nHeight = nNewHeight;

        // Update the OpenGL viewport
        // Note: The derived app class should do this during the Render() function!
        m_pApplication->Resize();
    }

        
private:

    //--------------------------------------------------------------------------------------    
    // Name: ThrowIfFailed
    // Desc: Utility function to throw exception on failure.  We need to throw because
    // the IFrameworkView initialization does not allow return codes
    //--------------------------------------------------------------------------------------    
    void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            // Set a breakpoint on this line to catch DX API errors.
            throw Platform::Exception::CreateException(hr);
        }
    }

    //--------------------------------------------------------------------------------------    
	// Name: OnActivated()
    // Desc: Callback for CoreApplicationView::Activated
    //--------------------------------------------------------------------------------------    
	void OnActivated( _In_ CoreApplicationView^ applicationView, _In_ IActivatedEventArgs^ args )
    {
        // Activate the application window, making it visible and enabling it to receive events.
        CoreWindow::GetForCurrentThread()->Activate();
    }

    //--------------------------------------------------------------------------------------    
	// Name: OnWindowSizeChanged()
    // Desc: Callback for CoreWindow::SizeChanged
    //--------------------------------------------------------------------------------------    	    
    void OnWindowSizeChanged( _In_ CoreWindow^ sender, _In_ WindowSizeChangedEventArgs^ args )
    {    
        m_renderTargetView = nullptr;
        m_depthStencilView = nullptr;
        CreateWindowSizeDependentResources();
    }

    //--------------------------------------------------------------------------------------    
	// Name: OnKeyDown()
    // Desc: Callback for CoreWindow::KeyDown
    //--------------------------------------------------------------------------------------    	        
    void OnKeyDown( _In_ CoreWindow^ /* sender */, _In_ KeyEventArgs^ args)
    {                
        HandleKey(args, false);
    }

    //--------------------------------------------------------------------------------------    
	// Name: OnKeyUp()
    // Desc: Callback for CoreWindow::KeyUp
    //--------------------------------------------------------------------------------------    	            
    void OnKeyUp( _In_ CoreWindow^ /* sender */, _In_ KeyEventArgs^ args)
    {
        HandleKey(args, true);
    }

    //--------------------------------------------------------------------------------------    
	// Name: HandleKey()
    // Desc: Handle key press/release events
    //--------------------------------------------------------------------------------------    	            
    void HandleKey( _In_ KeyEventArgs^ args, bool keyUp )
    {
        Windows::System::VirtualKey Key = args->VirtualKey;
        
        BOOL bShiftKeyState = bool( m_window->GetAsyncKeyState( VirtualKey::Shift) & CoreVirtualKeyStates::Down );
        BOOL bCtrlKeyState  = bool( m_window->GetAsyncKeyState( VirtualKey::Control) & CoreVirtualKeyStates::Down );

        if ( Key == VirtualKey::Escape )
        {
            m_bRunning = false;
        }

        if (!keyUp)
        {
            if( bCtrlKeyState && Key == VirtualKey::C )
            {
            
                // TODO: Copy framebuffer to clipboard
            }
        }

        // Handle remaining keys
        // Note: some mappings may only apply to US keyboards
        UINT32 nButtonMask = 0;
        switch( Key )
        {
        case VirtualKey::Number1:         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_1; break;
        case VirtualKey::Number2:         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_2; break;
        case VirtualKey::Number3:         nButtonMask = bShiftKeyState ? FRM_INPUT::KEY_HASH : FRM_INPUT::KEY_3; break;
        case VirtualKey::Number4:         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_4; break;
        case VirtualKey::Number5:         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_5; break;
        case VirtualKey::Number6:         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_6; break;
        case VirtualKey::Number7:         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_7; break;
        case VirtualKey::Number8:         nButtonMask = bShiftKeyState ? FRM_INPUT::KEY_STAR : FRM_INPUT::KEY_8; break;
        case VirtualKey::Number9:         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_9; break;
        case VirtualKey::Number0:         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_0; break;
        case VirtualKey::NumberPad1:      nButtonMask = FRM_INPUT::KEY_1; break;
        case VirtualKey::NumberPad2:      nButtonMask = FRM_INPUT::KEY_2; break;
        case VirtualKey::NumberPad3:      nButtonMask = FRM_INPUT::KEY_3; break;
        case VirtualKey::NumberPad4:      nButtonMask = FRM_INPUT::KEY_4; break;
        case VirtualKey::NumberPad5:      nButtonMask = FRM_INPUT::KEY_5; break;
        case VirtualKey::NumberPad6:      nButtonMask = FRM_INPUT::KEY_6; break;
        case VirtualKey::NumberPad7:      nButtonMask = FRM_INPUT::KEY_7; break;
        case VirtualKey::NumberPad8:      nButtonMask = FRM_INPUT::KEY_8; break;
        case VirtualKey::NumberPad9:      nButtonMask = FRM_INPUT::KEY_9; break;
        case VirtualKey::NumberPad0:      nButtonMask = FRM_INPUT::KEY_0; break;
        case VirtualKey::Multiply:        nButtonMask = FRM_INPUT::KEY_STAR;   break;
        case VirtualKey::Left:            nButtonMask = FRM_INPUT::DPAD_LEFT;  break;
        case VirtualKey::Right:           nButtonMask = FRM_INPUT::DPAD_RIGHT; break;
        case VirtualKey::Up:              nButtonMask = FRM_INPUT::DPAD_UP;    break;
        case VirtualKey::Down:            nButtonMask = FRM_INPUT::DPAD_DOWN;  break;
        case VirtualKey::Space:           nButtonMask = FRM_INPUT::SELECT;     break;
        case VirtualKey::Enter:           nButtonMask = FRM_INPUT::SELECT;     break;
        case VirtualKey::Back:            nButtonMask = FRM_INPUT::KEY_CLEAR;  break;
        case VirtualKey::Delete:          nButtonMask = FRM_INPUT::KEY_CLEAR;  break;
        case VirtualKey::Insert:          nButtonMask = FRM_INPUT::KEY_EDIT;   break;
        }

        if( nButtonMask )
        {
            if( !keyUp )
                m_pApplication->m_Input.m_nButtons |= nButtonMask;
            else
                m_pApplication->m_Input.m_nButtons &= ~nButtonMask;
        }

        // Toggle the window orientation between portrait and landscape mode
        if( !keyUp && nButtonMask == FRM_INPUT::KEY_STAR )
        {
            // TODO: Toggle orientation
            ToggleOrientation();    
        }

        // Update application so keyboard state is processed
        m_pApplication->Update();
    }

    //--------------------------------------------------------------------------------------    
	// Name: OnPointerPressed()
    // Desc: Callback for CoreWindow::PointerPressed
    //--------------------------------------------------------------------------------------    	            
    void OnPointerPressed( _In_ CoreWindow^ /* sender */, _In_ PointerEventArgs^ args )
    {
        PointerPoint^ point = args->CurrentPoint;
        uint32 pointerID = point->PointerId;
        Point pointerPosition = point->Position;
        PointerPointProperties^ pointProperties = point->Properties;
        
        bool rightButton = pointProperties->IsRightButtonPressed;
        bool leftButton = pointProperties->IsLeftButtonPressed;

        if (leftButton)
        {
            FRMVECTOR2 vPointerPosition = FRMVECTOR2(pointerPosition.X, pointerPosition.Y);
            vPointerPosition.x = +2.0f * vPointerPosition.x/(FLOAT32)(m_pApplication->m_nWidth -1) - 1.0f;
            vPointerPosition.y = -2.0f * vPointerPosition.y/(FLOAT32)(m_pApplication->m_nHeight-1) + 1.0f;

            m_pApplication->m_Input.m_vPointerPosition = vPointerPosition;
            m_pApplication->m_Input.m_nPointerState  = FRM_INPUT::POINTER_DOWN;
            m_pApplication->m_Input.m_nPointerState |= FRM_INPUT::POINTER_PRESSED;
        }
    }

    //--------------------------------------------------------------------------------------    
	// Name: OnPointerMoved()
    // Desc: Callback for CoreWindow::PointerMoved
    //--------------------------------------------------------------------------------------    	            
    void OnPointerMoved( _In_ CoreWindow^ /* sender */, _In_ PointerEventArgs^ args )
    {
        PointerPoint^ point = args->CurrentPoint;
        uint32 pointerID = point->PointerId;
        Point pointerPosition = point->Position;
        PointerPointProperties^ pointProperties = point->Properties;
        
        bool rightButton = pointProperties->IsRightButtonPressed;
        bool leftButton = pointProperties->IsLeftButtonPressed;

        if (leftButton)
        {
            FRMVECTOR2 vPointerPosition = FRMVECTOR2(pointerPosition.X, pointerPosition.Y);
            vPointerPosition.x = +2.0f * vPointerPosition.x/(FLOAT32)(m_pApplication->m_nWidth -1) - 1.0f;
            vPointerPosition.y = -2.0f * vPointerPosition.y/(FLOAT32)(m_pApplication->m_nHeight-1) + 1.0f;

            m_pApplication->m_Input.m_vPointerPosition = vPointerPosition;
            m_pApplication->m_Input.m_nPointerState  = FRM_INPUT::POINTER_DOWN;
        }            
    }

    //--------------------------------------------------------------------------------------    
	// Name: OnPointerReleased()
    // Desc: Callback for CoreWindow::PointerReleased
    //--------------------------------------------------------------------------------------    	            
    void OnPointerReleased( _In_ CoreWindow^ /* sender */, _In_ PointerEventArgs^ args )
    {
        PointerPoint^ point = args->CurrentPoint;
        uint32 pointerID = point->PointerId;
        Point pointerPosition = point->Position;
        PointerPointProperties^ pointProperties = point->Properties;
        
        bool rightButton = pointProperties->IsRightButtonPressed;
        bool leftButton = pointProperties->IsLeftButtonPressed;

        if (leftButton)
        {
            FRMVECTOR2 vPointerPosition = FRMVECTOR2(pointerPosition.X, pointerPosition.Y);
            vPointerPosition.x = +2.0f * vPointerPosition.x/(FLOAT32)(m_pApplication->m_nWidth -1) - 1.0f;
            vPointerPosition.y = -2.0f * vPointerPosition.y/(FLOAT32)(m_pApplication->m_nHeight-1) + 1.0f;

            m_pApplication->m_Input.m_vPointerPosition = vPointerPosition;
            m_pApplication->m_Input.m_nPointerState = FRM_INPUT::POINTER_RELEASED;
        }
    }
    //--------------------------------------------------------------------------------------    
	// Name: CreateWindowSizeDependentResources()
    // Desc: This method creates all application resources that depend on
    // the application window size.  It is called at app initialization,
    // and whenever the application window size changes.
    //--------------------------------------------------------------------------------------    	
    void CreateWindowSizeDependentResources()
    {
        if (m_swapChain != nullptr)
        {
            // If the swap chain already exists, resize it.
            ThrowIfFailed(
                m_swapChain->ResizeBuffers(
                    2,
                    0,
                    0,
                    DXGI_FORMAT_B8G8R8A8_UNORM,
                    0
                    )
                );
        }
        else
        {
            // If the swap chain does not exist, create it.
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};

            swapChainDesc.Stereo = false;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.Scaling = DXGI_SCALING_NONE;
            swapChainDesc.Flags = 0;

            // Use automatic sizing.
            swapChainDesc.Width = 0;
            swapChainDesc.Height = 0;

            // This is the most common swap chain format.
            swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;

            // Don't use multi-sampling.
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;

            // Use two buffers to enable flip effect.
            swapChainDesc.BufferCount = 2;

            // We recommend using this swap effect for all applications.
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;            


            // Once the swap chain description is configured, it must be
            // created on the same adapter as the existing D3D Device.

            // First, retrieve the underlying DXGI Device from the D3D Device.
            ComPtr<IDXGIDevice2> dxgiDevice;
            ThrowIfFailed(
                m_d3dDevice.As(&dxgiDevice)
                );

            // Ensure that DXGI does not queue more than one frame at a time. This both reduces 
            // latency and ensures that the application will only render after each VSync, minimizing 
            // power consumption.
            ThrowIfFailed(
                dxgiDevice->SetMaximumFrameLatency(1)
                );

            // Next, get the parent factory from the DXGI Device.
            ComPtr<IDXGIAdapter> dxgiAdapter;
            ThrowIfFailed(
                dxgiDevice->GetAdapter(&dxgiAdapter)
                );

            ComPtr<IDXGIFactory2> dxgiFactory;
            ThrowIfFailed(
                dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory))
                );

            // Finally, create the swap chain.
            CoreWindow^ window = m_window.Get();
            ThrowIfFailed(
                dxgiFactory->CreateSwapChainForCoreWindow(
                    m_d3dDevice.Get(),
                    reinterpret_cast<IUnknown*>(window),
                    &swapChainDesc,
                    nullptr, // allow on all displays
                    &m_swapChain
                    )
                );
        }

        // Once the swap chain is created, create a render target view.  This will
        // allow Direct3D to render graphics to the window.
        ComPtr<ID3D11Texture2D> backBuffer;
        ThrowIfFailed(
            m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))
            );

        ThrowIfFailed(
            m_d3dDevice->CreateRenderTargetView(
                backBuffer.Get(),
                nullptr,
                &m_renderTargetView
                )
            );


        // After the render target view is created, specify that the viewport,
        // which describes what portion of the window to draw to, should cover
        // the entire window.

        D3D11_TEXTURE2D_DESC backBufferDesc = {0};
        backBuffer->GetDesc(&backBufferDesc);

        D3D11_TEXTURE2D_DESC depthStencilDesc;
        depthStencilDesc.Width = backBufferDesc.Width;
        depthStencilDesc.Height = backBufferDesc.Height;
        depthStencilDesc.MipLevels = 1;
        depthStencilDesc.ArraySize = 1;
        depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilDesc.SampleDesc.Count = 1;
        depthStencilDesc.SampleDesc.Quality = 0;
        depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
        depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthStencilDesc.CPUAccessFlags = 0;
        depthStencilDesc.MiscFlags = 0;
        ComPtr<ID3D11Texture2D> depthStencil;
        ThrowIfFailed(
            m_d3dDevice->CreateTexture2D(
                &depthStencilDesc,
                nullptr,
                &depthStencil
                )
            );

        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
        depthStencilViewDesc.Format = depthStencilDesc.Format;
        depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        depthStencilViewDesc.Flags = 0;
        depthStencilViewDesc.Texture2D.MipSlice = 0;
        ThrowIfFailed(
            m_d3dDevice->CreateDepthStencilView(
                depthStencil.Get(),
                &depthStencilViewDesc,
                &m_depthStencilView
                )
            );

        // Set these for the application so that it can access them for restoring
        // the framebuffer
        m_pApplication->m_windowRenderTargetView = m_renderTargetView;
        m_pApplication->m_windowDepthStencilView = m_depthStencilView;


        D3D11_VIEWPORT viewport;
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(backBufferDesc.Width);
        viewport.Height = static_cast<float>(backBufferDesc.Height);
        viewport.MinDepth = D3D11_MIN_DEPTH;
        viewport.MaxDepth = D3D11_MAX_DEPTH;

        m_d3dDeviceContext->RSSetViewports(1, &viewport);

        m_pApplication->m_nWidth = (INT32)viewport.Width;
        m_pApplication->m_nHeight = (INT32)viewport.Height;
    }

    //--------------------------------------------------------------------------------------    
	// Name: OnWindowClosed()
    // Desc: Called when window is closed (callback)
    //--------------------------------------------------------------------------------------    	
    void OnWindowClosed( _In_ CoreWindow^ sender, _In_ CoreWindowEventArgs^ args )
    {
        m_bRunning = false;
    }


private:
    Platform::Agile<CoreWindow> m_window;
    ComPtr<IDXGISwapChain1> m_swapChain;
    ComPtr<ID3D11Device1> m_d3dDevice;
    ComPtr<ID3D11DeviceContext1> m_d3dDeviceContext;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    ComPtr<ID3D11DepthStencilView> m_depthStencilView;

    CFrmApplication*  m_pApplication;

    volatile bool m_bRunning;
    volatile bool m_bInitialized;

};

// This class defines how to create the custom View Provider defined above.
ref class Direct3DFrameworkViewSource : IFrameworkViewSource 
{
public:
    virtual IFrameworkView^ CreateView()
    {
        return ref new CFrmAppContainer();
    }
};

//--------------------------------------------------------------------------------------
// Global Reference to D3DDevice Pointer
//--------------------------------------------------------------------------------------
ComPtr<ID3D11Device> D3DDevice()
{
    FrmAssert(g_d3dDevice != nullptr);
    return g_d3dDevice;
}

ComPtr<ID3D11DeviceContext1> D3DDeviceContext()
{
    FrmAssert(g_d3dDeviceContext != nullptr);
    return g_d3dDeviceContext;
}

//--------------------------------------------------------------------------------------
// Name: WinMain()
// Desc: Application entry-point on the Windows platform
//--------------------------------------------------------------------------------------
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
    auto frameworkViewSource = ref new Direct3DFrameworkViewSource();
    CoreApplication::Run(frameworkViewSource);
    return 0;
}


//--------------------------------------------------------------------------------------
// Name: CopyFramebufferToClipboard()
// Desc: Helper function for taking screen captures
//--------------------------------------------------------------------------------------
#ifdef TEMP
VOID CFrmAppContainer::CopyFramebufferToClipboard()
{
    UINT32  nWidth  = m_pApplication->m_nWidth;
    UINT32  nHeight = m_pApplication->m_nHeight;

    HBITMAP hBitmap = CreateBitmap( nWidth, nHeight, 1, 32, NULL );
    BOOL bResult = eglCopyBuffers( m_pApplication->m_eglDisplay,
                                   m_pApplication->m_eglSurface, 
                                   hBitmap );

    OpenClipboard( GetDesktopWindow() );
    EmptyClipboard();
    SetClipboardData( CF_BITMAP, hBitmap );
    CloseClipboard();

    DeleteObject( hBitmap );
}
#endif
