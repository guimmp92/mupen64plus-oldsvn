/*
Copyright (C) 2003 Rice1964

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"
#define GLH_EXT_SINGLE_FILE
#include "glh_genext.h"

#include "liblinux/BMGLibPNG.h"

#include "_BldNum.h"

COGLGraphicsContext::COGLGraphicsContext() :
    m_pScreen(0),
	m_bSupportMultiTexture(false),
	m_bSupportTextureEnvCombine(false),
    m_bSupportSeparateSpecularColor(false),
    m_bSupportSecondColor(false),
    m_bSupportFogCoord(false),
	m_bSupportTextureObject(false),
	m_bSupportRescaleNormal(false),
	m_bSupportLODBias(false),
	m_bSupportTextureMirrorRepeat(false),
	m_bSupportTextureLOD(false),
	m_bSupportNVRegisterCombiner(false),
	m_bSupportBlendColor(false),
	m_bSupportBlendSubtract(false),
	m_bSupportNVTextureEnvCombine4(false),
    m_pVendorStr(NULL),
    m_pRenderStr(NULL),
    m_pExtensionStr(NULL),
    m_pVersionStr(NULL)
{
}


COGLGraphicsContext::~COGLGraphicsContext()
{
}

bool COGLGraphicsContext::Initialize(HWND hWnd, HWND hWndStatus, uint32 dwWidth, uint32 dwHeight, BOOL bWindowed )
{
    printf("Initializing OpenGL Device Context\n");
	Lock();

	CGraphicsContext::Get()->m_supportTextureMirror = false;
	CGraphicsContext::Initialize(hWnd, hWndStatus, dwWidth, dwHeight, bWindowed );

	if( bWindowed )
	{
		windowSetting.statusBarHeightToUse = windowSetting.statusBarHeight;
		windowSetting.toolbarHeightToUse = windowSetting.toolbarHeight;
	}
	else
	{
		windowSetting.statusBarHeightToUse = 0;
		windowSetting.toolbarHeightToUse = 0;
	}

	int	 depthBufferDepth = options.OpenglDepthBufferSetting;
	int	 colorBufferDepth = 32;
	if( options.colorQuality == TEXTURE_FMT_A4R4G4B4 ) colorBufferDepth = 16;

   // init sdl & gl
   const SDL_VideoInfo *videoInfo;
   Uint32 videoFlags = 0;
   
   /* Initialize SDL */
   printf("(II) Initializing SDL video subsystem...\n");
   if(SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
     {
	printf("(EE) Error initializing SDL video subsystem: %s\n", SDL_GetError());
	return false;
     }
   
   /* Video Info */
   printf("(II) Getting video info...\n");
   if(!(videoInfo = SDL_GetVideoInfo()))
     {
	printf("(EE) Video query failed: %s\n", SDL_GetError());
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	return false;
     }
   /* Setting the video mode */
   videoFlags |= SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_HWPALETTE;
   
   if(videoInfo->hw_available)
     videoFlags |= SDL_HWSURFACE;
   else
     videoFlags |= SDL_SWSURFACE;
   
   if(videoInfo->blit_hw)
     videoFlags |= SDL_HWACCEL;
   
   if(!bWindowed)
     videoFlags |= SDL_FULLSCREEN;
   
   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
   SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, colorBufferDepth);
   SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depthBufferDepth);
   
   printf("(II) Setting video mode %dx%d...\n", (int)windowSetting.uDisplayWidth, (int)windowSetting.uDisplayHeight);
   if(!(m_pScreen = SDL_SetVideoMode(windowSetting.uDisplayWidth, windowSetting.uDisplayHeight, colorBufferDepth, videoFlags)))
     {
	printf("(EE) Error setting video mode %dx%d: %s\n", (int)windowSetting.uDisplayWidth, (int)windowSetting.uDisplayHeight, SDL_GetError());
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	return false;
     }
   
   char caption[500];
   sprintf(caption, "RiceVideoLinux N64 Plugin %s", FILE_VERSION);
   SDL_WM_SetCaption(caption, caption);
   SetWindowMode();

	InitState();
	InitOGLExtension();
	sprintf(m_strDeviceStats, "%s - %s : %s", m_pVendorStr, m_pRenderStr, m_pVersionStr);
	TRACE0(m_strDeviceStats);
    printf("%s\n", m_strDeviceStats);

	Unlock();

	Clear(CLEAR_COLOR_AND_DEPTH_BUFFER);	// Clear buffers
	UpdateFrame();
	Clear(CLEAR_COLOR_AND_DEPTH_BUFFER);
	UpdateFrame();
	
	m_bReady = true;
	status.isVertexShaderEnabled = false;

	return true;
}

