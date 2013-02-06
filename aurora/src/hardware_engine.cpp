/*--- INCLUDES ---*/
#include "pspec/hardware_engine_inc.h"
#include "hardware_engine.h"
#include <string.h>
#include <iostream>
#include "pspec/thread.h"
#include "background_task_scheduler.h"

/*--- DEFINES ---*/

/*--- VARIABLES ---*/

// QC:P
HardwareEngine::HardwareEngine() {
	timeEng = NULL;
	graphical = NULL;
	input = NULL;
	sound = NULL;
	net = NULL;
}

// QC:P
HardwareEngine::HardwareEngine(TimeEngineInterface* timeEngine, GraphicalEngineInterface *graphicalEngine, InputEngineInterface *inputEngine,
                               SoundEngineInterface *soundEngine, NetworkEngineInterface *netEngine)
{
#ifdef DEBUG_KEYS
	renderingEnabled = true;
#endif

	timeEng = timeEngine;
	graphical = graphicalEngine;
	input = inputEngine;
	sound = soundEngine;
	net = netEngine;

	if(timeEng)
		timeEng->init(this);
	
	if(graphical)
		graphical->init(this);

	if(input)
		input->init(this);
	
	if(sound)
		sound->init(this);

	scheduler = new BackgroundTaskScheduler();

	// Test vSync and stabilize performance meters
	for(int i=0; i<3; i++) {
		preDraw();
		postDraw();
	}
	for(int i=0; i<6; i++) {
		preDraw();
		postDraw();
		syncToFrameRate();
	}
}

#ifdef PERFLOG
void HardwareEngine::displayMeter(int x, int y, int value, int min, int max, int bad, int good) {
	int h = getDisplaySize()->getH() / 2 - 16;
	x = x * 24 + 8;
	y = y*(h + 8) + 8;
	Rect rect(x, y, 16, h);
	drawRect(&rect, 255, 255, 255, 255);

	x += 2;
	y += 2;
	h = (h-4)/3;

	for(int i=0; i<h; i++) {
		if(value > i*(max-min)/h + min) {
			int r, g, b;
			if(good <= min) {
				int v = i*255/h;
				r = v;
				g = bad?255-v:0;
				b = bad?0:255-v;
			} else {
				r = value<bad?255:0;
				g = value>=bad?255:0;
				b = value>good?255:0;
			}

			Rect unit(x, y + (h-i)*3, 12, 1);
			drawRect(&unit, r, g, b, 192);
		}
	}
}
#endif

void HardwareEngine::displayStats() {
#ifdef PERFLOG
	static int schedulerMax = 0;

	int h = getDisplaySize()->getH();
	int fps = getCurrentFPS();
	int bgQueue = scheduler->getTaskCount();

	if(bgQueue > schedulerMax) {
		schedulerMax = bgQueue;
	} else if(bgQueue == 0) {
		schedulerMax = 0;
	}

	displayMeter(0, 0, getCurrentFPS(), 0, 120, 59, 62);
#ifdef BG_PRELOAD
	displayMeter(0, 1, bgQueue, 0, schedulerMax);
#endif
#endif
}

// QC:P
dur HardwareEngine::getElapsedTime() {
	if(timeEng) {
		return timeEng->getElapsedTime();
	}

	return 0;
}

// QC:P
void HardwareEngine::syncToFrameRate() {
	if(timeEng) {
		timeEng->syncToFrameRate();
	}
}

// QC:P
void HardwareEngine::setFrameRate(dur newFPS) {
	if(timeEng) {
		timeEng->setFrameRate(newFPS);
	}
}

// QC:P
dur HardwareEngine::getCurrentFPS() {
	if(timeEng) {
		return timeEng->getCurrentFPS();
	}

	return 0;
}

// QC:?
void HardwareEngine::processInput() {
	if(input) {
		input->processInput();
	}
}

// QC:L
HardwareEngine::~HardwareEngine() {
	if(timeEng) {
		delete timeEng;
	}

	if(graphical) {
		delete graphical;
	}

	if(input) {
		delete input;
	}

	if(sound) {
		delete sound;
	}

	if(net) {
		delete net;
	}

	delete scheduler;
}

// QC:P
void HardwareEngine::setGraphicalEngine(GraphicalEngineInterface *graphicalEngine) {
	if(graphical) {
		delete graphical;
	}

	graphical = graphicalEngine;
}

// QC:P
void HardwareEngine::setInputEngine(InputEngineInterface *inputEngine) {
	if(input) {
		delete input;
	}

	input = inputEngine;
}

// QC:P
void HardwareEngine::setSoundEngine(SoundEngineInterface *soundEngine) {
	if(sound) {
		delete sound;
	}

	sound = soundEngine;
}

