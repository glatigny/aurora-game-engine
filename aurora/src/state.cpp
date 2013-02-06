#include "state.h"

#include "factory.h"
#include "object_sorting.h"
#include "aurora_engine.h"
#include "assert.h"
#include "cameraman.h"
#include "game_map.h"
#include "vfs.h"

#include <iostream>

#define HEADER state
#define HELPER scriptable_object_cpp
#include "include_helper.h"

State::State() :
	Collidable(NULL, this)
{
	parentEngine = NULL;
	enabled = false;
	exclusive = false;
	controllerEnabled = false;
	renderingEnabled = true;
	m_map = NULL;
	m_hud = NULL;
	//script()->setval("state", this);
	//loader = new Factory(this);
	camTarget = NULL;
	camSecondaryTarget = NULL;
	camSwitching = false;
	maxCamSpeed = 10;
	bgm = NULL;
	loader = NULL; //new Factory(this);

#ifdef DISPLAY_COLLISIONS
	displayCollisions = false;
#endif

#ifdef MOUSE_SUPPORT
	dragging = NULL;
	pointerDeltaX = 0;
	pointerDeltaY = 0;
	mouseCursor = NULL;
#endif

	frameCounter = -1;
	stepFrameCount = 1;
}

State::State(AuroraEngine* newParentEngine, bool isEnabled, bool isExclusive, bool isControllerEnabled) :
	Entity(this),	SubScriptable(newParentEngine->script(), NULL), Collidable(newParentEngine, this)
{
	parentEngine = newParentEngine;
	enabled = false;
	exclusive = false;
	controllerEnabled = false;
	renderingEnabled = true;
	m_map = NULL;
	m_hud = NULL;
	script()->setval("state", this);
	script()->setsuper(newParentEngine->script());
	script()->registerScriptVariable("dataDir")->setval(this, engine()->datadir());
	loader = new Factory(this);
	enabled = isEnabled;
	exclusive = isExclusive;
	controllerEnabled = isControllerEnabled;
	camTarget = NULL;
	camSecondaryTarget = NULL;
	camSwitching = false;
	cameraman = NULL;
	maxCamSpeed = 15;
	bgm = NULL;

#ifdef DISPLAY_COLLISIONS
	displayCollisions = false;
#endif

#ifdef MOUSE_SUPPORT
	dragging = NULL;
	pointerDeltaX = 0;
	pointerDeltaY = 0;
	mouseCursor = NULL;
#endif

	frameCounter = -1;
	stepFrameCount = 1;
}

State::~State() {
	if(script()) {
		ScriptVariable* dataDirVar = script()->getVar("dataDir");
		if(dataDirVar)
			delete dataDirVar->getfile();
	}
		
#ifdef MOUSE_SUPPORT
	stopDrag();
#endif
	enabled = false;
	if(loader)
		delete loader;

	if(engine()) {
		assert(engine()->hardware());
		engine()->hardware()->waitForBackgroundTasks();
	}

	if(m_map)
		delete m_map;
}

void State::switchMap(GameMap* newMap) {
	setCamTarget(NULL);
	setCameraman(NULL);
	if(m_map) {
		m_map->doom();
	}

	m_map = newMap;

	if(m_map)
		m_map->setParent(this);
}

GameMap* State::map() {
	return m_map;
}

Renderable* State::hud() {
	return m_hud;
}

void State::setHud(Collidable* hud) {
	m_hud = hud;
}

void State::setCameraman(Cameraman* newCameraman) {
	if(cameraman)
		cameraman->doom();
	
	cameraman = newCameraman;
}

void State::setCamTarget(Renderable* object) {
	camTarget = object;
	camSecondaryTarget = NULL;
	if(camTarget) {
		camSwitching = true;
	}
}

void State::setCamTargets(Renderable* firstObject, Renderable* secondObject) {
	// camTarget cannot be NULL if camSecondaryTarget is not
	if(secondObject != NULL && firstObject == NULL) {
		Renderable* swap = firstObject;
		firstObject = secondObject;
		secondObject = swap;
	}

	camTarget = firstObject;
	camSecondaryTarget = secondObject;

	if(camTarget) {
		camSwitching = true;
	}
}

void State::setCamSpeed(int newCamSpeed) {
	maxCamSpeed = newCamSpeed;
}

bool State::camSwitch() {
	return camSwitching;
}

State* State::subState(VFS::File* scriptFile, bool enabled, bool exclusive, bool controlActive) {
	State* sub = new State(engine(), enabled, exclusive, controlActive);
	sub->Renderable::attachToState(this);
	//#ifdef SOUND_SUPPORT
	//	sub->Audible::attachToState(this);
	//#endif

	engine()->push(sub);
	sub->loadScript(scriptFile);
	return sub;
}

