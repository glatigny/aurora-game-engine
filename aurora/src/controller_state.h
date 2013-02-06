#ifndef CONTROLLER_STATE_H
#define CONTROLLER_STATE_H

#include <set>

class ControllerState {
public:

	static const short AXIS_DEAD_ZONE = 4095;

	typedef enum Direction {
		NO_DIR,
		DOWN_LEFT,
		DOWN,
		DOWN_RIGHT,
		LEFT,
		NO_MOVE,
		RIGHT,
		UP_LEFT,
		UP,
		UP_RIGHT
	} Direction;

	typedef enum Button {
		// Standard joystick-like input
		BTN_Up,
		BTN_Down,
		BTN_Left,
		BTN_Right,
		BTN_A,
		BTN_B,
		BTN_C,
		BTN_X, // 1 on the Wiimote
		BTN_Y, // 2 on the Wiimote
		BTN_Z,
		BTN_L,
		BTN_R,
		BTN_L2,
		BTN_R2,
		BTN_L3,
		BTN_R3,

		// Navigation
		BTN_Minus, // Back on the XBOX 360
		BTN_Plus,
		BTN_Home,
		BTN_Start,
		BTN_Select,

		// Debug input
		BTN_StateStep, // Pauses/steps the state
		BTN_StatePlay, // Restores normal operation
		BTN_IncreaseStateStep,
		BTN_DecreaseStateStep,

		// Keyboard modifiers
		BTN_Logo, // "Windows" key
		BTN_Shift,
		BTN_Ctrl,
		BTN_Alt,

#ifdef DEBUG_KEYS
		// Debug buttons
		BTN_DBG_ReloadGraphics,
		BTN_DBG_ResetState,
		BTN_DBG_StateStep,
		BTN_DBG_StatePlay,
		BTN_DBG_IncreaseStateStep,
		BTN_DBG_DecreaseStateStep,
		BTN_DBG_DebuggerStep,
		BTN_DBG_DebuggerAbort,
		BTN_DBG_ScreenShot,
		BTN_DBG_DisplayCollisions,
		BTN_DBG_SwitchStateRendering,
		BTN_DBG_SwitchHardwareRendering,
#endif
	} Button;
#ifdef DEBUG_KEYS
#define BUTTON_COUNT 41
#else
#define BUTTON_COUNT 29
#endif

	typedef enum Axis {
		// Main analog stick
		AXIS_X,
		AXIS_Y,
		AXIS_Z, // Joystick twist

		// Secondary analog stick or Nunchuk stick
		AXIS_X2,
		AXIS_Y2,
		AXIS_Z2,

		// Other input systems
		AXIS_Gearbox, // For manual gearbox on hi-end wheels : 0 = neutral, 1 = 1st gear, ...

		// Analog triggers
		AXIS_L,
		AXIS_R,

		// Accelerometers
		AXIS_ACCEL_X,
		AXIS_ACCEL_Y,
		AXIS_ACCEL_Z,

		AXIS_ACCEL2_X,
		AXIS_ACCEL2_Y,
		AXIS_ACCEL2_Z,
	} Axis;
#define AXIS_COUNT 15
#define AXIS_MAX 32767
#define AXIS_MIN -32767
#define AXIS_CENTER 0

	typedef enum FeedbackAxis {
		FFB_MainX,
		FFB_MainY,
		FFB_MainZ, // Rumble on normal controllers
		FFB_SecondaryX,
		FFB_SecondaryY,
		FFB_SecondaryZ,

		FFB_ViewportR, // May be used for motorized displays or arcade cabinets
		FFB_ViewportP,
		FFB_ViewportY,

		FFB_Speed,
	// To support speed-related effects, like wind in Outrun
	} FeedbackAxis;
#define FEEDBACK_AXIS_COUNT 10

#ifdef MOUSE_SUPPORT

#define MOUSE_RELEASED 0
#define MOUSE_ABSOLUTE 1
#define MOUSE_RELATIVE 2

#ifndef MOUSE_MODE_COUNT
#define MOUSE_MODE_COUNT 3
#endif

#ifndef MOUSE_BUTTON_COUNT
#define MOUSE_BUTTON_COUNT 12 // Maximum number of buttons on the mouse
#endif

#endif /* MOUSE_SUPPORT */

private:
	char buttonDown[BUTTON_COUNT];
	char buttonPressed[BUTTON_COUNT];
	char buttonReleased[BUTTON_COUNT];
	short axis[AXIS_COUNT];
	short ffb[FEEDBACK_AXIS_COUNT];

#ifdef MOUSE_SUPPORT
	short mouseX;
	short mouseY;
	short mouseAngle; // Rotation angle
	short mousePressure;
	short mouseWidth;

