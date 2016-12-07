//--------------------------------------------------------------------------------------
// File: FrmComputeGraphicsApplication_Win32.cpp
// Desc: Win32 implementation of the Compute/Graphics Framework application
//
// Author:      QUALCOMM, Advanced Content Group - Snapdragon SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include "FrmPlatform.h"
#include "FrmComputeGraphicsApplication.h"
#include "FrmUtils.h"
#include <stdio.h>
#include <crtdbg.h>


//--------------------------------------------------------------------------------------
// Name: class CFrmAppContainer()
// Desc: Application class container
//--------------------------------------------------------------------------------------
class CFrmAppContainer
{
public:
    NativeWindowType  m_hWindow;
    NativeDisplayType m_hDisplay;
    CFrmComputeGraphicsApplication*  m_pApplication;

    BOOL CreateNativeWindow( NativeWindowType* pWindow, NativeDisplayType* pDisplay );
    VOID CopyFramebufferToClipboard();
    
public:
    CFrmAppContainer();
    ~CFrmAppContainer();

    BOOL Run( cl_device_type devicetype );
    BOOL HandleEvents();
    VOID ToggleOrientation();
};


//--------------------------------------------------------------------------------------
// Name: WinMain()
// Desc: Application entry-point on the Windows platform
//--------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
    cl_device_type deviceType = CL_DEVICE_TYPE_ALL;
	CFrmAppContainer appContainer;
	return (INT32)appContainer.Run(deviceType);
}