void COGLGraphicsContext::InitState(void)
{
	m_pRenderStr = glGetString(GL_RENDERER);;
	m_pExtensionStr = glGetString(GL_EXTENSIONS);;
	m_pVersionStr = glGetString(GL_VERSION);;
	m_pVendorStr = glGetString(GL_VENDOR);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);

	glShadeModel(GL_SMOOTH);

    //position viewer 
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

	glDisable(GL_ALPHA_TEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);

	glFrontFace(GL_CCW);
	glDisable(GL_CULL_FACE);
	glDisable(GL_NORMALIZE);

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glDepthRange(-1, 1);
}

PFNGLGENFRAGMENTSHADERSATIPROC        glGenFragmentShadersATI;
PFNGLBINDFRAGMENTSHADERATIPROC        glBindFragmentShaderATI;
PFNGLDELETEFRAGMENTSHADERATIPROC      glDeleteFragmentShaderATI;
PFNGLBEGINFRAGMENTSHADERATIPROC       glBeginFragmentShaderATI;
PFNGLENDFRAGMENTSHADERATIPROC         glEndFragmentShaderATI;
PFNGLPASSTEXCOORDATIPROC              glPassTexCoordATI;
PFNGLSAMPLEMAPATIPROC                 glSampleMapATI;
PFNGLCOLORFRAGMENTOP1ATIPROC          glColorFragmentOp1ATI;
PFNGLCOLORFRAGMENTOP2ATIPROC          glColorFragmentOp2ATI;
PFNGLCOLORFRAGMENTOP3ATIPROC          glColorFragmentOp3ATI;
PFNGLALPHAFRAGMENTOP1ATIPROC          glAlphaFragmentOp1ATI;
PFNGLALPHAFRAGMENTOP2ATIPROC          glAlphaFragmentOp2ATI;
PFNGLALPHAFRAGMENTOP3ATIPROC          glAlphaFragmentOp3ATI;
PFNGLSETFRAGMENTSHADERCONSTANTATIPROC glSetFragmentShaderConstantATI;

PFNGLNEWOBJECTBUFFERATIPROC         glNewObjectBufferATI;
PFNGLISOBJECTBUFFERATIPROC          glIsObjectBufferATI;
PFNGLUPDATEOBJECTBUFFERATIPROC      glUpdateObjectBufferATI;
PFNGLGETOBJECTBUFFERFVATIPROC       glGetObjectBufferfvATI;
PFNGLGETOBJECTBUFFERIVATIPROC       glGetObjectBufferivATI;
PFNGLFREEOBJECTBUFFERATIPROC        glFreeObjectBufferATI;
PFNGLARRAYOBJECTATIPROC             glArrayObjectATI;
PFNGLGETARRAYOBJECTFVATIPROC        glGetArrayObjectfvATI;
PFNGLGETARRAYOBJECTIVATIPROC        glGetArrayObjectivATI;
PFNGLVARIANTARRAYOBJECTATIPROC      glVariantArrayObjectATI;
PFNGLGETVARIANTARRAYOBJECTFVATIPROC glGetVariantArrayObjectfvATI;
PFNGLGETVARIANTARRAYOBJECTIVATIPROC glGetVariantArrayObjectivATI;

