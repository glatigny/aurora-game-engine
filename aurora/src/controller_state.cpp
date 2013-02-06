#include "controller_state.h"

// QC:G
ControllerState::ControllerState()
{
	reset();
}

// QC:?
ControllerState::~ControllerState()
{
}

// QC:A (quick and dirty, but works OK)
void ControllerState::reset() {
	// Faster than hand-setting values !
#ifdef CUSTOM_KEY_EVENTS
	bzero(this, (char*)(&customKeyEvents) - (char*)this);
#else
	bzero(this, sizeof(ControllerState));
#endif
}

void ControllerState::setButton(Button b, bool newState) {
	bool prevState = buttonDown[b];
	buttonDown[b] = newState;
	buttonPressed[b] = !prevState && newState;
	buttonReleased[b] = prevState && !newState;
}

#ifdef MOUSE_SUPPORT

void ControllerState::setMouseButton(int button, bool newState) {
	xassert(button >= 0 && button < MOUSE_BUTTON_COUNT, "Mouse button out of range");
	bool prevState = mouseDown[button];
	mouseDown[button] = newState;
	mousePressed[button] = !prevState && newState;
	mouseReleased[button] = prevState && !newState;
}

void ControllerState::setMousePosition(int x, int y) {
	mouseX = x;
	mouseY = y;
}

void ControllerState::setMousePosition(int x, int y, int angle, int pressure, int width) {
	mouseX = x;
	mouseY = y;
	mouseAngle = angle;
	mousePressure = pressure;
	mouseWidth = width;
}

void ControllerState::setMouseMode(int mode) {
	xassert(mode >= MOUSE_RELEASED && mode < MOUSE_MODE_COUNT, "Mouse mode out of range");
	mouseMode = mode;
}
#endif /* MOUSE_SUPPORT */

ControllerState::Direction ControllerState::getDir() {
	if( isButtonDown(BTN_Left) || getAxis(AXIS_X) < -AXIS_DEAD_ZONE )
	{
		if( isButtonDown(BTN_Up) || getAxis(AXIS_Y) < -AXIS_DEAD_ZONE )
			return UP_LEFT;
		if( isButtonDown(BTN_Down) || getAxis(AXIS_Y) > AXIS_DEAD_ZONE )
			return DOWN_LEFT;
		return LEFT;
	}
	else if( isButtonDown(BTN_Right) || getAxis(AXIS_X) > AXIS_DEAD_ZONE )
	{
		if( isButtonDown(BTN_Up) || getAxis(AXIS_Y) < -AXIS_DEAD_ZONE )
			return UP_RIGHT;
		if( isButtonDown(BTN_Down) || getAxis(AXIS_Y) > AXIS_DEAD_ZONE )
			return DOWN_RIGHT;
		return RIGHT;
	}
	else if( isButtonDown(BTN_Down) || getAxis(AXIS_Y) > AXIS_DEAD_ZONE )
		return DOWN;
	else if( isButtonDown(BTN_Up) || getAxis(AXIS_Y) < -AXIS_DEAD_ZONE )
		return UP;
	return NO_DIR;
}

/*-EOF-*/
