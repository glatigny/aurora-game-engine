#ifndef SDL_GL_H
#define SDL_GL_H

#include "pspec/open_gl/open_gl.h"

#ifdef WIN32
#include <windows.h>
#include <SDL/SDL_syswm.h>
#endif
#include <SDL/SDL.h>


class GraphicalEngineSDL: public GraphicalEngineOGL {
	friend class InputEngineOGL;
	friend void run();
private:
	GraphicalEngineSDL(int width, int height, int refresh, bool fullscreen, VFS::File* processingEffects);
	bool init;

public:
	~GraphicalEngineSDL();

	static GraphicalEngineSDL* open(int width, int height, int refresh, bool fullscreen, VFS::File* processingEffects);

	/** Specific hardware Engine initialization of Graphic Engine.
	 */
	void initGraphics(int width, int height, int refresh, bool fullscreen);

	/* GraphicalEngine */

	void postDraw();

	bool terminationRequest();

	dur getNativeFrameRate() {
		return 60; /* TODO : Get actual refresh rate and VSync */
	}

	Resizable* getDisplaySize() {
		return &displaySize;
	}

};

#endif /* SDL_GL_H */