PFNGLPROGRAMSTRINGARBPROC glProgramStringARB = NULL;
PFNGLBINDPROGRAMARBPROC glBindProgramARB = NULL;
PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB = NULL;
PFNGLGENPROGRAMSARBPROC glGenProgramsARB = NULL;
PFNGLPROGRAMENVPARAMETER4DARBPROC glProgramEnvParameter4dARB = NULL;
PFNGLPROGRAMENVPARAMETER4DVARBPROC glProgramEnvParameter4dvARB = NULL;
PFNGLPROGRAMENVPARAMETER4FARBPROC glProgramEnvParameter4fARB = NULL;
PFNGLPROGRAMENVPARAMETER4FVARBPROC glProgramEnvParameter4fvARB = NULL;
PFNGLPROGRAMLOCALPARAMETER4DARBPROC glProgramLocalParameter4dARB = NULL;
PFNGLPROGRAMLOCALPARAMETER4DVARBPROC glProgramLocalParameter4dvARB = NULL;
PFNGLPROGRAMLOCALPARAMETER4FARBPROC glProgramLocalParameter4fARB = NULL;
PFNGLPROGRAMLOCALPARAMETER4FVARBPROC glProgramLocalParameter4fvARB = NULL;
PFNGLGETPROGRAMENVPARAMETERDVARBPROC glGetProgramEnvParameterdvARB = NULL;
PFNGLGETPROGRAMENVPARAMETERFVARBPROC glGetProgramEnvParameterfvARB = NULL;
PFNGLGETPROGRAMENVPARAMETERFVARBPROC glGetssProgramEnvParameterfvARB = NULL;
PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC glGetProgramLocalParameterdvARB = NULL;;
PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC glGetProgramLocalParameterfvARB = NULL;;
PFNGLGETPROGRAMIVARBPROC glGetProgramivARB = NULL;;
PFNGLGETPROGRAMSTRINGARBPROC glGetProgramStringARB = NULL;;
PFNGLISPROGRAMARBPROC glIsProgramARB = NULL;