// QC:P
dur HardwareEngine::getNativeFrameRate() {
	if(graphical) {
		return graphical->getNativeFrameRate();
	}

	return 60;
}

// QC:P
void HardwareEngine::preDraw() {
	if(timeEng) {
		timeEng->preDraw();
	}

	if(graphical)
		graphical->preDraw();
}

// QC:P
void HardwareEngine::postDraw() {
	if(timeEng) {
		timeEng->postDraw();
	}

	if(graphical)
		graphical->postDraw();
}

// QC:P
void HardwareEngine::draw(Texture* p_texture, Translatable* p_coordinates, GraphicalEffects* p_effect, int slot) {
#ifdef DEBUG_KEYS
	if(renderingEnabled)
#endif
	if(graphical)
		graphical->draw(p_texture, p_coordinates, p_effect, slot);
}

// QC:?
void HardwareEngine::drawRect(Resizable* rect, unsigned int r, unsigned int g, unsigned int b, unsigned int a) {
	if(graphical)
		graphical->drawRect(rect, r, g, b, a);
}

// QC:P
void HardwareEngine::fill(Resizable* p_coordinates, GraphicalEffects* p_effect, int slot) {
	if(graphical)
		graphical->fill(p_coordinates, p_effect, slot);
}

// QC:P
bool HardwareEngine::terminationRequest() {
	if(graphical && graphical->terminationRequest())
		return true;

	if(input && input->terminationRequest())
		return true;

	return false;
}

// QC:P
Texture* HardwareEngine::loadTexture(VFS::ImageSource* p_file) {
	if(graphical)
		return graphical->loadTexture(p_file);
	return NULL;
}

// QC:P
bool HardwareEngine::freeTexture(Texture* p_texture) {
	if(graphical)
		return graphical->freeTexture(p_texture);
	return true;
}

// QC:P
void HardwareEngine::pushRenderingSurface(Translatable* minimumSurfaceSize, uint32_t slots, void* preProcess, void* postProcess, int format) {
	if(graphical)
		graphical->pushRenderingSurface(minimumSurfaceSize, slots, preProcess, postProcess, format);
}

// QC:P
Renderable* HardwareEngine::popRenderingSurface(State* attachedState) {
	if(graphical)
		return graphical->popRenderingSurface(attachedState);

	return NULL;
}

// QC:P
bool HardwareEngine::loadImageProcess(const char* processString, void* &pspecData, uint32_t &neededSlots) {
#ifdef GPU_IMAGEPROCESS
	if(graphical)
		return graphical->loadImageProcess(processString, pspecData, neededSlots);
#endif

	return false;
}

// QC:P
void HardwareEngine::unloadImageProcess(void* &pspecData) {
#ifdef GPU_IMAGEPROCESS
	if(graphical)
		graphical->unloadImageProcess(pspecData);
#endif
}

// QC:P
Resizable* HardwareEngine::getDisplaySize() {
	static Rect noDisplaySize(0, 0, 0, 0);
	if(graphical)
		return graphical->getDisplaySize();
	return &noDisplaySize;
}

// QC:P
void HardwareEngine::resizeWindow(Resizable* newSize) {
	if(graphical) {
		graphical->resizeWindow(newSize);
	}
}

// QC:?
void HardwareEngine::setViewport(Resizable* physicalViewport, Resizable* virtualViewport) {
	if(graphical) {
		Resizable* p;
		Resizable* v;

		if(physicalViewport)
			p = physicalViewport;
		else
			p = getDisplaySize();

		if(virtualViewport)
			v = virtualViewport;
		else
			v = getDisplaySize();

		graphical->setViewport(p, v);
	}
}

// QC:P
ControllerState* HardwareEngine::getControllerState(int controllerID) {
	if(input)
		return input->getControllerState(controllerID);
	return NULL;
}

// QC:?
bool HardwareEngine::enableTextInput(int controllerID) {
	if(input)
		return input->enableTextInput(controllerID);
	return false;
}

// QC:?
bool HardwareEngine::disableTextInput(int controllerID) {
	if(input)
		return input->disableTextInput(controllerID);
	return false;
}

// QC:?
size_t HardwareEngine::readTextInput(char* buffer, size_t bufSize, int controllerID) {
	assert(buffer);
	if(input)
		return input->readTextInput(buffer, bufSize, controllerID);
	return 0;
}

// QC:?
size_t HardwareEngine::peekTextInput(char* buffer, size_t bufSize, int controllerID) {
	assert(buffer);
	if(input)
		return input->peekTextInput(buffer, bufSize, controllerID);
	return 0;
}

// QC:?
size_t HardwareEngine::feedTextInput(char* buffer, size_t bufSize, int controllerID) {
	assert(buffer);
	if(input)
		return input->feedTextInput(buffer, bufSize, controllerID);

	return 0;
}

