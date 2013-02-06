#ifndef SDL_INPUT_H
#define SDL_INPUT_H

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

#define KEY_REPEAT_DELAY 400
#define KEY_REPEAT_INTERVAL 32


class InputEngineOGL: public InputEngine {
private:
	InputEngineOGL();

	Uint8* kbdState;
	int kbdButtonMap[BUTTON_COUNT];
	int kbdButtonAxisMap[AXIS_COUNT][2]; // XXX : Should be [AXIS_COUNT][BUTTON_COUNT] but that would take too much space
	int kbdButtonAxisValueMap[AXIS_COUNT][2]; // XXX : Should be [AXIS_COUNT][BUTTON_COUNT] but that would take too much space

	void initKeyboard();
#ifdef JOYSTICK_SUPPORT
	SDL_Joystick* joy;
	void initJoystick();
#endif

public:
	static InputEngineOGL* open();
	~InputEngineOGL();

	bool terminationRequest();


	virtual std::list<std::string> getInputHardwareDeviceList();
	virtual std::list<std::string> getButtonList(const char* hardwareDeviceName);
	virtual std::list<std::string> getAxesList(const char* hardwareDeviceName);
	virtual std::list<std::string> getFeedbackAxesList(const char* hardwareDeviceName);

	virtual const char* getActiveButton(const char*) { return ""; }
	virtual const char* getActiveAxis(const char*) { return ""; }

	virtual void processInput();

#ifdef XML_SUPPORT
	virtual ControllerConfig* myLoadControllerConfig(TiXmlElement*) { return NULL; }
	virtual int addControllers(TiXmlElement*) { return 0; }
	virtual bool saveControllerConfig(ControllerConfig*, TiXmlElement*) { return false; }
#endif /* XML_SUPPORT */

	virtual ControllerConfig* myLoadControllerConfig(VFS::File*) { return NULL; }
	virtual int addControllers(VFS::File*) { return 0; }
	virtual bool saveControllerConfig(ControllerConfig*, VFS::File*) { return false; }


	/* HardwareEngine */
	virtual void setDefaultConfig();
	virtual bool enableTextInput(int controllerID = 0);
	virtual bool disableTextInput(int controllerID = 0);
	virtual size_t readTextInput(char* buffer, size_t bufSize, int controllerID = 0);
	virtual size_t peekTextInput(char* buffer, size_t bufSize, int controllerID = 0);
	virtual size_t feedTextInput(char* buffer, size_t bufSize, int controllerID = 0);
	virtual void clearInputBuffer(int controllerID = 0) { SDLInit::textInNext = SDLInit::textInStart; }

#ifdef MOUSE_SUPPORT
	virtual bool relativeMouse(int controllerID = 0);
	virtual bool absoluteMouse(int controllerID = 0);
	virtual bool releaseMouse(int controllerID = 0);
	virtual void setMouseCursor(GameObject* newCursor, int controllerID = 0);
	virtual GameObject* getMouseCursor(int controllerID = 0);
#endif
};

#endif /* SDL_INPUT_H */
