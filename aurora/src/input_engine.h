#ifndef INPUT_ENGINE_H
#define INPUT_ENGINE_H

#include "controller_state.h"
#include "game_object.h"
#include "vfs.h"

class HardwareEngine;

class ControllerConfig {
public:
	virtual ~ControllerConfig() {
	}

	/** Returns the physical name of a button.
	 * @param the logical button to query.
	 * @return the display name of the physical button associated to the given logical button.
	 */
	virtual const char* getButtonPhysicalName(ControllerState::Button b) = 0;

	/** Returns the physical name of an axis.
	 * @param the logical axis to query.
	 * @return the display name of the physical axis associated to the given logical axis.
	 */
	virtual const char* getAxisPhysicalName(ControllerState::Axis a) = 0;

	/** Returns the physical name of a feedback axis.
	 * @param the logical feedback axis to query.
	 * @return the display name of the physical feedback axis associated to the given logical feedback axis.
	 */
	virtual const char* getFeedbackAxisPhysicalName(ControllerState::FeedbackAxis a) = 0;

	/** Returns the physical name of a mouse.
	 * @return the display name of the mouse associated to this config.
	 */
	virtual const char* getMousePhysicalName() = 0;

	/** Maps a logical button to a physical button.
	 * @param b the button to map.
	 * @param hardwareDeviceName the physical piece of hardware which has the physical button to map.
	 * @param physicalButtonName the name of the button on the hardware device.
	 */
	virtual void mapButton(ControllerState::Button b, const char* hardwareDeviceName, const char* physicalButtonName) = 0;

	virtual void mapAxis(ControllerState::Axis a, const char* hardwareDeviceName, const char* physicalAxisName) = 0;

	/** Maps a physical button to an axis value.
	 * This feature allows to control analog axes with digital buttons.
	 * @param a the axis to map.
	 * @param hardwareDeviceName the physical piece of hardware which has the physical button to map.
	 * @param physicalButtonName the name of the button on the hardware device.
	 * @param axisValue the axis will be set to this value when the physical button is pressed. If multiple buttons are pressed, the implementation MUST return one steady value while pressure is kept.
	 */
	virtual void mapButtonToAxis(ControllerState::Axis a, const char* hardwareDeviceName, const char* physicalAxisName, int axisValue) = 0;

	virtual void mapFeedbackAxis(ControllerState::FeedbackAxis a, const char* hardwareDeviceName, const char* physicalFeedbackAxisName) = 0;

};

class InputEngineInterface {
public:
	virtual void init(HardwareEngine* hw) {}
	virtual ~InputEngineInterface() {
	}

	/** Processes input controllers.
	 * This function reads the actual state of hardware controllers.
	 */
	virtual void processInput() = 0;

	virtual ControllerState* getControllerState(int controllerID = 0) = 0;
	virtual bool terminationRequest() = 0;


	virtual bool enableTextInput(int controllerID = 0) = 0;
	virtual bool disableTextInput(int controllerID = 0) = 0;
	virtual size_t readTextInput(char* buffer, size_t bufSize, int controllerID = 0) = 0;
	virtual size_t peekTextInput(char* buffer, size_t bufSize, int controllerID = 0) = 0;
	virtual size_t feedTextInput(char* buffer, size_t bufSize, int controllerID = 0) = 0;
	virtual void clearInputBuffer(int controllerID = 0) = 0;

#ifdef MOUSE_SUPPORT
	virtual bool absoluteMouse(int controllerID = 0) = 0;
	virtual bool relativeMouse(int controllerID = 0) = 0;
	virtual bool releaseMouse(int controllerID = 0) = 0;
	virtual void setMouseCursor(GameObject* newCursor, int controllerID = 0) = 0;
	virtual GameObject* getMouseCursor(int controllerID = 0) = 0;
#endif

	// Input configuration

	/** Sets the default configuration for this engine.
	 * The default configuration maps devices, buttons, axes and force feedback
	 * to sensible defaults. It may use hardware probing or some kind of
	 * autodetection to suit the current layout.
	 */
	void setDefaultConfig() {}

	virtual std::list<std::string> getInputHardwareDeviceList() = 0;
	virtual std::list<std::string> getButtonList(const char* hardwareDeviceName) = 0;
	virtual std::list<std::string> getAxesList(const char* hardwareDeviceName) = 0;
	virtual std::list<std::string> getFeedbackAxesList(const char* hardwareDeviceName) = 0;

	/** Returns the currently pressed buttons.
	 * This function is intented to be used for the configuration phase.
	 * If more than one button is pressed, this function should return the
	 * most probable one. The function can return already mapped buttons. This
	 * is up to the high level engine to resolve the conflict.
	 */
	virtual const char* getActiveButton(const char* hardwareDeviceName) = 0;

	/** Returns the currently active axis.
	 * This function is intented to be used for the configuration phase.
	 * If more than one axis is active, this function should return the
	 * most probable one. The function can return already mapped axes. This
	 * is up to the high level engine to resolve the conflict.
	 *
	 * The low level layer should filter out axes with a value too near from
	 * its center point. Also, joysticks pushed in diagonals should not be
	 * returned, since it is obviously not a natural move when configuring
	 * a joypad.
	 */
	virtual const char* getActiveAxis(const char* hardwareDeviceName) = 0;
};

