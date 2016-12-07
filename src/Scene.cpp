//--------------------------------------------------------------------------------------
// File: Scene.cpp
// Desc: 
//
// Author:      QUALCOMM, Advanced Content Group - Snapdragon SDK
//
//               Copyright (c) 2013 QUALCOMM Technologies, Inc. 
//                         All Rights Reserved. 
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------
#include <FrmPlatform.h>
#define GL_GLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <FrmInput.h>

#include "FmtConversion.h"
#include <FrmApplication.h>
#include <OpenGLES/FrmShader.h>
#include <stdio.h>
#include "Scene.h"
#include "imgui.h"
#include "imgui_impl_android_gl3.h"

#if defined(__linux__)
#include <stdio.h>
#include <unistd.h>
#endif

void GLError(string info)
{
	GLenum error = glGetError();
	LOGI("%s", info.c_str());
	switch (error)
	{
	case GL_INVALID_ENUM:
		LOGI("GL_INVALID_ENUM");
		break;
	case GL_INVALID_VALUE:
		LOGI("GL_INVALID_VALUE");
		break;
	case GL_INVALID_OPERATION:
		LOGI("GL_INVALID_OPERATION");
		break;
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		LOGI("GL_INVALID_FRAMEBUFFER_OPERATION");
		break;
	case GL_OUT_OF_MEMORY:
		LOGI("GL_OUT_OF_MEMORY");
		break;
	default:
		LOGI("clean");
	}
}

//--------------------------------------------------------------------------------------
// Name: FrmCreateApplicationInstance()
// Desc: Global function to create the application instance
//--------------------------------------------------------------------------------------
CFrmApplication* FrmCreateApplicationInstance()
{
    return new CSample( "RenderToTextureOGLES30" );
}

//--------------------------------------------------------------------------------------
// Name: CSample()
// Desc: Constructor
//--------------------------------------------------------------------------------------
CSample::CSample( const CHAR* strName ) : CFrmApplication( strName )
{
    g_strWindowTitle = strName;
    g_nWindowWidth   = 1920;
    g_nWindowHeight  = 1080;
    g_fAspectRatio   = (FLOAT)g_nWindowWidth / (FLOAT)g_nWindowHeight;


    g_pOffscreenFBO = NULL;

    g_hShaderProgram        = 0;
    g_hOverlayShaderProgram = 0;
    g_hModelViewMatrixLoc   = 0;
    g_hProjMatrixLoc        = 0;
    g_hUniformBlock         = 0;
    g_hTransformBufferName  = 0;
    
    g_hBuffer			    = 0;
    
    g_hVertexArrayName		= 0;
    g_QueryName			    = 0;

}