ScriptableObject* State::load(VFS::File* xmlFile) {
	ownedset* owned = new ownedset;
	xmlFile = xmlFile->copy();
	ScriptableObject* instance = loader->load(xmlFile, NULL, owned);
	delete xmlFile;
	instance->setOwnedObjectSet(owned);
	return instance;
}

ScriptableObject* State::loadXml(TiXmlNode* node) {
	TiXmlElement* elt = node->ToElement();
	xerror(elt, "The XML node to instantiate is not an element.");
	ownedset* owned = new ownedset;
	ScriptableObject* instance = loader->createInstance(elt, source(), owned);
	instance->setOwnedObjectSet(owned);
	return instance;
}

ObjectMold* State::loadMold(VFS::File* xmlFile) {
	xmlFile = xmlFile->copy();
	ObjectMold* mold = loader->loadMold(xmlFile);
	delete xmlFile;
	return mold;
}

GameMap* State::loadMap(VFS::File* xmlFile) {
	ScriptableObject* obj = load(xmlFile);
	GameMap* m = scriptable_object_cast<GameMap*> (obj);
	if(m != NULL) {
		// Object is a map.
		switchMap(m);
	} else {
		// Object is not a map.
		if(obj != NULL) {
			// Destroy it.
			xwarn(obj, "Trying to load a non-map object as a map.");
			delete obj;
		}
	}

	return m;
}

Collidable* State::loadToMap(VFS::File* xmlFile) {
	ScriptableObject* obj = load(xmlFile);
	Collidable* c = scriptable_object_cast<Collidable*> (obj);
	if(c != NULL) {
		// Object is a Collidable.
		m_map->addObject(c);
		m_map->own(c);
	} else {
		// Object is not a Collidable.
		if(obj != NULL) {
			// Destroy it.
			xwarn(obj, "Trying to load a non-collidable object into the map.");
			delete obj;
		}
	}

	return c;
}

void State::setControlledObject(Controllable* newControlledObject, int controllerID) {
	xwarn(true, "No GC for Controllable.");
	if(controlledObjects.size() <= (unsigned int)controllerID) {
		controlledObjects.resize(controllerID+1, NULL);
	}
	controlledObjects[controllerID] = newControlledObject;
}

bool State::isFinished() {
	return SubScriptable::isFinished() && (m_map ? (m_map->isFinished()) : true) && Renderable::isFinished();
}

void State::kill() {
	disable();
	setCamTarget(NULL);
	setControlledObject(NULL);
	setHud(NULL);
	switchMap(NULL);
	script()->reset();
}

void State::updateObject(dur elapsedTime) {
	// Store current map in case of switch
	GameMap* current_map = m_map;

	if(!enabled) {
		return;
	}

	assert(engine() != NULL);

	// Process input controller
	if(engine()->hardware()) {
		processInput();
	}

	if(!frameCounter) {
		return;
	} else {
		if(frameCounter > 0) {
			frameCounter--;
		}
	}

	// Run scripts
	if(script()->isRunning()) {
		script()->framestep();
	}

	// Update map
	if(current_map != NULL) {
		// Make the cameraman work
		if(cameraman) {
			cameraman->update(elapsedTime);
			current_map->setPosition(cameraman->getCameraPosition());
		}

		// Center map
		if(camTarget) {
			int targetX;
			int targetY;
			int camX;
			int camY;
			int curX;
			int curY;
			int localMaxCamSpeed = maxCamSpeed * elapsedTime;

			// Compute actual target coordinates
			if(camSecondaryTarget) {
				targetX = (camTarget->getX() + camSecondaryTarget->getX()) / 2;
				targetY = (camTarget->getY() + camSecondaryTarget->getY()) / 2;
			} else {
				targetX = camTarget->getX();
				targetY = camTarget->getY();
			}
			// Smooth object switching
			if(camSwitching) {
				camX = (-current_map->getX() * 3 + targetX) / 4;
				camY = (-current_map->getY() * 3 + targetY) / 4;
				curX = current_map->getX();
				curY = current_map->getY();
			} else {
				camX = targetX;
				camY = targetY;
				curX = camX;
				curY = camY;
			}
			// Limit camera speed
			if(camX + current_map->getX() > localMaxCamSpeed) {
				camX = -current_map->getX() + localMaxCamSpeed;
			} else if(-current_map->getX() - camX > localMaxCamSpeed) {
				camX = -current_map->getX() - localMaxCamSpeed;
			}
			if(camY + current_map->getY() > localMaxCamSpeed) {
				camY = -current_map->getY() + localMaxCamSpeed;
			} else if(-current_map->getY() - camY > localMaxCamSpeed) {
				camY = -current_map->getY() - localMaxCamSpeed;
			}

			// set camera position
			current_map->setPosition(-camX, -camY);
			// test if object switching is finished
			if(camSwitching) {
				if(abs(camX - targetX) < 5 && abs(camY - targetY) < 5) {
					camSwitching = false;
				} else if((curX == current_map->getX()) && (curY == current_map->getY())) {
					camSwitching = false;
				}
			}
		}

		// Update map
		current_map->update(elapsedTime);

		// Update mouse cursor
#ifdef MOUSE_SUPPORT
		if(mouseCursor) {
			Collidable* current_mouse = mouseCursor;
			current_mouse->update(elapsedTime);

			Collision c = current_mouse->hitBy(current_map);
			if(c.collided()) {
				current_map->signal(current_mouse, MOUSE_MOVE, 0);
			}

			if(m_hud) {
				c = current_mouse->hitBy(m_hud);
				if(c.collided()) {
					m_hud->signal(current_mouse, MOUSE_MOVE, 0);
				}
			}
		}
#endif
	}

	// Update hud
	if(m_hud != NULL) {
		m_hud->update(elapsedTime);
	}
}

