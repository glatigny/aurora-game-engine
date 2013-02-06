#include "wgl_init.h"

#include "open_gl.h"

// Function pointers

// FBO Framebuffers
PFNGLGENFRAMEBUFFERSEXTPROC                     pglGenFramebuffersEXT = NULL;                      // FBO name generation procedure
PFNGLDELETEFRAMEBUFFERSEXTPROC                  pglDeleteFramebuffersEXT = NULL;                   // FBO deletion procedure
PFNGLBINDFRAMEBUFFEREXTPROC                     pglBindFramebufferEXT = NULL;                      // FBO bind procedure
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC              pglCheckFramebufferStatusEXT = NULL;               // FBO completeness test procedure
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC pglGetFramebufferAttachmentParameterivEXT = NULL;  // return various FBO parameters
PFNGLGENERATEMIPMAPEXTPROC                      pglGenerateMipmapEXT = NULL;                       // FBO automatic mipmap generation procedure
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC                pglFramebufferTexture2DEXT = NULL;                 // FBO texdture attachement procedure
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC             pglFramebufferRenderbufferEXT = NULL;              // FBO renderbuffer attachement procedure

// FBO Renderbuffers
PFNGLGENRENDERBUFFERSEXTPROC                    pglGenRenderbuffersEXT = NULL;                     // renderbuffer generation procedure
PFNGLDELETERENDERBUFFERSEXTPROC                 pglDeleteRenderbuffersEXT = NULL;                  // renderbuffer deletion procedure
PFNGLBINDRENDERBUFFEREXTPROC                    pglBindRenderbufferEXT = NULL;                     // renderbuffer bind procedure
PFNGLRENDERBUFFERSTORAGEEXTPROC                 pglRenderbufferStorageEXT = NULL;                  // renderbuffer memory allocation procedure
PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC          pglGetRenderbufferParameterivEXT = NULL;           // return various renderbuffer parameters
PFNGLISRENDERBUFFEREXTPROC                      pglIsRenderbufferEXT = NULL;                       // determine renderbuffer object type

