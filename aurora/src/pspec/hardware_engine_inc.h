/*--------- Plateform-specific graphic header ----------*/

#include "default_time_engine.h"

#ifdef DIRECTX_SUPPORT
#include "directx/directx.h"
#endif

#ifdef OPENGL_SUPPORT
#include "open_gl/open_gl.h"
#endif

#ifdef SDL_SUPPORT
#include "sdl_gl/sdl_init.h"
#include "sdl_gl/sdl_gl.h"
#include "sdl_gl/sdl_input.h"
#endif

#ifdef OPENAL_SUPPORT
#include "open_al/open_al.h"
#endif

#ifdef NET_SUPPORT
#ifdef TCP_SUPPORT
#include "tcp_net.h"
#endif
#endif
