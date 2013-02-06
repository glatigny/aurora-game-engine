#include "thread.h"

typedef struct ThreadParameters
{
	ThreadEntryPoint entryPoint;
	int priority;
	void* parameters;
} ThreadParameters;

#ifdef WIN32

DWORD WINAPI threadProxy(LPVOID threadParameters)
{
	ThreadParameters* thread = (ThreadParameters*)(threadParameters);
	SetThreadPriority(GetCurrentThread(), thread->priority);
	thread->entryPoint(thread->parameters);
	delete thread;
	return 0;
}

void createThread(ThreadEntryPoint entryPoint, void* parameters, int priority)
{
	ThreadParameters* threadParameters = new ThreadParameters;
	threadParameters->entryPoint = entryPoint;
	threadParameters->parameters = parameters;
	switch( priority )
	{
		case 2:
			threadParameters->priority = THREAD_PRIORITY_HIGHEST;
			break;
		case 1:
			threadParameters->priority = THREAD_PRIORITY_ABOVE_NORMAL;
			break;
		case -1:
			threadParameters->priority = THREAD_PRIORITY_BELOW_NORMAL;
			break;
		case 0:
		default:
			threadParameters->priority = THREAD_PRIORITY_NORMAL;
			break;
	}
	
	CreateThread(NULL, NULL, threadProxy, threadParameters, NULL, NULL);
}

#else

void* threadProxy(void* threadParameters)
{
	ThreadParameters* thread = (ThreadParameters*)(threadParameters);
	thread->entryPoint(thread->parameters);
	delete thread;
	return NULL;
}

void createThread(ThreadEntryPoint entryPoint, void* parameters, int priority)
{
	ThreadParameters* threadParameters = new ThreadParameters;
	threadParameters->entryPoint = entryPoint;
	threadParameters->parameters = parameters;
	
	pthread_t thread;
	pthread_create( &thread, NULL, threadProxy, threadParameters );
}

#endif

int getMaxThreads() {
#ifdef THREADS_SUPPORT
	return 1; // TODO
#else
	return 0;
#endif
}
