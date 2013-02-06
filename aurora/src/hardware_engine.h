#ifndef HARDWARE_ENGINE_H
#define HARDWARE_ENGINE_H

class HardwareEngine;
class BackgroundTaskScheduler;
typedef void (*ThreadEntryPoint)(void*);

#include "time_engine.h"
#include "graphical_engine.h"
#include "input_engine.h"
#include "sound_engine.h"
#include "network_engine.h"
#include "vfs.h"


class HardwareEngine: public GraphicalEngineInterface,
public InputEngineInterface,
public SoundEngineInterface,
public TimeEngineInterface,
public NetworkEngineInterface
{

private:
	TimeEngineInterface* timeEng;
	GraphicalEngineInterface* graphical;
	InputEngineInterface* input;
	SoundEngineInterface* sound;
	NetworkEngineInterface* net;
	BackgroundTaskScheduler* scheduler;

#ifdef DEBUG_KEYS
	bool renderingEnabled;
#endif

public:
	HardwareEngine();
	HardwareEngine(TimeEngineInterface* timeEngine, GraphicalEngineInterface *graphicalEngine, InputEngineInterface *inputEngine,
	               SoundEngineInterface *soundEngine, NetworkEngineInterface* net);
	virtual ~HardwareEngine();

	void setTimeEngine(TimeEngineInterface *timeEngine);
	void setGraphicalEngine(GraphicalEngineInterface *graphicalEngine);
	void setInputEngine(InputEngineInterface *inputEngine);
	void setSoundEngine(SoundEngineInterface *soundEngine);
	void setNetworkEngine(NetworkEngineInterface *networkEngine);

	TimeEngineInterface* getTimeEngine() { return timeEng; }
	GraphicalEngineInterface* getGraphicalEngine() { return graphical; }
	InputEngineInterface* getInputEngine() { return input; }
	SoundEngineInterface* getSoundEngine() { return sound; }
	NetworkEngineInterface* getNetworkEngine() { return net; }

	void setDefaultConfig();

#ifdef PERFLOG
	void displayMeter(int x, int y, int value, int min, int max, int bad = -1, int good = -1);
#endif

	/** Display statistics.
	 * This function displays statistics on the screen about performance and engine status.
	 */
	void displayStats();

#ifdef DEBUG_KEYS
	/** Toggle sprite rendering.
	 */
	void switchRendering() { renderingEnabled = !renderingEnabled; }
#endif

	/* Time Engine */

	/** Return time elapsed since last call.
	 * @return the time elapsed since this function was last called. Time is in milliseconds.
	 */
	virtual dur getElapsedTime();

	/** Synchronize to the FPS timer.
	 * This function returns when the next frame is ready to be drawn.
	 * In some cases, on some plateforms, this timer can be linked to the VSync,
	 * leading to a perfect timing in a perfect world !!!
	 */
	virtual void syncToFrameRate();

	/** Sets desired frame rate.
	 * Please note that you will get better results by querying multiples
	 * of getNativeFrameRate() because VSync will help a lot.
	 * The engine has an internal PLL, so fps will have very high time
	 * precision if timers are high quality and multitasking does not get in
	 * the way.
	 * On most game consoles, timing will be perfect.
	 * @param fps the new frame rate in frames per second.
	 */
	virtual void setFrameRate(dur fps);

	/** Returns the current frame rate.
	 * @return the current frame rate, in frames per seconds.
	 */
	virtual dur getCurrentFPS();

	/* Graphical Engine */

	/** Returns the native frame rate.
	 * On most plateforms, this 60, but it may be something else.
	 */
	virtual dur getNativeFrameRate();

	/** Prepare the renderer for drawing a frame.
	 * This function must be called before starting drawing a frame.
	 * Once called, you can use draw.
	 * @see HardwareEngine::draw
	 */
	virtual void preDraw();

	/** Terminate rendering of a frame.
	 * This function must be called when a frame has finished its rendering.
	 * Beware that this function may or may not have VSYNC. If VSYNC is
	 * available on the plateform and is enabled, this function will not return until buffers
	 * have been swapped.
	 */
	virtual void postDraw();

	/** Draw a texture.
	 * This function draws a single texture at a given position with given effects into a target texture slot.
	 * @param texture the texture to draw.
	 * @param coordinates the coordinates of the texture.
	 * @param effects the effects to apply to the texture.
	 * @param targetTextureSlot the texture slot to draw to. For the main framebuffer, this value must be 0.
	 */
	virtual void draw(Texture* texture, Translatable* coordinates, GraphicalEffects* effects, int targetTextureSlot = 0);

	/** Draw a rectangle in wireframe mode.
	 * @param rect the rectangle to draw.
	 * @param r color red.
	 * @param g color green.
	 * @param b color blue.
	 * @param a color alpha.
	 */
	virtual void drawRect(Resizable* rect, unsigned int r, unsigned int g, unsigned int b, unsigned int a);

	/** Fill a white rectangle.
	 * This function fills a rectangle with a solid color and with given effects.
	 * Use color modulation effect to give a color to this rectangle.
	 * @param coordinates the position and size of the rectangle.
	 * @param effects effects to apply to this rectangle.
	 * @param targetTextureSlot the texture slot to draw to.
	 */
	virtual void fill(Resizable* coordinates, GraphicalEffects* effects, int targetTextureSlot = 0);

	/** Tells whether the user queried engine termination.
	 * @return true if the user asked the engine to terminate (for example by closing the engine's window), false otherwise.
	 */
	virtual bool terminationRequest();

	/** Loads a texture from a file.
	 * This function loads a texture from a file and puts it in graphical memory.
	 * @param textureFile the file to read the texture from.
	 * @return a pointer to the plateform-specific texture. Use this pointer with draw() or freeTexture(). Returns NULL if the texture is not found or the graphical memory is full.
	 */
	virtual Texture* loadTexture(VFS::ImageSource* textureFile);

	/** Frees a texture from graphical memory.
	 * This function frees graphical memory by deleting the given texture. Once freed, the texture pointer becomes invalid.
	 * @param texture the texture to remove from graphical memory.
	 * @return true if the texture was successfully freed, false otherwise.
	 */
	virtual bool freeTexture(Texture* texture);

	/** Create a temporary rendering surface (render to texture).
	 * WARNING: This function assert()s that the rendering surface was successfully allocated.
	 * @param minimumSurfaceSize the minimum size of the surface to render to. If the surface is less than one pixel, then the default size will be used (typically, the size of the screen).
	 * @param slots bitmask that indicates which texture slots to push. All subsequent draw operations must be in the slots defined as active.
	 */
	virtual void pushRenderingSurface(Translatable* minimumSurfaceSize = NULL, uint32_t slots = 1, void* preProcess = NULL, void* postProcess = NULL, int format = IMAGE_R8G8B8A8);

	/** Get the rendered temporary surface.
	 * The temporary surface is returned as a Renderable.
	 * WARNING: memory deallocation of the Renderable is up to the caller. It is initially not REFed to any object.
	 * @param attachedState the high-level state to attach the produced object to.
	 * @return the rendered surface, or NULL if there was a problem. NULL is returned if the current rendering surface is the screen.
	 */
	virtual Renderable* popRenderingSurface(State* attachedState);

	/** Loads an image processing filter.
	 * @param processString the string that describes the image processing effect.
	 * @param pspecData OUTPUT : returns an opaque pointer that must be passed to imageProcess() to apply the effect.
	 * @param neededSlots OUTPUT : returns the list of texture slots needed by this image processing filter. When creating the rendering surface, at least these slots must be pushed.
	 * @return true if the image processing filter has been successfully set up, false otherwise.
	 */
	virtual bool loadImageProcess(const char* processString, void* &pspecData, uint32_t &neededSlots);

	/** Unloads a post process filter.
	 * @param pspecData the opaque pointer passed by loadImageProcess.
	 */
	virtual void unloadImageProcess(void* &pspecData);

	/** Returns the size of the display surface.
	 * @return the size of the display surface, in pixels.
	 */
	virtual Resizable* getDisplaySize();

	/** Set the viewport.
	 * @param physicalViewport the rectangle defining physical drawing size.
	 * @param virtualViewport coordinates system of the viewport.
	 */
	virtual void setViewport(Resizable* physicalViewport = NULL, Resizable* virtualViewport = NULL);

	/** Resize the window containing the viewport.
	 */
	virtual void resizeWindow(Resizable* newSize);

	/* Input engine */
	virtual void processInput();
	virtual ControllerState* getControllerState(int controllerID = 0);

	virtual bool enableTextInput(int controllerID = 0);
	virtual bool disableTextInput(int controllerID = 0);
	virtual size_t readTextInput(char* buffer, size_t bufSize, int controllerID = 0);
	virtual size_t peekTextInput(char* buffer, size_t bufSize, int controllerID = 0);
	virtual size_t feedTextInput(char* buffer, size_t bufSize, int controllerID = 0);
	virtual void clearInputBuffer(int controllerID = 0);

#ifdef MOUSE_SUPPORT
	virtual bool absoluteMouse(int controllerID = 0);
	virtual bool relativeMouse(int controllerID = 0);
	virtual bool releaseMouse(int controllerID = 0);
	virtual void setMouseCursor(GameObject* newCursor, int controllerID = 0);
	virtual GameObject* getMouseCursor(int controllerID = 0);
#endif
	// Input configuration

	std::list<std::string> getInputHardwareDeviceList();
	std::list<std::string> getButtonList(const char* hardwareDeviceName);
	std::list<std::string> getAxesList(const char* hardwareDeviceName);
	std::list<std::string> getFeedbackAxesList(const char* hardwareDeviceName);
	const char* getActiveButton(const char* hardwareDeviceName);
	const char* getActiveAxis(const char* hardwareDeviceName);

	/* Sound Engine */
	virtual AudioInstance* instantiateSound(VFS::File* pcmFile, int audioAccessMode, int initialVolume, int priority, bool positioned, int loop, Translatable* position);
	virtual void freeAudioInstance(AudioInstance* instance);
	virtual bool play(AudioInstance* sound);
	virtual bool isPlaying(AudioInstance* sound);
	virtual void pause(AudioInstance* sound);
	virtual void rewind(AudioInstance* sound);

	/* Network Engine */
	virtual NetSocket* connect(const char* uri);
	virtual void disconnect(NetSocket* socket);
	virtual bool send(NetSocket* hub, const char* packet);
	virtual const char* receive(NetSocket* hub);
	virtual bool connected(NetSocket* socket);

	/* Background tasks */

	/** Add a background task to the task queue.
	 * The background task will be executed when a core will be free to execute the request.
	 * Background tasks are executed in a priority immediately lower than the main loop, so it will
	 * basically fit in the sleep on single core architectures.
	 * There must be no assumption on the order of execution of the tasks,
	 * as well as their relative priority. They also may execute on distinct CPU cores, so they must
	 * all use thread-safe code paths.
	 *
	 * Note that the scheduler cannot handle duplicate tasks : two tasks cannot have the same entry point and data pointer. An assertion will block this, but on production systems you must ensure that such condition will never happen.
	 *
	 * @param task the entry point of the background task.
	 * @param data the pointer to pass to the entry function.
	 * @return the position in the queue or -1 if the task has been executed in-place (because of a full queue or thread inavailability).
	 */
	int addBackgroundTask(ThreadEntryPoint task, void* data, int queue);

	/** Tell whether a background task is in the queue.
	 * @param task the entry point of the background task.
	 * @param data the pointer passed to the entry function.
	 * @return true if the task is in the pending list or running, false otherwise.
	 */
	bool hasBackgroundTask(ThreadEntryPoint task, void* data);

	/** Cancel a pending background task.
	 * This function will remove a pending task from the queue.
	 * It cannot abort a currently running task. The task will run to the end.
	 * @param task the entry point of the background task.
	 * @param data the pointer passed to the entry function.
	 * @return true if the task was removed or not in the queue, false if it is running.
	 */
	bool cancelBackgroundTask(ThreadEntryPoint task, void* data);

	/** Execute pending background tasks.
	 */
	void doBackgroundTasks();

	/** Block main thread until all background tasks are finished.
	 */
	void waitForBackgroundTasks();

};

#endif /* HARDWARE_ENGINE_H */
