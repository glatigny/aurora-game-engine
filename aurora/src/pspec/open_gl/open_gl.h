#ifndef OPEN_GL_H
#define OPEN_GL_H

// Number of pre-allocated textures
#define PERMANENT_TEX 4
// Pre-allocated texture size
#define PERMANENT_TEX_W nextPowerofTwo(displayWidth)
#define PERMANENT_TEX_H nextPowerofTwo(displayHeight)
// Pre-allocated texture format
#define PERMANENT_TEX_F IMAGE_R8G8B8A8

#ifndef WIN32
#include "glx_init.h"
#else
#include "wgl_init.h"
#endif

#include "hardware_engine.h"
#include "rect.h"
#include "pspec/thread.h"

struct OGLImageProcess;
#ifdef GPU_IMAGEPROCESS
struct OGLImageProcess {
	GLuint program;
	GLint textureLocations[TEXTURE_SLOTS];
	GLint widthLocation;
	GLint heightLocation;
	bool use_mipmaps;
	uint32_t slots;
	GLclampf clearColors[TEXTURE_SLOTS][4];
	OGLImageProcess() {
		for(int i=0; i<TEXTURE_SLOTS; i++) {
			textureLocations[i] = 0;
			clearColors[i][0] = 0.0f;
			clearColors[i][1] = 0.0f;
			clearColors[i][2] = 0.0f;
			clearColors[i][3] = 0.0f;
		}
	}
};
#endif

#ifdef GPU_IMAGEPROCESS
#include "gl_shader_manager.h"
#endif

// Supported features flags.
extern bool oglFBOSupport;
extern bool oglShadersSupport;
extern bool oglImagingSupport;
extern bool oglSeparateBlending;
extern bool oglMultitexturing;

class GraphicalEngineOGL: public GraphicalEngine {
	friend class InputEngineOGL;
	friend void run();
protected:
	GraphicalEngineOGL(int width, int height, int refresh, bool fullscreen, VFS::File* processingEffects);
	bool init;
	Mutex uploadMutex;

	class RenderingSurfaceDescriptor {
	public:
#ifdef OPENGL_ENABLE_FBO
		uint32_t slots;
		GLuint targetSurfaces[TEXTURE_SLOTS];
#else
		GLuint saveunder;
#endif
		int px;
		int py;
		int pw;
		int ph;

		int vx;
		int vy;
		int vw;
		int vh;
	};

	// Requested display size
	int displayWidth;
	int displayHeight;
	Rect displaySize;

	// Display offset for letterboxing
	int displayOffsetX;
	int displayOffsetY;

#ifdef OPENGL_ENABLE_FBO
	GLuint fbo;
	GLuint currentRenderingTarget;
	void setRenderingTarget(GLuint target);
#endif

	RenderingSurfaceDescriptor rsd;
	std::list<RenderingSurfaceDescriptor> rsdList;

	GLuint createRenderingSurface(int tw, int th, int format);

#ifdef GPU_IMAGEPROCESS
	ShaderManager shaderManager;
	uint32_t slots;
#endif

	GLuint permanentTex[PERMANENT_TEX]; // Permanent textures.
	uint32_t permanentTexUsed; // Used permanent textures


#if defined(WIN32) && defined(BG_PRELOAD)
	HDC hDC;
	HGLRC mainGLContext;
	HGLRC workerGLContext;
	HANDLE mainThread;
#endif

// Temporary values
	float x;
	float y;
	float w;
	float h;
	float ow;
	float oh;

	float u1;
	float v1;
	float u2;
	float v2;

	void uploadTexture(Texture* texture, bool setTexture = true);

	GLuint texture;
	GraphicalEffects* effects;

	unsigned char opacity;

	/** Specific hardware Engine texture loading
	 * @param filename of texture we want to load
	 * @return texture pointer or NULL if failed
	 */
	Texture* myLoadTexture(VFS::ImageSource* p_file);
	/** Specific hardware Engine texture releasing
	 * @param texture pointer we want to release
	 * @return result of the operation
	 */
	bool myFreeTexture(Texture* p_texture);

	void beginPrimitiveDraw();
	void endPrimitiveDraw();

	void applyPreEffects();
	void applyCustomEffects();

	void applyPostEffects();
	void applyPostCustomEffects();

	void preProcess(GLuint textures[TEXTURE_SLOTS], OGLImageProcess* process);

	void draw(int slot);
	void fill(int slot);

	// Custom effects
	void processMBLR(unsigned char amount, unsigned char r, unsigned char g, unsigned char b, unsigned char a, int angle);
	void processBLUR(coord x, coord y);
	void processRBLR(coord x, coord y);

public:
	~GraphicalEngineOGL();

	static GraphicalEngineOGL* open(int width, int height, int refresh, bool fullscreen, VFS::File* processingEffects);

	bool myFreeTexture(GLuint texture);

	/** Specific hardware Engine initialization of Graphic Engine.
	 */
	virtual void initGraphics(int width, int height, int refresh, bool fullscreen);

	/* GraphicalEngine */

	void preDraw();
	void postDraw();
	void draw(Texture* texture, Translatable* coordinates, GraphicalEffects* effects, int slot);
	void drawRect(Resizable* rect, unsigned int r, unsigned int g, unsigned int b, unsigned int a);
	void drawWithProcessing(GLuint textures[TEXTURE_SLOTS], int width, int height, Translatable* coordinates, GraphicalEffects* effects);
	void fill(Resizable* coordinates, GraphicalEffects* effects, int slot = 0);
	bool terminationRequest();
	void pushRenderingSurface(Translatable* minimumSurface, uint32_t slots, void* preProcess, void* postProcess, int format);
	Renderable* popRenderingSurface(State* attachedState);
	virtual bool loadImageProcess(const char* processString, void* &pspecData, uint32_t &neededSlots);
	virtual void unloadImageProcess(void* &pspecData);

	dur getNativeFrameRate() {
		return 60; /* TODO : Get actual refresh rate and VSync */
	}

	Resizable* getDisplaySize() {
		return &displaySize;
	}

	virtual void resizeWindow(Resizable* newSize);
	virtual void setViewport(Resizable* physicalViewport = NULL, Resizable* virtualViewport = NULL);
	void resetViewport();
};

#endif /* OPEN_GL_H */
