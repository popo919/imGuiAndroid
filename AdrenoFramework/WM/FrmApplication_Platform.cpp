//--------------------------------------------------------------------------------------
// File: FrmApplication_Win32.cpp
// Desc: Win32 implementation of the Framework application
//
// Author:      QUALCOMM, Advanced Content Group - Snapdragon SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include "FrmPlatform.h"
#include "FrmApplication.h"
#include "FrmUtils.h"

#ifndef WINCE
#include <stdio.h>
#include <crtdbg.h>
#endif // !WINCE


//--------------------------------------------------------------------------------------
// Name: class CFrmAppContainer()
// Desc: Application class container
//--------------------------------------------------------------------------------------
class CFrmAppContainer
{
public:
    NativeWindowType  m_hWindow;
    NativeDisplayType m_hDisplay;
    CFrmApplication*  m_pApplication;

    BOOL CreateNativeWindow( NativeWindowType* pWindow, NativeDisplayType* pDisplay );
    VOID CopyFramebufferToClipboard();
    
public:
    CFrmAppContainer();
    ~CFrmAppContainer();

    BOOL Run();
    BOOL HandleEvents();
    VOID ToggleOrientation();
};


//--------------------------------------------------------------------------------------
// Name: WinMain()
// Desc: Application entry-point on the Windows platform
//--------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    CFrmAppContainer appContainer;
    return (INT32)appContainer.Run();
}


//--------------------------------------------------------------------------------------
// Name: Run()
// Desc: Create the framework, initialize the application, and render frames.
//--------------------------------------------------------------------------------------
BOOL CFrmAppContainer::Run()
{
    m_pApplication = FrmCreateApplicationInstance();
    if( NULL == m_pApplication )
        return FALSE;

    // Create the application window
    if( FALSE == CreateNativeWindow( &m_hWindow, &m_hDisplay ) )
        return FALSE;

    // Create the render context
    if( FALSE == m_pApplication->CreateRenderContext( m_hWindow, m_hDisplay ) )
    {
        MessageBox( NULL, L"Application failed to create render context!\n"
                          L"\n"
                          L"Please debug accordingly.", 
                          NULL, MB_ICONERROR );
        return FALSE;
    }

    //if(strcmp(_CRT_STRINGIZE(PLATFORM), "OPENGLES") == 0)
    //{
    //    eglBindAPI( EGL_OPENGL_ES_API );
    //    eglMakeCurrent( m_pApplication->m_eglDisplay, m_pApplication->m_eglSurface, m_pApplication->m_eglSurface, m_pApplication->m_eglContextGL );
    //}
    //else
    //{
    //    eglBindAPI( EGL_OPENVG_API );
    //    eglMakeCurrent( m_pApplication->m_eglDisplay, m_pApplication->m_eglSurface, m_pApplication->m_eglSurface, m_pApplication->m_eglContextVG );
    //}

    if( ( FALSE == m_pApplication->Initialize() ) ||
        ( FALSE == m_pApplication->Resize() ) )
    {
        MessageBox( NULL, L"Application failed during scene initialization!\n"
                          L"\n"
                          L"Please debug accordingly.", 
                          NULL, MB_ICONERROR );
        return FALSE;
    }

    // Re-register the VK_SNAPSHOT hotkey so we can trap the PrtScn key for screen captures
    UnregisterHotKey( GetParent( (HWND)m_hWindow ), IDHOT_SNAPWINDOW );
    RegisterHotKey( GetParent( (HWND)m_hWindow ), IDHOT_SNAPWINDOW, 0, VK_SNAPSHOT );

    // Show the parent window
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
	// Not supported under WM
}


