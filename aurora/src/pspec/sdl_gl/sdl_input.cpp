/*--- INCLUDES ---*/
#include <assert.h>
#include "sdl_init.h"
#include "sdl_input.h"
#include "../utils.h"
#include <iostream>
#include <SDL/SDL.h>

#include <tinyxml.h>

InputEngineOGL* InputEngineOGL::open() {
	return new InputEngineOGL();
}

InputEngineOGL::InputEngineOGL() :
	InputEngine()
{
	SDLInit::init();
	initKeyboard();

	assert(controllers.size() == 0);
	controllers.push_back(new ControllerState); // Only one player for now.

#ifdef JOYSTICK_SUPPORT
	joy = NULL;
	initJoystick();
#endif

#ifdef MOUSE_SUPPORT
	SDLInit::mouseCursor = NULL;
	// Setup mouse
	SDLInit::mouseCursor = NULL;
	releaseMouse();
	setMouseCursor(NULL);
#endif

	bzero(kbdButtonMap, sizeof(kbdButtonMap));
	bzero(kbdButtonAxisMap, sizeof(kbdButtonAxisMap));
	bzero(kbdButtonAxisValueMap, sizeof(kbdButtonAxisValueMap));

	setDefaultConfig();
}

InputEngineOGL::~InputEngineOGL() {
#ifdef MOUSE_SUPPORT
	// Release mouse pointer
	releaseMouse();
	setMouseCursor(NULL);
#endif

#ifdef JOYSTICK_SUPPORT
	if(joy)
		SDL_JoystickClose(joy);
#endif

	SDLInit::release();
}

// QC:A
void InputEngineOGL::initKeyboard() {
	SDL_InitSubSystem(SDL_INIT_JOYSTICK);
	SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);
	SDL_SetEventFilter(&SDLInit::filterEvents);

#ifdef MOUSE_SUPPORT
	SDLInit::pointerX = 0;
	SDLInit::pointerY = 0;
#endif

	kbdState = SDL_GetKeyState(NULL);
}

#ifdef JOYSTICK_SUPPORT
void InputEngineOGL::initJoystick() {
	if(SDL_NumJoysticks() > 0) {
		joy = SDL_JoystickOpen(0);
	}
}
#endif

void InputEngineOGL::setDefaultConfig() {
	// Map arrows
	kbdButtonAxisMap[ControllerState::AXIS_X][0] = SDLK_LEFT;
	kbdButtonAxisValueMap[ControllerState::AXIS_X][0] = AXIS_MIN;
	kbdButtonAxisMap[ControllerState::AXIS_X][1] = SDLK_RIGHT;
	kbdButtonAxisValueMap[ControllerState::AXIS_X][1] = AXIS_MAX;
	kbdButtonAxisMap[ControllerState::AXIS_Y][0] = SDLK_UP;
	kbdButtonAxisValueMap[ControllerState::AXIS_Y][0] = AXIS_MIN;
	kbdButtonAxisMap[ControllerState::AXIS_Y][1] = SDLK_DOWN;
	kbdButtonAxisValueMap[ControllerState::AXIS_Y][1] = AXIS_MAX;


	kbdButtonMap[ControllerState::BTN_Left] = SDLK_LEFT;
	kbdButtonMap[ControllerState::BTN_Right] = SDLK_RIGHT;
	kbdButtonMap[ControllerState::BTN_Down] = SDLK_DOWN;
	kbdButtonMap[ControllerState::BTN_Up] = SDLK_UP;

	kbdButtonMap[ControllerState::BTN_A] = SDLK_c;
	kbdButtonMap[ControllerState::BTN_B] = SDLK_LCTRL;
	kbdButtonMap[ControllerState::BTN_C] = SDLK_RCTRL;
	kbdButtonMap[ControllerState::BTN_Start] = SDLK_SPACE;
	kbdButtonMap[ControllerState::BTN_L] = SDLK_LSHIFT;
	kbdButtonMap[ControllerState::BTN_Select] = SDLK_RSHIFT;

#ifdef DEBUG_KEYS
	// Map debug keys
	kbdButtonMap[ControllerState::BTN_DBG_ReloadGraphics] = SDLK_F5;
	kbdButtonMap[ControllerState::BTN_DBG_StateStep] = SDLK_F8;
	kbdButtonMap[ControllerState::BTN_DBG_StatePlay] = SDLK_F7;
	kbdButtonMap[ControllerState::BTN_DBG_IncreaseStateStep] = SDLK_KP_PLUS;
	kbdButtonMap[ControllerState::BTN_DBG_DecreaseStateStep] = SDLK_KP_MINUS;
	kbdButtonMap[ControllerState::BTN_DBG_DebuggerStep] = SDLK_F10;
	kbdButtonMap[ControllerState::BTN_DBG_DebuggerAbort] = SDLK_F9;
	kbdButtonMap[ControllerState::BTN_DBG_ScreenShot] = SDLK_F12;
	kbdButtonMap[ControllerState::BTN_DBG_ResetState] = SDLK_F11;
	kbdButtonMap[ControllerState::BTN_DBG_DisplayCollisions] = SDLK_F4;
	kbdButtonMap[ControllerState::BTN_DBG_SwitchStateRendering] = SDLK_F6;
	kbdButtonMap[ControllerState::BTN_DBG_SwitchHardwareRendering] = SDLK_F3;
#endif /* DEBUG_KEYS */
}

