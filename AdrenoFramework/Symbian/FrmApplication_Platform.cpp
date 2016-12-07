//--------------------------------------------------------------------------------------
// File: FrmApplication_Symbian.cpp
// Desc: Base application support for NGI applications
//
// Author:      QUALCOMM, Advanced Content Group - Adreno SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include <ngi/ErrorCodes.h>
#include <ngi/Runtime.h>
#include <ngi/Idle.h>
#include <ngi/Display.h>
#include <ngi/DisplayManager.h>
#include <ngi/Input.h>
#include <ngi/ApplicationState.h>
#include <ngi/BackBuffer.h>

#ifdef _WIN32
    #undef _WIN32
#endif
#include <EGL/egl.h>
#include "FrmBasicTypes.h"
#include "FrmApplication.h"


// Helper macro
#define FAILED(err) ( (err) != ngi::OK ? TRUE : FALSE )


// Idle time priorities
const UINT32 IDLE_TIME_FOREGROUND = 500;
const UINT32 IDLE_TIME_BACKGROUND = 0;


//--------------------------------------------------------------------------------------
// Name: class CFrmAppContainer
// Desc: Application container class for NGI applications
//--------------------------------------------------------------------------------------
class CFrmAppContainer : public ngi::IInputDeviceObserver, 
                         public ngi::IApplicationStateObserver
{
    // NGI application objects
    ngi::IApplicationState*  m_pApplicationState;
    ngi::IDisplayManager*    m_pDisplayManager;
    ngi::IDisplay*           m_pDisplay;
    ngi::IWindow*            m_pWindow;
    ngi::IBackBufferFactory* m_pBackBufferFactory;
    ngi::IBackBuffer*        m_pBackBuffer;
    ngi::IInput*             m_pInput;
    ngi::IInputDevice*       m_pInputDevice;        

    // Application state
    BOOL             m_bIsRunning;
    BOOL             m_bHasFocus;

    // EGL objects
    EGLDisplay 		 m_eglDisplay;
    EGLContext 		 m_eglContext;
    EGLSurface		 m_eglSurface;
    int				 m_ColorDepth;
    NativeWindowType m_hWindow;

    // The user-provided application class
    CFrmApplication* m_pApplication;

protected: 
    // From ngi::IInputDeviceObserver
    virtual void InputKeyPressed( ngi::IInputDevice& aDevice, ngi::uint64 aTimeStamp, ngi::uint32 aKeyCode );
    virtual void InputKeyReleased( ngi::IInputDevice& aDevice, ngi::uint64 aTimeStamp, ngi::uint32 aKeyCode );
    virtual void InputAxisMoved( ngi::IInputDevice& aDevice, ngi::uint64 aTimeStamp, ngi::uint32 aAxisNumber, ngi::int32 aNewAxisValue );
    virtual void DeviceDisconnected( ngi::IInputDevice& aDevice );
    virtual void DeviceConnected( ngi::IInputDevice& aDevice ); 

    // From ngi::IApplicationStateObserver
    virtual void FocusGained();
    virtual void FocusLost();
    virtual void ExitRequested();

public:
    // Functions to create and run the application
    BOOL Initialize();
    BOOL Run();
    VOID Draw();

    CFrmAppContainer();
    ~CFrmAppContainer();
};


//--------------------------------------------------------------------------------------
// Name: main()
// Desc: The main entry point.
//--------------------------------------------------------------------------------------
int main()
{
    // Initialize the NGI runtime resources
    if( FAILED( ngi::CRuntime::Initialize() ) )
        return FALSE;

    // Run the application
    CFrmAppContainer theApplication;
    BOOL bResult = theApplication.Run();

    // Cleanup
    ngi::CRuntime::Close();

    // Return
    return bResult;
}


//--------------------------------------------------------------------------------------
// Name: CFrmAppContainer()
// Desc: Constructor
//--------------------------------------------------------------------------------------
CFrmAppContainer::CFrmAppContainer()
{
    m_pApplicationState  = NULL;
    m_pDisplayManager    = NULL;
    m_pDisplay           = NULL;
    m_pWindow            = NULL;
    m_pBackBufferFactory = NULL;
    m_pBackBuffer        = NULL;
    m_pInput             = NULL;
    m_pInputDevice       = NULL;

    m_bIsRunning = TRUE;
    m_bHasFocus  = TRUE;
}


