//--------------------------------------------------------------------------------------
// File: FrmComputeApplication_Android.cpp
// Desc: Android implementation of the Compute Framework application
//
// Author:      QUALCOMM, Advanced Content Group - Snapdragon SDK
//
//               Copyright (c) 2014 QUALCOMM Technologies, Inc.
//                         All Rights Reserved.
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include "FrmPlatform.h"
#include "FrmComputeApplication.h"
#include "FrmUtils.h"

extern char* g_pAppName;
extern AAssetManager* g_pAssetManager;
extern const char* g_pInternalDataPath;
extern android_app* g_pApp;

//--------------------------------------------------------------------------------------
// Name: class CFrmAppContainer()
// Desc: Application class container
//--------------------------------------------------------------------------------------
class CFrmAppContainer
{
public:
	CFrmAppContainer(cl_device_type deviceType, BOOL bRunTests,
                        int wgSize=100, int wg2DWHRatio=50, bool useLocal=false);
	~CFrmAppContainer();


	BOOL Run(cl_device_type deviceType);
	VOID Destroy();

	VOID HandleCommand(int32_t command);
	VOID SetAndroidApp(android_app* pApp)	{ m_pAndroidApp = pApp; }

protected:
	CFrmComputeApplication* m_pApplication;
	android_app* m_pAndroidApp;
	cl_device_type m_deviceType;
	BOOL m_bRunTests;
    int m_WGSize;
    int m_WG2DWHRatio;
    bool m_useLocal;
};

//--------------------------------------------------------------------------------------
// Name: CFrmAppContainer()
// Desc: Constructor
//--------------------------------------------------------------------------------------
CFrmAppContainer::CFrmAppContainer(cl_device_type deviceType, BOOL bRunTests,
                                    int wgSize, int wg2DWHRatio, bool useLocal) :
		m_pApplication(NULL),
		m_pAndroidApp(NULL),
		m_deviceType(deviceType),
		m_bRunTests(bRunTests),
        m_WGSize(wgSize),
        m_WG2DWHRatio(wg2DWHRatio),
        m_useLocal(useLocal)
{
}

//--------------------------------------------------------------------------------------
// Name: ~CFrmAppContainer()
// Desc: Destructor
//--------------------------------------------------------------------------------------
CFrmAppContainer::~CFrmAppContainer()
{
	if (m_pApplication)
	{
		m_pApplication->Destroy();
		delete m_pApplication;
	}

	if ( g_pAppName )
	{
		delete [] g_pAppName;
		g_pAppName = NULL;
	}
}

//--------------------------------------------------------------------------------------
// Name: Run()
// Desc: Run the computation
//--------------------------------------------------------------------------------------
BOOL CFrmAppContainer::Run(cl_device_type deviceType)
{
	m_pApplication = FrmCreateComputeApplicationInstance();
	if (NULL == m_pApplication)
	{
		return FALSE;
	}

	// Store the name of the sample for the log, the format of the log prefix will be
	// OpenCL 1.1 AppName
	const CHAR *pLogPrefix = "OpenCL 1.1 ";
	g_pAppName = new CHAR[ FrmStrlen( m_pApplication->m_strName ) + FrmStrlen( pLogPrefix ) + 1];
	FrmSprintf( g_pAppName, FrmStrlen( m_pApplication->m_strName ) + FrmStrlen( pLogPrefix ),
				"%s%s", pLogPrefix, m_pApplication->m_strName );

	// Set whether to run tests
	m_pApplication->m_bRunTests = m_bRunTests;
    if (m_WGSize == 0) m_WGSize = 1;
    m_pApplication->m_WGSize = m_WGSize;
    m_pApplication->m_WG2DWHRatio = m_WG2DWHRatio;
    m_pApplication->m_useLocal = m_useLocal;

	if (FALSE == m_pApplication->CreateOpenCLContext(deviceType))
	{
		return FALSE;
	}

	if (FALSE == m_pApplication->Initialize())
	{
		return FALSE;
	}

	BOOL result = m_pApplication->Compute();
	if ( m_bRunTests )
	{
		// Log test results if tests were run
		char msg[256];
		FrmSprintf( msg, sizeof(msg), "RunTests: %s\n", result ? "PASSED" : "FAILED" );
		FrmLogMessage( msg );
	}
}

