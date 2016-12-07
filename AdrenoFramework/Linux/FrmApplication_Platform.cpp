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


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>

#include <stdio.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>

#include <stdio.h>

#if OSX
#include <mach-o/dyld.h>	/* _NSGetExecutablePath */
#include <sys/param.h>
#endif

#include<OpenGLES/FrmGLES3.h>	// Get the extension functions and defines


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
    BOOL InitializeEGL();
    
public:
    CFrmAppContainer();
    ~CFrmAppContainer();

    BOOL Run();
    BOOL HandleEvents();
    VOID ToggleOrientation();
};


//--------------------------------------------------------------------------------------
// Name: main()
// Desc: Application entry-point on the Linux platform
//--------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    CFrmAppContainer appContainer;
    return (int)appContainer.Run();
}


bool HandleInput(XEvent ev, CFrmApplication* app)
{
   char string[25];
   int len;
   KeySym keysym;
   len = XLookupString(&ev.xkey, string, 25, &keysym, NULL);
   //if (len < 0)
   //{
      /* printf("if (len < 0)..\n");
      if (string[0] == 'r')
      {
         string[0] = 'n';
      }*/
      UINT32 nButtonMask = 0;


      BOOL bShiftKeyState = ( keysym==XK_Shift_L || keysym==XK_Shift_L ) ? TRUE : FALSE;
      BOOL bCtrlKeyState  = ( keysym==XK_Control_L || keysym==XK_Control_R ) ? TRUE : FALSE;



      // Apply native key presses to internal monitoring of keyboard/mouse state
      switch( keysym )
      {
            case XK_1:         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_1; break;
            case XK_2:         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_2; break;
            case XK_3:         nButtonMask = bShiftKeyState ? FRM_INPUT::KEY_HASH : FRM_INPUT::KEY_3; break;
            case XK_4:         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_4; break;
            case XK_5:         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_5; break;
            case XK_6:         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_6; break;
            case XK_7:         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_7; break;
            case XK_8:         nButtonMask = bShiftKeyState ? FRM_INPUT::KEY_STAR : FRM_INPUT::KEY_8; break;
            case XK_9:         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_9; break;
            case XK_0:         nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_0; break;
            case XK_KP_1:  nButtonMask = FRM_INPUT::KEY_1; break;
            case XK_KP_2:  nButtonMask = FRM_INPUT::KEY_2; break;
            case XK_KP_3:  nButtonMask = FRM_INPUT::KEY_3; break;
            case XK_KP_4:  nButtonMask = FRM_INPUT::KEY_4; break;
            case XK_KP_5:  nButtonMask = FRM_INPUT::KEY_5; break;
            case XK_KP_6:  nButtonMask = FRM_INPUT::KEY_6; break;
            case XK_KP_7:  nButtonMask = FRM_INPUT::KEY_7; break;
            case XK_KP_8:  nButtonMask = FRM_INPUT::KEY_8; break;
            case XK_KP_9:  nButtonMask = FRM_INPUT::KEY_9; break;
            case XK_KP_0:  nButtonMask = FRM_INPUT::KEY_0; break;
            case XK_KP_Multiply: nButtonMask = FRM_INPUT::KEY_STAR; break;
            case XK_Left:     nButtonMask = FRM_INPUT::DPAD_LEFT;   break;
            case XK_Right:    nButtonMask = FRM_INPUT::DPAD_RIGHT;  break;
            case XK_Up:       nButtonMask = FRM_INPUT::DPAD_UP;     break;
            case XK_Down:     nButtonMask = FRM_INPUT::DPAD_DOWN;   break;
            case XK_KP_Space:    nButtonMask = FRM_INPUT::SELECT;   break;
            case XK_Return:   nButtonMask = FRM_INPUT::SELECT;      break;
            case XK_BackSpace:     nButtonMask = FRM_INPUT::KEY_CLEAR;  break;
            case XK_KP_Delete:   nButtonMask = FRM_INPUT::KEY_CLEAR;  break;
            case XK_Insert:   nButtonMask = FRM_INPUT::KEY_EDIT;   break;
            case XK_Escape: 
               return FALSE;
               break;
      }

      if( nButtonMask )
      {
         if( ev.type == KeyPress )
            app->m_Input.m_nButtons |= nButtonMask;
         else
            app->m_Input.m_nButtons &= ~nButtonMask;
      }


   //}

      return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: HandleEvents()
// Desc: Pumps the application window's message queue
//--------------------------------------------------------------------------------------
BOOL CFrmAppContainer::HandleEvents()
{
   if(XPending(this->m_hDisplay))
   {
       XEvent ev;

      // event handling goes here
      XNextEvent(this->m_hDisplay, &ev);
      switch(ev.type)
      {
         case KeymapNotify:
            XRefreshKeyboardMapping(&ev.xmapping);
            break;
         case KeyPress:
         case KeyRelease:
            {
               // (Handle keyboard/mouse input) If Escape key was pressed...
               if( FALSE==HandleInput(ev,m_pApplication) )
               {
                  return FALSE;   // Exit out.
               }
            }
            break;


            case ButtonPress:
            {
               int32_t mx= ev.xbutton.x, my= ev.xbutton.y;
               FRMVECTOR2 vPointerPosition;
               vPointerPosition.x = +2.0f * mx/(FLOAT32)(this->m_pApplication->m_nWidth -1) - 1.0f;
               vPointerPosition.y = -2.0f * my/(FLOAT32)(this->m_pApplication->m_nHeight-1) + 1.0f;

               this->m_pApplication->m_Input.m_vPointerPosition = vPointerPosition;
               this->m_pApplication->m_Input.m_nPointerState  = FRM_INPUT::POINTER_DOWN;
               this->m_pApplication->m_Input.m_nPointerState |= FRM_INPUT::POINTER_PRESSED;
               break;
            }


            case MotionNotify:
               if(this->m_pApplication->m_Input.m_nPointerState == FRM_INPUT::POINTER_DOWN)
               { 
                  int32_t dx = ev.xmotion.x;
                  int32_t dy = ev.xmotion.y;
                     
                  FRMVECTOR2 vPointerPosition;
                  vPointerPosition.x = +2.0f * dx/(FLOAT32)(this->m_pApplication->m_nWidth -1) - 1.0f;
                  vPointerPosition.y = -2.0f * dy/(FLOAT32)(this->m_pApplication->m_nHeight-1) + 1.0f;

                  this->m_pApplication->m_Input.m_vPointerPosition = vPointerPosition;
                  this->m_pApplication->m_Input.m_nPointerState = FRM_INPUT::POINTER_DOWN;
               } 
               break; 
 
            case ButtonRelease:
               {
                  int32_t mx= ev.xbutton.x, my= ev.xbutton.y;
                  FRMVECTOR2 vPointerPosition;
                  vPointerPosition.x = +2.0f * mx/(FLOAT32)(this->m_pApplication->m_nWidth -1) - 1.0f;
                  vPointerPosition.y = -2.0f * my/(FLOAT32)(this->m_pApplication->m_nHeight-1) + 1.0f;

                  this->m_pApplication->m_Input.m_vPointerPosition = vPointerPosition;
                  this->m_pApplication->m_Input.m_nPointerState = FRM_INPUT::POINTER_RELEASED;
                  break;
               }

         //case EnterNotify:
         //case LeaveNotify: 

         case ConfigureNotify:
            // TODO: Get a new framebuffer/FB config when window resized..
            //if (width != ev.xconfigure.width || height != ev.xconfigure.height) 
            //{
            //   width = ev.xconfigure.width;
            //   height = ev.xconfigure.height;
            //   //printf("Size changed to: %d by %dn", width, height);
            //}
            break;
         //case Expose:   // Draw window event..
         //   if(ev.xexpose.count==0)
         //   {
         //      m_pApplication->Render();
         //      XFlush(this->m_hDisplay); 
         //   }
         //   break;
      }


   }
   return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: Run()
// Desc: Create the framework, initialize the application, and render frames.
//--------------------------------------------------------------------------------------
BOOL CFrmAppContainer::Run()
{
    // Use the executable's directory so that relative ../Assets/Samples paths work correctly
    {
      char processDir[PATH_MAX];
      char currentDir[PATH_MAX];

      struct stat dirInfo;
      pid_t pid = getpid();

	  uint bufsize= PATH_MAX;
	  char *result=getcwd(currentDir,PATH_MAX);	  
      
	  if(result!= 0)
      {
         int ret = chdir(currentDir);
         
         if(0!=ret)
         {
         	printf("CFrmAppContainer::Run(): chdir FAILED..\n");
         }
      }
      else
      {
         printf("CFrmAppContainer::Run() Error finding executable directory.\n");
         return FALSE;
      }
    }

    // Create the Application
    m_pApplication = FrmCreateApplicationInstance();

    if (NULL == m_pApplication)
        return FALSE;


    // do some EGL stuff before creating the window
    if (FALSE == InitializeEGL())
        return FALSE;

    // Create the application window
    if (FALSE == CreateNativeWindow(&m_hWindow, &m_hDisplay))
        return FALSE;


    // Create a window surface
    m_pApplication->m_eglSurface = eglCreateWindowSurface( m_pApplication->m_eglDisplay, m_pApplication->m_eglConfig, m_hWindow, NULL );
    if( EGL_NO_SURFACE == m_pApplication->m_eglSurface )
    {
        return false;
    }

    EGLint ContextAttribList[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    m_pApplication->m_eglContextGL = eglCreateContext( m_pApplication->m_eglDisplay, m_pApplication->m_eglConfig, EGL_NO_CONTEXT, ContextAttribList );
    if( EGL_NO_CONTEXT == m_pApplication->m_eglContextGL )
    {
        return false;
    }


    // Make the context current
    // eglDisplay, eglSurface, eglSurface, eglContext
    if( false == eglMakeCurrent( m_pApplication->m_eglDisplay, m_pApplication->m_eglSurface, m_pApplication->m_eglSurface, m_pApplication->m_eglContextGL ) )
    {
        return false;
    }


    // TODO: rest of EGL setup goes here
    //eglMakeCurrent(m_pApplication->m_eglDisplay, m_pApplication->m_eglSurface, m_pApplication->m_eglSurface, m_pApplication->m_eglContextGL);
    if ((FALSE == m_pApplication->Initialize()) || (FALSE == m_pApplication->Resize()))
    {
	     printf("Error %s: Application failed during scene initialization!\n", m_pApplication->m_strName);
        return FALSE;
    }
    // Run the main loop
    while (TRUE == this->HandleEvents())
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
/*    SetWindowPos( (HWND)m_hWindow, NULL, 0, 0, nNewWidth, nNewHeight,
                  SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);*/

    // Resize the parent frame window
/*
    RECT rc;
    GetWindowRect( GetParent((HWND)m_hWindow), &rc );
    SetWindowPos( GetParent((HWND)m_hWindow), NULL, 0, 0, 
                  ((rc.right-rc.left)-nOldWidth)+nNewWidth, 
                  ((rc.bottom-rc.top)-nOldHeight)+nNewHeight,
                  SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);*/

    // Update the app variables
    m_pApplication->m_nWidth  = nNewWidth;
    m_pApplication->m_nHeight = nNewHeight;

    // Update the OpenGL viewport
    // Note: The derived app class should do this during the Render() function!
    m_pApplication->Resize();
}





//--------------------------------------------------------------------------------------
// Name: CreateNativeWindow()
// Desc: Creates a window for the application
//--------------------------------------------------------------------------------------
BOOL CFrmAppContainer::CreateNativeWindow( NativeWindowType* pWindow,
                                           NativeDisplayType* pDisplay )
{
    //Display *display = XOpenDisplay(NULL);
    
    if (this->m_hDisplay == NULL)
    {
        FrmLogMessage("ERROR: Unable to open a connection to the X server.\n");
        return FALSE;
    }

    //Visual *visual = DefaultVisual(display, DefaultScreen(display));






   EGLint vid;
   XVisualInfo *vi, visTemplate;
   int num_visuals;

   if (!eglGetConfigAttrib(this->m_pApplication->m_eglDisplay, this->m_pApplication->m_eglConfig, EGL_NATIVE_VISUAL_ID, &vid)) 
   {
      printf("Error: eglGetConfigAttrib() failed\n");
      return false;
   }

   visTemplate.visualid = vid;
   vi = XGetVisualInfo(this->m_hDisplay, VisualIDMask, &visTemplate, &num_visuals);
   if (!vi) 
   {
      printf("Error: couldn't get X visual\n");
      return false;
   }
    // TODO: tutorial did something different here
    /*visTemplate.visualid = XVisualIDFromVisual(visual);
    vi = XGetVisualInfo(display, VisualIDMask, &visTemplate, &num_visuals);*/
    if (!vi) 
    {
       printf("ERROR: XGetVisualInfo failed with visualid %d.\n", visTemplate.visualid);
       return FALSE;
    }

    XSetWindowAttributes attributes;
    attributes.border_pixel = 0;
    attributes.background_pixmap = None;
    attributes.event_mask = StructureNotifyMask;
    attributes.colormap = XCreateColormap(this->m_hDisplay, RootWindow(this->m_hDisplay, vi->screen), vi->visual, AllocNone);

    Window window = XCreateWindow(this->m_hDisplay, RootWindow(this->m_hDisplay, vi->screen), 50, 50,
				  m_pApplication->m_nWidth, m_pApplication->m_nHeight,
				  0, vi->depth, InputOutput, vi->visual, 
				  CWColormap | CWBorderPixel | CWEventMask | ExposureMask,
				  &attributes);

    XSelectInput(this->m_hDisplay,window,ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | KeyPressMask | KeyReleaseMask);

    XFree(vi);
    XMapWindow(this->m_hDisplay, window);

     

    // Note: We delay showing the window until after Initialization() succeeds
    // Otherwise, an unsightly, empty window briefly appears during initialization

    (*pWindow)  = (NativeWindowType)window;
    return TRUE;
}

BOOL CFrmAppContainer::InitializeEGL()
{
   this->m_hDisplay = XOpenDisplay(0);
	this->m_pApplication->m_eglDisplay = eglGetDisplay(this->m_hDisplay);
	eglInitialize(this->m_pApplication->m_eglDisplay, 0, 0);
   eglBindAPI(EGL_OPENGL_ES_API);

	EGLConfig config;
	EGLint numConfigs;
	EGLint format;
	EGLint configAttribs[] = 
	{
		EGL_SURFACE_TYPE,		EGL_WINDOW_BIT,
		EGL_RED_SIZE,			5,
		EGL_GREEN_SIZE,	    	6,
		EGL_BLUE_SIZE,	    	5,
		EGL_DEPTH_SIZE,			16,
		EGL_STENCIL_SIZE,       8,
#ifdef _OGLES3
        EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES3_BIT_KHR,
#else
        EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES2_BIT,
#endif
		EGL_NONE
	};
	
   // Determine number of configs..
   if( false == eglGetConfigs( this->m_pApplication->m_eglDisplay, NULL, 0, &numConfigs ) )
   {
      return false;
   }

   EGLConfig configs[10];
   // Get the first fb config..
   if (false == eglGetConfigs(this->m_pApplication->m_eglDisplay, configs, 1, &numConfigs) )
   {
      return false;
   }

    EGLint nBuffSize;
    eglGetConfigAttrib(m_pApplication->m_eglDisplay, configs[0], EGL_BUFFER_SIZE, &nBuffSize);



    // Test the display configuration
    //EGLConfig m_pApplication->m_eglConfig;

    if (nBuffSize == 32)
    {
        // Build the attibute list
        EGLint AttributeList[] = 
        { 
            EGL_RED_SIZE,		  8,
            EGL_GREEN_SIZE,	  8,
            EGL_BLUE_SIZE,	     8,
            EGL_ALPHA_SIZE,	  8,
            EGL_DEPTH_SIZE,	  24,
            EGL_STENCIL_SIZE,   8,
            EGL_SAMPLE_BUFFERS, 0,
            EGL_SAMPLES,		  0,
            EGL_NONE
        };

        // Choose config based on the requested attributes
        if( false == eglChooseConfig( m_pApplication->m_eglDisplay, AttributeList, &m_pApplication->m_eglConfig, 1, &numConfigs ) )
        {
            return false;
        }
    }
    else if (nBuffSize == 16)
    {
        EGLint AttributeList[] = 
        { 
            EGL_RED_SIZE,		   5,
            EGL_GREEN_SIZE,	   6,
            EGL_BLUE_SIZE,	      5,
            EGL_ALPHA_SIZE,	   0,
            EGL_DEPTH_SIZE,	   24,
            EGL_STENCIL_SIZE,    8,
            EGL_SAMPLE_BUFFERS,  0,
            EGL_SAMPLES,		   0,
            EGL_NONE
        };
        
        // Choose config based on the requested attributes
        if( false == eglChooseConfig( m_pApplication->m_eglDisplay, AttributeList, &m_pApplication->m_eglConfig, 1, &numConfigs ) )
        {
            return false;
        }
    }
    else 
    {
        return false; // unsupported display
    }

	//EGLSurface surface = eglCreateWindowSurface(display, config, m_hWindow, NULL);
	//
	//EGLint contextAttribs[] = 
 //  { 
 //     EGL_CONTEXT_CLIENT_VERSION,  2, 
 //     EGL_NONE
 //  };


	//EGLContext context = eglCreateContext(display, config, NULL, contextAttribs);
 // 
	//// TODO: eglMakeCurrent here may be too early.. verify with tutorial code
	//if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
	//{
	//	return FALSE;
	//}
	

	
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

    XDestroyWindow(m_hDisplay, m_hWindow);

}