// Shaders
PFNGLBLENDEQUATIONSEPARATEPROC     pglBlendEquationSeparate = NULL;
PFNGLDRAWBUFFERSPROC               pglDrawBuffers = NULL;   
PFNGLSTENCILOPSEPARATEPROC		   pglStencilOpSeparate = NULL;
PFNGLSTENCILFUNCSEPARATEPROC	   pglStencilFuncSeparate = NULL;
PFNGLSTENCILMASKSEPARATEPROC	   pglStencilMaskSeparate = NULL;
PFNGLATTACHSHADERPROC			   pglAttachShader = NULL;
PFNGLBINDATTRIBLOCATIONPROC		   pglBindAttribLocation = NULL;
PFNGLCOMPILESHADERPROC			   pglCompileShader = NULL;
PFNGLCREATEPROGRAMPROC			   pglCreateProgram = NULL;
PFNGLCREATESHADERPROC			   pglCreateShader = NULL;
PFNGLDELETEPROGRAMPROC			   pglDeleteProgram = NULL;
PFNGLDELETESHADERPROC			   pglDeleteShader = NULL;
PFNGLDETACHSHADERPROC			   pglDetachShader = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC   pglDisableVertexAttribArray = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC   pglEnableVertexAttribArray = NULL;
PFNGLGETACTIVEATTRIBPROC		   pglGetActiveAttrib = NULL;
PFNGLGETACTIVEUNIFORMPROC		   pglGetActiveUniform = NULL;
PFNGLGETATTACHEDSHADERSPROC		   pglGetAttachedShaders = NULL;
PFNGLGETATTRIBLOCATIONPROC		   pglGetAttribLocation = NULL;
PFNGLGETPROGRAMIVPROC			   pglGetProgramiv = NULL;
PFNGLGETPROGRAMINFOLOGPROC		   pglGetProgramInfoLog = NULL;
PFNGLGETSHADERIVPROC			   pglGetShaderiv = NULL;
PFNGLGETSHADERINFOLOGPROC		   pglGetShaderInfoLog = NULL;
PFNGLGETSHADERSOURCEPROC		   pglGetShaderSource = NULL;
PFNGLGETUNIFORMLOCATIONPROC		   pglGetUniformLocation = NULL;
PFNGLGETUNIFORMFVPROC			   pglGetUniformfv = NULL;
PFNGLGETUNIFORMIVPROC			   pglGetUniformiv = NULL;
PFNGLGETVERTEXATTRIBDVPROC		   pglGetVertexAttribdv = NULL;
PFNGLGETVERTEXATTRIBFVPROC		   pglGetVertexAttribfv = NULL;
PFNGLGETVERTEXATTRIBIVPROC		   pglGetVertexAttribiv = NULL;
PFNGLGETVERTEXATTRIBPOINTERVPROC   pglGetVertexAttribPointerv = NULL;
PFNGLISPROGRAMPROC				   pglIsProgram = NULL;
PFNGLISSHADERPROC				   pglIsShader = NULL;
PFNGLLINKPROGRAMPROC			   pglLinkProgram = NULL;
PFNGLSHADERSOURCEPROC			   pglShaderSource = NULL;
PFNGLUSEPROGRAMPROC				   pglUseProgram = NULL;
PFNGLUNIFORM1FPROC				   pglUniform1f = NULL;
PFNGLUNIFORM2FPROC				   pglUniform2f = NULL;
PFNGLUNIFORM3FPROC				   pglUniform3f = NULL;
PFNGLUNIFORM4FPROC				   pglUniform4f = NULL;
PFNGLUNIFORM1IPROC				   pglUniform1i = NULL;
PFNGLUNIFORM2IPROC				   pglUniform2i = NULL;
PFNGLUNIFORM3IPROC				   pglUniform3i = NULL;
PFNGLUNIFORM4IPROC				   pglUniform4i = NULL;
PFNGLUNIFORM1FVPROC				   pglUniform1fv = NULL;
PFNGLUNIFORM2FVPROC				   pglUniform2fv = NULL;
PFNGLUNIFORM3FVPROC				   pglUniform3fv = NULL;
PFNGLUNIFORM4FVPROC				   pglUniform4fv = NULL;
PFNGLUNIFORM1IVPROC				   pglUniform1iv = NULL;
PFNGLUNIFORM2IVPROC				   pglUniform2iv = NULL;
PFNGLUNIFORM3IVPROC				   pglUniform3iv = NULL;
PFNGLUNIFORM4IVPROC				   pglUniform4iv = NULL;
PFNGLUNIFORMMATRIX2FVPROC		   pglUniformMatrix2fv = NULL;
PFNGLUNIFORMMATRIX3FVPROC		   pglUniformMatrix3fv = NULL;
PFNGLUNIFORMMATRIX4FVPROC		   pglUniformMatrix4fv = NULL;
PFNGLVALIDATEPROGRAMPROC		   pglValidateProgram = NULL;
PFNGLVERTEXATTRIB1DPROC			   pglVertexAttrib1d = NULL;
PFNGLVERTEXATTRIB1DVPROC		   pglVertexAttrib1dv = NULL;
PFNGLVERTEXATTRIB1FPROC			   pglVertexAttrib1f = NULL;
PFNGLVERTEXATTRIB1FVPROC		   pglVertexAttrib1fv = NULL;
PFNGLVERTEXATTRIB1SPROC			   pglVertexAttrib1s = NULL;
PFNGLVERTEXATTRIB1SVPROC		   pglVertexAttrib1sv = NULL;
PFNGLVERTEXATTRIB2DPROC			   pglVertexAttrib2d = NULL;
PFNGLVERTEXATTRIB2DVPROC		   pglVertexAttrib2dv = NULL;
PFNGLVERTEXATTRIB2FPROC			   pglVertexAttrib2f = NULL;
PFNGLVERTEXATTRIB2FVPROC		   pglVertexAttrib2fv = NULL;
PFNGLVERTEXATTRIB2SPROC			   pglVertexAttrib2s = NULL;
PFNGLVERTEXATTRIB2SVPROC		   pglVertexAttrib2sv = NULL;
PFNGLVERTEXATTRIB3DPROC			   pglVertexAttrib3d = NULL;
PFNGLVERTEXATTRIB3DVPROC		   pglVertexAttrib3dv = NULL;
PFNGLVERTEXATTRIB3FPROC			   pglVertexAttrib3f = NULL;
PFNGLVERTEXATTRIB3FVPROC		   pglVertexAttrib3fv = NULL;
PFNGLVERTEXATTRIB3SPROC			   pglVertexAttrib3s = NULL;
PFNGLVERTEXATTRIB3SVPROC		   pglVertexAttrib3sv = NULL;
PFNGLVERTEXATTRIB4NBVPROC		   pglVertexAttrib4Nbv = NULL;
PFNGLVERTEXATTRIB4NIVPROC		   pglVertexAttrib4Niv = NULL;
PFNGLVERTEXATTRIB4NSVPROC		   pglVertexAttrib4Nsv = NULL;
PFNGLVERTEXATTRIB4NUBPROC		   pglVertexAttrib4Nub = NULL;
PFNGLVERTEXATTRIB4NUBVPROC		   pglVertexAttrib4Nubv = NULL;
PFNGLVERTEXATTRIB4NUIVPROC		   pglVertexAttrib4Nuiv = NULL;
PFNGLVERTEXATTRIB4NUSVPROC		   pglVertexAttrib4Nusv = NULL;
PFNGLVERTEXATTRIB4BVPROC		   pglVertexAttrib4bv = NULL;
PFNGLVERTEXATTRIB4DPROC			   pglVertexAttrib4d = NULL;
PFNGLVERTEXATTRIB4DVPROC		   pglVertexAttrib4dv = NULL;
PFNGLVERTEXATTRIB4FPROC			   pglVertexAttrib4f = NULL;
PFNGLVERTEXATTRIB4FVPROC		   pglVertexAttrib4fv = NULL;
PFNGLVERTEXATTRIB4IVPROC		   pglVertexAttrib4iv = NULL;
PFNGLVERTEXATTRIB4SPROC			   pglVertexAttrib4s = NULL;
PFNGLVERTEXATTRIB4SVPROC		   pglVertexAttrib4sv = NULL;
PFNGLVERTEXATTRIB4UBVPROC		   pglVertexAttrib4ubv = NULL;
PFNGLVERTEXATTRIB4UIVPROC		   pglVertexAttrib4uiv = NULL;
PFNGLVERTEXATTRIB4USVPROC		   pglVertexAttrib4usv = NULL;
PFNGLVERTEXATTRIBPOINTERPROC	   pglVertexAttribPointer = NULL;

