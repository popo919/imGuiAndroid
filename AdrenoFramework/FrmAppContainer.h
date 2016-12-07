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
#include "FrmInput.h"

#include <stdio.h>
#if defined (_WIN32) || (_WIN64)
#include <crtdbg.h>
#endif
#include "glfw3.h"

// to make this function c-compatible, use static
static void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);


//--------------------------------------------------------------------------------------
// Name: class CFrmAppContainer()
// Desc: Application class container
//--------------------------------------------------------------------------------------
class CFrmAppContainer
{
public:
	GLFWwindow*		  m_Window;
	CFrmApplication*  m_pApplication;	

public:
	CFrmAppContainer();
	~CFrmAppContainer();

	BOOL Run();

};
