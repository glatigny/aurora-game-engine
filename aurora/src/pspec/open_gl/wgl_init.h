#ifndef WGL_INIT_H
#define WGL_INIT_H

#ifndef WIN32
#error WGL support is for WIN32 only. Please use glx_init.h for other platforms.
#endif

#include <GL/gl.h>
#include <GL/glu.h>

// This module provides OpenGL dynamic linking and initialization for wGL extensions.

// Pointer initialization function.
bool wglInit();

// OpenGL function declarations

#ifdef OPENGL_ENABLE_FBO
// Framebuffer object
extern PFNGLGENFRAMEBUFFERSEXTPROC                     pglGenFramebuffersEXT;                      // FBO name generation procedure
extern PFNGLDELETEFRAMEBUFFERSEXTPROC                  pglDeleteFramebuffersEXT;                   // FBO deletion procedure
extern PFNGLBINDFRAMEBUFFEREXTPROC                     pglBindFramebufferEXT;                      // FBO bind procedure
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC              pglCheckFramebufferStatusEXT;               // FBO completeness test procedure
extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC pglGetFramebufferAttachmentParameterivEXT;  // return various FBO parameters
extern PFNGLGENERATEMIPMAPEXTPROC                      pglGenerateMipmapEXT;                       // FBO automatic mipmap generation procedure
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC                pglFramebufferTexture2DEXT;                 // FBO texdture attachement procedure
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC             pglFramebufferRenderbufferEXT;              // FBO renderbuffer attachement procedure

#define glGenFramebuffersEXT                        pglGenFramebuffersEXT
#define glDeleteFramebuffersEXT                     pglDeleteFramebuffersEXT
#define glBindFramebufferEXT                        pglBindFramebufferEXT
#define glCheckFramebufferStatusEXT                 pglCheckFramebufferStatusEXT
#define glGetFramebufferAttachmentParameterivEXT    pglGetFramebufferAttachmentParameterivEXT
#define glGenerateMipmapEXT                         pglGenerateMipmapEXT
#define glFramebufferTexture2DEXT                   pglFramebufferTexture2DEXT
#define glFramebufferRenderbufferEXT                pglFramebufferRenderbufferEXT

// Renderbuffer object
extern PFNGLGENRENDERBUFFERSEXTPROC                    pglGenRenderbuffersEXT;                     // renderbuffer generation procedure
extern PFNGLDELETERENDERBUFFERSEXTPROC                 pglDeleteRenderbuffersEXT;                  // renderbuffer deletion procedure
extern PFNGLBINDRENDERBUFFEREXTPROC                    pglBindRenderbufferEXT;                     // renderbuffer bind procedure
extern PFNGLRENDERBUFFERSTORAGEEXTPROC                 pglRenderbufferStorageEXT;                  // renderbuffer memory allocation procedure
extern PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC          pglGetRenderbufferParameterivEXT;           // return various renderbuffer parameters
extern PFNGLISRENDERBUFFEREXTPROC                      pglIsRenderbufferEXT;                       // determine renderbuffer object type

#define glGenRenderbuffersEXT                       pglGenRenderbuffersEXT
#define glDeleteRenderbuffersEXT                    pglDeleteRenderbuffersEXT
#define glBindRenderbufferEXT                       pglBindRenderbufferEXT
#define glRenderbufferStorageEXT                    pglRenderbufferStorageEXT
#define glGetRenderbufferParameterivEXT             pglGetRenderbufferParameterivEXT
#define glIsRenderbufferEXT                         pglIsRenderbufferEXT

#endif /* OPENGL_ENABLE_FBO */


#ifdef GPU_IMAGEPROCESS
// Shaders