// QC:A
void InputEngineOGL::processInput() {
	SDL_PumpEvents();
	kbdState = SDL_GetKeyState(NULL);

	xassert(controllers.size() >= 1, "Cannot find default controller");
	ControllerState* controller = controllers[0];

	// Read buttons status from keyboard
	for(int i=0; i<BUTTON_COUNT; i++) {
		if(kbdButtonMap[i]) {
			bool buttonState = kbdState[kbdButtonMap[i]];
#ifdef JOYSTICK_SUPPORT
			if(joy) {
				if(i == (int)ControllerState::BTN_Up)
					buttonState = buttonState || SDL_JoystickGetHat(joy, 0) == SDL_HAT_UP || SDL_JoystickGetAxis(joy, 1) < -ControllerState::AXIS_DEAD_ZONE;
				else if(i == (int)ControllerState::BTN_Left)
					buttonState = buttonState || SDL_JoystickGetHat(joy, 0) == SDL_HAT_LEFT || SDL_JoystickGetAxis(joy, 0) < -ControllerState::AXIS_DEAD_ZONE;
				else if(i == (int)ControllerState::BTN_Down)
					buttonState = buttonState || SDL_JoystickGetHat(joy, 0) == SDL_HAT_DOWN || SDL_JoystickGetAxis(joy, 1) > ControllerState::AXIS_DEAD_ZONE;
				else if(i == (int)ControllerState::BTN_Right)
					buttonState = buttonState || SDL_JoystickGetHat(joy, 0) == SDL_HAT_RIGHT || SDL_JoystickGetAxis(joy, 0) > ControllerState::AXIS_DEAD_ZONE;
				else if(i>=(int)ControllerState::BTN_A && i<=(int)ControllerState::BTN_C)
					buttonState = buttonState || SDL_JoystickGetButton(joy, (int)ControllerState::BTN_A - i);
			}
#endif
			controller->setButton((ControllerState::Button)i, buttonState);
		}
	}

	// Read mapped axes status from keyboard
	for(int i=0; i<AXIS_COUNT; i++) {
		if(kbdButtonAxisMap[i][0] && kbdButtonAxisMap[i][1]) {
			if(kbdState[(int)(kbdButtonAxisMap[i][0])]) {
				controller->setAxis((ControllerState::Axis)i, kbdButtonAxisValueMap[i][0]);
			} else if(kbdState[(int)(kbdButtonAxisMap[i][1])]) {
				controller->setAxis((ControllerState::Axis)i, kbdButtonAxisValueMap[i][1]);
			} else {
				controller->setAxis((ControllerState::Axis)i, AXIS_CENTER);
			}
		}
	}

#ifdef JOYSTICK_SUPPORT
	if(joy) {
		// TODO : Joystick axis/button mapping
		controller->addAxis(ControllerState::AXIS_X, SDL_JoystickGetAxis(joy, 0));
		controller->addAxis(ControllerState::AXIS_Y, SDL_JoystickGetAxis(joy, 1));

		int x = 0, y = 0;
		switch(SDL_JoystickGetHat(joy, 0)) {
			case SDL_HAT_LEFTUP:
				y = AXIS_MIN;
			case SDL_HAT_LEFT:
				x = AXIS_MIN;
				break;

			case SDL_HAT_RIGHTUP:
				y = AXIS_MIN;
			case SDL_HAT_RIGHT:
				x = AXIS_MAX;
				break;

			case SDL_HAT_LEFTDOWN:
				x = AXIS_MIN;
			case SDL_HAT_DOWN:
				y = AXIS_MAX;
				break;

			case SDL_HAT_RIGHTDOWN:
				x = AXIS_MAX;
				y = AXIS_MAX;
				break;

			case SDL_HAT_UP:
				y = AXIS_MIN;
				break;
		}
		controller->addAxis(ControllerState::AXIS_X, x);
		controller->addAxis(ControllerState::AXIS_Y, y);
	}
#endif

#ifdef MOUSE_SUPPORT
	int& mousex = SDLInit::pointerX;
	int& mousey = SDLInit::pointerY;

	Uint8 mouseButtons;

	switch(SDLInit::mouseMode) { // TODO : Use ControllerState::getMouseMode()
	case MOUSE_RELEASED:
		mouseButtons = SDL_GetMouseState(&mousex, &mousey);
		controller->setMousePosition(mousex, mousey);
		break;
	
	case MOUSE_ABSOLUTE:
		mouseButtons = SDL_GetRelativeMouseState(&mousex, &mousey);
		controller->setMousePosition(controller->getMouseX() + mousex, controller->getMouseY() + mousey);
		break;

	case MOUSE_RELATIVE:
		mouseButtons = SDL_GetRelativeMouseState(&mousex, &mousey);
		controller->setMousePosition(mousex, mousey);
		break;
	
	default:
		xassert(false, "Unknown mouse mode.");
		return;
	}

	for(int i=0; i<MOUSE_BUTTON_COUNT; i++) {
		controller->setMouseButton(i, mouseButtons & SDL_BUTTON(i+1));
	}
#endif
}

