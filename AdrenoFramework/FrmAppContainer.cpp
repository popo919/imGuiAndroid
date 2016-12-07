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

#include "FrmAppContainer.h"

#if OSX
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>
#include <unistd.h>
#endif
//--------------------------------------------------------------------------------------
// Name: WinMain()
// Desc: Application entry-point on the Windows platform
//--------------------------------------------------------------------------------------
#if LINUX_OR_OSX
int main(int argc, char *argv[])
#else
int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
#endif
{
	CFrmAppContainer appContainer;
	return (INT32)appContainer.Run();
}

UINT32 nButtonMask = 0;
UINT32 currentAction = 0;
void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	currentAction = action;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);	
	}
	else
	{
		BOOL bShiftKeyState = mods & GLFW_MOD_SHIFT;
		BOOL bCtrlKeyState  = mods & GLFW_MOD_CONTROL;

		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			switch( key )
			{
			case GLFW_KEY_1: nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_1; break;
			case GLFW_KEY_2: nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_2; break;
			case GLFW_KEY_3: nButtonMask = bShiftKeyState ? FRM_INPUT::KEY_HASH : FRM_INPUT::KEY_3; break;
			case GLFW_KEY_4: nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_4; break;
			case GLFW_KEY_5: nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_5; break;
			case GLFW_KEY_6: nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_6; break;
			case GLFW_KEY_7: nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_7; break;
			case GLFW_KEY_8: nButtonMask = bShiftKeyState ? FRM_INPUT::KEY_STAR : FRM_INPUT::KEY_8; break;
			case GLFW_KEY_9: nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_9; break;
			case GLFW_KEY_0: nButtonMask = bShiftKeyState ? FRM_INPUT::NONE     : FRM_INPUT::KEY_0; break;

			case GLFW_KEY_KP_1:  nButtonMask = FRM_INPUT::KEY_1; break;
			case GLFW_KEY_KP_2:  nButtonMask = FRM_INPUT::KEY_2; break;
			case GLFW_KEY_KP_3:  nButtonMask = FRM_INPUT::KEY_3; break;
			case GLFW_KEY_KP_4:  nButtonMask = FRM_INPUT::KEY_4; break;
			case GLFW_KEY_KP_5:  nButtonMask = FRM_INPUT::KEY_5; break;
			case GLFW_KEY_KP_6:  nButtonMask = FRM_INPUT::KEY_6; break;
			case GLFW_KEY_KP_7:  nButtonMask = FRM_INPUT::KEY_7; break;
			case GLFW_KEY_KP_8:  nButtonMask = FRM_INPUT::KEY_8; break;
			case GLFW_KEY_KP_9:  nButtonMask = FRM_INPUT::KEY_9; break;
			case GLFW_KEY_KP_0:  nButtonMask = FRM_INPUT::KEY_0; break;

			}
		}

		// <SRAUT>: TODO
		// 	// Handle Ctrl+C, to copy framebuffer contents to the clipboard
		// 	if( uMsg == WM_KEYDOWN )
		// 	{
		// 		if( bCtrlKeyState && wParam == 'C' )
		// 		{
		// 			PrintWindow( hWnd, GetWindowDC( hWnd ), PW_CLIENTONLY );
		// 
		// 			pAppContainer->CopyFramebufferToClipboard();
		// 		}
		// 	}
	}	
}

BOOL CFrmAppContainer::Run()
{
#if _WIN32 || _WIN64
	// Use the executable's directory so that relative ../Assets/Samples paths work correctly
	{
		CHAR strFilePath[256];
		GetModuleFileName( NULL, strFilePath, 256 );
		*( strrchr( strFilePath, '\\' ) ) = '\0';
		SetCurrentDirectory( strFilePath );
	}
#endif

	// Create the Application
	m_pApplication = FrmCreateApplicationInstance();

	if( NULL == m_pApplication )
		return FALSE;

	// Create the application window
	// create GLFW Window
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		exit( EXIT_FAILURE );
	}

#if CORE_GL_CONTEXT
 	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
 	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
 	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
 	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
	
	// <SRAUT> Using default glfw context attribs (major version = 1, minor version = 0, GLFW_OPENGL_PROFILE = GLFW_OPENGL_ANY_PROFILE )
	// Note GLFW_OPENGL_PROFILE = GLFW_OPENGL_CORE_PROFILE does not allow use of glVertexAttribPointer without using VAOs
	GLFWwindow *window = glfwCreateWindow( m_pApplication->m_nWidth, m_pApplication->m_nHeight, "SDK Toolkit", NULL, NULL );
	if (!window)
	{
		fprintf( stderr, "Failed to open GLFW window\n" );
		glfwTerminate();
		exit( EXIT_FAILURE );
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval( 1 );

#if LINUX_OR_OSX
	// <SRAUT> TODO: Figure out pathing for Linux and OSX 
	// ch dir to ../MacOS/
	//chdir("../../../../");
	char processDir[PATH_MAX];
	char currentDir[PATH_MAX];
	
	struct stat dirInfo;
	pid_t pid = getpid();
	
	sprintf(processDir, "/proc/%d/exe", (int)pid);
	if (readlink(processDir, currentDir, PATH_MAX))
	{
		strcpy(currentDir, dirname(currentDir));
		fprintf(stderr, "Changing dir to %s", currentDir);
		chdir(currentDir);
	}
	//uint size = sizeof(path);
	//if (_NSGetExecutablePath(path, &size) == 0)
    //	fprintf(stderr, "executable path is %s\n", path);
	//else
    //	fprintf(stderr, "buffer too small; need size %u\n", size);
	
#else
// TODO
#endif

	printf( "Calling init\n" );
	if( ( FALSE == m_pApplication->Initialize() ) ||
		( FALSE == m_pApplication->Resize() ) )
	{
#if LINUX_OR_OSX
		printf("Error %s: Application failed during scene intialization!\n", m_pApplication->m_strName);
#else
		MessageBox( NULL, "Application failed during scene initialization!\n"
			"\n"
			"Please debug accordingly.", 
			m_pApplication->m_strName, MB_ICONERROR );
#endif
		return FALSE;
	}
	printf("Main loop\n" );
	// set key handler
	glfwSetKeyCallback(window, (GLFWkeyfun)&KeyboardCallback);


	// Main loop
	while( !glfwWindowShouldClose(window) )
	{
		// update input
		if( nButtonMask )
		{
			if (currentAction == GLFW_PRESS)
			{
				m_pApplication->m_Input.m_nButtons |= nButtonMask;
			}
			else
			{
				m_pApplication->m_Input.m_nButtons &= ~nButtonMask;
			}
		}

		// Update and render the application
		m_pApplication->Update();
		m_pApplication->Render();

		// Swap buffers
		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	printf("Calling terminate\n");
	// Terminate GLFW
	glfwTerminate();	

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
// 	if( m_pApplication )
// 	{
// 		m_pApplication->Destroy();
// 		delete m_pApplication;
// 	}
}

