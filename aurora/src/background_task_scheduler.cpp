#include "background_task_scheduler.h"

extern "C" {

// This function runs in its own thread.
void runScheduling(void* schedulerPtr) {
	BackgroundTaskScheduler* scheduler = (BackgroundTaskScheduler*)schedulerPtr;


	for(;;) {
		// Wait for next event
		scheduler->schedulingThreadMutex.p();
		scheduler->schedulingThreadMutex.v();

		bool tasksWaiting = true;
		do {
			scheduler->taskMutex.p();
			if(!scheduler->runSchedulingThread) {
				xassert(!scheduler->running.entryPoint, "Stopping a task scheduler with running tasks");
				scheduler->runSchedulingThread = true;
				scheduler->taskMutex.v();
				return;
			}

			if(!scheduler->pending.empty()) {
				// Move from pending to running
				Task& running = scheduler->running;
				running = scheduler->pending.front();
				scheduler->pending.pop_front();

				// Get the task's entry point
				ThreadEntryPoint entryPoint = running.entryPoint;
				void* data = running.data;

				// Run the task
				scheduler->taskMutex.v();
				running.entryPoint(running.data);
				scheduler->taskMutex.p();

				// Remove the task
				running.entryPoint = NULL;
			} else {
				tasksWaiting = false;
			}
			scheduler->taskMutex.v();
		} while(tasksWaiting);
	}
}

} /* extern "C" */

// QC:A (TODO : detect number of CPU cores)
BackgroundTaskScheduler::BackgroundTaskScheduler(int threads) {
	if(threads == 0) {
		threads = getMaxThreads();
	}
	maxThreads = threads;

	running.entryPoint = NULL;

	// Start worker in blocked mode
	runSchedulingThread = true;
	schedulingThreadMutex.p();
	createThread(runScheduling, (void*)this, -1);
}

BackgroundTaskScheduler::~BackgroundTaskScheduler() {
	// Stop the worker thread.

	taskMutex.p();
	runSchedulingThread = false;
	taskMutex.v();

	while(true) {
		taskMutex.p();
		if(runSchedulingThread) {
			taskMutex.v();
			schedulingThreadMutex.v();
			return;
		}
		taskMutex.v();
		schedulingThreadMutex.v();
		schedulingThreadMutex.p();
	}
}

// QC:P
void BackgroundTaskScheduler::doBackgroundTasks() {
	taskMutex.p();
	pending.insert(pending.end(), waiting.begin(), waiting.end());
	taskMutex.v();
	schedulingThreadMutex.v();
	schedulingThreadMutex.p();
	waiting.clear();
}

// QC:?
void BackgroundTaskScheduler::waitForBackgroundTasks() {
	doBackgroundTasks();
	while(getTaskCount()) {
		waitMillis(20);
	}
}

// QC:P
int BackgroundTaskScheduler::getTaskCount() {
	taskMutex.p();

	int result = pending.size();
	result += waiting.size();
	if(running.entryPoint)
		result++;

	taskMutex.v();

	return result;
}

// QC:?
int BackgroundTaskScheduler::addBackgroundTask(ThreadEntryPoint task, void* data, int bgQueue) {
	// Add the task to the queue
	Task t;
	t.entryPoint = task;
	t.data = data;
	waiting.push_back(t);

	int taskPosition = waiting.size();
	return taskPosition;
}

// QC:?
bool BackgroundTaskScheduler::hasBackgroundTask(ThreadEntryPoint task, void* data) {
	taskMutex.p();

	if(running.entryPoint == task && running.data == data) {
		taskMutex.v();
		return true;
	}

	for(TaskList::iterator it = pending.begin(); it != pending.end(); it++) {
		if(it->entryPoint == task && it->data == data) {
			taskMutex.v();
			return true;
		}
	}

	taskMutex.v();
	return false;
}

bool BackgroundTaskScheduler::cancelBackgroundTask(ThreadEntryPoint task, void* data) {
	taskMutex.p();

	if(running.entryPoint == task && running.data == data) {
		taskMutex.v();
		return false;
	}

	for(TaskList::iterator it = pending.begin(); it != pending.end(); it++) {
		if(it->entryPoint == task && it->data == data) {
			pending.erase(it);
			taskMutex.v();
			return true;
		}
	}

	taskMutex.v();
	return true;
}
