#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <string.h>
#include "state.h"
#include "hardware_engine.h"
#include "aurora_engine.h"
#include "pspec/thread.h"
#include "vfs.h"

class State;
class AuroraEngine;
class HardwareEngine;
#ifdef THREADS_SUPPORT
template <class Resource> void bgPreloadResource(void* data);
#endif

/** Manages automatically resources provided by the hardware manager.
 * If the hardware manager is switched, resource is automatically reloaded if needed.
 */
template <class Resource> class ResourceManager {
private:
	HardwareEngine* hw;
	Resource* res;
	State* currentState;
	VFS::File* resFile;

public:
#ifdef THREADS_SUPPORT
	Mutex resMutex;
#endif

	// QC:?
	ResourceManager() :
		hw(NULL), res(NULL), currentState(NULL), resFile(NULL)
	{
	}

	// QC:P
	ResourceManager(State* attachedState, VFS::File* resourceFile) {
		assert(attachedState && attachedState->engine());

		currentState = attachedState;
		if(resourceFile) {
			resFile = resourceFile->copy();
			hw = currentState->engine()->hardware();
		} else {
			hw = NULL;
			resFile = NULL;
		}
		res = NULL;
	}

	ResourceManager(State* attachedState, Resource* volatileResource) : hw(attachedState->engine()->hardware()), res(volatileResource), currentState(attachedState), resFile(NULL) {
		assert(hw);
	}

	// QC:P
	virtual ~ResourceManager() {
		if(res) {
			resUnload(res);
		}
		if(resFile) {
			delete resFile;
		}
	}

	/** Interface used to load a resource.
	 * The implementation must overload this resource with a function that actually loads the resource.
	 * Loads a resource from a file and returns a descriptor to it.
	 * @param file the file to load resource from.
	 * @param returnValue the loaded resource.
	 * @return true if loading was successful, false otherwise.
	 */
	// QC:S (should be pure virtual)
	virtual bool resLoad(Resource* &returnValue, VFS::File* file) {
		returnValue = NULL;
		return false;
	}

	/** Interface used to free an allocated resource.
	 * The implementation must overload this resource with a function that actually frees the resource.
	 * @param r the resource to free.
	 */
	// QC:S (should be pure virtual)
	virtual void resUnload(Resource* r) {
	}

	/** Returns the current resource.
	 * This function returns the current resource. If the hardware engine has changed, the resource is reloaded
	 * before being returned.
	 * @param resource will be set to the loaded resource, or NULL if no resource is available.
	 * @return true if loading was successful, false otherwise.
	 */
	// QC:P
	bool getResource(Resource* &resource) {
#ifdef THREADS_SUPPORT
		resMutex.p();
#endif
		if(hw != currentState->engine()->hardware()) {
			hw = currentState->engine()->hardware();
			res = NULL;
		}

		if(res == NULL && hw != NULL && resFile) {
			xadvice(!res, "Resource %s not preloaded.", resFile->name());
			if(!resLoad(res, resFile)) {
				resource = NULL;
				return false;
			}
		}

#ifdef THREADS_SUPPORT
		resMutex.v();
#endif

		resource = res;
		return true;
	}

	/** Changes the resource's file.
	 * The previous resource is freed, if any.
	 * @param referenceResource a pointer to the pointer of the resource.
	 * @param newFile the new file to point to.
	 */
	void switchResource(Resource* &referenceResource, VFS::File* newFile) {
		bool reloadRes = (res != NULL);
		freeResource();
		if(resFile) {
			delete resFile;
		}
		if(newFile) {
			resFile = newFile->copy();
			if(reloadRes)
				preloadResource();
		} else {
			referenceResource = NULL;
			resFile = NULL;
		}
	}

	/** Forces loading of the current resource.
	 * The resource is loaded immediately.
	 */
	// QC:P
	void loadResource() {
#ifdef THREADS_SUPPORT
		resMutex.p();
#endif
		if(hw != currentState->engine()->hardware()) {
			hw = currentState->engine()->hardware();
			res = NULL;
		}

		if(res == NULL && hw != NULL && resFile) {
			if(!resLoad(res, resFile)) {
				res = NULL;
			}
		}
#ifdef THREADS_SUPPORT
		resMutex.v();
#endif
	}

	/** Starts loading resource in a background thread.
	 * The function returns immediately but the resource may not be available instantly.
	 * If the loading process is not finished when getResource is called, getResource is interrupted until the resource is loaded by the background thread, exactly like if it were not loaded.
	 */
	// QC:W
	void preloadResource() {
#if defined(THREADS_SUPPORT) && defined(BG_PRELOAD)
		if(!res) {
			hw->addBackgroundTask(bgPreloadResource<Resource>, (void*)(ResourceManager<Resource>*)this, 1);
		}
#else
		xadvice(false, "No threads support. Preloading in foreground.");
		loadResource();
#endif
	}

	/** Forces unloading of the current resource.
	 * The resource is freed immediately.
	 */
	// QC:P
	void freeResource() {
		if(res) {
			resUnload(res);
			res = NULL;
		}
	}

	VFS::File* source() {
		return resFile;
	}

};

#ifdef THREADS_SUPPORT
template <class Resource> void bgPreloadResource(void* data) {
	ResourceManager<Resource>* manager = (ResourceManager<Resource>*)data;
	manager->loadResource();
}
#endif /* THREADS_SUPPORT */

#endif /* RESOURCE_MANAGER_H */