extern PFNGLBLENDEQUATIONSEPARATEPROC     pglBlendEquationSeparate;
extern PFNGLDRAWBUFFERSPROC               pglDrawBuffers;   
extern PFNGLSTENCILOPSEPARATEPROC		   pglStencilOpSeparate;
extern PFNGLSTENCILFUNCSEPARATEPROC	   pglStencilFuncSeparate;
extern PFNGLSTENCILMASKSEPARATEPROC	   pglStencilMaskSeparate;
extern PFNGLATTACHSHADERPROC			   pglAttachShader;
extern PFNGLBINDATTRIBLOCATIONPROC		   pglBindAttribLocation;
extern PFNGLCOMPILESHADERPROC			   pglCompileShader;
extern PFNGLCREATEPROGRAMPROC			   pglCreateProgram;
extern PFNGLCREATESHADERPROC			   pglCreateShader;
extern PFNGLDELETEPROGRAMPROC			   pglDeleteProgram;
extern PFNGLDELETESHADERPROC			   pglDeleteShader;
extern PFNGLDETACHSHADERPROC			   pglDetachShader;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC   pglDisableVertexAttribArray;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC   pglEnableVertexAttribArray;
extern PFNGLGETACTIVEATTRIBPROC		   pglGetActiveAttrib;
extern PFNGLGETACTIVEUNIFORMPROC		   pglGetActiveUniform;
extern PFNGLGETATTACHEDSHADERSPROC		   pglGetAttachedShaders;
extern PFNGLGETATTRIBLOCATIONPROC		   pglGetAttribLocation;
extern PFNGLGETPROGRAMIVPROC			   pglGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC		   pglGetProgramInfoLog;
extern PFNGLGETSHADERIVPROC			   pglGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC		   pglGetShaderInfoLog;
extern PFNGLGETSHADERSOURCEPROC		   pglGetShaderSource;
extern PFNGLGETUNIFORMLOCATIONPROC		   pglGetUniformLocation;
extern PFNGLGETUNIFORMFVPROC			   pglGetUniformfv;
extern PFNGLGETUNIFORMIVPROC			   pglGetUniformiv;
extern PFNGLGETVERTEXATTRIBDVPROC		   pglGetVertexAttribdv;
extern PFNGLGETVERTEXATTRIBFVPROC		   pglGetVertexAttribfv;
extern PFNGLGETVERTEXATTRIBIVPROC		   pglGetVertexAttribiv;
extern PFNGLGETVERTEXATTRIBPOINTERVPROC   pglGetVertexAttribPointerv;
extern PFNGLISPROGRAMPROC				   pglIsProgram;
extern PFNGLISSHADERPROC				   pglIsShader;
extern PFNGLLINKPROGRAMPROC			   pglLinkProgram;
extern PFNGLSHADERSOURCEPROC			   pglShaderSource;
extern PFNGLUSEPROGRAMPROC				   pglUseProgram;
extern PFNGLUNIFORM1FPROC				   pglUniform1f;
extern PFNGLUNIFORM2FPROC				   pglUniform2f;
extern PFNGLUNIFORM3FPROC				   pglUniform3f;
extern PFNGLUNIFORM4FPROC				   pglUniform4f;
extern PFNGLUNIFORM1IPROC				   pglUniform1i;
extern PFNGLUNIFORM2IPROC				   pglUniform2i;
extern PFNGLUNIFORM3IPROC				   pglUniform3i;
extern PFNGLUNIFORM4IPROC				   pglUniform4i;
extern PFNGLUNIFORM1FVPROC				   pglUniform1fv;
extern PFNGLUNIFORM2FVPROC				   pglUniform2fv;
extern PFNGLUNIFORM3FVPROC				   pglUniform3fv;
extern PFNGLUNIFORM4FVPROC				   pglUniform4fv;
extern PFNGLUNIFORM1IVPROC				   pglUniform1iv;
extern PFNGLUNIFORM2IVPROC				   pglUniform2iv;
extern PFNGLUNIFORM3IVPROC				   pglUniform3iv;
extern PFNGLUNIFORM4IVPROC				   pglUniform4iv;
extern PFNGLUNIFORMMATRIX2FVPROC		   pglUniformMatrix2fv;
extern PFNGLUNIFORMMATRIX3FVPROC		   pglUniformMatrix3fv;
extern PFNGLUNIFORMMATRIX4FVPROC		   pglUniformMatrix4fv;
extern PFNGLVALIDATEPROGRAMPROC		   pglValidateProgram;
extern PFNGLVERTEXATTRIB1DPROC			   pglVertexAttrib1d;
extern PFNGLVERTEXATTRIB1DVPROC		   pglVertexAttrib1dv;
extern PFNGLVERTEXATTRIB1FPROC			   pglVertexAttrib1f;
extern PFNGLVERTEXATTRIB1FVPROC		   pglVertexAttrib1fv;
extern PFNGLVERTEXATTRIB1SPROC			   pglVertexAttrib1s;
extern PFNGLVERTEXATTRIB1SVPROC		   pglVertexAttrib1sv;
extern PFNGLVERTEXATTRIB2DPROC			   pglVertexAttrib2d;
extern PFNGLVERTEXATTRIB2DVPROC		   pglVertexAttrib2dv;
extern PFNGLVERTEXATTRIB2FPROC			   pglVertexAttrib2f;
extern PFNGLVERTEXATTRIB2FVPROC		   pglVertexAttrib2fv;
extern PFNGLVERTEXATTRIB2SPROC			   pglVertexAttrib2s;
extern PFNGLVERTEXATTRIB2SVPROC		   pglVertexAttrib2sv;
extern PFNGLVERTEXATTRIB3DPROC			   pglVertexAttrib3d;
extern PFNGLVERTEXATTRIB3DVPROC		   pglVertexAttrib3dv;
extern PFNGLVERTEXATTRIB3FPROC			   pglVertexAttrib3f;
extern PFNGLVERTEXATTRIB3FVPROC		   pglVertexAttrib3fv;
extern PFNGLVERTEXATTRIB3SPROC			   pglVertexAttrib3s;
extern PFNGLVERTEXATTRIB3SVPROC		   pglVertexAttrib3sv;
extern PFNGLVERTEXATTRIB4NBVPROC		   pglVertexAttrib4Nbv;
extern PFNGLVERTEXATTRIB4NIVPROC		   pglVertexAttrib4Niv;
extern PFNGLVERTEXATTRIB4NSVPROC		   pglVertexAttrib4Nsv;
extern PFNGLVERTEXATTRIB4NUBPROC		   pglVertexAttrib4Nub;
extern PFNGLVERTEXATTRIB4NUBVPROC		   pglVertexAttrib4Nubv;
extern PFNGLVERTEXATTRIB4NUIVPROC		   pglVertexAttrib4Nuiv;
extern PFNGLVERTEXATTRIB4NUSVPROC		   pglVertexAttrib4Nusv;
extern PFNGLVERTEXATTRIB4BVPROC		   pglVertexAttrib4bv;
extern PFNGLVERTEXATTRIB4DPROC			   pglVertexAttrib4d;
extern PFNGLVERTEXATTRIB4DVPROC		   pglVertexAttrib4dv;
extern PFNGLVERTEXATTRIB4FPROC			   pglVertexAttrib4f;
extern PFNGLVERTEXATTRIB4FVPROC		   pglVertexAttrib4fv;
extern PFNGLVERTEXATTRIB4IVPROC		   pglVertexAttrib4iv;
extern PFNGLVERTEXATTRIB4SPROC			   pglVertexAttrib4s;
extern PFNGLVERTEXATTRIB4SVPROC		   pglVertexAttrib4sv;
extern PFNGLVERTEXATTRIB4UBVPROC		   pglVertexAttrib4ubv;
extern PFNGLVERTEXATTRIB4UIVPROC		   pglVertexAttrib4uiv;
extern PFNGLVERTEXATTRIB4USVPROC		   pglVertexAttrib4usv;
extern PFNGLVERTEXATTRIBPOINTERPROC	   pglVertexAttribPointer;