void COGLGraphicsContext::InitOGLExtension_ATI(void)
{
	#define SAFE_GET_PROC( func, type, fail)   func = (type) SDL_GL_GetProcAddress( #func);   fail &= (func != NULL);

	if (IsExtensionSupported("GL_ATI_fragment_shader"))
	{
		glGenFragmentShadersATI        = (PFNGLGENFRAGMENTSHADERSATIPROC) SDL_GL_GetProcAddress("glGenFragmentShadersATI");
		glBindFragmentShaderATI        = (PFNGLBINDFRAGMENTSHADERATIPROC) SDL_GL_GetProcAddress("glBindFragmentShaderATI");
		glDeleteFragmentShaderATI      = (PFNGLDELETEFRAGMENTSHADERATIPROC) SDL_GL_GetProcAddress("glDeleteFragmentShaderATI");
		glBeginFragmentShaderATI       = (PFNGLBEGINFRAGMENTSHADERATIPROC) SDL_GL_GetProcAddress("glBeginFragmentShaderATI");
		glEndFragmentShaderATI         = (PFNGLENDFRAGMENTSHADERATIPROC) SDL_GL_GetProcAddress("glEndFragmentShaderATI");
		glPassTexCoordATI              = (PFNGLPASSTEXCOORDATIPROC) SDL_GL_GetProcAddress("glPassTexCoordATI");
		glSampleMapATI                 = (PFNGLSAMPLEMAPATIPROC) SDL_GL_GetProcAddress("glSampleMapATI");
		glColorFragmentOp1ATI          = (PFNGLCOLORFRAGMENTOP1ATIPROC) SDL_GL_GetProcAddress("glColorFragmentOp1ATI");
		glColorFragmentOp2ATI          = (PFNGLCOLORFRAGMENTOP2ATIPROC) SDL_GL_GetProcAddress("glColorFragmentOp2ATI");
		glColorFragmentOp3ATI          = (PFNGLCOLORFRAGMENTOP3ATIPROC) SDL_GL_GetProcAddress("glColorFragmentOp3ATI");
		glAlphaFragmentOp1ATI          = (PFNGLALPHAFRAGMENTOP1ATIPROC) SDL_GL_GetProcAddress("glAlphaFragmentOp1ATI");
		glAlphaFragmentOp2ATI          = (PFNGLALPHAFRAGMENTOP2ATIPROC) SDL_GL_GetProcAddress("glAlphaFragmentOp2ATI");
		glAlphaFragmentOp3ATI          = (PFNGLALPHAFRAGMENTOP3ATIPROC) SDL_GL_GetProcAddress("glAlphaFragmentOp3ATI");
		glSetFragmentShaderConstantATI = (PFNGLSETFRAGMENTSHADERCONSTANTATIPROC) SDL_GL_GetProcAddress("glSetFragmentShaderConstantATI");

		if (glGenFragmentShadersATI == NULL)
		{
			ErrorMsg("Can't init glGenFragmentShadersATI");
		}

		if (glBindFragmentShaderATI == NULL)
		{
			ErrorMsg ("Can't init glBindFragmentShaderATI");
		}

		if (glDeleteFragmentShaderATI == NULL)
		{
			ErrorMsg ("Can't init glDeleteFragmentShaderATI");
		}

		if (glBeginFragmentShaderATI == NULL)
		{
			ErrorMsg ("Can't init glBeginFragmentShaderATI");
		}

		if (glEndFragmentShaderATI == NULL)
		{
			ErrorMsg ("Can't init glEndFragmentShaderATI");
		}

		if (glPassTexCoordATI == NULL)
		{
			ErrorMsg ("Can't init glPassTexCoordATI");
		}

		if (glColorFragmentOp1ATI == NULL)
		{
			ErrorMsg ("Can't init glColorFragmentOp1ATI");
		}

		if (glColorFragmentOp2ATI == NULL)
		{
			ErrorMsg ("Can't init glColorFragmentOp2ATI");
		}

		if (glColorFragmentOp3ATI == NULL)
		{
			ErrorMsg ("Can't init glColorFragmentOp3ATI");
		}

		if (glAlphaFragmentOp1ATI == NULL)
		{
			ErrorMsg ("Can't init glAlphaFragmentOp1ATI");
		}

		if (glAlphaFragmentOp2ATI == NULL)
		{
			ErrorMsg ("Can't init glAlphaFragmentOp2ATI");
		}

		if (glAlphaFragmentOp2ATI == NULL)
		{
			ErrorMsg ("Can't init glAlphaFragmentOp2ATI");
		}

		if (glAlphaFragmentOp3ATI == NULL)
		{
			ErrorMsg ("Can't init glAlphaFragmentOp3ATI");
		}

		if (glSetFragmentShaderConstantATI == NULL)
		{
			ErrorMsg ("Can't init glSetFragmentShaderConstantATI");
		}
	}
	
	if (IsExtensionSupported("GL_ARB_fragment_program"))
	{
		int ret = TRUE;

		SAFE_GET_PROC( glProgramStringARB, PFNGLPROGRAMSTRINGARBPROC, ret);
		SAFE_GET_PROC( glBindProgramARB, PFNGLBINDPROGRAMARBPROC, ret);
		SAFE_GET_PROC( glDeleteProgramsARB, PFNGLDELETEPROGRAMSARBPROC, ret);
		SAFE_GET_PROC( glGenProgramsARB, PFNGLGENPROGRAMSARBPROC, ret);
		SAFE_GET_PROC( glProgramEnvParameter4dARB, PFNGLPROGRAMENVPARAMETER4DARBPROC, ret);
		SAFE_GET_PROC( glProgramEnvParameter4dvARB, PFNGLPROGRAMENVPARAMETER4DVARBPROC, ret);
		SAFE_GET_PROC( glProgramEnvParameter4fARB, PFNGLPROGRAMENVPARAMETER4FARBPROC, ret);
		SAFE_GET_PROC( glProgramEnvParameter4fvARB, PFNGLPROGRAMENVPARAMETER4FVARBPROC, ret);
		SAFE_GET_PROC( glProgramLocalParameter4dARB, PFNGLPROGRAMLOCALPARAMETER4DARBPROC, ret);
		SAFE_GET_PROC( glProgramLocalParameter4dvARB, PFNGLPROGRAMLOCALPARAMETER4DVARBPROC, ret);
		SAFE_GET_PROC( glProgramLocalParameter4fARB, PFNGLPROGRAMLOCALPARAMETER4FARBPROC, ret);
		SAFE_GET_PROC( glProgramLocalParameter4fvARB, PFNGLPROGRAMLOCALPARAMETER4FVARBPROC, ret);
		SAFE_GET_PROC( glGetProgramEnvParameterdvARB, PFNGLGETPROGRAMENVPARAMETERDVARBPROC, ret);
		SAFE_GET_PROC( glGetProgramEnvParameterfvARB, PFNGLGETPROGRAMENVPARAMETERFVARBPROC, ret);
		SAFE_GET_PROC( glGetProgramLocalParameterdvARB, PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC, ret);
		SAFE_GET_PROC( glGetProgramLocalParameterfvARB, PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC, ret);
		SAFE_GET_PROC( glGetProgramivARB, PFNGLGETPROGRAMIVARBPROC, ret);
		SAFE_GET_PROC( glGetProgramStringARB, PFNGLGETPROGRAMSTRINGARBPROC, ret);
		SAFE_GET_PROC( glIsProgramARB, PFNGLISPROGRAMARBPROC, ret);
	}
	else
	{
		//ErrorMsg("No fragment shader support!");
	}
}