//--------------------------------------------------------------------------------------
// Name: Run()
// Desc: Create the framework, initialize the application, and render frames.
//--------------------------------------------------------------------------------------
BOOL CFrmAppContainer::Run( cl_device_type deviceType )
{
    // Use the executable's directory so that relative media paths work correctly
    {
        CHAR strFilePath[256];
        GetModuleFileName( NULL, strFilePath, 256 );
        *( strrchr( strFilePath, '\\' ) ) = '\0';
        SetCurrentDirectory( strFilePath );
    }

	// Create the Application
	m_pApplication = FrmCreateComputeGraphicsApplicationInstance();

    m_pApplication->m_bRunTests = TRUE;

	if( NULL == m_pApplication )
        return FALSE;

    // Create the application window
    if( FALSE == CreateNativeWindow( &m_hWindow, &m_hDisplay ) )
        return FALSE;

    // Create the render context
    if( FALSE == m_pApplication->CreateRenderContext( m_hWindow, m_hDisplay ) )
    {
        // Display the message log, which might indicate what failed
        const CHAR* strErrorMessage = FrmGetMessageLog();
        if( strErrorMessage && strErrorMessage[0] )
        {
            MessageBox( NULL, strErrorMessage, m_pApplication->m_strName, MB_ICONERROR );
        }
        else
        {
            MessageBox( NULL, "Application failed to create render context!\n"
                              "\n"
                              "Please debug accordingly.", 
                              m_pApplication->m_strName, MB_ICONERROR );
        }

        return FALSE;
    }

    if(strcmp(_CRT_STRINGIZE(PLATFORM), "OPENGLES") == 0)
    {
        eglBindAPI( EGL_OPENGL_ES_API );
        eglMakeCurrent( m_pApplication->m_eglDisplay, m_pApplication->m_eglSurface, m_pApplication->m_eglSurface, m_pApplication->m_eglContextGL );
    }

    if( FALSE == m_pApplication->CreateOpenCLContext( m_hWindow, deviceType ) )
    {
        return FALSE;
    }

    if( ( FALSE == m_pApplication->Initialize() ) ||
        ( FALSE == m_pApplication->Resize() ) )
    {
        MessageBox( NULL, "Application failed during scene initialization!\n"
                          "\n"
                          "Please debug accordingly.", 
                          m_pApplication->m_strName, MB_ICONERROR );
        return FALSE;
    }

    // Re-register the VK_SNAPSHOT hotkey so we can trap the PrtScn key for screen captures
    UnregisterHotKey( GetParent( (HWND)m_hWindow ), IDHOT_SNAPWINDOW );
    RegisterHotKey( GetParent( (HWND)m_hWindow ), IDHOT_SNAPWINDOW, 0, VK_SNAPSHOT );

    // Show the window
    ShowWindow( GetParent( (HWND)m_hWindow ), TRUE );
    ShowWindow( (HWND)m_hWindow, SW_SHOW );
    SetForegroundWindow( (HWND)m_hWindow );
    SetFocus( (HWND)m_hWindow );
    
    // Run the main loop
    while( HandleEvents() )
    {
        // Update and render the application
        m_pApplication->Update();
        m_pApplication->Render();

        // Present the scene
        m_pApplication->SwapDrawBuffers();
    }

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: CopyFramebufferToClipboard()
// Desc: Helper function for taking screen captures
//--------------------------------------------------------------------------------------
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


//--------------------------------------------------------------------------------------
// Name: WndProc()
// Desc: The application window's message proc
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    // Get the application data
    CFrmAppContainer* pAppContainer = (CFrmAppContainer*)GetWindowLong( hWnd, GWLP_USERDATA );

    switch( uMsg )
    {
        case WM_HOTKEY:
            // Handle screenshot captures, saving the framebuffer to a user-selected TGA file
            if( wParam == IDHOT_SNAPWINDOW )
            {
                CHAR strFileName[MAX_PATH] = "ScreenShot.tga";
                OPENFILENAME ofn = { sizeof(OPENFILENAME) };
                ofn.lpstrFilter = "Targa Image Files (*.tga)\0*.tga\0All Files (*.*)\0*.*\0";
                ofn.lpstrFile   = strFileName;
                ofn.nMaxFile    = MAX_PATH;
                ofn.Flags       = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
                ofn.lpstrDefExt = "tga";

                if( GetSaveFileName( &ofn ) )
                {
                    EGLDisplay eglDisplay = pAppContainer->m_pApplication->m_eglDisplay;
                    EGLSurface eglSurface = pAppContainer->m_pApplication->m_eglSurface;
                    UINT32     nWidth     = pAppContainer->m_pApplication->m_nWidth;
                    UINT32     nHeight    = pAppContainer->m_pApplication->m_nHeight;

                    HBITMAP hBitmap = CreateBitmap( nWidth, nHeight, 1, 32, NULL );
                    eglCopyBuffers( eglDisplay, eglSurface, hBitmap );

                    UINT32* pPixels = new UINT32[ nWidth * nHeight ];
                    HDC hDC = CreateCompatibleDC( NULL ); 
                    BITMAPINFO bmi = { sizeof(BITMAPINFOHEADER), nWidth, nHeight, 1, 32, BI_RGB };
                    GetDIBits( hDC, hBitmap, 0, nHeight, pPixels, &bmi, DIB_RGB_COLORS );
                    
                    FrmSaveImageAsTGA( strFileName, nWidth, nHeight, pPixels );

                    delete[] pPixels;
                    DeleteDC( hDC );
                    DeleteObject( hBitmap );
                }
            }
            break;

        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            // Ignore redundant messages from automatically repeated keystrokes
            if( ( uMsg == WM_KEYDOWN ) && ( HIWORD(lParam) & KF_REPEAT ) )
                return 0;

            BOOL bShiftKeyState = ( GetKeyState( VK_SHIFT )   & 0x8000 ) ? TRUE : FALSE;
            BOOL bCtrlKeyState  = ( GetKeyState( VK_CONTROL ) & 0x8000 ) ? TRUE : FALSE;

            // Handle the ESC key
            if( wParam == VK_ESCAPE )
            {
                PostQuitMessage( 0 );
                return 0;
            }

            // Handle Ctrl+C, to copy framebuffer contents to the clipboard
            if( uMsg == WM_KEYDOWN )
            {
                if( bCtrlKeyState && wParam == 'C' )
                {
                    PrintWindow( hWnd, GetWindowDC( hWnd ), PW_CLIENTONLY );

                    pAppContainer->CopyFramebufferToClipboard();
                }
            }

            // Handle remaining keys
            // Note: some mappings may only apply to US keyboards
            UINT32 nButtonMask = 0;
            switch( wParam )
            {
                case '1':         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_1; break;
                case '2':         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_2; break;
                case '3':         nButtonMask = bShiftKeyState ? FRM_INPUT::KEY_HASH : FRM_INPUT::KEY_3; break;
                case '4':         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_4; break;
                case '5':         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_5; break;
                case '6':         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_6; break;
                case '7':         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_7; break;
                case '8':         nButtonMask = bShiftKeyState ? FRM_INPUT::KEY_STAR : FRM_INPUT::KEY_8; break;
                case '9':         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_9; break;
                case '0':         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_0; break;
                case VK_NUMPAD1:  nButtonMask = FRM_INPUT::KEY_1; break;
                case VK_NUMPAD2:  nButtonMask = FRM_INPUT::KEY_2; break;
                case VK_NUMPAD3:  nButtonMask = FRM_INPUT::KEY_3; break;
                case VK_NUMPAD4:  nButtonMask = FRM_INPUT::KEY_4; break;
                case VK_NUMPAD5:  nButtonMask = FRM_INPUT::KEY_5; break;
                case VK_NUMPAD6:  nButtonMask = FRM_INPUT::KEY_6; break;
                case VK_NUMPAD7:  nButtonMask = FRM_INPUT::KEY_7; break;
                case VK_NUMPAD8:  nButtonMask = FRM_INPUT::KEY_8; break;
                case VK_NUMPAD9:  nButtonMask = FRM_INPUT::KEY_9; break;
                case VK_NUMPAD0:  nButtonMask = FRM_INPUT::KEY_0; break;
                case VK_MULTIPLY: nButtonMask = FRM_INPUT::KEY_STAR;   break;
                case VK_LEFT:     nButtonMask = FRM_INPUT::DPAD_LEFT;  break;
                case VK_RIGHT:    nButtonMask = FRM_INPUT::DPAD_RIGHT; break;
                case VK_UP:       nButtonMask = FRM_INPUT::DPAD_UP;    break;
                case VK_DOWN:     nButtonMask = FRM_INPUT::DPAD_DOWN;  break;
                case VK_SPACE:    nButtonMask = FRM_INPUT::SELECT;     break;
                case VK_RETURN:   nButtonMask = FRM_INPUT::SELECT;     break;
                case VK_BACK:     nButtonMask = FRM_INPUT::KEY_CLEAR;  break;
                case VK_DELETE:   nButtonMask = FRM_INPUT::KEY_CLEAR;  break;
                case VK_INSERT:   nButtonMask = FRM_INPUT::KEY_EDIT;   break;
            }

            if( nButtonMask )
            {
                if( uMsg == WM_KEYDOWN )
                    pAppContainer->m_pApplication->m_Input.m_nButtons |= nButtonMask;
                else
                    pAppContainer->m_pApplication->m_Input.m_nButtons &= ~nButtonMask;
            }

            // Toggle the window orientation between portrait and landscape mode
            EGLenum nAPI = eglQueryAPI();
            if( nAPI == EGL_OPENGL_ES_API && uMsg == WM_KEYDOWN && nButtonMask == FRM_INPUT::KEY_STAR )
                pAppContainer->ToggleOrientation();

            return 0;
        }

        case WM_LBUTTONDOWN: 
        {
            POINTS vMousePt = MAKEPOINTS(lParam);
            FRMVECTOR2 vPointerPosition;
            vPointerPosition.x = +2.0f * vMousePt.x/(FLOAT32)(pAppContainer->m_pApplication->m_nWidth -1) - 1.0f;
            vPointerPosition.y = -2.0f * vMousePt.y/(FLOAT32)(pAppContainer->m_pApplication->m_nHeight-1) + 1.0f;

            pAppContainer->m_pApplication->m_Input.m_vPointerPosition = vPointerPosition;
            pAppContainer->m_pApplication->m_Input.m_nPointerState  = FRM_INPUT::POINTER_DOWN;
            pAppContainer->m_pApplication->m_Input.m_nPointerState |= FRM_INPUT::POINTER_PRESSED;

            SetCapture( hWnd  ); 
            break;
        }

        case WM_MOUSEMOVE: 
            if( wParam & MK_LBUTTON ) 
            { 
                POINTS vMousePt = MAKEPOINTS(lParam);
                FRMVECTOR2 vPointerPosition;
                vPointerPosition.x = +2.0f * vMousePt.x/(FLOAT32)(pAppContainer->m_pApplication->m_nWidth -1) - 1.0f;
                vPointerPosition.y = -2.0f * vMousePt.y/(FLOAT32)(pAppContainer->m_pApplication->m_nHeight-1) + 1.0f;

                pAppContainer->m_pApplication->m_Input.m_vPointerPosition = vPointerPosition;

                pAppContainer->m_pApplication->m_Input.m_nPointerState = FRM_INPUT::POINTER_DOWN;
            } 
            break; 
 
        case WM_LBUTTONUP: 
        {
            POINTS vMousePt = MAKEPOINTS(lParam);
            FRMVECTOR2 vPointerPosition;
            vPointerPosition.x = +2.0f * vMousePt.x/(FLOAT32)(pAppContainer->m_pApplication->m_nWidth -1) - 1.0f;
            vPointerPosition.y = -2.0f * vMousePt.y/(FLOAT32)(pAppContainer->m_pApplication->m_nHeight-1) + 1.0f;

            pAppContainer->m_pApplication->m_Input.m_vPointerPosition = vPointerPosition;
            pAppContainer->m_pApplication->m_Input.m_nPointerState = FRM_INPUT::POINTER_RELEASED;
            ReleaseCapture(); 
            return 0; 
        }

        case WM_DESTROY:
        case WM_CLOSE:
            PostQuitMessage( 0 );
            return 0;
    }

    // Pass all unhandled messages to the default WndProc
    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}