//--------------------------------------------------------------------------------------
// Name: WndProc()
// Desc: The application window's message proc
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    // Get the application data
    CFrmAppContainer* pAppContainer = (CFrmAppContainer*)GetWindowLong( hWnd, GWL_USERDATA );

    switch( uMsg )
    {
	case WM_SETCURSOR:
            // User has tapped the screen
            PostQuitMessage( 0 );
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
	LPCTSTR	pszTitle = _T("Snapdragon SDK Window");

    // Need a WCHAR version of m_pApplication->m_strName to pass to window creation
    WCHAR wszName[MAX_PATH];
    mbstowcs(wszName, m_pApplication->m_strName, MAX_PATH);

    // The global instance
    HINSTANCE hInstance  = GetModuleHandle( NULL );

	// If the size is <= 0 then we are running if full screen mode
	BOOL fFullScreen = FALSE;
	if(m_pApplication->m_nWidth <= 0 || m_pApplication->m_nHeight <= 0)
	{
		fFullScreen = TRUE;
	}

    // Register the window class
    WNDCLASS wc = {0};
    wc.style          = CS_HREDRAW | CS_VREDRAW;  // Window style
    wc.lpfnWndProc    = (WNDPROC)WndProc;                    // WndProc message handler
    wc.cbClsExtra     = 0;                                   // No extra window data
    wc.cbWndExtra     = 0;                                   // No extra window data
    wc.hInstance      = hInstance;                           // Instance
    wc.hIcon          = NULL;
    wc.hCursor        = LoadCursor( NULL, IDC_ARROW );       // Cursor
    wc.hInstance      = hInstance;
    wc.hbrBackground  = NULL;                                // No Background
    wc.lpszMenuName   = NULL;                                // No menu
    wc.lpszClassName  = pszTitle;                 // Set the class name

    if( FALSE == RegisterClass(&wc) )
    {
        FrmLogMessage( "ERROR: Failed to register window class.\n" );
        return FALSE;
    }

	HWND hParentWnd = NULL;
	HWND hClientWnd = NULL;
	DWORD dwWindowStyle;
	if(!fFullScreen)
	{
		// Adjust the window size to fit our rectangle
		dwWindowStyle =	WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION | WS_BORDER |
						WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		RECT rcWindow;
		SetRect( &rcWindow, 0, 0, m_pApplication->m_nWidth, m_pApplication->m_nHeight );
		AdjustWindowRectEx( &rcWindow, dwWindowStyle, FALSE, 0 );
		if(rcWindow.left < 0)
		{
			// AdjustWindowRectEx gives back negative values if the window is placed
			// such that it overlaps the WM start bar.  This always happens if the 
			// rectangle starts at (0,0) like above.
			rcWindow.right += -rcWindow.left;
			rcWindow.left = 0;
		}
		if(rcWindow.top < 0)
		{
			rcWindow.bottom += -rcWindow.top;
			rcWindow.top = 0;
		}

    // Create the parent window
		hParentWnd = CreateWindow(	wc.lpszClassName,					//Class
									wszName,			                //Title
									dwWindowStyle,						//Style
									50+rcWindow.left,50+rcWindow.top,	//Position
									(rcWindow.right-rcWindow.left),		//Width
									(rcWindow.bottom-rcWindow.top),		//Height
									NULL,								//Noparentwindow
									NULL,								//Nomenu
									hInstance,							//Instance
									NULL);								//Creationparameter
		if( NULL == hParentWnd )
		{
			FrmLogMessage( "ERROR: Failed to create window.\n" );
			return FALSE;
		}

	}
	else
	{
		// Full Screen Mode
		m_pApplication->m_nWidth = GetSystemMetrics(SM_CXSCREEN);
		m_pApplication->m_nHeight = GetSystemMetrics(SM_CYSCREEN);
	}

	// Create the client window
	if(!fFullScreen)
		dwWindowStyle = WS_CHILD;
	else
		dwWindowStyle = WS_VISIBLE;

	hClientWnd = CreateWindow(	wc.lpszClassName,			//Class
								wszName,	                //Title
								dwWindowStyle,				//Style
								0,0,						//Pos and size
								m_pApplication->m_nWidth,
								m_pApplication->m_nHeight,	//Pos and size
								hParentWnd,					//Parent window
								NULL,						//No menu
								hInstance,					//Instance
								NULL);						//Creation parameter
	if( NULL == hClientWnd )
	{
		DWORD dwLastError = GetLastError();
		FrmLogMessage( "ERROR: Failed to create window.\n" );
		return FALSE;
	}

	if(fFullScreen)
	{
		// Hide the taskbars
		// SHFullScreen(hClientWnd, SHFS_HIDETASKBAR | SHFS_HIDESTARTICON | SHFS_HIDESIPBUTTON);
	}

	// Pass application data pointer to the windows for later use
	if(hParentWnd != NULL)
		SetWindowLong( hParentWnd, GWL_USERDATA, (LONG)this );

	if(hClientWnd != NULL)
		SetWindowLong( hClientWnd, GWL_USERDATA, (LONG)this );

    // Note: We delay showing the window until after Initialization() succeeds
    // Otherwise, an unsightly, empty window briefly appears during initialization

    // Return
    (*pWindow)  = (NativeWindowType)hClientWnd;
    (*pDisplay) = NULL;


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

    DestroyWindow( (HWND)m_hWindow );
}