void State::render() {
	if(!enabled || !renderingEnabled) {
		return;
	}

	Renderable::render();
}

void State::renderGraphics() {
	// Render map
	if(m_map != NULL) {
		m_map->render();
	}

	// Render HUD
	if(m_hud != NULL) {
		m_hud->render();
	}

	// Render mouse cursor
#ifdef MOUSE_SUPPORT
	if(mouseCursor) {
		mouseCursor->render();
	}
#endif
}

void State::loadGraphics() {
	if(m_map != NULL) {
		m_map->loadGraphics();
	}

	if(m_hud != NULL) {
		m_hud->loadGraphics();
	}
}

void State::unloadGraphics() {
	if(m_map != NULL) {
		m_map->unloadGraphics();
	}

	if(m_hud != NULL) {
		m_hud->unloadGraphics();
	}
}

int State::signal(Collidable* other, SignalId signal, int data) {
	if(!enabled) {
		return 0;
	}
	bool propagateSignal =
	script()->callFunction(*this, (char*) "onSignal", "oii", (ScriptableObject*) other, (int) signal, data).getbool();

	if(!propagateSignal) {
		return 1;
	}

	return m_map ? m_map->signal(other, signal, data) : 0;
}

bool State::onSignal(Collidable* other, int signal, int data) {
#if defined(SYSLOG) && SYSLOG_VERBOSE >= 2
	syslog("state.onSignal(%p, %d, %d);", (AOEObject*)other, signal, data);
#endif
	return true;
}

Collision State::hitBy(Collidable* other) {
	if(!collisionsEnabled || !enabled || this == other || !m_map) {
		return Collision();
	}

	// Test collisions with the map
	Collision c = m_map->hitBy(other);

	return c;
}

#ifdef MOUSE_SUPPORT
void State::sendMouseSignal(SignalId sid, int data) {
	Collidable* signaler = dynamic_cast<Collidable*>(dragging);
	if(!signaler) {
		signaler = mouseCursor;
		if(!signaler) {
			signaler = this;
		}
	}

	signal(signaler, sid, data);
}
#endif

