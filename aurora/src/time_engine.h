#ifndef TIME_ENGINE
#define TIME_ENGINE

class HardwareEngine;

class TimeEngineInterface {
	public:
	virtual ~TimeEngineInterface() {}
	virtual void init(HardwareEngine* hw) {}

	virtual void preDraw() = 0;
	virtual void postDraw() = 0;
	virtual dur getElapsedTime() = 0;
	virtual void syncToFrameRate() = 0;
	virtual void setFrameRate(dur fps) = 0;
	virtual dur getCurrentFPS() = 0;
};

class TimeEngine : public TimeEngineInterface {
};

#endif /* TIME_ENGINE */
