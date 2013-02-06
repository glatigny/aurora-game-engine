#ifndef STATE_H
#define STATE_H
class State;
class GameMap;
class Point;
class AuroraEngine;
class Factory;
class Cameraman;
class AudioInstance;

namespace VFS {
	class File;
}

#include <vector>
#include "sub_scriptable.h"
#include "resizable.h"
#include "collidable.h"
#include "controllable.h"
#include "audible.h"
#include "abstract.h"

class GameObject;

/** A state is a layer of activity. It manages objects, controls and map
 * specific to that layer.
 */
class State: public SubScriptable, public Collidable, public Resizable, public Audible {
	friend class AuroraEngine;
private:
	AuroraEngine* parentEngine;
	bool enabled; /** The state is enabled, updated and rendered */
	bool exclusive; /** Blocks other states below this one in the stack */
	std::vector<Controllable*> controlledObjects; /** objects which receives controller state */
	bool controllerEnabled; /** Are controllers events sent to the controlledObject ? */
	bool renderingEnabled; /** Is the state rendered ? */
	GameMap* m_map; /** Current map. */
	Collidable* m_hud; /** Current HUD. */
	Factory* loader; /** XML data loader. */
	Cameraman* cameraman;
	Renderable* camTarget; /** Object which will stay at the center of the screen. */
	Renderable* camSecondaryTarget; /** Used when there are two objects to center. */
	int maxCamSpeed; /** Maximum camera speed. */
	bool camSwitching; /** True while the camera is switching between two objects. */
	AudioInstance* bgm;

	// Playback control
	int frameCounter;
	int stepFrameCount;

#ifdef MOUSE_SUPPORT
	Collidable* mouseCursor;
	Renderable* dragging; // Object being currently dragged
	int pointerDeltaX; // Horizontal movement of the pointer
	int pointerDeltaY; // Vertical movement of the pointer
	void sendMouseSignal(SignalId sid, int data);
#endif

protected:
	bool onSignal(Collidable* other, int signal, int data);

	/** Process input by sending processInput calls to every controllable in the controledObjects vector.
	 */
	void processInput();

	/* Audible */

	void commitSounds(int finalVolume, AudioEffect* finalEffect);

public:
#define HEADER state
#define HELPER scriptable_object_h
#include "include_helper.h"
	State();
	State(AuroraEngine* parentEngine, bool enabled, bool exclusive, bool controlActive);
	virtual ~State();

#ifdef DISPLAY_COLLISIONS
	bool displayCollisions;
#endif

	/** Kill the state.
	 * The state will be killed. That means :
	 *  - All threads will be killed.
	 *  - map, hud and camTargets will be set to NULL.
	 *  - The state will be disabled.
	 *
	 * The basic rule here is that a killed state should unload itself if
	 * there are no external references to it. It will also be removed of
	 * the AuroraEngine because it will be considered as finished.
	 */
	void kill();

	/** Register a lively object.
	 * This function adds an object to the update list.
	 *
	 * WARNING:still unimplemented
	 */
	void registerRealtimeObject(Realtime* obj) { xassert(obj->state(), "The object %p has no state.", (AOEObject*)obj);  syslog("Registering object %p with state %p", (AOEObject*)obj, (AOEObject*)this); }
	void unregisterRealtimeObject(Realtime* obj) {}

	/** Switch the map.
	 * @param newMap the new map to display.
	 */
	void switchMap(GameMap* newMap);

	/** Return the current map.
	 * @return the current map associated to this state.
	 */
	GameMap* map();

	/** Return the current hud.
	 * @return the current hud associated to this state.
	 */
	Renderable* hud();

	/** Set the current hud.
	 * @param the new hud which will be associated to this state.
	 */
	void setHud(Collidable* hud);

	/** Enable this state.
	 */
	void enable() {
		enabled = true;
	}

	/** Disable this state.
	 */
	void disable() {
		enabled = false;
	}

	/** Disable underlying states to take exclusive control.
	 */
	void takeExclusiveControl() {
		exclusive = true;
	}

	/** Enable underlying states.
	 */
	void releaseExclusiveControl() {
		exclusive = false;
	}

	/** Enable sending events to the controlledObject.
	 */
	void enableController() {
		controllerEnabled = true;
	}

	/** Disable all input processing for this State.
	 */
	void disableController() {
		controllerEnabled = false;
	}

	/** Enable rendering of this state.
	 */
	void enableRendering() {
		renderingEnabled = true;
	}

	/** Disable rendering of this state.
	 */
	void disableRendering() {
		renderingEnabled = false;
	}

	/** Change the cameraman.
	 * @param cameraman the new cameraman. Set to NULL to disable cameraman management.
	 */
	void setCameraman(Cameraman* cameraman);

	/** Set the object which will be kept at the center of the screen.
	 */
	void setCamTarget(Renderable* object);
	bool setCamTarget_block(Renderable* object) { return camSwitching; }

