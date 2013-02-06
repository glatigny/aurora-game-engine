#include "default_time_engine.h"
#ifdef PERFLOG
#include "hardware_engine.h"
#endif

#ifndef DEFAULT_FPS
#define DEFAULT_FPS 60
#endif

#ifdef WIN32
#include <mmsystem.h>
#endif

DefaultTimeEngine::DefaultTimeEngine() : targetFrameTime(1000/DEFAULT_FPS), frameTime(1000/DEFAULT_FPS), delayLoop(0), lastGetTime(0), now(targetFrameTime), elapsedTime(targetFrameTime), overload(0), inSync(-10)
#ifdef PERFLOG
	, hardware(NULL)
#endif
{
#ifdef WIN32
	timeBeginPeriod(1);
	
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
#endif
}

DefaultTimeEngine::~DefaultTimeEngine() {
#ifdef WIN32
  	timeEndPeriod(1);
#endif
}

// QC:?
void DefaultTimeEngine::preDraw() {
	if(lastGetTime) {
		lastGetTime = now;
		now = getTime();
	} else {
		now = getTime();
		lastGetTime = now - 1000 / targetFrameTime;
	}

	elapsedTime = now - lastGetTime;

	if(elapsedTime >= 1 && elapsedTime < 1000) {
		frameTime = (frameTime * 3 + elapsedTime) / 4; // Filter FPS variations

		if(frameTime < targetFrameTime && delayLoop < targetFrameTime/2) {
			delayLoop++;
		} else if(frameTime > targetFrameTime && delayLoop > -targetFrameTime/2) {
			delayLoop--;
		}

	} else {
		// Obviously, there was a skip : just ignore it.
		elapsedTime = targetFrameTime;
		lastGetTime = now - elapsedTime;
	}

	assert(absd(elapsedTime - (now - lastGetTime) <= 0.01));
}

static int lastWait = 0;

// QC:P
void DefaultTimeEngine::postDraw() {
	processTime = getTime() - now;
	assert(absd(elapsedTime - (now - lastGetTime) <= 0.01));

#ifdef PERFLOG
	if(hardware) {
		//hardware->displayMeter(2, 0, inSync, 0, 6, 4, 5);
		//hardware->displayMeter(2, 1, lastWait, 0, targetFrameTime);
		//hardware->displayMeter(3, 1, delayLoop, -targetFrameTime / 2, targetFrameTime / 2);
	}
#endif
}

// QC:?
void DefaultTimeEngine::syncToFrameRate() {
#ifdef PERFLOG
	lastWait = 0;
#endif
	dur remainingTime = targetFrameTime - (getTime() - now);
	assert(targetFrameTime - remainingTime >= processTime);

	if(overload > 0) {
		if(overload > 30) {
			xadvice(overload > 30, "### ENGINE IS ON FIRE ### - delayLoop = %d", delayLoop);
			overload = 0;
			delayLoop = 0;
			return;
		}
		overload--;
	}

	if(
		(remainingTime < 5) // Not much remaining time
		&& (frameTime <= targetFrameTime + 2 && frameTime >= targetFrameTime - 1) // FPS correct
		&& (processTime < targetFrameTime - 7) // Process time reasonable
	) {
		if(inSync < 8)
			inSync += 2;
	}

	if(inSync > 0 && frameTime <= targetFrameTime - 2)
		inSync--;

	if(inSync > 6) {
		if(delayLoop < 0)
			delayLoop++;
		else if(delayLoop > 0)
			delayLoop--;
		return;
	} else if(delayLoop <= -targetFrameTime) {
		// slow PC.
		overload += 10;
		return;
	}

	if(inSync > 0)
		inSync--;

	if(remainingTime + delayLoop < 0) {
		overload += 5;
		return;
	}

#ifdef PERFLOG
	lastWait = remainingTime + delayLoop;
#endif
	waitMillis(remainingTime + delayLoop);
}

// QC:?
dur DefaultTimeEngine::getElapsedTime() {
#ifdef TIME_BASED
	return elapsedTime;
#else
	return targetFrameTime;
#endif
}

// QC:?
void DefaultTimeEngine::setFrameRate(dur newFPS) {
	xerror(newFPS >= 5 && newFPS < 120, "Frame rate out of range :%d", newFPS);
	targetFrameTime = 1000/newFPS;
	delayLoop = 0;
}