//--------------------------------------------------------------------------------------
// Name: ~CFrmAppContainer()
// Desc: Destructor.
//--------------------------------------------------------------------------------------
CFrmAppContainer::~CFrmAppContainer()
{
    // Destroy the application
    if( m_pApplication )
    {
        m_pApplication->Destroy();
        delete m_pApplication;
    }

    // Close input
    {
        if( m_pInputDevice )
        {
            m_pInputDevice->Stop();
            m_pInputDevice->Release();
        }
        
        if( m_pInput )
            m_pInput->Release();

        m_pInputDevice = NULL;
        m_pInput = NULL;    
    }

    // Close backbuffer
    {
        if( m_pBackBuffer )
            m_pBackBuffer->Release();
         
        if( m_pBackBufferFactory )
            m_pBackBufferFactory->Release();

        m_pBackBuffer = NULL;
        m_pBackBufferFactory = NULL;
    }

    // Close window
    {
        if( m_pWindow )
            m_pWindow->Release();
         
        if( m_pDisplay )
            m_pDisplay->Release();
        
        if( m_pDisplayManager )
            m_pDisplayManager->Release();

        m_pWindow = NULL;
        m_pDisplay = NULL;
        m_pDisplayManager = NULL;
    }

    // Close app state listener
    {
        if( m_pApplicationState )
            m_pApplicationState->Release();
        m_pApplicationState = NULL;
    }
}