	/** Set two objects as camera target.
	 * The camera will center at an average coordinate between the two objects.
	 */
	void setCamTargets(Renderable* firstObject, Renderable* secondObject);
	bool setCamTargets_block(Renderable* firstObject, Renderable* secondObject) { return camSwitching; }

	/** Set the maximum speed of the camera.
	 * This is the maximum delta of camera movement.
	 */
	void setCamSpeed(int newCamSpeed);

	/** Tells if the camera is currently switching between two different targets.
	 * @return true if the camera is switching, false otherwise.
	 */
	bool camSwitch();

	/** Select which object will receive input events.
	 * @param newControlledObject the object which will receive input events.
	 * @param controllerID the ID of the controller to link to this object.
	 */
	void setControlledObject(Controllable* newControlledObject, int controllerID = 0);


	/** Make a substate.
	 * @param script the script to load into the state.
	 * @param enabled is the substate enabled ?
	 * @param exclusive set to true if you wish the substate to freeze the current one ?
	 * @param controllerActive set to true to capture controller events.
	 * @return the new state.
	 */
	State* subState(VFS::File* script, bool enabled, bool exclusive, bool controlActive);

	/** Load an object from a file.
	 * Loads an object from a file. The object is automatically linked to this state.
	 * Only the first node of the file will be loaded.
	 * @param filename the XML file used to load the object.
	 * @return the loaded object.
	 */
	ScriptableObject* load(VFS::File* xmlFile);

	/** Load an object from an XML element.
	 * @param node the root of the AOD to instantiate. Must be a TiXmlElement.
	 * @return the instanciated object.
	 */
	ScriptableObject* loadXml(TiXmlNode* node);

	/** Create the mold of an object.
	 * @param filename the XML file used to make the object mold.
	 * @return a mold to instantiate the object.
	 */
	ObjectMold* loadMold(VFS::File* xmlFile);

	/** Load a map and switch to it.
	 * @param filename the XML file used to load the map. The file must start with one GameMap tag.
	 * @return the loaded map.
	 */
	GameMap* loadMap(VFS::File* xmlFile);

	/** Load an object from a file and add it to the current map.
	 * Loads an object and add it to the map.
	 * Only the first node of the file will be loaded.
	 * @param filename the XML file used to load the object.
	 * @return the loaded object.
	 */
	Collidable* loadToMap(VFS::File* xmlFile);

	/** Sets background music.
	 * @param bgm the file to play as background music.
	 */
	void setBGM(VFS::File* soundFile, int loop);

#ifdef MOUSE_SUPPORT
	/** Returns the current mouse cursor.
	 * @return the current mouse cursor, or NULL il no pointer has been set.
	 */
	Collidable* cursor() {
		return mouseCursor;
	}

	/** Sets the current mouse cursor.
	 * @param cursor the new mouse cursor to set.
	 */
	void setCursor(Collidable* cursor);

	/** Starts dragging an object.
	 * @param object the object to drag.
	 */
	void startDrag(Renderable* object);

	/** Stops dragging.
	 */
	void stopDrag();

	/** Drags for one step.
	 */
	bool drag();

	/** Sets the mouse cursor to released mode.
	 * Coordinates given to the controllable will be absolute and the cursor can escape the game window.
	 */
	bool releaseMouse();

	/** Sets the mouse cursor in relative mode.
	 * Coordinates given to the controllable will be relative to the previous frame.
	 * Pointer is grabbed.
	 */
	bool relativeMouse();

	/** Like releaseMouse but with a grabbed pointer.
	 * Watch out : the pointer may leave the current window, so be sure to restrain its position via a script or something.
	 */
	bool absoluteMouse();
#endif

	/* Blocking */

	virtual bool isFinished();

	/* Renderable */

	virtual void render();
	virtual void renderGraphics();
	virtual void loadGraphics();
	virtual void unloadGraphics();

	/* Realtime */

	virtual bool updatePreScript(dur elapsedTime);
	virtual void updateObject(dur elapsedTime);
	virtual void updatePostScript(dur elapsedTime);

	/* Collidable */

	virtual int signal(Collidable* other, SignalId signal, int data);

	virtual Translatable* getLeftmostPoint() {
		return NULL;
	}

	virtual Translatable* getRightmostPoint() {
		return NULL;
	}

	Point getSize() {
		return Point::Point(0, 0);
	}
	Collision hitBy(Collidable* other);

	/* Translatable */

	virtual bool inside(Translatable& firstCorner, Translatable& secondCorder) {
		return true;
	}

	/* Audible */

	virtual void loadSounds();
	virtual void unloadSounds();

	/* Entity */

	virtual State* state() { return this; }
	
	/* AOEObject */
	
	virtual AuroraEngine* engine() {
		return parentEngine;
	}

	virtual VFS::File* source() {
		return SubScriptable::scriptSource;
	}

};

#endif

