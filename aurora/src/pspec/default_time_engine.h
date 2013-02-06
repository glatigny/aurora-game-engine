#ifndef DEFAULT_TIME_ENGINE_H
#define DEFAULT_TIME_ENGINE_H
#include "time_engine.h"

class DefaultTimeEngine : public TimeEngine {
	private:
	dur targetFrameTime;
	dur frameTime;
	dur delayLoop;
	dur lastGetTime;
	dur now;
	dur elapsedTime;
	dur processTime;
	int overload;
	int inSync;
#ifdef PERFLOG
	HardwareEngine* hardware;
#endif
	public:
	DefaultTimeEngine();
	virtual ~DefaultTimeEngine();

#ifdef PERFLOG
	virtual void init(HardwareEngine* newHardware) { hardware = newHardware; }
#endif

	virtual void preDraw();
	virtual void postDraw();
	virtual dur getElapsedTime();
	virtual void syncToFrameRate();
	virtual void setFrameRate(dur fps);
	virtual dur getCurrentFPS() { return 1000/frameTime; }
};

#endif /* DEFAULT_TIME_ENGINE_H */