//--------------------------------------------------------------------------------------
// Name: Initialize()
// Desc: Initialization function to create objects needed to run the application.
//--------------------------------------------------------------------------------------
BOOL CFrmAppContainer::Initialize()
{
    ngi::ReturnCode err;    

    // Init the app state listener
    {
        if( FAILED( err = ngi::CRuntime::CreateInstance( m_pApplicationState ) ) )
            return FALSE;
        m_pApplicationState->SetObserver( this );
    }

    // Init the window
    {
        if( FAILED( err = ngi::CRuntime::CreateInstance( m_pDisplayManager ) ) )
            return FALSE;

        if( FAILED( err = m_pDisplayManager->CreateDisplay( 0, m_pDisplay ) ) )
            return FALSE;

        ngi::IDisplayConfiguration* pDisplayConfiguration = NULL;
        if( FAILED( err = m_pDisplay->GetDisplayConfiguration( 0, pDisplayConfiguration ) ) )
            return FALSE;
            
        if( FAILED( err = m_pDisplay->CreateWindow( *pDisplayConfiguration, m_pWindow ) ) )
            return FALSE;

        m_hWindow = m_pWindow->GetHandle(); 
    }

    // Init the backbuffer
    {
        if( FAILED( err = ngi::CRuntime::CreateInstance( m_pBackBufferFactory ) ) )
            return FALSE;

        ngi::CBackBufferResolution bbResolution;
        ngi::uint32 nColorDepthMask;
        const UINT32 nResolutionMask = ngi::BACKBUFFER_RESOLUTION_352x416 | ngi::BACKBUFFER_RESOLUTION_416x352 |
                                       ngi::BACKBUFFER_RESOLUTION_240x320 | ngi::BACKBUFFER_RESOLUTION_320x240 |
                                       ngi::BACKBUFFER_RESOLUTION_176x208 | ngi::BACKBUFFER_RESOLUTION_208x176;
        m_pBackBufferFactory->GetBackBufferConfiguration( 0, nResolutionMask,
                                                          ngi::GRAPHICS_FORMAT_RGB565,
                                                          bbResolution, nColorDepthMask );
        
        if( FAILED( err = m_pBackBufferFactory->CreateBackBuffer( m_pWindow, bbResolution,
                                                                  nColorDepthMask, ngi::BACKBUFFER_FLAG_NONE,
                                                                  m_pBackBuffer ) ) )
            return FALSE;

        m_pBackBuffer->SetClearColor( 0xffffffff );
    }
        
    // Init the EGL render context
    {
        // Get the display for drawing graphics
        m_eglDisplay = eglGetDisplay( EGL_DEFAULT_DISPLAY );
        if( NULL == m_eglDisplay )
            return FALSE;

        // Initialize display 
        if( FALSE == eglInitialize( m_eglDisplay, NULL, NULL ) )
            return FALSE;

        // Get the number of possible EGLConfigs 
        EGLint nNumConfigs = 0; 
        if( FALSE == eglGetConfigs( m_eglDisplay, NULL, 0, &nNumConfigs ) )
            return FALSE;

        // Allocate memory for the configList 
        EGLint     nConfigSize = nNumConfigs;
        EGLConfig* pConfigList = new EGLConfig[nNumConfigs];
        if( pConfigList == NULL )
            return FALSE;
         
        if( m_ColorDepth!=12 && m_ColorDepth!=16 && m_ColorDepth!=18 && m_ColorDepth!=24 )
            return FALSE;
        
        // Define properties for the wanted EGLSurface 
        const EGLint attrib_list[] =
        { 
            EGL_BUFFER_SIZE, m_ColorDepth,
            EGL_NONE 
        };

        // Choose an EGLConfig that best matches to the properties in attrib_list 
        if( FALSE == eglChooseConfig( m_eglDisplay, attrib_list, pConfigList,
                                      nConfigSize, &nNumConfigs ) )
            return FALSE;

        // Choose the best EGLConfig, which is sorted to the top of the list
        EGLConfig config = pConfigList[0];
        delete[] pConfigList;

        // Create a window where the graphics are blitted 
        m_eglSurface = eglCreateWindowSurface( m_eglDisplay, config, (NativeWindowType)m_hWindow, NULL );
        if( m_eglSurface == NULL )
            return FALSE;

        // Create a rendering context 
        m_eglContext = eglCreateContext( m_eglDisplay, config, NULL, NULL );
        if( m_eglContext == NULL )
            return FALSE;

        // Make the context current
        if( FALSE == eglMakeCurrent( m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext ) )
            return FALSE;
        
        // glViewport( 0, 0, scrWidth, scrHeight );
    }

    // Init the input
    {
        if( FAILED( err = ngi::CRuntime::CreateInstance( m_pInput ) ) )
            return FALSE;

        if( FAILED( err = m_pInput->CreateDevice( 0, m_pInputDevice ) ) )
            return FALSE;

        m_pInputDevice->SetObserver( this );
        m_pInputDevice->Start();    
    }

    m_pApplication = FrmCreateApplicationInstance();
    if( NULL == m_pApplication )
        return FALSE;

    if( FALSE == m_pApplication->Initialize() )
        return FALSE;

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: Run()
// Desc: Main function to initialize the app and process the loop
//--------------------------------------------------------------------------------------
BOOL CFrmAppContainer::Run()
{
    // Initialize the application
    if( FALSE == Initialize() )
        return FALSE;

    // Create an idle-object to handle system calls
    ngi::IIdle* pIdle = NULL;
    if( FAILED( ngi::CRuntime::CreateInstance( pIdle ) ) )
        return FALSE;

    // Run the main loop
    while( m_bIsRunning )
    {
        if( m_bHasFocus )
        {
            Draw();
            
            // Use non-blocking mode while in foreground
            pIdle->Process( IDLE_TIME_FOREGROUND, FALSE );
            
            // Draw one last time after a focus lost. This is needed for correct
            // display at end-key press and for task list.
            if( FALSE == m_bHasFocus )
                Draw();
        }
        else
        {
            // Use blocking mode while in background
            pIdle->Process( IDLE_TIME_BACKGROUND, TRUE );
        }
    }

    pIdle->Release();
    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: Draw()
// Desc: Called from the main loop to draw the scene.
//--------------------------------------------------------------------------------------
VOID CFrmAppContainer::Draw()
{
    if( FAILED( m_pBackBuffer->Lock() ) )
        return;

    m_pBackBuffer->Clear();
        m_pApplication->Update();
        m_pApplication->Render();
    m_pBackBuffer->Unlock();

    m_pBackBuffer->Swap();
    eglSwapBuffers( m_eglDisplay, m_eglSurface );

}


//--------------------------------------------------------------------------------------
// Name: FocusGained()
// Desc: Callback for when focus is gained.
//--------------------------------------------------------------------------------------
void CFrmAppContainer::FocusGained()
{
    m_bHasFocus = TRUE;

    // TODO: Re-activate all threads and timers.
}


//--------------------------------------------------------------------------------------
// Name: FocusLost()
// Desc: Callback for when focus is lost.
//--------------------------------------------------------------------------------------
void CFrmAppContainer::FocusLost()
{
    m_bHasFocus = FALSE;

    // TODO: Stop all threads and timers.
}


//--------------------------------------------------------------------------------------
// Name: ExitRequested()
// Desc: Callback for when something external is requesting the app to exit.
//--------------------------------------------------------------------------------------
void CFrmAppContainer::ExitRequested()
{
    m_bIsRunning = FALSE;
}


//--------------------------------------------------------------------------------------
// Name: InputKeyPressed()
// Desc: Callback for when a key is pressed.
//--------------------------------------------------------------------------------------
void CFrmAppContainer::InputKeyPressed( ngi::IInputDevice& /*aInputDevice*/,
                                        ngi::uint64 /*aTimeStamp*/, 
                                        ngi::uint32 aKeyCode )
{
    UINT32 nButtonMask = 0;

    if( ngi::INPUT_KEY_1 & aKeyCode ) nButtonMask |= INPUT_KEY_1;
    if( ngi::INPUT_KEY_2 & aKeyCode ) nButtonMask |= INPUT_KEY_2;
    if( ngi::INPUT_KEY_3 & aKeyCode ) nButtonMask |= INPUT_KEY_3;
    if( ngi::INPUT_KEY_4 & aKeyCode ) nButtonMask |= INPUT_KEY_4;
    if( ngi::INPUT_KEY_5 & aKeyCode ) nButtonMask |= INPUT_KEY_5;
    if( ngi::INPUT_KEY_6 & aKeyCode ) nButtonMask |= INPUT_KEY_6;
    if( ngi::INPUT_KEY_7 & aKeyCode ) nButtonMask |= INPUT_KEY_7;
    if( ngi::INPUT_KEY_8 & aKeyCode ) nButtonMask |= INPUT_KEY_8;
    if( ngi::INPUT_KEY_9 & aKeyCode ) nButtonMask |= INPUT_KEY_9;
    if( ngi::INPUT_KEY_0 & aKeyCode ) nButtonMask |= INPUT_KEY_0;

    if( ngi::INPUT_KEY_STAR & aKeyCode ) nButtonMask |= INPUT_KEY_STAR;
    if( ngi::INPUT_KEY_HASH & aKeyCode ) nButtonMask |= INPUT_KEY_HASH;

    if( ngi::INPUT_KEY_DIGITAL_LEFT & aKeyCode )  nButtonMask |= INPUT_DPAD_LEFT;
    if( ngi::INPUT_KEY_DIGITAL_RIGHT & aKeyCode ) nButtonMask |= INPUT_DPAD_RIGHT;
    if( ngi::INPUT_KEY_DIGITAL_UP & aKeyCode )    nButtonMask |= INPUT_DPAD_UP;
    if( ngi::INPUT_KEY_DIGITAL_DOWN & aKeyCode )  nButtonMask |= INPUT_DPAD_DOWN;
    
    if( ngi::INPUT_KEY_LSK & aKeyCode ) nButtonMask |= INPUT_LEFT_BUTTON;
    if( ngi::INPUT_KEY_RSK & aKeyCode ) nButtonMask |= INPUT_RIGHT_BUTTON;

    if( ngi::INPUT_KEY_OK & aKeyCode )    nButtonMask |= INPUT_SELECT;
    if( ngi::INPUT_KEY_CLEAR & aKeyCode ) nButtonMask |= INPUT_KEY_CLEAR;
    if( ngi::INPUT_KEY_EDIT & aKeyCode )  nButtonMask |= INPUT_KEY_EDIT;
    if( ngi::INPUT_KEY_SEND & aKeyCode )  nButtonMask |= INPUT_KEY_SEND;

    if( ngi::INPUT_KEY_VOLUME_UP & aKeyCode )   nButtonMask |= INPUT_KEY_VOL_UP;
    if( ngi::INPUT_KEY_VOLUME_DOWN & aKeyCode ) nButtonMask |= INPUT_KEY_VOL_DOWN;

    if( ngi::INPUT_KEY_EXTRA_A & aKeyCode ) nButtonMask |= INPUT_KEY_EXTRA_A;
    if( ngi::INPUT_KEY_EXTRA_B & aKeyCode ) nButtonMask |= INPUT_KEY_EXTRA_B;
    if( ngi::INPUT_KEY_EXTRA_C & aKeyCode ) nButtonMask |= INPUT_KEY_EXTRA_C;
    if( ngi::INPUT_KEY_EXTRA_D & aKeyCode ) nButtonMask |= INPUT_KEY_EXTRA_D;
    if( ngi::INPUT_KEY_EXTRA_E & aKeyCode ) nButtonMask |= INPUT_KEY_EXTRA_E;
    if( ngi::INPUT_KEY_EXTRA_F & aKeyCode ) nButtonMask |= INPUT_KEY_EXTRA_F;
    if( ngi::INPUT_KEY_EXTRA_G & aKeyCode ) nButtonMask |= INPUT_KEY_EXTRA_G;
    if( ngi::INPUT_KEY_EXTRA_H & aKeyCode ) nButtonMask |= INPUT_KEY_EXTRA_H;

    // Update the application's list of which buttons are currently pressed
    if( nButtonMask )
        m_pApplication->m_Input.m_nButtons |= nButtonMask;
    
    // Exit the app when the right soft key is pressed
    if( ngi::INPUT_KEY_RSK & aKeyCode )
        m_bIsRunning = FALSE;
}


//--------------------------------------------------------------------------------------
// Name: InputKeyReleased()
// Desc: Callback for when a key is released.
//--------------------------------------------------------------------------------------
void CFrmAppContainer::InputKeyReleased( ngi::IInputDevice& /*aInputDevice*/,
                                         ngi::uint64 /*aTimeStamp*/, 
                                         ngi::uint32 aKeyCode )
{
    UINT32 nButtonMask = 0;

    if( ngi::INPUT_KEY_1 & aKeyCode ) nButtonMask |= INPUT_KEY_1;
    if( ngi::INPUT_KEY_2 & aKeyCode ) nButtonMask |= INPUT_KEY_2;
    if( ngi::INPUT_KEY_3 & aKeyCode ) nButtonMask |= INPUT_KEY_3;
    if( ngi::INPUT_KEY_4 & aKeyCode ) nButtonMask |= INPUT_KEY_4;
    if( ngi::INPUT_KEY_5 & aKeyCode ) nButtonMask |= INPUT_KEY_5;
    if( ngi::INPUT_KEY_6 & aKeyCode ) nButtonMask |= INPUT_KEY_6;
    if( ngi::INPUT_KEY_7 & aKeyCode ) nButtonMask |= INPUT_KEY_7;
    if( ngi::INPUT_KEY_8 & aKeyCode ) nButtonMask |= INPUT_KEY_8;
    if( ngi::INPUT_KEY_9 & aKeyCode ) nButtonMask |= INPUT_KEY_9;
    if( ngi::INPUT_KEY_0 & aKeyCode ) nButtonMask |= INPUT_KEY_0;

    if( ngi::INPUT_KEY_STAR & aKeyCode ) nButtonMask |= INPUT_KEY_STAR;
    if( ngi::INPUT_KEY_HASH & aKeyCode ) nButtonMask |= INPUT_KEY_HASH;

    if( ngi::INPUT_KEY_DIGITAL_LEFT & aKeyCode )  nButtonMask |= INPUT_DPAD_LEFT;
    if( ngi::INPUT_KEY_DIGITAL_RIGHT & aKeyCode ) nButtonMask |= INPUT_DPAD_RIGHT;
    if( ngi::INPUT_KEY_DIGITAL_UP & aKeyCode )    nButtonMask |= INPUT_DPAD_UP;
    if( ngi::INPUT_KEY_DIGITAL_DOWN & aKeyCode )  nButtonMask |= INPUT_DPAD_DOWN;
    
    if( ngi::INPUT_KEY_LSK & aKeyCode ) nButtonMask |= INPUT_LEFT_BUTTON;
    if( ngi::INPUT_KEY_RSK & aKeyCode ) nButtonMask |= INPUT_RIGHT_BUTTON;

    if( ngi::INPUT_KEY_OK & aKeyCode )    nButtonMask |= INPUT_SELECT;
    if( ngi::INPUT_KEY_CLEAR & aKeyCode ) nButtonMask |= INPUT_KEY_CLEAR;
    if( ngi::INPUT_KEY_EDIT & aKeyCode )  nButtonMask |= INPUT_KEY_EDIT;
    if( ngi::INPUT_KEY_SEND & aKeyCode )  nButtonMask |= INPUT_KEY_SEND;

    if( ngi::INPUT_KEY_VOLUME_UP & aKeyCode )   nButtonMask |= INPUT_KEY_VOL_UP;
    if( ngi::INPUT_KEY_VOLUME_DOWN & aKeyCode ) nButtonMask |= INPUT_KEY_VOL_DOWN;

    if( ngi::INPUT_KEY_EXTRA_A & aKeyCode ) nButtonMask |= INPUT_KEY_EXTRA_A;
    if( ngi::INPUT_KEY_EXTRA_B & aKeyCode ) nButtonMask |= INPUT_KEY_EXTRA_B;
    if( ngi::INPUT_KEY_EXTRA_C & aKeyCode ) nButtonMask |= INPUT_KEY_EXTRA_C;
    if( ngi::INPUT_KEY_EXTRA_D & aKeyCode ) nButtonMask |= INPUT_KEY_EXTRA_D;
    if( ngi::INPUT_KEY_EXTRA_E & aKeyCode ) nButtonMask |= INPUT_KEY_EXTRA_E;
    if( ngi::INPUT_KEY_EXTRA_F & aKeyCode ) nButtonMask |= INPUT_KEY_EXTRA_F;
    if( ngi::INPUT_KEY_EXTRA_G & aKeyCode ) nButtonMask |= INPUT_KEY_EXTRA_G;
    if( ngi::INPUT_KEY_EXTRA_H & aKeyCode ) nButtonMask |= INPUT_KEY_EXTRA_H;

    // Update the application's list of which buttons are currently pressed
    if( nButtonMask )
        m_pApplication->m_Input.m_nButtons &= ~nButtonMask;
}


//--------------------------------------------------------------------------------------
// Name: InputAxisMoved()
// Desc: Callback for when an input axis is moved.
//--------------------------------------------------------------------------------------
void CFrmAppContainer::InputAxisMoved( ngi::IInputDevice& /*aInputDevice*/,
                                       ngi::uint64 /*aTimeStamp*/,
                                       ngi::uint32 /*aAxisNumber*/,
                                       ngi::int32 /*aNewAxisValue*/ )
{
}


//--------------------------------------------------------------------------------------
// Name: DeviceDisconnected()
// Desc: Callback for when an input device is disconnected.
//--------------------------------------------------------------------------------------
void CFrmAppContainer::DeviceDisconnected( ngi::IInputDevice& /*aDevice*/ )
{
}


//--------------------------------------------------------------------------------------
// Name: DeviceConnected()
// Desc: Callback for when an input device is connected.
//--------------------------------------------------------------------------------------
void CFrmAppContainer::DeviceConnected( ngi::IInputDevice& /*aDevice*/ )
{   
}