	char mouseDown[MOUSE_BUTTON_COUNT];
	char mousePressed[MOUSE_BUTTON_COUNT];
	char mouseReleased[MOUSE_BUTTON_COUNT];
	char mouseMode; /* MOUSE_RELEASED, RELATIVE or ABSOLUTE */
#endif

#ifdef CUSTOM_KEY_EVENTS
	class CustomKeyEvent {
		private:
			uint32_t name;

		public:
			CustomKeyEvent(const char* newName) : name(0) {
				xassert(strlen(newName) <= 4, "Invalid custom key event name : %d is too long.", (int)strlen(newName));
				strncpy((char*)&name, newName, 4);
			}

			bool operator==(const CustomKeyEvent& other) const {
				return name == other.name;
			}

			bool operator!=(const char* other) const {
				return *this != other;
			}
			
			bool operator<(const CustomKeyEvent other) const {
				return name < other.name;
			}
	};
	std::set<CustomKeyEvent> customKeyEvents;
#endif

public:
	ControllerState();
	~ControllerState();

	/** Reset the internal state of the object.
	 *
	 */
	void reset();

	/** Set the status of a button.
	 */
	void setButton(Button b, bool newState);

	/** Set all keys at once.
	 *
	 */
	void setButtons(char keys[BUTTON_COUNT]);
	void setAxes(int x, int y);
	void getAxes(int& x, int& y);
	int getAxis(Axis a) {
		return axis[a];
	}
	void setAxis(Axis a, int value) {
		axis[a] = value;
	}
	void addAxis(Axis a, int value) {
		if(value + axis[a] > 32767) {
			axis[a] = 32767;
		} else if(value + axis[a] < -32768) {
			axis[a] = -32768;
		} else {
			axis[a] += value;
		}
	}

	int getFeedbackAxis(FeedbackAxis a) {
		return ffb[a];
	}
	void setFeedbackAxis(FeedbackAxis a, int value) {
		ffb[a] = value;
	}

	inline bool isButtonDown(Button b) {
		return (bool) buttonDown[b];
	}
	inline bool isButtonPressed(Button b) {
		return (bool) buttonPressed[b];
	}
	inline bool isButtonReleased(Button b) {
		return (bool) buttonPressed[b];
	}

	/** Returns the next event in te event queue.
	 * @return the next event of the queue, or NULL if the queue is empty.
	 */
	inline const char* popEvent();

	/** Push an event on the queue.
	 * Events are mapped to keys, and are useful for games with many keys, like simulators.
	 * Events mark pressing one key : it is not a button, with down/pressed/released events, just a pressed event.
	 * @param name the name of the event to push.
	 */
	inline void pushEvent(const char* name);

#ifdef CUSTOM_KEY_EVENTS
	inline bool isCustomEventSet(const char* eventName) {
		return customKeyEvents.count(CustomKeyEvent(eventName)) > 0;
	}

	inline void setCustomEvent(const char* eventName) {
		customKeyEvents.insert(CustomKeyEvent(eventName));
	}

	inline void clearCustomEvents() {
		customKeyEvents.clear();
	}
#endif

#ifdef MOUSE_SUPPORT
	inline int getMouseMode() {
		return mouseMode;
	}
	inline int getMouseX() {
		return mouseX;
	}
	inline int getMouseY() {
		return mouseY;
	}
	inline int getMouseAngle() {
		return mouseAngle;
	}
	inline int getMousePressure() {
		return mousePressure;
	}
	inline int getMouseWidth() {
		return mouseWidth;
	}
	inline bool isMouseButtonDown(int button) {
		return mouseDown[button];
	}
	inline bool isMouseButtonPressed(int button) {
		return mousePressed[button];
	}
	inline bool isMouseButtonReleased(int button) {
		return mouseReleased[button];
	}
	void setMouseButton(int button, bool state);
	void setMousePosition(int x, int y);
	void setMousePosition(int x, int y, int angle, int pressure, int width);
#endif

	Direction getDir();
};

#endif /* CONTROLLER_STATE_H */
