#ifndef BACKGROUND_TASK_SCHEDULER_H
#define BACKGROUND_TASK_SCHEDULER_H

#ifdef THREADS_SUPPORT
#define MAX_THREADS 8
#include "pspec/thread.h"
#include <list>
#endif

struct Task {
	ThreadEntryPoint entryPoint;
	void* data;
};

class BackgroundTaskScheduler {
public:
	typedef std::list<Task> TaskList;

#ifdef THREADS_SUPPORT
	Mutex schedulingThreadMutex;
	
	bool runSchedulingThread;
	Task running;
	uint32_t bgQueueLocks;
	TaskList waiting;
	TaskList pending;
	Mutex taskMutex;

	int maxThreads;

	BackgroundTaskScheduler(int threads = 0);
	~BackgroundTaskScheduler();

	int addBackgroundTask(ThreadEntryPoint task, void* data, int bgQueue = 0);
	bool hasBackgroundTask(ThreadEntryPoint task, void* data);
	bool cancelBackgroundTask(ThreadEntryPoint task, void* data);
	void doBackgroundTasks();
	int getTaskCount();
	void waitForBackgroundTasks();
#else
	int addBackgroundTask(ThreadEntryPoint task, void* data, int bgQueue = 0) {
		task(data);
		return 0;
	}
	bool hasBackgroundTask(ThreadEntryPoint task, void* data) { return false; }
	bool cancelBackgroundTask(ThreadEntryPoint task, void* data) { return true; }
	int getTaskCount() { return 0; }
	void doBackgroundTasks() {}
	void waitForBackgroundTasks() {}
#endif
};

#endif /* BACKGROUND_TASK_SCHEDULER_H */