#define glBlendEquationSeparate pglBlendEquationSeparate
#define glDrawBuffers pglDrawBuffers   
#define glStencilOpSeparate pglStencilOpSeparate
#define glStencilFuncSeparate pglStencilFuncSeparate
#define glStencilMaskSeparate pglStencilMaskSeparate
#define glAttachShader pglAttachShader
#define glBindAttribLocation pglBindAttribLocation
#define glCompileShader pglCompileShader
#define glCreateProgram pglCreateProgram
#define glCreateShader pglCreateShader
#define glDeleteProgram pglDeleteProgram
#define glDeleteShader pglDeleteShader
#define glDetachShader pglDetachShader
#define glDisableVertexAttribArray pglDisableVertexAttribArray
#define glEnableVertexAttribArray pglEnableVertexAttribArray
#define glGetActiveAttrib pglGetActiveAttrib
#define glGetActiveUniform pglGetActiveUniform
#define glGetAttachedShaders pglGetAttachedShaders
#define glGetAttribLocation pglGetAttribLocation
#define glGetProgramiv pglGetProgramiv
#define glGetProgramInfoLog pglGetProgramInfoLog
#define glGetShaderiv pglGetShaderiv
#define glGetShaderInfoLog pglGetShaderInfoLog
#define glGetShaderSource pglGetShaderSource
#define glGetUniformLocation pglGetUniformLocation
#define glGetUniformfv pglGetUniformfv
#define glGetUniformiv pglGetUniformiv
#define glGetVertexAttribdv pglGetVertexAttribdv
#define glGetVertexAttribfv pglGetVertexAttribfv
#define glGetVertexAttribiv pglGetVertexAttribiv
#define glGetVertexAttribPointerv pglGetVertexAttribPointerv
#define glIsProgram pglIsProgram
#define glIsShader pglIsShader
#define glLinkProgram pglLinkProgram
#define glShaderSource pglShaderSource
#define glUseProgram pglUseProgram
#define glUniform1f pglUniform1f
#define glUniform2f pglUniform2f
#define glUniform3f pglUniform3f
#define glUniform4f pglUniform4f
#define glUniform1i pglUniform1i
#define glUniform2i pglUniform2i
#define glUniform3i pglUniform3i
#define glUniform4i pglUniform4i
#define glUniform1fv pglUniform1fv
#define glUniform2fv pglUniform2fv
#define glUniform3fv pglUniform3fv
#define glUniform4fv pglUniform4fv
#define glUniform1iv pglUniform1iv
#define glUniform2iv pglUniform2iv
#define glUniform3iv pglUniform3iv
#define glUniform4iv pglUniform4iv
#define glUniformMatrix2fv pglUniformMatrix2fv
#define glUniformMatrix3fv pglUniformMatrix3fv
#define glUniformMatrix4fv pglUniformMatrix4fv
#define glValidateProgram pglValidateProgram
#define glVertexAttrib1d pglVertexAttrib1d
#define glVertexAttrib1dv pglVertexAttrib1dv
#define glVertexAttrib1f pglVertexAttrib1f
#define glVertexAttrib1fv pglVertexAttrib1fv
#define glVertexAttrib1s pglVertexAttrib1s
#define glVertexAttrib1sv pglVertexAttrib1sv
#define glVertexAttrib2d pglVertexAttrib2d
#define glVertexAttrib2dv pglVertexAttrib2dv
#define glVertexAttrib2f pglVertexAttrib2f
#define glVertexAttrib2fv pglVertexAttrib2fv
#define glVertexAttrib2s pglVertexAttrib2s
#define glVertexAttrib2sv pglVertexAttrib2sv
#define glVertexAttrib3d pglVertexAttrib3d
#define glVertexAttrib3dv pglVertexAttrib3dv
#define glVertexAttrib3f pglVertexAttrib3f
#define glVertexAttrib3fv pglVertexAttrib3fv
#define glVertexAttrib3s pglVertexAttrib3s
#define glVertexAttrib3sv pglVertexAttrib3sv
#define glVertexAttrib4Nbv pglVertexAttrib4Nbv
#define glVertexAttrib4Niv pglVertexAttrib4Niv
#define glVertexAttrib4Nsv pglVertexAttrib4Nsv
#define glVertexAttrib4Nub pglVertexAttrib4Nub
#define glVertexAttrib4Nubv pglVertexAttrib4Nubv
#define glVertexAttrib4Nuiv pglVertexAttrib4Nuiv
#define glVertexAttrib4Nusv pglVertexAttrib4Nusv
#define glVertexAttrib4bv pglVertexAttrib4bv
#define glVertexAttrib4d pglVertexAttrib4d
#define glVertexAttrib4dv pglVertexAttrib4dv
#define glVertexAttrib4f pglVertexAttrib4f
#define glVertexAttrib4fv pglVertexAttrib4fv
#define glVertexAttrib4iv pglVertexAttrib4iv
#define glVertexAttrib4s pglVertexAttrib4s
#define glVertexAttrib4sv pglVertexAttrib4sv
#define glVertexAttrib4ubv pglVertexAttrib4ubv
#define glVertexAttrib4uiv pglVertexAttrib4uiv
#define glVertexAttrib4usv pglVertexAttrib4usv
#define glVertexAttribPointer pglVertexAttribPointer