/** InputEngine defines the way users interoperate with the game.
 * Each user is assigned to a controller. Each controller has a unique ID.
 * For example, player 1 has controllerID 0, player 2 has ID 1, ...
 * A controller is NOT the controlling device (keyboard, joypad). Rather, it is
 * the sum of all input device the user may use to control the game. For example,
 * if the user has a steering wheel, a keyboard and a mouse, there still will be
 * only one controller bound to individual axes and buttons of these input devices.
 *
 * Controllers can be set up with configuration files, internally represented by
 * the ControllerConfig abstract class.
 */
class InputEngine: public InputEngineInterface {
protected:
	std::vector<ControllerState*> controllers;

#ifdef XML_SUPPORT
	virtual ControllerConfig* myLoadControllerConfig(TiXmlElement* configFile) = 0;
	virtual ControllerConfig* myLoadControllerConfig(VFS::File* configFile);
#endif /* XML_SUPPORT */

public:
	InputEngine();
	virtual ~InputEngine();

	/** Tells whether a request to close the engine has been emitted.
	 * This function is called every frame. The engine will exit its main loop forcibly
	 * whenever this function returns true.
	 * It is generally used to implement quick exit key combinations.
	 * The engine will be shut down cleanly, freeing any allocated resource.
	 * @return true if the engine is required to shut down, false otherwise.
	 */
	virtual bool terminationRequest() {
		return false;
	}



	/** Add controllers stored in a config file.
	 * A controller corresponds to the interfaces linked to one human person.
	 * Adding a controller is like adding a player. To add peripherals used by the same player, see
	 * controller config.
	 * @param configFile the file to read configuration from.
	 * @return the number of added controllers.
	 */
#ifdef XML_SUPPORT
	virtual int addControllers(TiXmlElement* configFile) = 0;
	virtual int addControllers(VFS::File* configFile);
#endif /* XML_SUPPORT */

	/** Add a controller.
	 * A controller corresponds to the interfaces linked to one human person.
	 * Adding a controller is like adding a player. To add peripherals used by the same player, see
	 * controller config.
	 * @param config the configuration to use. It must be of a valid type.
	 * @return the ID of the controller.
	 */
	virtual int addController(ControllerConfig* config) { return 0; } // TODO

	/** Remove a controller.
	 * FIXME : Not sure about reordering. Maybe letting NULL controllers would be better.
	 * This function removes a controller. Beware that other controllers will be reordered. It means that
	 * if you remove controller 3, controller 4 will become controller 3, controller 5 will become controller 4, ...
	 * @param controllerID the ID of the controller to remove.
	 */
	virtual void removeController(int controllerID) {} // TODO

	/** Binds a controller to an ID.
	 * TODO : Specify what happens if there are "holes" (unconfigured controllers) generated by a too high controllerID.
	 * @param config the configuration of the controller.
	 * @param controllerID the ID of the controller to set.
	 * @return true if the controller was successfully set up, false otherwise.
	 */
	virtual bool setController(ControllerConfig* config, int controllerID = 0) {return false;} // TODO

	/** Return the number of controllers allocated in the engine.
	 * @return the number of controllers allocated in the engine.
	 */
	virtual unsigned int getControllerCount() { return 1; } // TODO

	/** Returns the current configuration of a controller.
	 * @param controllerID the ID of the controller to query.
	 * @return an object storing the whole configuration for
	 */
	virtual ControllerConfig* getControllerConfig(int controllerID = 0) { return NULL; } // TODO


	/** Returns the current state of a controller.
	 * @param controllerID the ID of the controller to query.
	 * @return the state of the controller.
	 */
	virtual ControllerState* getControllerState(int controllerID = 0) { return controllers[controllerID]; }

	/** Enables textual input.
	 * This function enables text input system. Depending on the environment, this may
	 * disable normal keyboard mappinng, display an on-screen virtual keyboard, or whatever
	 * will allow to type text.
	 * The function will NOT disable external peripherals associated to actions, so
	 * joysticks, gamepads or remote controls will remain active.
	 * You must always remember that text input will have an impact on the behaviour of
	 * the engine.
	 *
	 * Th status of the input buffer is not guaranteed after enabling text input. Particulary,
	 * disabling, then re-enabling the text input will not necessarily empty the internal
	 * input buffer.
	 *
	 * The only guaranteed fact is that input buffer never contains garbage, that is, its
	 * content is always determined by keystrokes, whether they could happened while the system
	 * is enabled or not.
	 *
	 * @param controllerID the ID of the controller to query.
	 * @return true if the text input mode is enabled, false otherwise.
	 */
	virtual bool enableTextInput(int controllerID = 0) = 0;

	/** Disables textual input.
	 * This function will return to the normal way of handling input.
	 * @param controllerID the ID of the controller to query.
	 * @return true if the text input mode is disabled, false otherwise.
	 * @see enableTextInput
	 */
	virtual bool disableTextInput(int controllerID = 0) = 0;