// OpenGL 1.2 imaging
PFNGLBLENDCOLORPROC pglBlendColor = NULL;
PFNGLBLENDEQUATIONPROC pglBlendEquation = NULL;

// OpenGL 1.4 separate blending functions
PFNGLBLENDFUNCSEPARATEPROC pglBlendFuncSeparate = NULL;

// ARB multitexturing
PFNGLACTIVETEXTUREARBPROC             pglActiveTextureARB = NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC		  pglClientActiveTextureARB = NULL;
PFNGLMULTITEXCOORD1DARBPROC			  pglMultiTexCoord1dARB = NULL;
PFNGLMULTITEXCOORD1DVARBPROC		  pglMultiTexCoord1dvARB = NULL;
PFNGLMULTITEXCOORD1FARBPROC			  pglMultiTexCoord1fARB = NULL;
PFNGLMULTITEXCOORD1FVARBPROC		  pglMultiTexCoord1fvARB = NULL;
PFNGLMULTITEXCOORD1IARBPROC			  pglMultiTexCoord1iARB = NULL;
PFNGLMULTITEXCOORD1IVARBPROC		  pglMultiTexCoord1ivARB = NULL;
PFNGLMULTITEXCOORD1SARBPROC			  pglMultiTexCoord1sARB = NULL;
PFNGLMULTITEXCOORD1SVARBPROC		  pglMultiTexCoord1svARB = NULL;
PFNGLMULTITEXCOORD2DARBPROC			  pglMultiTexCoord2dARB = NULL;
PFNGLMULTITEXCOORD2DVARBPROC		  pglMultiTexCoord2dvARB = NULL;
PFNGLMULTITEXCOORD2FARBPROC			  pglMultiTexCoord2fARB = NULL;
PFNGLMULTITEXCOORD2FVARBPROC		  pglMultiTexCoord2fvARB = NULL;
PFNGLMULTITEXCOORD2IARBPROC			  pglMultiTexCoord2iARB = NULL;
PFNGLMULTITEXCOORD2IVARBPROC		  pglMultiTexCoord2ivARB = NULL;
PFNGLMULTITEXCOORD2SARBPROC			  pglMultiTexCoord2sARB = NULL;
PFNGLMULTITEXCOORD2SVARBPROC		  pglMultiTexCoord2svARB = NULL;
PFNGLMULTITEXCOORD3DARBPROC			  pglMultiTexCoord3dARB = NULL;
PFNGLMULTITEXCOORD3DVARBPROC		  pglMultiTexCoord3dvARB = NULL;
PFNGLMULTITEXCOORD3FARBPROC			  pglMultiTexCoord3fARB = NULL;
PFNGLMULTITEXCOORD3FVARBPROC		  pglMultiTexCoord3fvARB = NULL;
PFNGLMULTITEXCOORD3IARBPROC			  pglMultiTexCoord3iARB = NULL;
PFNGLMULTITEXCOORD3IVARBPROC		  pglMultiTexCoord3ivARB = NULL;
PFNGLMULTITEXCOORD3SARBPROC			  pglMultiTexCoord3sARB = NULL;
PFNGLMULTITEXCOORD3SVARBPROC		  pglMultiTexCoord3svARB = NULL;
PFNGLMULTITEXCOORD4DARBPROC			  pglMultiTexCoord4dARB = NULL;
PFNGLMULTITEXCOORD4DVARBPROC		  pglMultiTexCoord4dvARB = NULL;
PFNGLMULTITEXCOORD4FARBPROC			  pglMultiTexCoord4fARB = NULL;
PFNGLMULTITEXCOORD4FVARBPROC		  pglMultiTexCoord4fvARB = NULL;
PFNGLMULTITEXCOORD4IARBPROC			  pglMultiTexCoord4iARB = NULL;
PFNGLMULTITEXCOORD4IVARBPROC		  pglMultiTexCoord4ivARB = NULL;
PFNGLMULTITEXCOORD4SARBPROC			  pglMultiTexCoord4sARB = NULL;
PFNGLMULTITEXCOORD4SVARBPROC		  pglMultiTexCoord4svARB = NULL;