#endif GPU_IMAGEPROCESS

// OpenGL 1.2 imaging
extern PFNGLBLENDCOLORPROC pglBlendColor;
extern PFNGLBLENDEQUATIONPROC pglBlendEquation;

#define glBlendColor pglBlendColor
#define glBlendEquation pglBlendEquation

// OpenGL 1.4 separate blending functions
extern PFNGLBLENDFUNCSEPARATEPROC pglBlendFuncSeparate;

// ARB multitexture                 
extern PFNGLACTIVETEXTUREARBPROC             pglActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC		  pglClientActiveTextureARB;
extern PFNGLMULTITEXCOORD1DARBPROC			  pglMultiTexCoord1dARB;
extern PFNGLMULTITEXCOORD1DVARBPROC		  pglMultiTexCoord1dvARB;
extern PFNGLMULTITEXCOORD1FARBPROC			  pglMultiTexCoord1fARB;
extern PFNGLMULTITEXCOORD1FVARBPROC		  pglMultiTexCoord1fvARB;
extern PFNGLMULTITEXCOORD1IARBPROC			  pglMultiTexCoord1iARB;
extern PFNGLMULTITEXCOORD1IVARBPROC		  pglMultiTexCoord1ivARB;
extern PFNGLMULTITEXCOORD1SARBPROC			  pglMultiTexCoord1sARB;
extern PFNGLMULTITEXCOORD1SVARBPROC		  pglMultiTexCoord1svARB;
extern PFNGLMULTITEXCOORD2DARBPROC			  pglMultiTexCoord2dARB;
extern PFNGLMULTITEXCOORD2DVARBPROC		  pglMultiTexCoord2dvARB;
extern PFNGLMULTITEXCOORD2FARBPROC			  pglMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD2FVARBPROC		  pglMultiTexCoord2fvARB;
extern PFNGLMULTITEXCOORD2IARBPROC			  pglMultiTexCoord2iARB;
extern PFNGLMULTITEXCOORD2IVARBPROC		  pglMultiTexCoord2ivARB;
extern PFNGLMULTITEXCOORD2SARBPROC			  pglMultiTexCoord2sARB;
extern PFNGLMULTITEXCOORD2SVARBPROC		  pglMultiTexCoord2svARB;
extern PFNGLMULTITEXCOORD3DARBPROC			  pglMultiTexCoord3dARB;
extern PFNGLMULTITEXCOORD3DVARBPROC		  pglMultiTexCoord3dvARB;
extern PFNGLMULTITEXCOORD3FARBPROC			  pglMultiTexCoord3fARB;
extern PFNGLMULTITEXCOORD3FVARBPROC		  pglMultiTexCoord3fvARB;
extern PFNGLMULTITEXCOORD3IARBPROC			  pglMultiTexCoord3iARB;
extern PFNGLMULTITEXCOORD3IVARBPROC		  pglMultiTexCoord3ivARB;
extern PFNGLMULTITEXCOORD3SARBPROC			  pglMultiTexCoord3sARB;
extern PFNGLMULTITEXCOORD3SVARBPROC		  pglMultiTexCoord3svARB;
extern PFNGLMULTITEXCOORD4DARBPROC			  pglMultiTexCoord4dARB;
extern PFNGLMULTITEXCOORD4DVARBPROC		  pglMultiTexCoord4dvARB;
extern PFNGLMULTITEXCOORD4FARBPROC			  pglMultiTexCoord4fARB;
extern PFNGLMULTITEXCOORD4FVARBPROC		  pglMultiTexCoord4fvARB;
extern PFNGLMULTITEXCOORD4IARBPROC			  pglMultiTexCoord4iARB;
extern PFNGLMULTITEXCOORD4IVARBPROC		  pglMultiTexCoord4ivARB;
extern PFNGLMULTITEXCOORD4SARBPROC			  pglMultiTexCoord4sARB;
extern PFNGLMULTITEXCOORD4SVARBPROC		  pglMultiTexCoord4svARB;