//--------------------------------------------------------------------------------------
// Name: InitShaders()
// Desc: Initialize the shaders
//--------------------------------------------------------------------------------------
BOOL CSample::InitShaders()
{
    g_strVertexShader = 
	"#version 300 es													\n"
	"// Declare all the semantics										\n"
	"#define ATTR_POSITION	0											\n"
	"#define ATTR_COLOR		1											\n"
	"																	\n"
	"layout(location = ATTR_POSITION) in vec4 g_vPosition;				\n"
	"layout(location = ATTR_COLOR) in vec4 g_vColor;					\n"
	"                                                                   \n"
	"// API will use Transform.matModelView                             \n"
	"// in case of array the shader will use transform[2].matModelView  \n"
	"// otherwise the variable names can be directly used				\n"
	"uniform Transform													\n"
	"{																	\n"
	"	mat4x4 matModelView;											\n"
	"	mat4x4 matProj;													\n"
	"};																	\n"
	"                                                                   \n"
	// OGLES 3.0 guide: Vertex shaders cannot have output layout qualifiers
	"out vec4 VSColor;													\n"
    "                                                                   \n"
    "void main()														\n"
    "{																	\n"
	"	 vec4 Position = g_vPosition;									\n"
	"    vec4 vPositionES = matModelView * (Position);                  \n"
    "    gl_Position  = (matProj * vPositionES);                        \n"
    "    VSColor = g_vColor;                                            \n"
    "}																   	\n";

    g_strFragmentShader = 
	"#version 300 es	 												\n"
	"// Declare all the semantics										\n"
	"#define FRAG_COLOR		0											\n"
	"																	\n"
    "#ifdef GL_FRAGMENT_PRECISION_HIGH									\n"
    "   precision highp float;											\n"
    "#else																\n"
    "   precision mediump float;										\n"
    "#endif																\n"
    "																	\n"
	"in vec4 VSColor;													\n"
	"		                                                            \n"
    "layout(location = FRAG_COLOR) out vec4 Color;						\n"
    "																	\n"
    "void main()														\n"
    "{																	\n"
    "    Color = VSColor;   									        \n"
    "}																	\n";


//--------------------------------------------------------------------------------------
// Name: g_strOverlayVertexShader / g_strOverlayFragmentShader
// Desc: The vertex and fragment shader programs
//--------------------------------------------------------------------------------------
    g_strOverlayVertexShader =
	"#version 300 es													\n"
	"// Declare all the semantics										\n"
	"#define ATTR_POSITION	0										    \n"
	"                                                                   \n"
	"layout(location = ATTR_POSITION) in vec4 g_vVertex;				\n"
    "out vec2 g_vTexCoord;                                              \n"
    "														            \n"
    "void main()											            \n"
    "{														            \n"
    "    float InstanceIDf = float(gl_InstanceID);                      \n"
	// play around with the instance value to place the render target previews 
    "    gl_Position.x = g_vVertex.x + InstanceIDf * 0.2;               \n"
    "    if(gl_InstanceID > 1)							                \n"
	"    {												                \n"
	"		gl_Position.x = g_vVertex.x + (InstanceIDf - 2.0)* 0.2;     \n"
	"		gl_Position.y = g_vVertex.y - 0.2;                          \n"
	"	 }												                \n"
	"    else				                                            \n"
	"		gl_Position.y = g_vVertex.y;					            \n"
	"                                                                   \n"
    "    gl_Position.z = 0.0;                                           \n"
    "    gl_Position.w = 1.0;                                           \n"
    "                                                                   \n"
    "    g_vTexCoord.x = g_vVertex.z;                                   \n"
    "    g_vTexCoord.y = g_vVertex.w;                                   \n"
    "}                                                                  \n";

    g_strOverlayFragmentShader =
	"#version 300 es                    								\n"
	"// Declare all the semantics										\n"
	"#define FRAG_COLOR		0			    							\n"
	"                                                                   \n"
    "#ifdef GL_FRAGMENT_PRECISION_HIGH						            \n"
    "   precision highp float;					                        \n"
    "#else													            \n"
    "   precision mediump float;					                    \n"
    "#endif							                                    \n"
	"                                                                   \n"
    "uniform sampler2D g_Texture;                                       \n"
	"                                                                   \n"
    "in vec2 g_vTexCoord;									            \n"	
	"layout(location = FRAG_COLOR) out vec4 Color;			            \n"
    "                                                                   \n"
    "void main()                                                        \n"
    "{                                                                  \n"
    "    Color = texture( g_Texture, g_vTexCoord );                     \n"
    "}                                                                  \n";
    
    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: CreateFBO()
// Desc: Create objects needed for rendering to an FBO
//--------------------------------------------------------------------------------------
BOOL CSample::CreateFBO( UINT32 nWidth, UINT32 nHeight, UINT32 nFormat, UINT32 nType,
                FrameBufferObject** ppFBO )
{
    (*ppFBO) = new FrameBufferObject;
    (*ppFBO)->m_nWidth  = nWidth;
    (*ppFBO)->m_nHeight = nHeight;

    // Create an offscreen texture
    glGenTextures( 1, &(*ppFBO)->m_hTexture );
    glBindTexture( GL_TEXTURE_2D, (*ppFBO)->m_hTexture );
    
    // ARB_texture_swizzle support goes here ...
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);

	// Create a sampler object
	{
		// The following parameters are not part of the OpenGL ES 3.0 spec:
		// GL_TEXTURE_BORDER_COLOR
		// GL_TEXTURE_LOD_BIAS
		glGenSamplers(1, &(*ppFBO)->m_hSampler);
		glSamplerParameteri((*ppFBO)->m_hSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glSamplerParameteri((*ppFBO)->m_hSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glSamplerParameteri((*ppFBO)->m_hSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri((*ppFBO)->m_hSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glSamplerParameteri((*ppFBO)->m_hSampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glSamplerParameterf((*ppFBO)->m_hSampler, GL_TEXTURE_MIN_LOD, -1000.f);
		glSamplerParameterf((*ppFBO)->m_hSampler, GL_TEXTURE_MAX_LOD, 1000.f);
		glSamplerParameteri((*ppFBO)->m_hSampler, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		glSamplerParameteri((*ppFBO)->m_hSampler, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);	
	}

	glTexImage2D( GL_TEXTURE_2D, 0, nFormat, nWidth, nHeight, 0, nFormat, nType, NULL );
    

    // Create the depth buffer
    glGenRenderbuffers( 1, &(*ppFBO)->m_hRenderBuffer );
    glBindRenderbuffer( GL_RENDERBUFFER, (*ppFBO)->m_hRenderBuffer );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, nWidth, nHeight );

    // Create the color buffer
    glGenFramebuffers( 1, &(*ppFBO)->m_hFrameBuffer );
    glBindFramebuffer( GL_FRAMEBUFFER, (*ppFBO)->m_hFrameBuffer );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, (*ppFBO)->m_hTexture, 0 );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (*ppFBO)->m_hRenderBuffer );

    if( GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus( GL_FRAMEBUFFER ) )
        return FALSE;

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );
    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: DestroyFBO()
// Desc: Destroy the FBO
//--------------------------------------------------------------------------------------
VOID CSample::DestroyFBO( FrameBufferObject* pFBO )
{
    glDeleteTextures( 1, &pFBO->m_hTexture );
    glDeleteFramebuffers( 1, &pFBO->m_hFrameBuffer );
    glDeleteRenderbuffers( 1, &pFBO->m_hRenderBuffer );
	glDeleteSamplers(1, &pFBO->m_hSampler);    

    delete pFBO;
}


//--------------------------------------------------------------------------------------
// Name: BeginFBO()
// Desc: Bind objects for rendering to the frame buffer object
//--------------------------------------------------------------------------------------
VOID CSample::BeginFBO( FrameBufferObject* pFBO )
{
    glBindFramebuffer( GL_FRAMEBUFFER, pFBO->m_hFrameBuffer );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pFBO->m_hTexture, 0 );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pFBO->m_hRenderBuffer );
    glViewport( 0, 0, pFBO->m_nWidth, pFBO->m_nHeight );
}


//--------------------------------------------------------------------------------------
// Name: EndFBO()
// Desc: Restore rendering back to the primary frame buffer
//--------------------------------------------------------------------------------------
VOID CSample::EndFBO( FrameBufferObject* pFBO )
{
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glViewport( 0, 0, g_nWindowWidth, g_nWindowHeight );
}


//--------------------------------------------------------------------------------------
// Name: Initialize()
// Desc: 
//--------------------------------------------------------------------------------------
BOOL CSample::Initialize()
{
	//imgui
	ImGui_ImplAndroidGL3_Init();

	/*FILE* file = fopen("/sdcard/textTest.txt", "w+");
	fputs("HELLO WORLD!\n", file);
	fclose(file);*/
	{
		glGenVertexArrays(1, &g_hVao);	
	}

    InitShaders();

    //
	// create array buffer with glMapBufferRange and VAO
	//
	{
		// test the half data type vertex buffer object support
		unsigned short vQuad[] = 
		{
			FloatToHalf(0.55f), FloatToHalf(0.75f),  FloatToHalf(0.0f), FloatToHalf(1.0f),
			FloatToHalf(0.55f), FloatToHalf(0.55f),  FloatToHalf(0.0f), FloatToHalf(0.0f),
			FloatToHalf(0.75f), FloatToHalf(0.55f),  FloatToHalf(1.0f), FloatToHalf(0.0f),
			FloatToHalf(0.75f), FloatToHalf(0.75f),  FloatToHalf(1.0f), FloatToHalf(1.0f),
		};
		
		GLsizeiptr const PositionSize = sizeof(vQuad);
		
		// Generate a buffer object
		glGenBuffers(1, &g_hBuffer);

		// Bind the buffer for use
		glBindBuffer(GL_ARRAY_BUFFER, g_hBuffer);

		// Reserve buffer memory but don't copy the values
		glBufferData(GL_ARRAY_BUFFER, 
					PositionSize, 
					vQuad, 
					GL_STATIC_DRAW);	

		// Create a dummy vertex array object where all the attribute buffers and element buffers would be attached 
		glGenVertexArrays(1, &g_hVertexArrayName);
		glBindVertexArray(g_hVertexArrayName);

		// half data type
		glVertexAttribPointer(ATTR_POSITION, 4, GL_HALF_FLOAT, GL_FALSE, 0, 0);
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(ATTR_POSITION);
		glBindVertexArray(0);	
	}
	
	
    // Create the shader program needed to render the scene
    {
        // Compile the shaders
        GLuint hVertexShader = glCreateShader( GL_VERTEX_SHADER );
        glShaderSource( hVertexShader, 1, &g_strVertexShader, NULL );
        glCompileShader( hVertexShader );

        // Check for compile success
        GLint nCompileResult = 0;
        glGetShaderiv( hVertexShader, GL_COMPILE_STATUS, &nCompileResult );
        if( 0 == nCompileResult )
        {
            CHAR  strLog[1024];
            GLint nLength;
            glGetShaderInfoLog( hVertexShader, 1024, &nLength, strLog );
            return FALSE;
        }

        GLuint hFragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
        glShaderSource( hFragmentShader, 1, &g_strFragmentShader, NULL );
        glCompileShader( hFragmentShader );

        // Check for compile success
        glGetShaderiv( hFragmentShader, GL_COMPILE_STATUS, &nCompileResult );
        if( 0 == nCompileResult )
        {
            CHAR  strLog[1024];
            GLint nLength;
            glGetShaderInfoLog( hFragmentShader, 1024, &nLength, strLog );
            return FALSE;
        }

        // Attach the individual shaders to the common shader program
        g_hShaderProgram = glCreateProgram();
        glAttachShader( g_hShaderProgram, hVertexShader );
        glAttachShader( g_hShaderProgram, hFragmentShader );

        // Link the vertex shader and fragment shader together
        glLinkProgram( g_hShaderProgram );

        // Check for link success
        GLint nLinkResult = 0;
        glGetProgramiv( g_hShaderProgram, GL_LINK_STATUS, &nLinkResult );
        if( 0 == nLinkResult )
        {
            CHAR strLog[1024];
            GLint nLength;
            glGetProgramInfoLog( g_hShaderProgram, 1024, &nLength, strLog );
            return FALSE;
        }
        
        // query support stats
		GLint n = 0;
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &n);
		
		GLint fn = 0;
		glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &fn);
		
		GLint size = 0;
		glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &size);
		
		// how many active uniform blocks are there
		GLint NoOfActiveBlocks = 0;
		glGetProgramiv(g_hShaderProgram, GL_ACTIVE_UNIFORM_BLOCKS, &NoOfActiveBlocks);

		// the name is the name of the uniform block name ... not the name of the GLSL scope for the uniform
		// there seems to be a bug in ATI drivers that does that wrong
        g_hUniformBlock = glGetUniformBlockIndex(g_hShaderProgram, "Transform");
        
        // index is invalid or too many uniform buffers
        if(g_hUniformBlock == GL_INVALID_INDEX 
				|| g_hUniformBlock >= (GLuint)n 
				|| g_hUniformBlock >= (GLuint)fn
				|| g_hUniformBlock >= (GLuint)NoOfActiveBlocks)
			return FALSE;
			

		// get the uniform block's name string
		char BlockName[64];
		GLint length;
		glGetActiveUniformBlockName(g_hShaderProgram, g_hUniformBlock, 64, &length, BlockName);

		//
		// Create uniform block
		//
		// get the uniforms block size
		GLint UniformBlockSize = 0;
		GLint NoOfUniformsInBlock = 0;
		glGetActiveUniformBlockiv(g_hShaderProgram, g_hUniformBlock, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &NoOfUniformsInBlock);
		glGetActiveUniformBlockiv(g_hShaderProgram, g_hUniformBlock, GL_UNIFORM_BLOCK_DATA_SIZE, &UniformBlockSize);
		
		glGenBuffers(1, &g_hTransformBufferName);
		glBindBuffer(GL_UNIFORM_BUFFER, g_hTransformBufferName);
		glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		// Now we attach the buffer to UBO binding point 0...
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, g_hTransformBufferName);
		// associate the uniform block to binding point 0
		glUniformBlockBinding(g_hShaderProgram, g_hUniformBlock, 0);

        glDeleteShader( hVertexShader );
        glDeleteShader( hFragmentShader );
    }

    // Create the shader program needed to render the overlay
    {
        // Compile the shaders
        GLuint hVertexShader = glCreateShader( GL_VERTEX_SHADER );
        glShaderSource( hVertexShader, 1, &g_strOverlayVertexShader, NULL );
        glCompileShader( hVertexShader );

        // Check for compile success
        GLint nCompileResult = 0;
        glGetShaderiv( hVertexShader, GL_COMPILE_STATUS, &nCompileResult );
        if( 0 == nCompileResult )
        {
            CHAR  strLog[1024];
            GLint nLength;
            glGetShaderInfoLog( hVertexShader, 1024, &nLength, strLog );
            return FALSE;
        }

        GLuint hFragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
        glShaderSource( hFragmentShader, 1, &g_strOverlayFragmentShader, NULL );
        glCompileShader( hFragmentShader );

        // Check for compile success
        glGetShaderiv( hFragmentShader, GL_COMPILE_STATUS, &nCompileResult );
        if( 0 == nCompileResult )
        {
            CHAR  strLog[1024];
            GLint nLength;
            glGetShaderInfoLog( hFragmentShader, 1024, &nLength, strLog );
            return FALSE;
        }

        // Attach the individual shaders to the common shader program
        g_hOverlayShaderProgram = glCreateProgram();
        glAttachShader( g_hOverlayShaderProgram, hVertexShader );
        glAttachShader( g_hOverlayShaderProgram, hFragmentShader );

        // Link the vertex shader and fragment shader together
        glLinkProgram( g_hOverlayShaderProgram );

        // Check for link success
        GLint nLinkResult = 0;
        glGetProgramiv( g_hOverlayShaderProgram, GL_LINK_STATUS, &nLinkResult );
        if( 0 == nLinkResult )
        {
            CHAR strLog[1024];
            GLint nLength;
            glGetProgramInfoLog( g_hOverlayShaderProgram, 1024, &nLength, strLog );
            return FALSE;
        }

        glDeleteShader( hVertexShader );
        glDeleteShader( hFragmentShader );
        
    }

    // Create an offscreen FBO to render to
    if( FALSE == CreateFBO( 128, 128, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, &g_pOffscreenFBO ) )
        return FALSE;
        
	glGenQueries(1, &g_QueryName);        
	