void COGLGraphicsContext::InitOGLExtension(void)
{
	// important extension features, it is very bad not to have these feature
	m_bSupportMultiTexture = IsExtensionSupported("GL_ARB_multitexture");
	m_bSupportTextureEnvCombine = IsExtensionSupported("GL_EXT_texture_env_combine");
	
	m_bSupportSeparateSpecularColor = IsExtensionSupported("GL_EXT_separate_specular_color");
	m_bSupportSecondColor = IsExtensionSupported("GL_EXT_secondary_color");
	m_bSupportFogCoord = IsExtensionSupported("GL_EXT_fog_coord");
	m_bSupportTextureObject = IsExtensionSupported("GL_EXT_texture_object");

	// Optional extension features
	m_bSupportRescaleNormal = IsExtensionSupported("GL_EXT_rescale_normal");
	m_bSupportLODBias = IsExtensionSupported("GL_EXT_texture_lod_bias");

	// Nvidia only extension features (optional)
	m_bSupportNVRegisterCombiner = IsExtensionSupported("GL_NV_register_combiners");
	m_bSupportTextureMirrorRepeat = IsExtensionSupported("GL_IBM_texture_mirrored_repeat") || IsExtensionSupported("ARB_texture_mirrored_repeat");
	m_supportTextureMirror = m_bSupportTextureMirrorRepeat;
	m_bSupportTextureLOD = IsExtensionSupported("GL_EXT_texture_lod");
	m_bSupportBlendColor = IsExtensionSupported("GL_EXT_blend_color");
	m_bSupportBlendSubtract = IsExtensionSupported("GL_EXT_blend_subtract");
	m_bSupportNVTextureEnvCombine4 = IsExtensionSupported("GL_NV_texture_env_combine4");

	glh_init_extension("GL_ARB_multitexture");
	glh_init_extension("GL_EXT_texture_env_combine");
	glh_init_extension("GL_EXT_separate_specular_color");
	glh_init_extension("GL_EXT_secondary_color");
	glh_init_extension("GL_EXT_fog_coord");
	glh_init_extension("GL_EXT_texture_object");
	glh_init_extension("GL_EXT_rescale_normal");
	glh_init_extension("GL_EXT_texture_lod_bias");
	glh_init_extension("GL_NV_register_combiners");
	glh_init_extension("GL_IBM_texture_mirrored_repeat");
	glh_init_extension("GL_EXT_texture_lod");
	glh_init_extension("GL_EXT_blend_color");
	glh_init_extension("GL_EXT_blend_subtract");
	glh_init_extension("GL_NV_texture_env_combine4");
	glh_init_extension("GL_EXT_copy_texture");

	InitOGLExtension_ATI();
}

bool COGLGraphicsContext::IsExtensionSupported(const char* pExtName)
{
	if( strstr((const char*)m_pExtensionStr, pExtName) != NULL )
		return true;
	else
		return false;
}

bool COGLGraphicsContext::IsWglExtensionSupported(const char* pExtName)
{
	if( m_pWglExtensionStr == NULL )
		return false;

	if( strstr((const char*)m_pWglExtensionStr, pExtName) != NULL )
		return true;
	else
		return false;
}


void COGLGraphicsContext::CleanUp()
{
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    m_pScreen = NULL;
	m_bReady = false;
}


void COGLGraphicsContext::Clear(ClearFlag dwFlags, uint32 color, float depth)
{
	uint32 flag=0;
	if( dwFlags&CLEAR_COLOR_BUFFER )	flag |= GL_COLOR_BUFFER_BIT;
	if( dwFlags&CLEAR_DEPTH_BUFFER )	flag |= GL_DEPTH_BUFFER_BIT;

	float r = ((color>>16)&0xFF)/255.0f;
	float g = ((color>> 8)&0xFF)/255.0f;
	float b = ((color    )&0xFF)/255.0f;
	float a = ((color>>24)&0xFF)/255.0f;
	glClearColor(r, g, b, a);
	glClearDepth(depth);
	glClear(flag);	//Clear color buffer and depth buffer
}