void State::processInput() {
	ControllerState* mainController = engine()->hardware()->getControllerState(0);
	if(!mainController) {
		return;
	}

#ifdef DEBUG_KEYS
	if(mainController->isButtonPressed(ControllerState::BTN_DBG_StateStep)) {
		if(frameCounter < 0) {
			syslog("State %p pause", (AOEObject*)this);
			frameCounter = 0;
		} else {
			syslog("State %p step", (AOEObject*)this);
			frameCounter += stepFrameCount;
		}
	}

	if(mainController->isButtonPressed(ControllerState::BTN_DBG_StatePlay)) {
		syslog("State %p play", (AOEObject*)this);
		frameCounter = -1;
	}

	if(mainController->isButtonPressed(ControllerState::BTN_DBG_IncreaseStateStep)) {
		if(stepFrameCount < 240) {
			stepFrameCount++;
		}syslog("Step by step : %d frames", stepFrameCount);
	}

	if(mainController->isButtonPressed(ControllerState::BTN_DBG_DecreaseStateStep)) {
		if(stepFrameCount > 1) {
			stepFrameCount--;
		}syslog("Step by step : %d frames", stepFrameCount);
	}

	if(mainController->isButtonPressed(ControllerState::BTN_DBG_ReloadGraphics)) {
		unloadGraphics();
	}

	if(mainController->isButtonPressed(ControllerState::BTN_DBG_SwitchStateRendering)) {
		renderingEnabled = !renderingEnabled;
	}

	if(mainController->isButtonPressed(ControllerState::BTN_DBG_SwitchHardwareRendering)) {
		engine()->hardware()->switchRendering();
	}

#ifdef DISPLAY_COLLISIONS
	if(mainController->isButtonPressed(ControllerState::BTN_DBG_DisplayCollisions)) {
		displayCollisions = !displayCollisions;
	}
#endif /* DISPLAY_COLLISIONS */

#endif /* DEBUG_KEYS */

	if(controllerEnabled) {
		if(!controlledObjects.empty()) {
			controlledObjects[0]->processInput(mainController);
			for(unsigned int i=1; i<controlledObjects.size(); i++) {
				controlledObjects[i]->processInput(engine()->hardware()->getControllerState(i));
			}
		}

#ifdef MOUSE_SUPPORT
		if(mouseCursor) {
			if(mainController->getMouseMode() == MOUSE_RELATIVE) {
				mouseCursor->translate(mainController->getMouseX(), mainController->getMouseY());
			} else {
				Point mousePosition(mainController->getMouseX(), mainController->getMouseY());
				mouseCursor->setAbsolutePosition(mousePosition);
			}
			mouseCursor->setAbsoluteRotation(mainController->getMouseAngle());
		}

		if(dragging) {
			pointerDeltaX = mainController->getMouseX();
			pointerDeltaY = mainController->getMouseY();

			drag();
		}


		if(mainController->isMouseButtonReleased(0)) {
			sendMouseSignal(RELEASE_CLICK, 0);
		}
		if(mainController->isMouseButtonReleased(1) || mainController->isMouseButtonReleased(2)) {
			sendMouseSignal(RELEASE_CLICK, 1);
		}

		if(mainController->isMouseButtonPressed(0)) {
			sendMouseSignal(CLICK, 0);
		}

		if(mainController->isMouseButtonPressed(1) || mainController->isMouseButtonPressed(2)) {
			sendMouseSignal(CLICK, 1);
		}

#endif
	}
}

#ifdef MOUSE_SUPPORT
void State::startDrag(Renderable* object) {
	pointerDeltaX = 0;
	pointerDeltaY = 0;
	if(dragging != object) {
		stopDrag();
		if(object) {
			relativeMouse();
			syslog("Start dragging %p", (AOEObject*)object);
			dragging = object;
			dragging->setParent(this);
		}
	}
}

void State::stopDrag() {
	if(dragging) {
		syslog("Stop dragging %p", (AOEObject*)dragging);
		dragging = NULL;
		pointerDeltaX = 0;
		pointerDeltaY = 0;
	}
}

bool State::drag() {
	if(dragging) {
		dragging->translate(pointerDeltaX, pointerDeltaY);
	}
	return (bool) dragging;
}
#endif

// QC:A
void State::setBGM(VFS::File* xmlFile, int loop) {
	if(bgm) {
		engine()->hardware()->freeAudioInstance(bgm);
	}

	if( xmlFile ) {
		xmlFile = xmlFile->copy();
		//VFS::PCMSource* soundFile = VFS::openPCM(xmlFile);
		bgm = engine()->hardware()->instantiateSound(xmlFile, AUDIO_FILE_STREAM, 0, 0, false, loop, NULL);

		if(bgm) {
			engine()->hardware()->play(bgm);
		}
		delete xmlFile;
	} else {
		bgm = NULL;
	}
}

// QC:W
void State::loadSounds() {
}

// QC:W
void State::unloadSounds() {
}

// QC:W
void State::commitSounds(int finalVolume, AudioEffect* finalEffects) {

}

#ifdef MOUSE_SUPPORT

// QC:G
bool State::releaseMouse() {
	return engine()->hardware()->releaseMouse();
}

// QC:G
bool State::relativeMouse() {
	return engine()->hardware()->relativeMouse();
}

// QC:G
bool State::absoluteMouse() {
	return engine()->hardware()->absoluteMouse();
}

// QC:G
void State::setCursor(Collidable* newCursor) {
	mouseCursor = newCursor;
	if(mouseCursor) {
		mouseCursor->setParent(this);
	}
}
#endif

// QC:W
bool State::updatePreScript(dur elapsedTime) { return true; }

// QC:W
void State::updatePostScript(dur elapsedTime) {}

