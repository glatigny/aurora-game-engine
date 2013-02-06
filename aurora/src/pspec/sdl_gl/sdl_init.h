#ifndef SDL_INIT_H
#define SDL_INIT_H

#include "../../hardware_engine.h"

#ifdef WIN32
# include <windows.h>
# include <SDL/SDL_syswm.h>
#endif
#ifdef __APPLE__
#include <SDL.h>
#else
#include <SDL/SDL.h>
#endif

#define TEXT_INPUT_BUFFER_SIZE 64

class SDLInit { friend class InputEngineOGL; friend class GraphicalEngineOGL;
private:
	static int useCount;
	static bool windowClosed;
	static unsigned long textIn[TEXT_INPUT_BUFFER_SIZE]; // Internally, the buffer stores text as UTF-32
	static unsigned int textInStart;
	static unsigned int textInNext;

#ifdef MOUSE_SUPPORT
public:
	static GameObject* mouseCursor;
	static int pointerX;
	static int pointerY;
	static int mouseMode;
#endif

public:

	static void init();
	static void release();
	static bool terminationRequest();
	static bool textInEnabled;

	SDLInit() {
	}
	~SDLInit() {
	}

	/** Specific hardware Engine event reader.
	 * Using in order to know if someone wants to close the window.
	 */
	static int filterEvents(const SDL_Event *event);

	/** Push one character at the end of the text input buffer.
	 * @param character the character codepoint to push.
	 * @return true if it was added, false otherwise.
	 */
	static bool pushChar(unsigned long character);

	/** Pop one character in front of the text input buffer.
	 * @return the popped character, or 0 if the buffer is empty.
	 */
	static unsigned long popChar();

};

#endif /* SDL_INIT_H */
