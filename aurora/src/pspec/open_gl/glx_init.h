#ifndef GLX_INIT_H
#define GLX_INIT_H

#ifdef WIN32
#error No GLX support in Win32. Please include wgl_init.h
#endif

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#endif // GLX_INIT_H