bool wglInit() {
	bool support = true;

#ifdef OPENGL_ENABLE_FBO

	// FBO Framebuffers
    glGenFramebuffersEXT                     = (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT");
    glDeleteFramebuffersEXT                  = (PFNGLDELETEFRAMEBUFFERSEXTPROC)wglGetProcAddress("glDeleteFramebuffersEXT");
    glBindFramebufferEXT                     = (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT");
    glCheckFramebufferStatusEXT              = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatusEXT");
    glGetFramebufferAttachmentParameterivEXT = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)wglGetProcAddress("glGetFramebufferAttachmentParameterivEXT");
    glGenerateMipmapEXT                      = (PFNGLGENERATEMIPMAPEXTPROC)wglGetProcAddress("glGenerateMipmapEXT");
    glFramebufferTexture2DEXT                = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)wglGetProcAddress("glFramebufferTexture2DEXT");
    glFramebufferRenderbufferEXT             = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)wglGetProcAddress("glFramebufferRenderbufferEXT");

	// FBO Renderbuffers
    glGenRenderbuffersEXT                    = (PFNGLGENRENDERBUFFERSEXTPROC)wglGetProcAddress("glGenRenderbuffersEXT");
    glDeleteRenderbuffersEXT                 = (PFNGLDELETERENDERBUFFERSEXTPROC)wglGetProcAddress("glDeleteRenderbuffersEXT");
    glBindRenderbufferEXT                    = (PFNGLBINDRENDERBUFFEREXTPROC)wglGetProcAddress("glBindRenderbufferEXT");
    glRenderbufferStorageEXT                 = (PFNGLRENDERBUFFERSTORAGEEXTPROC)wglGetProcAddress("glRenderbufferStorageEXT");
    glGetRenderbufferParameterivEXT          = (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)wglGetProcAddress("glGetRenderbufferParameterivEXT");
    glIsRenderbufferEXT                      = (PFNGLISRENDERBUFFEREXTPROC)wglGetProcAddress("glIsRenderbufferEXT");

    if(!(glGenFramebuffersEXT && glDeleteFramebuffersEXT && glBindFramebufferEXT && glCheckFramebufferStatusEXT &&
       glGetFramebufferAttachmentParameterivEXT && glGenerateMipmapEXT && glFramebufferTexture2DEXT && glFramebufferRenderbufferEXT &&
       glGenRenderbuffersEXT && glDeleteRenderbuffersEXT && glBindRenderbufferEXT && glRenderbufferStorageEXT &&
       glGetRenderbufferParameterivEXT && glIsRenderbufferEXT))
    {
		xwarn(false, "Graphics card does not support FBO.");
		oglFBOSupport = false;
		support &= false;
	}

#endif /* OPENGL_ENABLE_FBO */

