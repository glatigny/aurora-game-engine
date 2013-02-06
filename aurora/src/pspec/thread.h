#ifndef THREAD_H
#define THREAD_H

typedef void (*ThreadEntryPoint)(void*);

/** Return the maximum number of concurrent threads (CPU cores). */
int getMaxThreads();

#ifdef THREADS_SUPPORT
void createThread(ThreadEntryPoint entryPoint, void* parameters, int priority);

#ifdef WIN32
class Mutex {
private:
	HANDLE mutex;
public:
	Mutex() {
		mutex = CreateMutex(NULL, FALSE, NULL);
	}

	~Mutex() {
		CloseHandle(mutex);
	}

	/** Lock */
	bool p() {
		WaitForSingleObject(mutex, INFINITE);
		return true;
	}

	/** Unlock */
	bool v() {
		ReleaseMutex(mutex);
		return true;
	}
};
#else /* WIN32 */
#include <pthread.h>
class Mutex {
private:
	pthread_mutex_t mutex;
	pthread_mutexattr_t mutexattr;
public:
	Mutex() {
		pthread_mutexattr_init(&mutexattr);
		pthread_mutex_init(&mutex, &mutexattr);
	}

	~Mutex() {
		pthread_mutex_destroy(&mutex);
		pthread_mutexattr_destroy(&mutexattr);
	}

	/** Lock */
	bool p() {
		return pthread_mutex_lock(&mutex) == 0;
	}

	/** Unlock */
	bool v() {
		return pthread_mutex_unlock(&mutex) == 0;
	}
};
#endif /* WIN32 */

#else /* THREADS_SUPPORT */

inline void createThread(ThreadEntryPoint entryPoint, void* parameters, int priority) {
	entryPoint(parameters);
}

class Mutex {
public:
	bool p() { return true; }
	bool v() { return true; }
};

#endif /* THREADS_SUPPORT */

class SharedMutex {
private:
	struct MutexContainer {
		Mutex llMutex;
		int useCount;

		MutexContainer() : useCount(0) {}
	};

	MutexContainer* mc;
public:
	SharedMutex() {
		mc = new MutexContainer();
	}

	SharedMutex(const SharedMutex& other) {
		mc = other.mc;
		mc->useCount++;
	}

	~SharedMutex() {
		mc->useCount--;
		if(mc->useCount == 0) {
			delete mc;
		}
	}

	bool operator==(const SharedMutex& other) const {
		return mc == other.mc;
	}

	bool p() { return mc->llMutex.p(); }
	bool v() { return mc->llMutex.v(); }
};

#endif /* THREAD_H */