// QC:?
void HardwareEngine::clearInputBuffer(int controllerID) {
	if(input)
		input->clearInputBuffer(controllerID);
}

#ifdef MOUSE_SUPPORT

// QC:P
bool HardwareEngine::absoluteMouse(int controllerID) {
	if(input)
		return input->absoluteMouse(controllerID);
	return false;
}

// QC:P
bool HardwareEngine::relativeMouse(int controllerID) {
	if(input)
		return input->relativeMouse(controllerID);
	return false;
}

// QC:P
bool HardwareEngine::releaseMouse(int controllerID) {
	if(input)
		return input->releaseMouse(controllerID);
	return false;
}

// QC:P
void HardwareEngine::setMouseCursor(GameObject* newCursor, int controllerID) {
	if(input)
		input->setMouseCursor(newCursor, controllerID);
}

// QC:P
GameObject* HardwareEngine::getMouseCursor(int controllerID) {
	if(input)
		return input->getMouseCursor(controllerID);
	return NULL;
}

#endif /* MOUSE_SUPPORT */

std::list<std::string> HardwareEngine::getInputHardwareDeviceList() {
	if(input)
		return input->getInputHardwareDeviceList();

	return std::list<std::string>();
}

std::list<std::string> HardwareEngine::getButtonList(const char* hardwareDeviceName) {
	if(input)
		return input->getButtonList(hardwareDeviceName);

	return std::list<std::string>();
}
std::list<std::string> HardwareEngine::getAxesList(const char* hardwareDeviceName) {
	if(input)
		return input->getAxesList(hardwareDeviceName);

	return std::list<std::string>();
}
std::list<std::string> HardwareEngine::getFeedbackAxesList(const char* hardwareDeviceName) {
	if(input)
		return input->getFeedbackAxesList(hardwareDeviceName);

	return std::list<std::string>();
}

const char* HardwareEngine::getActiveButton(const char* hardwareDeviceName) {
	if(input)
		return input->getActiveButton(hardwareDeviceName);

	return NULL;
}

const char* HardwareEngine::getActiveAxis(const char* hardwareDeviceName) {
	if(input)
		return input->getActiveAxis(hardwareDeviceName);

	return NULL;
}


// QC:?
AudioInstance* HardwareEngine::instantiateSound(VFS::File* file, int audioAccessMode, int initialVolume, int priority,
                                                bool positioned, int loop, Translatable* position)
{
	if(sound && file)
		return sound->instantiateSound(file, audioAccessMode, initialVolume, priority, positioned, loop, position);
	return NULL;
}

// QC:?
void HardwareEngine::freeAudioInstance(AudioInstance* instance) {
	assert(instance);
	if(sound)
		sound->freeAudioInstance(instance);
}

// QC:?
bool HardwareEngine::play(AudioInstance* instance) {
	assert(instance);
	if(sound)
		return sound->play(instance);
	return false;
}

// QC:?
bool HardwareEngine::isPlaying(AudioInstance* instance) {
	assert(instance);
	if(sound)
		return sound->isPlaying(instance);
	return false;
}

// QC:?
void HardwareEngine::pause(AudioInstance* instance) {
	assert(instance);
	if(sound)
		sound->pause(instance);
}

// QC:?
void HardwareEngine::rewind(AudioInstance* instance) {
	assert(instance);
	if(sound)
		sound->rewind(instance);
}

SoundEngineInterface::~SoundEngineInterface() {

}

NetSocket* HardwareEngine::connect(const char* uri) {
	if(net)
		return net->connect(uri);

	return NULL;
}

void HardwareEngine::disconnect(NetSocket* socket) {
	if(net)
		net->disconnect(socket);
}

bool HardwareEngine::send(NetSocket* socket, const char* message) {
	xadvice((NTOH_UINT32(*((uint32_t*) (message)))) > 65536, "Network sending extra long message (> 64k) or transmitting invalid message.");
	if(net)
		return net->send(socket, message);

	return false;
}

// QC:?
const char* HardwareEngine::receive(NetSocket* socket) {
	if(net)
		return net->receive(socket);

	return NULL;
}

// QC:?
bool HardwareEngine::connected(NetSocket* socket) {
	if(net)
		return net->connected(socket);

	return false;
}

// QC:P
int HardwareEngine::addBackgroundTask(ThreadEntryPoint task, void* data, int queue) {
	return scheduler->addBackgroundTask(task, data, queue);
}

// QC:P
void HardwareEngine::doBackgroundTasks() {
	scheduler->doBackgroundTasks();
}

void HardwareEngine::waitForBackgroundTasks() {
	scheduler->waitForBackgroundTasks();
}
