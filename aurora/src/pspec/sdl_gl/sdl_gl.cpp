#include <assert.h>
#include "sdl_gl.h"
#include "pspec/utils.h"
#include <iostream>
#include <SDL/SDL.h>
#include "sdl_init.h"

#include <tinyxml.h>

GraphicalEngineSDL* GraphicalEngineSDL::open(int width, int height, int refresh, bool fullscreen, VFS::File* processingEffects) {
	return new GraphicalEngineSDL(width, height, refresh, fullscreen, processingEffects);
}

GraphicalEngineSDL::GraphicalEngineSDL(int width, int height, int refresh, bool fullscreen, VFS::File* processingEffects) :
	GraphicalEngineOGL(width, height, refresh, fullscreen, processingEffects)
{
	initGraphics(width, height, refresh, fullscreen);
}

GraphicalEngineSDL::~GraphicalEngineSDL() {
	SDLInit::release();
}

// QC:G
void GraphicalEngineSDL::initGraphics(int width, int height, int refresh, bool fullscreen) {
	const SDL_VideoInfo* info;

	SDLInit::init();
	SDL_InitSubSystem(SDL_INIT_VIDEO);
	SDL_WM_SetCaption(AURORA_WINDOW_TITLE, NULL);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_SetEventFilter(&SDLInit::filterEvents);

	if(!width || !height) {
		info = SDL_GetVideoInfo();
		if(fullscreen) {
			width = info->current_w;
			height = info->current_h;
		} else {
			width = info->current_w * 3/4;
			height = info->current_h * 3/4;
		}
	}

	//SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);

	if(fullscreen)
		SDL_SetVideoMode(width, height, 32, SDL_OPENGL | SDL_FULLSCREEN );
	else
		SDL_SetVideoMode(width, height, 32, SDL_OPENGL);

	GraphicalEngineOGL::initGraphics(width, height, refresh, fullscreen);
	
	info = SDL_GetVideoInfo();
	displayOffsetX = (info->current_w - displayWidth) / 2;
	displayOffsetY = (info->current_h - displayHeight) / 2;

	rsd.px = displayOffsetX;
	rsd.py = displayOffsetY;
	rsd.pw = displayWidth;
	rsd.ph = displayHeight;

	rsd.vx = 0;
	rsd.vy = 0;
	rsd.vw = displayWidth;
	rsd.vh = displayHeight;

	xadvice(!displayOffsetX && !displayOffsetY, "Letterboxing with offset %d:%d. Display size : %d:%d", displayOffsetX, displayOffsetY, info->current_w, info->current_h);

	SDL_ShowCursor(SDL_DISABLE);

#ifdef WIN32
	SDL_SysWMinfo i;
	SDL_VERSION( &i.version );
	if ( SDL_GetWMInfo ( &i) ) {
		HWND hwnd = i.window;
#ifdef LOG_WINDOW
		RECT rc;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
		SetWindowPos( hwnd, HWND_TOP, rc.left, rc.top, 0, 0, SWP_NOSIZE );
#endif // LOG_WINDOW
	}
#endif
	
	SDL_EventState(SDL_QUIT, SDL_ENABLE);

	init = true;
}

// QC:G
void GraphicalEngineSDL::postDraw() {
	assert(rsdList.empty());
	xassert(init, "Engine not initialized.");

	GraphicalEngineOGL::postDraw();

#ifdef MOUSE_SUPPORT
	Point pointerPos(SDLInit::pointerX, SDLInit::pointerY);
	if((SDLInit::mouseMode == MOUSE_ABSOLUTE || SDLInit::mouseMode == MOUSE_RELEASED) && SDLInit::mouseCursor) {
		SDLInit::mouseCursor->setPosition(&pointerPos);
		SDLInit::mouseCursor->render();
	}
#endif

	SDL_GL_SwapBuffers();
}

// QC:G
bool GraphicalEngineSDL::terminationRequest() {
	return SDLInit::terminationRequest() || GraphicalEngineOGL::terminationRequest();
}