	/** Read characters from text input system.
	 * This function reads some characters from the text input buffer and return them in
	 * UTF-8 format.
	 * @param buffer the buffer that will receive characters in UTF-8 format.
	 * @param bufSize the size of the buffer.
	 * @param controllerID the ID of the controller to query.
	 * @return the number of bytes written to buffer.
	 * @see feedTextInput
	 * @see peekTextInput
	 * @see clearTextInput
	 */
	virtual size_t readTextInput(char* buffer, size_t bufSize, int controllerID = 0) = 0;

	/** Read characters from text input without removing them from the internal buffer.
	 * @param buffer the buffer that will receive characters in UTF-8 format.
	 * @param bufSize the size of the buffer.
	 * @param controllerID the ID of the controller to query.
	 * @return the number of bytes written to buffer.
	 * @see readTextInput
	 * @see feedTextInput
	 * @see clearTextInput
	 */
	virtual size_t peekTextInput(char* buffer, size_t bufSize, int controllerID = 0) = 0;

	/** Simulate key presses by appending data to the input buffer.
	 * @param buffer data to pass to the text input system, in UTF-8 format.
	 * @param bufSize the size of the passed buffer.
	 * @param controllerID the ID of the controller to query.
	 * @return the number of bytes effectively added to the queue.
	 * @see readTextInput
	 * @see peekTextInput
	 * @see clearTextInput
	 */
	virtual size_t feedTextInput(char* buffer, size_t bufSize, int controllerID = 0) = 0;

	/** Clear the text input buffer.
	 * @param controllerID the ID of the controller to query.
	 */
	virtual void clearInputBuffer(int controllerID = 0) = 0;


#ifdef MOUSE_SUPPORT
	/** Sets the mouse in absolute mode.
	 * When in absolute mode, coordinates returned by getControllerState are
	 * the coordinates of the pointer.
	 * In absolute mode, mouse pointer is shown when the plateform supports it.
	 * Pointer may be disabled if outside the application or it may be constrained
	 * to the area occupied by the window. This behaviour, if supported by the
	 * operating system, is desirable for fullscreen, where some bugs may confuse
	 * application focus or where pointer may still hit system tools.
	 * @param controllerID the ID of the controller to query.
	 * @return true if successful, false if the plateform does not support absolute mode.
	 */
	virtual bool absoluteMouse(int controllerID = 0) = 0;

	/** Sets the mouse in relative mode.
	 * When in relative mode, coordinates returned by getControllerState are
	 * the difference between the current position and the position it was before.
	 * In relative mode, mouse pointer is hidden when the plateform supports it.
	 * To implement relative mode, the plateform-specific code may move the pointer
	 * so showing absolute coordinates is irrelevant.
	 * @param controllerID the ID of the controller to query.
	 * @return true if successful, false if the plaeform does not support relative mode.
	 */
	virtual bool relativeMouse(int controllerID = 0) = 0;

	/** Releases mouse pointer.
	 * When the mouse is in absolute mode, the pointer may be constrained to the window.
	 * To support every possible case, it is possible to release the mouse completely,
	 * leaving the pointer to act normally for the underlying operating system.
	 * @param controllerID the ID of the controller to query.
	 * @return true if the pointer was successfully released, false otherwise.
	 */
	virtual bool releaseMouse(int controllerID = 0) = 0;

	/** Specifies the mouse cursor to use.
	 * Displaying mouse cursor is plateform-specific. Some plateforms (PDA, NDS or any
	 * touchscreen system) will not display it because they don't need it.
	 * Rendering is done via the internal sprite engine, so any renderable may be used.
	 * Be aware that this renderable will not receive update, so animations won't play.
	 * The mouse cursor is a game object in order to have some collision data.
	 * @param controllerID the ID of the controller to query.
	 * @param newCursor the game object to use as a cursor. Set to NULL to disable cursors.
	 */
	virtual void setMouseCursor(GameObject* newCursor, int controllerID = 0) = 0;

	/** Returns the current mouse cursor.
	 * The mouse cursor is plateform-specific. Some plateforms (PDA, NDS or any
	 * touchscreen system) will not display it because they don't need it.
	 * The mouse cursor is a game object in order to have some collision data.
	 * @param controllerID the ID of the controller to query.
	 * @return the game object used as a cursor. NULL if disabled cursor.
	 */
	virtual GameObject* getMouseCursor(int controllerID = 0) = 0;
#endif

#ifdef XML_SUPPORT
	/** Save all controller configurations to a file.
	 * @param configFile the file to save configurations to.
	 * @return true if the config was sucessfully saved, false otherwise.
	*/
	virtual bool saveControllerConfigs(TiXmlElement* configFile);
	virtual bool saveControllerConfigs(VFS::File* configFile);

	/** Save a controller config to a file.
	 * @param config the controller config to save.
	 * @param configFile the file to save configurations to.
	 * @return true if the config was sucessfully saved, false otherwise.
	 */
	virtual bool saveControllerConfig(ControllerConfig* config, TiXmlElement* configFile) = 0;
	virtual bool saveControllerConfig(ControllerConfig* config, VFS::File* configFile);
#endif

};

#endif /* INPUT_ENGINE_H */