#ifdef GPU_IMAGEPROCESS
	// Shaders
	glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC)wglGetProcAddress("glBlendEquationSeparate");
	glDrawBuffers = (PFNGLDRAWBUFFERSPROC)wglGetProcAddress("glDrawBuffers");   
	glStencilOpSeparate = (PFNGLSTENCILOPSEPARATEPROC)wglGetProcAddress("glStencilOpSeparate");
	glStencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATEPROC)wglGetProcAddress("glStencilFuncSeparate");
	glStencilMaskSeparate = (PFNGLSTENCILMASKSEPARATEPROC)wglGetProcAddress("glStencilMaskSeparate");
	glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
	glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)wglGetProcAddress("glBindAttribLocation");
	glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
	glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
	glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
	glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
	glDetachShader = (PFNGLDETACHSHADERPROC)wglGetProcAddress("glDetachShader");
	glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glDisableVertexAttribArray");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
	glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC)wglGetProcAddress("glGetActiveAttrib");
	glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC)wglGetProcAddress("glGetActiveUniform");
	glGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC)wglGetProcAddress("glGetAttachedShaders");
	glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)wglGetProcAddress("glGetAttribLocation");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
	glGetShaderSource = (PFNGLGETSHADERSOURCEPROC)wglGetProcAddress("glGetShaderSource");
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
	glGetUniformfv = (PFNGLGETUNIFORMFVPROC)wglGetProcAddress("glGetUniformfv");
	glGetUniformiv = (PFNGLGETUNIFORMIVPROC)wglGetProcAddress("glGetUniformiv");
	glGetVertexAttribdv = (PFNGLGETVERTEXATTRIBDVPROC)wglGetProcAddress("glGetVertexAttribdv");
	glGetVertexAttribfv = (PFNGLGETVERTEXATTRIBFVPROC)wglGetProcAddress("glGetVertexAttribfv");
	glGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC)wglGetProcAddress("glGetVertexAttribiv");
	glGetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERVPROC)wglGetProcAddress("glGetVertexAttribPointerv");
	glIsProgram = (PFNGLISPROGRAMPROC)wglGetProcAddress("glIsProgram");
	glIsShader = (PFNGLISSHADERPROC)wglGetProcAddress("glIsShader");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
	glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
	glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
	glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
	glUniform2f = (PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f");
	glUniform3f = (PFNGLUNIFORM3FPROC)wglGetProcAddress("glUniform3f");
	glUniform4f = (PFNGLUNIFORM4FPROC)wglGetProcAddress("glUniform4f");
	glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
	glUniform2i = (PFNGLUNIFORM2IPROC)wglGetProcAddress("glUniform2i");
	glUniform3i = (PFNGLUNIFORM3IPROC)wglGetProcAddress("glUniform3i");
	glUniform4i = (PFNGLUNIFORM4IPROC)wglGetProcAddress("glUniform4i");
	glUniform1fv = (PFNGLUNIFORM1FVPROC)wglGetProcAddress("glUniform1fv");
	glUniform2fv = (PFNGLUNIFORM2FVPROC)wglGetProcAddress("glUniform2fv");
	glUniform3fv = (PFNGLUNIFORM3FVPROC)wglGetProcAddress("glUniform3fv");
	glUniform4fv = (PFNGLUNIFORM4FVPROC)wglGetProcAddress("glUniform4fv");
	glUniform1iv = (PFNGLUNIFORM1IVPROC)wglGetProcAddress("glUniform1iv");
	glUniform2iv = (PFNGLUNIFORM2IVPROC)wglGetProcAddress("glUniform2iv");
	glUniform3iv = (PFNGLUNIFORM3IVPROC)wglGetProcAddress("glUniform3iv");
	glUniform4iv = (PFNGLUNIFORM4IVPROC)wglGetProcAddress("glUniform4iv");
	glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC)wglGetProcAddress("glUniformMatrix2fv");
	glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)wglGetProcAddress("glUniformMatrix3fv");
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
	glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)wglGetProcAddress("glValidateProgram");
	glVertexAttrib1d = (PFNGLVERTEXATTRIB1DPROC)wglGetProcAddress("glVertexAttrib1d");
	glVertexAttrib1dv = (PFNGLVERTEXATTRIB1DVPROC)wglGetProcAddress("glVertexAttrib1dv");
	glVertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC)wglGetProcAddress("glVertexAttrib1f");
	glVertexAttrib1fv = (PFNGLVERTEXATTRIB1FVPROC)wglGetProcAddress("glVertexAttrib1fv");
	glVertexAttrib1s = (PFNGLVERTEXATTRIB1SPROC)wglGetProcAddress("glVertexAttrib1s");
	glVertexAttrib1sv = (PFNGLVERTEXATTRIB1SVPROC)wglGetProcAddress("glVertexAttrib1sv");
	glVertexAttrib2d = (PFNGLVERTEXATTRIB2DPROC)wglGetProcAddress("glVertexAttrib2d");
	glVertexAttrib2dv = (PFNGLVERTEXATTRIB2DVPROC)wglGetProcAddress("glVertexAttrib2dv");
	glVertexAttrib2f = (PFNGLVERTEXATTRIB2FPROC)wglGetProcAddress("glVertexAttrib2f");
	glVertexAttrib2fv = (PFNGLVERTEXATTRIB2FVPROC)wglGetProcAddress("glVertexAttrib2fv");
	glVertexAttrib2s = (PFNGLVERTEXATTRIB2SPROC)wglGetProcAddress("glVertexAttrib2s");
	glVertexAttrib2sv = (PFNGLVERTEXATTRIB2SVPROC)wglGetProcAddress("glVertexAttrib2sv");
	glVertexAttrib3d = (PFNGLVERTEXATTRIB3DPROC)wglGetProcAddress("glVertexAttrib3d");
	glVertexAttrib3dv = (PFNGLVERTEXATTRIB3DVPROC)wglGetProcAddress("glVertexAttrib3dv");
	glVertexAttrib3f = (PFNGLVERTEXATTRIB3FPROC)wglGetProcAddress("glVertexAttrib3f");
	glVertexAttrib3fv = (PFNGLVERTEXATTRIB3FVPROC)wglGetProcAddress("glVertexAttrib3fv");
	glVertexAttrib3s = (PFNGLVERTEXATTRIB3SPROC)wglGetProcAddress("glVertexAttrib3s");
	glVertexAttrib3sv = (PFNGLVERTEXATTRIB3SVPROC)wglGetProcAddress("glVertexAttrib3sv");
	glVertexAttrib4Nbv = (PFNGLVERTEXATTRIB4NBVPROC)wglGetProcAddress("glVertexAttrib4Nbv");
	glVertexAttrib4Niv = (PFNGLVERTEXATTRIB4NIVPROC)wglGetProcAddress("glVertexAttrib4Niv");
	glVertexAttrib4Nsv = (PFNGLVERTEXATTRIB4NSVPROC)wglGetProcAddress("glVertexAttrib4Nsv");
	glVertexAttrib4Nub = (PFNGLVERTEXATTRIB4NUBPROC)wglGetProcAddress("glVertexAttrib4Nub");
	glVertexAttrib4Nubv = (PFNGLVERTEXATTRIB4NUBVPROC)wglGetProcAddress("glVertexAttrib4Nubv");
	glVertexAttrib4Nuiv = (PFNGLVERTEXATTRIB4NUIVPROC)wglGetProcAddress("glVertexAttrib4Nuiv");
	glVertexAttrib4Nusv = (PFNGLVERTEXATTRIB4NUSVPROC)wglGetProcAddress("glVertexAttrib4Nusv");
	glVertexAttrib4bv = (PFNGLVERTEXATTRIB4BVPROC)wglGetProcAddress("glVertexAttrib4bv");
	glVertexAttrib4d = (PFNGLVERTEXATTRIB4DPROC)wglGetProcAddress("glVertexAttrib4d");
	glVertexAttrib4dv = (PFNGLVERTEXATTRIB4DVPROC)wglGetProcAddress("glVertexAttrib4dv");
	glVertexAttrib4f = (PFNGLVERTEXATTRIB4FPROC)wglGetProcAddress("glVertexAttrib4f");
	glVertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC)wglGetProcAddress("glVertexAttrib4fv");
	glVertexAttrib4iv = (PFNGLVERTEXATTRIB4IVPROC)wglGetProcAddress("glVertexAttrib4iv");
	glVertexAttrib4s = (PFNGLVERTEXATTRIB4SPROC)wglGetProcAddress("glVertexAttrib4s");
	glVertexAttrib4sv = (PFNGLVERTEXATTRIB4SVPROC)wglGetProcAddress("glVertexAttrib4sv");
	glVertexAttrib4ubv = (PFNGLVERTEXATTRIB4UBVPROC)wglGetProcAddress("glVertexAttrib4ubv");
	glVertexAttrib4uiv = (PFNGLVERTEXATTRIB4UIVPROC)wglGetProcAddress("glVertexAttrib4uiv");
	glVertexAttrib4usv = (PFNGLVERTEXATTRIB4USVPROC)wglGetProcAddress("glVertexAttrib4usv");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");

	if(!(
		glBlendEquationSeparate &&
		glDrawBuffers &&   
		glStencilOpSeparate &&
		glStencilFuncSeparate &&
		glStencilMaskSeparate &&
		glAttachShader &&
		glBindAttribLocation &&
		glCompileShader &&
		glCreateProgram &&
		glCreateShader &&
		glDeleteProgram &&
		glDeleteShader &&
		glDetachShader &&
		glDisableVertexAttribArray &&
		glEnableVertexAttribArray &&
		glGetActiveAttrib &&
		glGetActiveUniform &&
		glGetAttachedShaders &&
		glGetAttribLocation &&
		glGetProgramiv &&
		glGetProgramInfoLog &&
		glGetShaderiv &&
		glGetShaderInfoLog &&
		glGetShaderSource &&
		glGetUniformLocation &&
		glGetUniformfv &&
		glGetUniformiv &&
		glGetVertexAttribdv &&
		glGetVertexAttribfv &&
		glGetVertexAttribiv &&
		glGetVertexAttribPointerv &&
		glIsProgram &&
		glIsShader &&
		glLinkProgram &&
		glShaderSource &&
		glUseProgram &&
		glUniform1f &&
		glUniform2f &&
		glUniform3f &&
		glUniform4f &&
		glUniform1i &&
		glUniform2i &&
		glUniform3i &&
		glUniform4i &&
		glUniform1fv &&
		glUniform2fv &&
		glUniform3fv &&
		glUniform4fv &&
		glUniform1iv &&
		glUniform2iv &&
		glUniform3iv &&
		glUniform4iv &&
		glUniformMatrix2fv &&
		glUniformMatrix3fv &&
		glUniformMatrix4fv &&
		glValidateProgram &&
		glVertexAttrib1d &&
		glVertexAttrib1dv &&
		glVertexAttrib1f &&
		glVertexAttrib1fv &&
		glVertexAttrib1s &&
		glVertexAttrib1sv &&
		glVertexAttrib2d &&
		glVertexAttrib2dv &&
		glVertexAttrib2f &&
		glVertexAttrib2fv &&
		glVertexAttrib2s &&
		glVertexAttrib2sv &&
		glVertexAttrib3d &&
		glVertexAttrib3dv &&
		glVertexAttrib3f &&
		glVertexAttrib3fv &&
		glVertexAttrib3s &&
		glVertexAttrib3sv &&
		glVertexAttrib4Nbv &&
		glVertexAttrib4Niv &&
		glVertexAttrib4Nsv &&
		glVertexAttrib4Nub &&
		glVertexAttrib4Nubv &&
		glVertexAttrib4Nuiv &&
		glVertexAttrib4Nusv &&
		glVertexAttrib4bv &&
		glVertexAttrib4d &&
		glVertexAttrib4dv &&
		glVertexAttrib4f &&
		glVertexAttrib4fv &&
		glVertexAttrib4iv &&
		glVertexAttrib4s &&
		glVertexAttrib4sv &&
		glVertexAttrib4ubv &&
		glVertexAttrib4uiv &&
		glVertexAttrib4usv &&
		glVertexAttribPointer)) {		
		xwarn(false, "Graphics card does not support shaders.");
		oglShadersSupport = false;
		support &= false;
	}