#ifdef _DEBUG
	int NumberOfExtensions;

	// The old method to get the extension list is obsolete.
	// You must use glGetIntegerv and glGetStringi
	glGetIntegerv(GL_NUM_EXTENSIONS, &NumberOfExtensions);
	
	OutputDebugString("List of Extensions supported on the GPU: \n");

	// 
	for(int i = 0; i < NumberOfExtensions; i++)
	{
		const GLubyte *one_string = glGetStringi(GL_EXTENSIONS, i);
		OutputDebugString( (char8*)one_string);
		OutputDebugString("\n");
	}
#endif	

    return TRUE;
}



//--------------------------------------------------------------------------------------
// Name: Resize()
// Desc: 
//--------------------------------------------------------------------------------------
BOOL CSample::Resize()
{
    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: Destroy()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CSample::Destroy()
{
	ImGui_ImplAndroidGL3_Shutdown();
    glDeleteProgram( g_hShaderProgram );
	glDeleteBuffers(1, &g_hTransformBufferName);
	glDeleteVertexArrays(1, &g_hVertexArrayName);	
	glDeleteQueries(1, &g_QueryName);
	
	glDeleteBuffers(1, &g_hBuffer);	
	
    DestroyFBO( g_pOffscreenFBO );
}


//--------------------------------------------------------------------------------------
// Name: Update()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CSample::Update()
{
   return;
}

//--------------------------------------------------------------------------------------
// Name: RenderScene()
// Desc: Render the scene
//--------------------------------------------------------------------------------------
VOID CSample::RenderScene( FLOAT fTime )
{
	
    // Rotate and translate the model view matrix
    float matModelView[16] = {0};
    matModelView[ 0] = +cosf( fTime );
    matModelView[ 2] = +sinf( fTime );
    matModelView[ 5] = 1.0f;
    matModelView[ 8] = -sinf( fTime );
    matModelView[10] = +cosf( fTime );
    matModelView[14] = -5.0f;
    matModelView[15] = 1.0f;

    // Build a perspective projection matrix could be 4x3 or 3x4
    float matProj[16] = {0};
    matProj[ 0] = cosf( 0.5f ) / sinf( 0.5f );
    matProj[ 5] = matProj[0] * g_fAspectRatio;
    matProj[10] = -( 10.0f ) / ( 9.0f );
    matProj[11] = -1.0f;
    matProj[14] = -( 10.0f ) / ( 9.0f );

    // Define vertice for a 4-sided pyramid
    FLOAT VertexPositions[] =
    {
        0.0f,+1.0f, 0.0f,      -1.0f,-1.0f, 1.0f,      +1.0f,-1.0f, 1.0f,
        0.0f,+1.0f, 0.0f,      +1.0f,-1.0f, 1.0f,      +1.0f,-1.0f,-1.0f,
        0.0f,+1.0f, 0.0f,      +1.0f,-1.0f,-1.0f,      -1.0f,-1.0f,-1.0f,
        0.0f,+1.0f, 0.0f,      -1.0f,-1.0f,-1.0f,      -1.0f,-1.0f, 1.0f,
    };
/*    
    FLOAT VertexColors[] =
    {
        1.0f, 0.0f, 0.0f,       1.0f, 0.0f, 0.0f,       1.0f, 0.0f, 0.0f, // Red
        0.0f, 1.0f, 0.0f,       0.0f, 1.0f, 0.0f,       0.0f, 1.0f, 0.0f, // Green
        0.0f, 0.0f, 1.0f,       0.0f, 0.0f, 1.0f,       0.0f, 0.0f, 1.0f, // Blue
        1.0f, 1.0f, 0.0f,       1.0f, 1.0f, 0.0f,       1.0f, 1.0f, 0.0f, // Yellow
    };
*/
	// new 2:10:10:10 vertex buffer format
	unsigned int VertexColors[] =
	{
		FloatsToUint2_10_10_10(0.0f, 0.0f, 0.0f, 1.0f), FloatsToUint2_10_10_10(0.0f, 0.0f, 0.0f, 1.0f), FloatsToUint2_10_10_10(0.0f, 0.0f, 0.0f, 1.0f), // Red
		FloatsToUint2_10_10_10(0.0f, 0.0f, 1.0f, 0.0f), FloatsToUint2_10_10_10(0.0f, 0.0f, 1.0f, 0.0f), FloatsToUint2_10_10_10(0.0f, 0.0f, 1.0f, 0.0f), // Green
		FloatsToUint2_10_10_10(0.0f, 1.0f, 0.0f, 0.0f), FloatsToUint2_10_10_10(0.0f, 1.0f, 0.0f, 0.0f), FloatsToUint2_10_10_10(0.0f, 1.0f, 0.0f, 0.0f), // Blue
		FloatsToUint2_10_10_10(0.0f, 0.0f, 1.0f, 1.0f), FloatsToUint2_10_10_10(0.0f, 0.0f, 1.0f, 1.0f), FloatsToUint2_10_10_10(0.0f, 0.0f, 1.0f, 1.0f), // Yellow
	};

    // Clear the colorbuffer and depth-buffer
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Set some state
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );

    // Set the shader program
    glUseProgram( g_hShaderProgram );
    
	//glBindVertexArray(g_hVao);

    // update uniform buffer object
	glBindBuffer(GL_UNIFORM_BUFFER, g_hTransformBufferName);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(matModelView), &matModelView);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(matModelView), sizeof(matProj), &matProj);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    // Bind the vertex attributes
    glVertexAttribPointer( ATTR_POSITION, 3, GL_FLOAT, 0, 0, VertexPositions );
    glEnableVertexAttribArray( ATTR_POSITION );

	//
	// new 2:10:10:10 vertex buffer format
	// 
    glVertexAttribPointer( ATTR_COLOR, 4,  GL_UNSIGNED_INT_2_10_10_10_REV, 0, 0, VertexColors );
    glEnableVertexAttribArray( ATTR_COLOR );
    
	// Draw the cubemap-reflected triangle
	glDrawArrays( GL_TRIANGLES, 0, 12);
    
	
    // Cleanup
    glDisableVertexAttribArray( ATTR_POSITION );
    glDisableVertexAttribArray( ATTR_COLOR );

	//glBindVertexArray(0);
}