#define glActiveTextureARB pglActiveTextureARB
#define glClientActiveTextureARB pglClientActiveTextureARB
#define glMultiTexCoord1dARB pglMultiTexCoord1dARB
#define glMultiTexCoord1dvARB pglMultiTexCoord1dvARB
#define glMultiTexCoord1fARB pglMultiTexCoord1fARB
#define glMultiTexCoord1fvARB pglMultiTexCoord1fvARB
#define glMultiTexCoord1iARB pglMultiTexCoord1iARB
#define glMultiTexCoord1ivARB pglMultiTexCoord1ivARB
#define glMultiTexCoord1sARB pglMultiTexCoord1sARB
#define glMultiTexCoord1svARB pglMultiTexCoord1svARB
#define glMultiTexCoord2dARB pglMultiTexCoord2dARB
#define glMultiTexCoord2dvARB pglMultiTexCoord2dvARB
#define glMultiTexCoord2fARB pglMultiTexCoord2fARB
#define glMultiTexCoord2fvARB pglMultiTexCoord2fvARB
#define glMultiTexCoord2iARB pglMultiTexCoord2iARB
#define glMultiTexCoord2ivARB pglMultiTexCoord2ivARB
#define glMultiTexCoord2sARB pglMultiTexCoord2sARB
#define glMultiTexCoord2svARB pglMultiTexCoord2svARB
#define glMultiTexCoord3dARB pglMultiTexCoord3dARB
#define glMultiTexCoord3dvARB pglMultiTexCoord3dvARB
#define glMultiTexCoord3fARB pglMultiTexCoord3fARB
#define glMultiTexCoord3fvARB pglMultiTexCoord3fvARB
#define glMultiTexCoord3iARB pglMultiTexCoord3iARB
#define glMultiTexCoord3ivARB pglMultiTexCoord3ivARB
#define glMultiTexCoord3sARB pglMultiTexCoord3sARB
#define glMultiTexCoord3svARB pglMultiTexCoord3svARB
#define glMultiTexCoord4dARB pglMultiTexCoord4dARB
#define glMultiTexCoord4dvARB pglMultiTexCoord4dvARB
#define glMultiTexCoord4fARB pglMultiTexCoord4fARB
#define glMultiTexCoord4fvARB pglMultiTexCoord4fvARB
#define glMultiTexCoord4iARB pglMultiTexCoord4iARB
#define glMultiTexCoord4ivARB pglMultiTexCoord4ivARB
#define glMultiTexCoord4sARB pglMultiTexCoord4sARB
#define glMultiTexCoord4svARB pglMultiTexCoord4svARB


#define glBlendFuncSeparate pglBlendFuncSeparate

#endif /* WGL_INIT_H */