//--------------------------------------------------------------------------------------
// Name: ToggleOrientation()
// Desc: Toggle the window orientation between portrait and landscape mode
//--------------------------------------------------------------------------------------
VOID CFrmAppContainer::ToggleOrientation()
{
    // Switch orientation by swapping the width and height
    INT32 nOldWidth  = m_pApplication->m_nWidth;
    INT32 nOldHeight = m_pApplication->m_nHeight;
    INT32 nNewWidth  = nOldHeight;
    INT32 nNewHeight = nOldWidth;

    // Resize the client window
    SetWindowPos( (HWND)m_hWindow, NULL, 0, 0, nNewWidth, nNewHeight,
                  SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    // Resize the parent frame window
    RECT rc;
    GetWindowRect( GetParent((HWND)m_hWindow), &rc );
    SetWindowPos( GetParent((HWND)m_hWindow), NULL, 0, 0, 
                  ((rc.right-rc.left)-nOldWidth)+nNewWidth, 
                  ((rc.bottom-rc.top)-nOldHeight)+nNewHeight,
                  SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    // Update the app variables
    m_pApplication->m_nWidth  = nNewWidth;
    m_pApplication->m_nHeight = nNewHeight;

    // Update the OpenGL viewport
    // Note: The derived app class should do this during the Render() function!
    m_pApplication->Resize();
}


//--------------------------------------------------------------------------------------
// Name: HandleEvents()
// Desc: Pumps the application window's message queue
//--------------------------------------------------------------------------------------
BOOL CFrmAppContainer::HandleEvents()
{
    MSG msg;
    if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
    {
        if( msg.message == WM_QUIT )
            return FALSE;

        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: CreateNativeWindow()
// Desc: Creates a window for the application
//--------------------------------------------------------------------------------------
BOOL CFrmAppContainer::CreateNativeWindow( NativeWindowType* pWindow,
                                           NativeDisplayType* pDisplay )
{
    // The global instance
    HINSTANCE hInstance  = GetModuleHandle( NULL );

    // Register the window class
    WNDCLASS wc;
    wc.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;  // Window style
    wc.lpfnWndProc    = (WNDPROC)WndProc;                    // WndProc message handler
    wc.cbClsExtra     = 0;                                   // No extra window data
    wc.cbWndExtra     = 0;                                   // No extra window data
    wc.hInstance      = hInstance;                           // Instance
    wc.hIcon          = (HICON)LoadImage( NULL, "Adreno.ico",
                                          IMAGE_ICON, 16, 16, LR_LOADFROMFILE );
    wc.hCursor        = LoadCursor( NULL, IDC_ARROW );       // Cursor
    wc.hbrBackground  = NULL;                                // No Background
    wc.lpszMenuName   = NULL;                                // No menu
    wc.lpszClassName  = "Adreno SDK Window";                 // Set the class name

    if( FALSE == RegisterClass(&wc) )
    {
        FrmLogMessage( "ERROR: Failed to register window class.\n" );
        return FALSE;
    }

    // Adjust the window size to fit our rectangle
    DWORD dwWindowStyle = WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION | WS_BORDER |
                          WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    RECT rcWindow;
    SetRect( &rcWindow, 0, 0, m_pApplication->m_nWidth, m_pApplication->m_nHeight );
    AdjustWindowRect( &rcWindow, dwWindowStyle, FALSE );

    // Create the parent window
    HWND hParentWnd = CreateWindowEx(
        WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,    // Extended style
        "Adreno SDK Window",                   // Class
        m_pApplication->m_strName,             // Title
        dwWindowStyle,                         // Style
        50 + rcWindow.left, 50 + rcWindow.top, // Position
        (rcWindow.right-rcWindow.left),        // Width
        (rcWindow.bottom-rcWindow.top),        // Height
        NULL,                                  // No parent window
        NULL,                                  // No menu
        hInstance,                             // Instance
        NULL );                                // Creation parameter
    if( NULL == hParentWnd )
    {
        FrmLogMessage( "ERROR: Failed to create window.\n" );
        return FALSE;
    }

    // Create the client window
    HWND hClientWnd = CreateWindow(
        "Adreno SDK Window",                   // Class
        NULL,                                  // Title
        WS_CHILD,                              // Style
        0, 0, m_pApplication->m_nWidth, m_pApplication->m_nHeight,             // Pos and size
        hParentWnd,                            // Parent window
        NULL,                                  // No menu
        hInstance,                             // Instance
        NULL );                                // Creation parameter
    if( NULL == hClientWnd )
    {
        FrmLogMessage( "ERROR: Failed to create window.\n" );
        return FALSE;
    }

    // Pass application data pointer to the windows for later use
    SetWindowLong( hParentWnd, GWLP_USERDATA, (LONG)this );
    SetWindowLong( hClientWnd, GWLP_USERDATA, (LONG)this );

    // Note: We delay showing the window until after Initialization() succeeds
    // Otherwise, an unsightly, empty window briefly appears during initialization

    // Return
    (*pWindow)  = (NativeWindowType)hClientWnd;
    (*pDisplay) = (NativeDisplayType)GetDC( hClientWnd );
    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: CFrmAppContainer()
// Desc: Constructor
//--------------------------------------------------------------------------------------
CFrmAppContainer::CFrmAppContainer()
{
    m_pApplication   = NULL;
}


//--------------------------------------------------------------------------------------
// Name: ~CFrmAppContainer()
// Desc: Destroys the application's window, etc.
//--------------------------------------------------------------------------------------
CFrmAppContainer::~CFrmAppContainer()
{
    if( m_pApplication )
    {
        m_pApplication->Destroy();
        delete m_pApplication;
    }

    ReleaseDC( (HWND)m_hWindow, (HDC)m_hDisplay );
    DestroyWindow( (HWND)m_hWindow );
}