// QC:?
bool InputEngineOGL::enableTextInput(int controllerID) {
	xwarn(controllerID, "SDL does not handle multiple controllers for keyboard input.");
	SDLInit::textInEnabled = true;
	SDL_EnableKeyRepeat(KEY_REPEAT_DELAY, KEY_REPEAT_INTERVAL);
	SDL_EnableUNICODE(true);
	return true;
}

// QC:?
bool InputEngineOGL::disableTextInput(int controllerID) {
	xwarn(controllerID, "SDL does not handle multiple controllers for keyboard input.");
	SDLInit::textInEnabled = false;
	SDL_EnableKeyRepeat(0, 0);
	SDL_EnableUNICODE(false);
	return true;
}

// QC:?
size_t InputEngineOGL::readTextInput(char* buffer, size_t bufSize, int controllerID) {
	size_t outLen = 0;
	unsigned long nextChar = SDLInit::popChar();
	unsigned int nextCharSize = getUTF8CharLen(nextChar);
	while(nextChar && nextCharSize <= bufSize) {
		codepointToUTF8(buffer, nextChar);
		buffer += nextCharSize;
		outLen += nextCharSize;
		bufSize -= nextCharSize;
		nextChar = SDLInit::popChar();
		nextCharSize = getUTF8CharLen(nextChar);
	}

	return outLen;
}

// QC:W
size_t InputEngineOGL::peekTextInput(char* buffer, size_t bufSize, int controllerID) {
	/* TODO */
	return 0;
}

// QC:W
size_t InputEngineOGL::feedTextInput(char* buffer, size_t bufSize, int controllerID) {
	/* TODO */
	return 0;
}