void CSample::UpdateGUI(float fTime)
{
	//cursor input
	//LOGI("cursor: %.2f %.2f", m_Input.m_vPointerPosition.x, m_Input.m_vPointerPosition.y);
	bool cursorDown = false;
	if (m_Input.m_nPointerState & m_Input.POINTER_DOWN)
		cursorDown = true;

	ImGui_ImplAndroidGL3_NewFrame(fTime, ImVec2(m_Input.m_vPointerPosition.x, m_Input.m_vPointerPosition.y), cursorDown);
	ImGui::Begin("Test");
	ImGui::SetWindowFontScale(3.0f);
	ImGui::Button("button", ImVec2(300, 75));
	ImGui::Button("button2", ImVec2(300, 75));
	
	ImGui::DragFloat3("LightDir", &t[0], 0.05);
	
	ImGui::Checkbox("Show ShadowMap", &b1);
	ImGui::Checkbox("Linear Depth", &b2);
	ImGui::Checkbox("UseAutoCamera", &b3);
	
	const char* SamplePatterns[] = { "POISSON_25_25", "POISSON_32_64", "POISSON_100_100", "POISSON_64_128", "REGULAR_49_225" };
	ImGui::ListBox("SamplePattern", &i, SamplePatterns, 5);
	ImGui::End();
	ImGui::Render();
}