void COGLGraphicsContext::UpdateFrame(bool swaponly)
{
    status.gFrameCount++;

    glFlush();
    //glFinish();
    //wglSwapIntervalEXT(0);

    /*
    if (debuggerPauseCount == countToPause)
    {
        static int iShotNum = 0;
        // get width, height, allocate buffer to store image
        int width = windowSetting.uDisplayWidth;
        int height = windowSetting.uDisplayHeight;
        printf("Saving debug images: width=%i  height=%i\n", width, height);
        short *buffer = (short *) malloc(((width+3)&~3)*(height+1)*4);
        glReadBuffer( GL_FRONT );
        // set up a BMGImage struct
        struct BMGImageStruct img;
        memset(&img, 0, sizeof(BMGImageStruct));
        InitBMGImage(&img);
        img.bits = (unsigned char *) buffer;
        img.bits_per_pixel = 32;
        img.height = height;
        img.width = width;
        img.scan_width = width * 4;
        // store the RGB color image
        char chFilename[64];
        sprintf(chFilename, "dbg_rgb_%03i.png", iShotNum);
        glReadPixels(0,0,width,height, GL_BGRA, GL_UNSIGNED_BYTE, buffer);
        WritePNG(chFilename, img);
        // store the Z buffer
        sprintf(chFilename, "dbg_Z_%03i.png", iShotNum);
        glReadPixels(0,0,width,height, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, buffer);
        //img.bits_per_pixel = 16;
        //img.scan_width = width * 2;
        WritePNG(chFilename, img);
        // dump a subset of the Z data
        for (int y = 0; y < 480; y += 16)
        {
            for (int x = 0; x < 640; x+= 16)
                printf("%4hx ", buffer[y*640 + x]);
            printf("\n");
        }
        printf("\n");
        // free memory and get out of here
        free(buffer);
        iShotNum++;
        }
    */

   SDL_GL_SwapBuffers();
   
   /*if(options.bShowFPS)
     {
	static unsigned int lastTick=0;
	static int frames=0;
	unsigned int nowTick = SDL_GetTicks();
	frames++;
	if(lastTick + 5000 <= nowTick)
	  {
	     char caption[200];
	     sprintf(caption, "RiceVideoLinux N64 Plugin %s - %.3f VI/S", FILE_VERSION, frames/5.0);
	     SDL_WM_SetCaption(caption, caption);
	     frames = 0;
	     lastTick = nowTick;
	  }
     }*/

	glDepthMask(GL_TRUE);
	glClearDepth(1.0);
	if( !g_curRomInfo.bForceScreenClear ) 
		glClear(GL_DEPTH_BUFFER_BIT);
	else
		needCleanScene = true;

	status.bScreenIsDrawn = false;
}

bool COGLGraphicsContext::SetFullscreenMode()
{
	windowSetting.uDisplayWidth = windowSetting.uFullScreenDisplayWidth;
	windowSetting.uDisplayHeight = windowSetting.uFullScreenDisplayHeight;
	windowSetting.statusBarHeightToUse = 0;
	windowSetting.toolbarHeightToUse = 0;
	return true;
}

bool COGLGraphicsContext::SetWindowMode()
{
	windowSetting.uDisplayWidth = windowSetting.uWindowDisplayWidth;
	windowSetting.uDisplayHeight = windowSetting.uWindowDisplayHeight;
	windowSetting.statusBarHeightToUse = windowSetting.statusBarHeight;
	windowSetting.toolbarHeightToUse = windowSetting.toolbarHeight;
	return true;
}
int COGLGraphicsContext::ToggleFullscreen()
{
   if(SDL_WM_ToggleFullScreen(m_pScreen) == 1)
     {
	m_bWindowed = 1 - m_bWindowed;
	if(m_bWindowed)
	  SetWindowMode();
	else
	  SetFullscreenMode();
     }

	return m_bWindowed?0:1;
}

// This is a static function, will be called when the plugin DLL is initialized
void COGLGraphicsContext::InitDeviceParameters()
{
	status.isVertexShaderEnabled = false;	// Disable it for now
}