#endif /* GPU_IMAGEPROCESS */

	// OpenGL 1.2 imaging
	glBlendColor = (PFNGLBLENDCOLORPROC)wglGetProcAddress("glBlendColor");
	glBlendEquation = (PFNGLBLENDEQUATIONPROC)wglGetProcAddress("glBlendEquation");

	if(!(
		glBlendColor &&
		glBlendEquation)) {
		xwarn(false, "Graphics card does not support imaging extension.");
		oglImagingSupport = false;
		support &= false;
	}

	// OpenGL 1.4 separate blending functions
	glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)wglGetProcAddress("glBlendFuncSeparate");

	if(!glBlendFuncSeparate) {
		xwarn(false, "Graphics card does not support separate blending.");
		oglSeparateBlending = false;
		support &= false;
	}

	// ARB multitexturing
	glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
	glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC)wglGetProcAddress("glClientActiveTextureARB");
	glMultiTexCoord1dARB = (PFNGLMULTITEXCOORD1DARBPROC)wglGetProcAddress("glMultiTexCoord1dARB");
	glMultiTexCoord1dvARB = (PFNGLMULTITEXCOORD1DVARBPROC)wglGetProcAddress("glMultiTexCoord1dvARB");
	glMultiTexCoord1fARB = (PFNGLMULTITEXCOORD1FARBPROC)wglGetProcAddress("glMultiTexCoord1fARB");
	glMultiTexCoord1fvARB = (PFNGLMULTITEXCOORD1FVARBPROC)wglGetProcAddress("glMultiTexCoord1fvARB");
	glMultiTexCoord1iARB = (PFNGLMULTITEXCOORD1IARBPROC)wglGetProcAddress("glMultiTexCoord1iARB");
	glMultiTexCoord1ivARB = (PFNGLMULTITEXCOORD1IVARBPROC)wglGetProcAddress("glMultiTexCoord1ivARB");
	glMultiTexCoord1sARB = (PFNGLMULTITEXCOORD1SARBPROC)wglGetProcAddress("glMultiTexCoord1sARB");
	glMultiTexCoord1svARB = (PFNGLMULTITEXCOORD1SVARBPROC)wglGetProcAddress("glMultiTexCoord1svARB");
	glMultiTexCoord2dARB = (PFNGLMULTITEXCOORD2DARBPROC)wglGetProcAddress("glMultiTexCoord2dARB");
	glMultiTexCoord2dvARB = (PFNGLMULTITEXCOORD2DVARBPROC)wglGetProcAddress("glMultiTexCoord2dvARB");
	glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");
	glMultiTexCoord2fvARB = (PFNGLMULTITEXCOORD2FVARBPROC)wglGetProcAddress("glMultiTexCoord2fvARB");
	glMultiTexCoord2iARB = (PFNGLMULTITEXCOORD2IARBPROC)wglGetProcAddress("glMultiTexCoord2iARB");
	glMultiTexCoord2ivARB = (PFNGLMULTITEXCOORD2IVARBPROC)wglGetProcAddress("glMultiTexCoord2ivARB");
	glMultiTexCoord2sARB = (PFNGLMULTITEXCOORD2SARBPROC)wglGetProcAddress("glMultiTexCoord2sARB");
	glMultiTexCoord2svARB = (PFNGLMULTITEXCOORD2SVARBPROC)wglGetProcAddress("glMultiTexCoord2svARB");
	glMultiTexCoord3dARB = (PFNGLMULTITEXCOORD3DARBPROC)wglGetProcAddress("glMultiTexCoord3dARB");
	glMultiTexCoord3dvARB = (PFNGLMULTITEXCOORD3DVARBPROC)wglGetProcAddress("glMultiTexCoord3dvARB");
	glMultiTexCoord3fARB = (PFNGLMULTITEXCOORD3FARBPROC)wglGetProcAddress("glMultiTexCoord3fARB");
	glMultiTexCoord3fvARB = (PFNGLMULTITEXCOORD3FVARBPROC)wglGetProcAddress("glMultiTexCoord3fvARB");
	glMultiTexCoord3iARB = (PFNGLMULTITEXCOORD3IARBPROC)wglGetProcAddress("glMultiTexCoord3iARB");
	glMultiTexCoord3ivARB = (PFNGLMULTITEXCOORD3IVARBPROC)wglGetProcAddress("glMultiTexCoord3ivARB");
	glMultiTexCoord3sARB = (PFNGLMULTITEXCOORD3SARBPROC)wglGetProcAddress("glMultiTexCoord3sARB");
	glMultiTexCoord3svARB = (PFNGLMULTITEXCOORD3SVARBPROC)wglGetProcAddress("glMultiTexCoord3svARB");
	glMultiTexCoord4dARB = (PFNGLMULTITEXCOORD4DARBPROC)wglGetProcAddress("glMultiTexCoord4dARB");
	glMultiTexCoord4dvARB = (PFNGLMULTITEXCOORD4DVARBPROC)wglGetProcAddress("glMultiTexCoord4dvARB");
	glMultiTexCoord4fARB = (PFNGLMULTITEXCOORD4FARBPROC)wglGetProcAddress("glMultiTexCoord4fARB");
	glMultiTexCoord4fvARB = (PFNGLMULTITEXCOORD4FVARBPROC)wglGetProcAddress("glMultiTexCoord4fvARB");
	glMultiTexCoord4iARB = (PFNGLMULTITEXCOORD4IARBPROC)wglGetProcAddress("glMultiTexCoord4iARB");
	glMultiTexCoord4ivARB = (PFNGLMULTITEXCOORD4IVARBPROC)wglGetProcAddress("glMultiTexCoord4ivARB");
	glMultiTexCoord4sARB = (PFNGLMULTITEXCOORD4SARBPROC)wglGetProcAddress("glMultiTexCoord4sARB");
	glMultiTexCoord4svARB = (PFNGLMULTITEXCOORD4SVARBPROC)wglGetProcAddress("glMultiTexCoord4svARB");

	if(!(
		glActiveTextureARB &&
		glClientActiveTextureARB &&
		glMultiTexCoord1dARB &&
		glMultiTexCoord1dvARB &&
		glMultiTexCoord1fARB &&
		glMultiTexCoord1fvARB &&
		glMultiTexCoord1iARB &&
		glMultiTexCoord1ivARB &&
		glMultiTexCoord1sARB &&
		glMultiTexCoord1svARB &&
		glMultiTexCoord2dARB &&
		glMultiTexCoord2dvARB &&
		glMultiTexCoord2fARB &&
		glMultiTexCoord2fvARB &&
		glMultiTexCoord2iARB &&
		glMultiTexCoord2ivARB &&
		glMultiTexCoord2sARB &&
		glMultiTexCoord2svARB &&
		glMultiTexCoord3dARB &&
		glMultiTexCoord3dvARB &&
		glMultiTexCoord3fARB &&
		glMultiTexCoord3fvARB &&
		glMultiTexCoord3iARB &&
		glMultiTexCoord3ivARB &&
		glMultiTexCoord3sARB &&
		glMultiTexCoord3svARB &&
		glMultiTexCoord4dARB &&
		glMultiTexCoord4dvARB &&
		glMultiTexCoord4fARB &&
		glMultiTexCoord4fvARB &&
		glMultiTexCoord4iARB &&
		glMultiTexCoord4ivARB &&
		glMultiTexCoord4sARB &&
		glMultiTexCoord4svARB )) {

		xwarn(false, "Graphics card does not support multitexturing.");
		oglMultitexturing = false;
		support &= false;
	}
	return support;
}