//--------------------------------------------------------------------------------------
// Name: Render()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CSample::Render()
{

    static float fTime = 0.0f;
    fTime += 0.01f;
    
	// The purpose of sync objects is to synchronize the CPU with the 
	// GPU's actions. To do this, sync objects have the concept of a current 
	// status. The status of a sync object can be signaled or unsignaled; 
	// this state represents some condition of the GPU, depending on the 
	// particular type of sync object and how it was used. This is similar 
	// to how mutual exclusives are used to synchronize behavior between 
	// threads; when a mutex becomes signaled, it allows other threads that 
	// are waiting on it to activate.	
//	GLsync syncFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
//
//    // Render the scene into the offscreen FBO with a green background
//    {
//        BeginFBO( g_pOffscreenFBO );
//        glClearColor( 0.0f, 0.5f, 0.0f, 1.0f );
//        
//		// Beginning of the samples count query
//		// GL_ANY_SAMPLES_PASSED - Subsequent rendering causes the flag to be set to GL_TRUE if 
//		// any sample passes the depth test
//		glBeginQuery(GL_ANY_SAMPLES_PASSED, g_QueryName);
//
//		RenderScene( fTime );
//		
//		// End of the samples count query
//		glEndQuery(GL_ANY_SAMPLES_PASSED);		
//		
//
//		GLuint SamplesCount = 0;
//		// Subsequent rendering causes the flag to be set to GL_TRUE if any sample passes the depth test		
//		glGetQueryObjectuiv(g_QueryName, GL_QUERY_RESULT, &SamplesCount);
//		//fprintf(stdout, "Samples count: %d\r", SamplesCount);		
//		
//        EndFBO( g_pOffscreenFBO );
//    }
//
//	// wait for glDrawArrays to finish
//	// To block all CPU operations until a sync object is signaled, 
//	// you call this function:	
//	GLenum syncResult = glClientWaitSync(syncFence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
//
//	if (syncResult == GL_CONDITION_SATISFIED || syncResult == GL_ALREADY_SIGNALED)
//	{    
//		// rendering has finished
//		// Render the scene into the primary backbuffer with a dark blue background
//		glClearColor( 0.0f, 0.0f, 0.5f, 1.0f );
//		RenderScene( fTime );
//
//	}
//	else if(syncResult == GL_TIMEOUT_EXPIRED || syncResult == GL_WAIT_FAILED)
//	{
//		// Error occured
//		// Render the scene into the primary backbuffer with a dark blue background
//		glClearColor( 1.0f, 0.0f, 0.0f, 1.0f );
//	}
//	
//	// Cleanup fence
//	glDeleteSync(syncFence);	
//
//    // Display the offscreen FBO texture as an overlay
//    {
///*    
//        float vQuad[] = 
//        {
//            0.35f, 0.95f,  0.0f, 1.0f,
//            0.35f, 0.35f,  0.0f, 0.0f,
//            0.95f, 0.35f,  1.0f, 0.0f,
//            0.95f, 0.95f,  1.0f, 1.0f,
//        };
//        glVertexAttribPointer( 0, 4, GL_FLOAT, 0, 0, vQuad );
//        glEnableVertexAttribArray( 0 );
//*/
//		glBindVertexArray(g_hVertexArrayName);
//
//        // Set the texture
//        glActiveTexture( GL_TEXTURE0 );
//        glBindTexture( GL_TEXTURE_2D, g_pOffscreenFBO->m_hTexture );
//        
//        glBindSampler(0, g_pOffscreenFBO->m_hSampler);
//        
//        glDisable( GL_DEPTH_TEST );
//
//        // Set the shader program
//        glUseProgram( g_hOverlayShaderProgram );
//
//		//
//        // Instanced rendering
//        //
//		// Draw the quad four times instanced
//		glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 4);        
//
//		// unbind it
//        glBindVertexArray(0);
//    }   

	glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
	RenderScene(fTime);
	UpdateGUI(fTime);
	
}