#ifdef MOUSE_SUPPORT
bool InputEngineOGL::relativeMouse(int controllerID) {
	controllers[controllerID]->setMouseMode(MOUSE_RELATIVE);
	SDLInit::mouseMode = MOUSE_RELATIVE;
	SDL_ShowCursor(false);
#ifdef MOUSE_GRAB
	SDL_WM_GrabInput(SDL_GRAB_ON);
#else
	SDL_WM_GrabInput(SDL_GRAB_OFF);
#endif
	syslog("Grabbed relative mouse cursor");

	int mousex, mousey;
	SDL_GetRelativeMouseState(&mousex, &mousey);
	return true;
}

// QC:G
bool InputEngineOGL::absoluteMouse(int controllerID) {
	controllers[controllerID]->setMouseMode(MOUSE_ABSOLUTE);
	SDLInit::mouseMode = MOUSE_ABSOLUTE;
	SDL_ShowCursor(false);
#ifdef MOUSE_GRAB
	SDL_WM_GrabInput(SDL_GRAB_ON);
#else
	SDL_WM_GrabInput(SDL_GRAB_OFF);
#endif
	syslog("Grabbed absolute mouse cursor");

	return true;
}

// QC:G
bool InputEngineOGL::releaseMouse(int controllerID) {
	controllers[controllerID]->setMouseMode(MOUSE_RELEASED);
	SDLInit::mouseMode = MOUSE_RELEASED;
	SDL_ShowCursor(false);
	SDL_WM_GrabInput(SDL_GRAB_OFF);
	syslog("Released mouse cursor");
	return true;
}

// QC:?
void InputEngineOGL::setMouseCursor(GameObject* newCursor, int controllerID) {
	SDLInit::mouseCursor = newCursor;
}

GameObject* InputEngineOGL::getMouseCursor(int controllerID) {
	if(SDLInit::mouseCursor)
		return SDLInit::mouseCursor;
	return NULL;
}

#endif

// QC:G
bool InputEngineOGL::terminationRequest() {
	return SDLInit::terminationRequest();
}

// QC:A
std::list<std::string> InputEngineOGL::getInputHardwareDeviceList() {
	std::list<std::string> list;

	list.push_back("SDL_KBD"); // Standard keyboard.
#ifdef MOUSE_SUPPORT
	list.push_back("SDL_PTR"); // Standard mouse pointer
#endif /* MOUSE_SUPPORT */

	// TODO : Add joystick in the list

	return list;
}

// QC:W
std::list<std::string> InputEngineOGL::getButtonList(const char* hardwareDeviceName) {
	std::list<std::string> blist;

	if(strcmp(hardwareDeviceName, "SDL_KBD") == 0) {
		blist.push_back("UP");
		blist.push_back("DOWN");
		blist.push_back("LEFT");
		blist.push_back("RIGHT");
		blist.push_back("CTRL");
		blist.push_back("ALT");
		blist.push_back("SHIFT");
		blist.push_back("ESC");
		blist.push_back("SPACE");
		blist.push_back("F1");
		blist.push_back("A");
		blist.push_back("B");
		blist.push_back("C");
		// TODO : complete key list ...
	} else if(strcmp(hardwareDeviceName, "SDL_PTR") == 0) {
		blist.push_back("BTN0");
		blist.push_back("BTN1");
		blist.push_back("BTN2");
		blist.push_back("BTN3");
		blist.push_back("BTN4");
		blist.push_back("BTN5");
		blist.push_back("BTN6");
		blist.push_back("BTN7");
		blist.push_back("WHEEL_UP");
		blist.push_back("WHEEL_DOWN");
		blist.push_back("WHEEL_LEFT");
		blist.push_back("WHEEL_RIGHT");
	}

	return blist;
}

std::list<std::string> InputEngineOGL::getAxesList(const char* hardwareDeviceName) {
#ifdef MOUSE_SUPPORT
	if(strcmp(hardwareDeviceName, "SDL_PTR") == 0) {
		std::list<std::string> alist;
		alist.push_back("X");
		alist.push_back("Y");
		return alist;
	}
#endif /* MOUSE_SUPPORT */
	return std::list<std::string>();
}

std::list<std::string> InputEngineOGL::getFeedbackAxesList(const char* hardwareDeviceName) {
	return std::list<std::string>();
}

