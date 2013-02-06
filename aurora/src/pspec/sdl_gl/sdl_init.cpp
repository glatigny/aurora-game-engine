#include <assert.h>
#include "sdl_init.h"
#include "../utils.h"
#include <iostream>
#include <SDL/SDL.h>

#include <tinyxml.h>

int SDLInit::useCount = 0;
bool SDLInit::windowClosed = false;
bool SDLInit::textInEnabled = false;
unsigned long SDLInit::textIn[TEXT_INPUT_BUFFER_SIZE]; // Internally, the buffer stores text as UTF-32
unsigned int SDLInit::textInStart = 0;
unsigned int SDLInit::textInNext = 0;
#ifdef MOUSE_SUPPORT
GameObject* SDLInit::mouseCursor;
int SDLInit::pointerX;
int SDLInit::pointerY;
int SDLInit::mouseMode;
#endif

// QC:G
void SDLInit::init() {
	textInEnabled = false;
	textInStart = 0;
	textInNext = 0;

	if(useCount == 0) {
		syslog("Initializing SDL subsystem.");
		SDL_Init(0);
	}
	useCount++;
}

// QC:G
void SDLInit::release() {
	useCount--;
	if(useCount == 0) {
		syslog("Stopping SDL subsystem.");
		SDL_Quit();
	}
}

// QC:G
bool SDLInit::terminationRequest() {
	return windowClosed;
}

// QC:G
int SDLInit::filterEvents(const SDL_Event *event) {

	if((event->type == SDL_QUIT) || (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_ESCAPE)) {
		windowClosed = true;
		return 0;
	}

	if(textInEnabled && event->type == SDL_KEYDOWN) {
		if(event->key.keysym.sym != 0 || (event->key.keysym.sym >= 32 && event->key.keysym.sym <= 127)) {
			pushChar(event->key.keysym.unicode);
		}

		return 0;
	}

	return 1;
}


// QC:?
bool SDLInit::pushChar(unsigned long character) {
	unsigned int nextPos = (textInNext + 1) % TEXT_INPUT_BUFFER_SIZE;
	if(nextPos == textInStart) {
		// Buffer overflow
		return false;
	}

	textIn[textInNext] = character;
	textInNext = nextPos;
	return true;
}

// QC:?
unsigned long SDLInit::popChar() {
	if(textInStart == textInNext) {
		return 0;
	}

	unsigned long chr = textIn[textInStart];
	textInStart++;
	textInStart %= TEXT_INPUT_BUFFER_SIZE;
	return chr;
}