//--------------------------------------------------------------------------------------
// Name: Destroy()
// Desc:
//--------------------------------------------------------------------------------------
void CFrmAppContainer::Destroy()
{
	// destroy the process
	exit(0);
}

//--------------------------------------------------------------------------------------
// Name: HandleCommand()
// Desc: Filter and respond to Android commands
//--------------------------------------------------------------------------------------
VOID CFrmAppContainer::HandleCommand(int32_t cmd)
{
	switch (cmd)
	{
	case APP_CMD_GAINED_FOCUS:
		Run( m_deviceType );
		break;

	case APP_CMD_TERM_WINDOW:
		Destroy();
		break;

	default:
		break;
	}
}

//--------------------------------------------------------------------------------------
// Name: CommandCallback()
// Desc: Android command handler
//--------------------------------------------------------------------------------------
static void CommandCallback(android_app* pApp, int32_t cmd)
{
	CFrmAppContainer* pAppContainer = (CFrmAppContainer*) pApp->userData;
	pAppContainer->HandleCommand(cmd);
}


//--------------------------------------------------------------------------------------
// Name: android_main()
// Desc: Application entry point
//--------------------------------------------------------------------------------------
void android_main(android_app* pApp)
{
	// Native glue may get stripped if this is omitted
	app_dummy();

	g_pApp = pApp;

	// Parse command-line options.  Options can be specified using
	// "am start" command.  The following options are supported
	// -e DEVICE [cpu|gpu|all] <- OpenCL device type (default: gpu)
	// -e RUNTESTS [1|0]   <- Whether to run tests (default: 0)
	JavaVM* vm = pApp->activity->vm;
	JNIEnv* env;

	// Call into the Java Method getIntent().getStringExtra() to get the options
	// string.  Use the JVM to invoke the method.
	vm->AttachCurrentThread( &env, 0 );
	jobject me = pApp->activity->clazz;
	jclass acl = env->GetObjectClass( me );
	jmethodID giid = env->GetMethodID( acl, "getIntent", "()Landroid/content/Intent;" );
	jobject intent = env->CallObjectMethod( me, giid );
	jclass icl = env->GetObjectClass( intent );
	jmethodID gseid = env->GetMethodID( icl, "getStringExtra", "(Ljava/lang/String;)Ljava/lang/String;" );

    jmethodID getIntID = env->GetMethodID( icl, "getIntExtra", "(Ljava/lang/String;I)I" );
    jmethodID getBoolID = env->GetMethodID( icl, "getBooleanExtra", "(Ljava/lang/String;Z)Z" );

	// Get the options
    jstring strDEVICE = env->NewStringUTF( "DEVICE" );
    jstring strRUNTESTS = env->NewStringUTF( "RUNTESTS" );
    jstring jsParamDevice = (jstring)env->CallObjectMethod( intent, gseid, strDEVICE );
    jstring jsParamRunTests = (jstring)env->CallObjectMethod( intent, gseid, strRUNTESTS );

    jstring strMAX_WORKGROUP_SIZE = env->NewStringUTF( "MAX_WORKGROUP_SIZE" );
    jstring strWORKGROUP_WH_RATIO = env->NewStringUTF( "WORKGROUP_WH_RATIO" );
    jstring strUSE_LOCAL_MEM =  env->NewStringUTF( "USE_LOCAL_MEM" );
    int jsParamWGSize = (int)env->CallIntMethod( intent, getIntID, strMAX_WORKGROUP_SIZE, 100);
    int jsParamWGRatio = (int)env->CallIntMethod( intent, getIntID, strWORKGROUP_WH_RATIO, 0);
    bool jsParamLocalMem = (bool)env->CallBooleanMethod( intent, getBoolID, strUSE_LOCAL_MEM, false);

    env->DeleteLocalRef(strDEVICE);
    env->DeleteLocalRef(strRUNTESTS);
    env->DeleteLocalRef(strMAX_WORKGROUP_SIZE);
    env->DeleteLocalRef(strWORKGROUP_WH_RATIO);
    env->DeleteLocalRef(strUSE_LOCAL_MEM);

	const char* deviceOpt = env->GetStringUTFChars( jsParamDevice, 0 );
	const char* runTestsOpt = env->GetStringUTFChars( jsParamRunTests, 0 );

	// Parse option:
	// -e DEVICE [cpu|gpu] <- OpenCL device type (default: gpu)
	cl_device_type deviceType = CL_DEVICE_TYPE_GPU;
	char msgDevice[256];
	if ( deviceOpt != NULL )
	{
		FrmSprintf( msgDevice, sizeof(msgDevice), "Parsing option DEVICE = '%s'\n", deviceOpt );
		if ( FrmStricmp( deviceOpt, "cpu" ) == 0 )
		{
			deviceType = CL_DEVICE_TYPE_CPU;
		}
		else if ( FrmStricmp( deviceOpt, "gpu" ) == 0 )
		{
			deviceType = CL_DEVICE_TYPE_GPU;
		}
		else if ( FrmStricmp( deviceOpt, "all" ) == 0 )
		{
			deviceType = CL_DEVICE_TYPE_ALL;
		}
		else
		{
			FrmLogMessage( "Unknown value for DEVICE, defaulting to GPU\n");
		}
	}

	// Parse option:
	// -e RUNTESTS [1|0]   <- Whether to run tests (default: 0)
	BOOL bRunTests = FALSE;
	char msgRunTests[256];
	if ( runTestsOpt != NULL )
	{
		FrmSprintf( msgRunTests, sizeof(msgRunTests), "Parsing option RUNTESTS = '%s'\n", runTestsOpt );
		if ( FrmStricmp( runTestsOpt, "1" ) == 0 || FrmStricmp( runTestsOpt, "true" ) == 0 )
		{
			bRunTests = TRUE;
		}
		else if ( FrmStricmp( runTestsOpt, "0" ) || FrmStricmp( runTestsOpt, "false" ) == 0 )
		{
			bRunTests = FALSE;
		}
		else
		{
			FrmLogMessage( "Unknown value for RUNTESTS, defaulting to 0\n.");
		}
	}


	env->ReleaseStringUTFChars( jsParamDevice, deviceOpt );
	env->ReleaseStringUTFChars( jsParamRunTests, runTestsOpt );
	vm->DetachCurrentThread();

	FrmLogMessage( msgDevice );
	FrmLogMessage( msgRunTests );

	CFrmAppContainer appContainer(deviceType, bRunTests, jsParamWGSize, jsParamWGRatio, jsParamLocalMem);
	appContainer.SetAndroidApp(pApp);
	pApp->userData = &appContainer;
	pApp->onAppCmd = CommandCallback;

	g_pAssetManager = pApp->activity->assetManager;
	g_pInternalDataPath = pApp->activity->internalDataPath;

	while (1)
	{
		// Read all pending events.
		int ident;
		int events;
		struct android_poll_source *source=NULL;


		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue
		// to draw the next frame of animation.
		while ((ident=ALooper_pollAll(-1, NULL, &events,
				(void**)&source)) >= 0) {

			// Process this event.
			if (source != NULL)
			{
				source->process(pApp, source);
			}

			// Check if we are exiting.
			if (pApp->destroyRequested != 0)
			{
				return;
			}
		}
	}
}
