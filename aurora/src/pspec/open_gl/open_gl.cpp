#include <assert.h>
#include "open_gl.h"
#include "pspec/utils.h"
#include <iostream>
#include "renderable.h"
#include "state.h"
#include "frame.h"
#include "gl_framebuffer.h"

#include <tinyxml.h>

bool oglFBOSupport = true;
bool oglShadersSupport = true;
bool oglImagingSupport = true;
bool oglSeparateBlending = true;
bool oglMultitexturing = true;

#define ZOOMOUT 1

GraphicalEngineOGL* GraphicalEngineOGL::open(int width, int height, int refresh, bool fullscreen, VFS::File* processingEffects) {
	return new GraphicalEngineOGL(width, height, refresh, fullscreen, processingEffects);
}

GraphicalEngineOGL::GraphicalEngineOGL(int width, int height, int refresh, bool fullscreen, VFS::File* processingEffects) :
	GraphicalEngine()
#ifdef GPU_IMAGEPROCESS
	, shaderManager(processingEffects)
#endif
{
	init = false;
}

GraphicalEngineOGL::~GraphicalEngineOGL() {
#ifdef OPENGL_ENABLE_FBO
	if(oglFBOSupport)
		glDeleteFramebuffersEXT(1, &fbo);
#endif

	for(int i=0; i<PERMANENT_TEX; i++) {
		glDeleteTextures(1, &permanentTex[i]);
		AOEVIDFREE((void*)permanentTex[i]);
	}
}

// QC:G
void GraphicalEngineOGL::initGraphics(int width, int height, int refresh, bool fullscreen) {
	
	displayWidth = width;
	displayHeight = height;
	displaySize = Rect(0, 0, displayWidth, displayHeight);

	displayOffsetX = 0;
	displayOffsetY = 0;

	rsd.px = displayOffsetX;
	rsd.py = displayOffsetY;
	rsd.pw = displayWidth;
	rsd.ph = displayHeight;

	rsd.vx = 0;
	rsd.vy = 0;
	rsd.vw = displayWidth;
	rsd.vh = displayHeight;

#ifdef WIN32
	if(!wglInit())
	{
		xwarn(false, "Could not initialize OpenGL properly : there were missing functions.");
	}

#endif // WIN32
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(displayOffsetX, displayOffsetY, displayWidth, displayHeight);
	glLoadIdentity();
	glOrtho(- displayWidth * (ZOOMOUT-1), displayWidth * ZOOMOUT, displayHeight * ZOOMOUT, - displayHeight * (ZOOMOUT-1), 1, -1);

	glEnable(GL_TEXTURE_2D);
	glDepthFunc(GL_ALWAYS);
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	if(oglImagingSupport) {
		glBlendEquation(GL_FUNC_ADD);
	}
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	permanentTexUsed = (1<<PERMANENT_TEX) - 1;
	for(int i=0; i<PERMANENT_TEX; i++) {
		permanentTex[i] = createRenderingSurface(PERMANENT_TEX_W, PERMANENT_TEX_H, PERMANENT_TEX_F);
	}
	permanentTexUsed = 0;
	
#ifdef OPENGL_ENABLE_FBO
	if(oglFBOSupport) {
		glGenFramebuffersEXT(1, &fbo);
		currentRenderingTarget = 0;
	}
#endif

#ifdef GPU_IMAGEPROCESS
	if(oglShadersSupport) {
		shaderManager.loadShaders();
	}
#endif

	// Compute functionality dependencies
	if(!oglFBOSupport) {
		oglShadersSupport = false;
	}

	if(!oglMultitexturing) {
		oglShadersSupport = false;
	}

#if defined(WIN32) && defined(BG_PRELOAD)
	mainThread = GetCurrentThread();
	hDC = wglGetCurrentDC();
	mainGLContext = wglGetCurrentContext();
	workerGLContext = wglCreateContext(hDC);
	wglMakeCurrent(hDC, mainGLContext);
	wglShareLists(workerGLContext, mainGLContext);
#endif

#ifdef WIN32
	typedef void (APIENTRY * WGLSWAPINTERVALEXT) (int);

	WGLSWAPINTERVALEXT wglSwapIntervalEXT = (WGLSWAPINTERVALEXT) 
	wglGetProcAddress("wglSwapIntervalEXT");
	if (wglSwapIntervalEXT)
	{
		wglSwapIntervalEXT(1); // set vertical synchronisation
	}
#endif

	init = true;
}

void GraphicalEngineOGL::resizeWindow(Resizable* newSize) {
	if(!rsdList.empty()) {
		return;
	}

	int width = newSize->getW();
	int height = newSize->getH();

	displayWidth = width;
	displayHeight = height;
	displaySize = Rect(0, 0, displayWidth, displayHeight);

	displayOffsetX = 0;
	displayOffsetY = 0;

	rsd.px = displayOffsetX;
	rsd.py = displayOffsetY;
	rsd.pw = displayWidth;
	rsd.ph = displayHeight;

	rsd.vx = newSize->getX();
	rsd.vy = newSize->getY();
	rsd.vw = displayWidth;
	rsd.vh = displayHeight;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(displayOffsetX, displayOffsetY, displayWidth, displayHeight);
	glLoadIdentity();
	glOrtho(- displayWidth * (ZOOMOUT-1) + rsd.vx, displayWidth * ZOOMOUT + rsd.vx, displayHeight * ZOOMOUT + rsd.vy, - displayHeight * (ZOOMOUT-1) + rsd.vy, 1, -1);
}

void GraphicalEngineOGL::setViewport(Resizable* physicalViewport, Resizable* virtualViewport) {
	assert(physicalViewport);
	assert(virtualViewport);

	rsd.px = physicalViewport->getX() + displayOffsetX;
	rsd.py = physicalViewport->getY() + displayOffsetY;
	rsd.pw = physicalViewport->getW();
	rsd.ph = physicalViewport->getH();

	if(rsd.px < displayOffsetX)
		rsd.px = displayOffsetX;
	else if(rsd.px >= displayWidth)
		rsd.px = displayWidth + displayOffsetX - 1;

	if(rsd.py < displayOffsetY)
		rsd.py = displayOffsetY;
	else if(rsd.py >= displayHeight)
		rsd.py = displayWidth + displayOffsetY - 1;

	if(rsd.pw < 1)
		rsd.pw = 1;
	else if(rsd.pw > displayWidth - rsd.px - displayOffsetX)
		rsd.pw = displayWidth - rsd.px - displayOffsetX;

	if(rsd.ph < 1)
		rsd.ph = 1;
	else if(rsd.ph > displayHeight - rsd.py - displayOffsetY)
		rsd.ph = displayHeight - rsd.py - displayOffsetY;

	rsd.vx = virtualViewport->getX();
	rsd.vy = virtualViewport->getY();
	rsd.vw = virtualViewport->getW();
	rsd.vh = virtualViewport->getH();

	resetViewport();
}

void GraphicalEngineOGL::resetViewport() {
	glViewport(rsd.px, rsd.py, rsd.pw, rsd.ph);
	glLoadIdentity();
	glOrtho(rsd.vx, rsd.vw + rsd.vx, rsd.vh + rsd.vy, rsd.vy, 1, -1);
}

// QC:?
Texture* GraphicalEngineOGL::myLoadTexture(VFS::ImageSource* src) {
	if(!init) {
		return NULL;
	}

	// Creation d'un objet texture completement vide
	Texture *n_texture = new Texture;
	
	char* pixels;
	Rect bufferSize;
	size_t bufferLen;
	size_t pitch;
	int format = IMAGE_R8G8B8A8; // Preferred format
	Rect imageSize;
	src->load(pixels, bufferSize, bufferLen, pitch, format, imageSize, true);

	// Affectation des informations
	n_texture->vData = 0;
	n_texture->sData = pixels;
	n_texture->format = format;
	n_texture->bufferLen = bufferLen;
	n_texture->pitch = pitch;
	n_texture->w = bufferSize.getW();
	n_texture->h = bufferSize.getH();
	n_texture->ow = imageSize.getW();
	n_texture->oh = imageSize.getH();

#if defined(WIN32) && defined(BG_PRELOAD)
	if(GetCurrentThread() != mainThread) {
		if(wglGetCurrentContext() != workerGLContext)
			wglMakeCurrent(hDC, workerGLContext);

		uploadTexture(n_texture, false);

		xadvice(GetCurrentThread() == mainThread, "Texture %s not preloaded.", src->source()->name());
	}
#endif

	return n_texture;
}

// QC:?
void GraphicalEngineOGL::uploadTexture(Texture *hlTexture, bool setTexture) {
	uploadMutex.p();

	if(hlTexture->vData) {
		if(setTexture) {
			// Get already loaded texture.
			texture = (GLuint)hlTexture->vData;
		}

		uploadMutex.v();
		return;
	}

	// Creation
	GLuint tex;
	glGenTextures(1, &tex);
	xassert(tex, "OpenGL could not create texture.");
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	switch(hlTexture->format) {
		case IMAGE_R8G8B8:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, hlTexture->w, hlTexture->h, 0, GL_RGB, GL_UNSIGNED_BYTE, hlTexture->sData);
			break;
		case IMAGE_R8G8B8A8:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, hlTexture->w, hlTexture->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, hlTexture->sData);
			break;
		case IMAGE_G8:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8, hlTexture->w, hlTexture->h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, hlTexture->sData);
			break;
		case IMAGE_G8A8:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8_ALPHA8, hlTexture->w, hlTexture->h, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, hlTexture->sData);
			break;
		default:
			xerror(false, "The OpenGL backend cannot handle images with format %d", hlTexture->format);
			break;
	}
	AOEVIDALLOC((void*)tex, hlTexture->bufferLen);

	// Affectation de l'image
	hlTexture->vData = ((void*) tex);

	// Libere la RAM systeme
	AOEFREE(hlTexture->sData);
	hlTexture->format = IMAGE_BLANK;

	if(setTexture) {
		texture = tex;
	}

	uploadMutex.v();
}

// QC:G
bool GraphicalEngineOGL::myFreeTexture(Texture *p_texture) {
	if(p_texture->format != IMAGE_BLANK)
		AOEFREE(p_texture->sData);
	if(p_texture->vData)
		myFreeTexture((GLuint)p_texture->vData);
	delete p_texture;
	return true;
}

bool GraphicalEngineOGL::myFreeTexture(GLuint texture) {
	if(permanentTexUsed) {
		for(int i=0; i<PERMANENT_TEX; i++) {
			if(permanentTex[i] == texture) {
				xassert(permanentTexUsed & (1<<i), "Permanent texture used flag is not in sync.");
				permanentTexUsed &=~ (1<<i);
				return true;
			}
		}
	}
	glDeleteTextures(1, &texture);
	AOEVIDFREE((void*)texture);
	return true;
}

// QC:? (TODO : add uniform parameters support)
bool GraphicalEngineOGL::loadImageProcess(const char* processString, void* &pspecData, uint32_t &neededSlots) {
#ifdef GPU_IMAGEPROCESS
	xassert(oglShadersSupport, "Image processing needs shaders.");
	xassert(strnlen(processString, 64) < 64, "Process string too long.");
	char filename[64];
	strcpy(filename, processString);
	char* paramsep = strchr(filename, ':');
/*	coord p0;
	coord p1;
	coord p2;
	coord p3;
	int pcount = stringToCoordList(paramsep+1, &p0, &p1, &p2, &p3);*/
	if(paramsep)
		*paramsep = '\0';
	
	bool success = shaderManager.getProcess(filename, pspecData, neededSlots);

	return success;
#else /* GPU_IMAGEPROCESS */
	return false;
#endif /* GPU_IMAGEPROCESS */
}

// QC:P
void GraphicalEngineOGL::unloadImageProcess(void* &pspecData) {
	// Nothing to unload since all shaders are loaded at instantiation
}

// QC:G
void GraphicalEngineOGL::preDraw() {
	xassert(init, "Engine not initialized.");

	if(rsdList.empty()) {
#ifdef GPU_IMAGEPROCESS
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
#endif
		glClear(GL_COLOR_BUFFER_BIT);
	}

	assert(rsdList.empty());
}

// QC:G
void GraphicalEngineOGL::postDraw() {
	assert(rsdList.empty());
	xassert(init, "Engine not initialized.");

	//glFinish();
}

// QC:?
inline void GraphicalEngineOGL::beginPrimitiveDraw() {
	// Save initial matrix
	glPushMatrix();

	// Put the sprite at its final position
	glTranslatef(x, y, 0);
}

// QC:?
inline void GraphicalEngineOGL::applyPreEffects() {
	unsigned char redFilter = 255;
	unsigned char greenFilter = 255;
	unsigned char blueFilter = 255;
	opacity = 255;

	if(effects->hasBasicEffects()) {
		glPushAttrib(GL_COLOR_BUFFER_BIT); // TODO

		opacity = effects->getOpacity();

		Rect section(*(effects->getSection()));
		// Clip the section inside image boundaries
		if(section.getW() >= 0 || section.getH() >= 0) {
			section.setX(CLIP(section.getX(), 0, ow));
			section.setY(CLIP(section.getY(), 0, oh));
			section.setW(CLIP(section.getW(), 0, ow - section.getX()));
			section.setH(CLIP(section.getH(), 0, oh - section.getY()));
			u1 = (section.getLeftmostX()) / ow;
			v1 = (section.getTopmostY()) / oh;
			u2 = (section.getRightmostX()) / ow;
			v2 = (section.getBottommostY()) / oh;

			x += section.getX();
			y += section.getY();
			ow = section.getW();
			oh = section.getH();
		}

		// Set the blending mode
		switch(effects->getBlendMode()){
		case BLEND_INHERIT:
		case BLEND_NORMAL:
			// Stay in normal mode
			break;

		case BLEND_ADD:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			break;

		case BLEND_MULTIPLY:
			glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
			break;

		case BLEND_SUB:
			if(oglImagingSupport) {
				glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
			}

		case BLEND_ADD_ALPHA:
			if(oglSeparateBlending) {
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
			} else {
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			}
			break;

		case BLEND_ADDMUL:
			glBlendFunc(GL_DST_COLOR, GL_ONE);
			break;

		case BLEND_INVERSE:
			glPixelTransferf(GL_RED_SCALE, -1.0f);
			glPixelTransferf(GL_GREEN_SCALE, -1.0f);
			glPixelTransferf(GL_BLUE_SCALE, -1.0f);
			glPixelTransferf(GL_RED_BIAS, 1.0f);
			glPixelTransferf(GL_GREEN_BIAS, 1.0f);
			glPixelTransferf(GL_BLUE_BIAS, 1.0f);
			break;

		case BLEND_ALPHAMASK:
			if(oglSeparateBlending) {
				glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO);
			} else {
				glBlendFunc(GL_ZERO, GL_ONE); // Do not display anything
			}
			break;

		case BLEND_ALPHALOCK:
			if(oglSeparateBlending) {
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
			} else {
				glBlendFunc(GL_ZERO, GL_ONE);
			}
			break;

		case BLEND_ADD_ALPHALOCK:
			if(oglSeparateBlending) {
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
			} else {
				glBlendFunc(GL_ZERO, GL_ONE);
				break;
			}

		}

		glRotatef(effects->getRotation(), 0.0f, 0.0f, -1.0f);
		glScalef(effects->getHorizontalScale() / 100.0f, effects->getVerticalScale() / 100.0f, 1.0f);
	}

	if(effects->hasCustomEffects()) {
		// Process custom effects
		CustomGraphicalEffectsValue params;

		// Process color filter (CFIL)
		params = effects->getValue("CFIL");
		redFilter = params.color.r;
		greenFilter = params.color.g;
		blueFilter = params.color.b;
	}

	glColor4ub(redFilter, greenFilter, blueFilter, opacity);
}

// QC:P
inline void GraphicalEngineOGL::applyCustomEffects() {
	if(effects->hasCustomEffects()) {
		CustomGraphicalEffectsValue params;

		// Process motion blur (MBLR)
		params = effects->getValue("MBLR");
		processMBLR(params.color.a, params.color.r, params.color.g, params.color.b, opacity, params.color.amount);
	}
}

// QC:?
inline void GraphicalEngineOGL::applyPostEffects() {
	if(effects->hasBasicEffects()) {
		glPopAttrib();
	}
}

// QC:P
inline void GraphicalEngineOGL::applyPostCustomEffects() {
	if(effects->hasCustomEffects()) {
		CustomGraphicalEffectsValue params;

		// Process gaussian blur (BLUR)
		params = effects->getValue("BLUR");
		processBLUR(params.point.x, params.point.y);

		// Process radial blur (RBLR)
		params = effects->getValue("RBLR");
		processRBLR(params.point.x, params.point.y);
	}
}

// QC:?
inline void GraphicalEngineOGL::endPrimitiveDraw() {
	// Restore initial matrix
	glPopMatrix();
}

// QC:P
void GraphicalEngineOGL::draw(Texture *hlTexture, Translatable* coordinates, GraphicalEffects* hlEffects, int slot) {
	assert(hlTexture);
	assert(hlEffects);
	assert(coordinates);
#ifdef GPU_IMAGEPROCESS
	xassert(slot >= 0 && slot < TEXTURE_SLOTS, "Invalid slot : %d", slot);
#else
	xadvice(slot >= 0 && slot < TEXTURE_SLOTS, "Drawing on an unused texture slot.");
#endif

	uploadTexture(hlTexture);

	x = coordinates->getX();
	y = coordinates->getY();
	w = hlTexture->w;
	h = hlTexture->h;
	ow = hlTexture->ow;
	oh = hlTexture->oh;

	u1 = 0.0f;
	v1 = 0.0f;
	u2 = (float) ow / w;
	v2 = (float) oh / h;

	effects = hlEffects;

	draw(slot);
}

// QC:P
void GraphicalEngineOGL::fill(Resizable* coordinates, GraphicalEffects* hlEffects, int slot) {
	assert(hlEffects);
	assert(coordinates);
#ifdef GPU_IMAGEPROCESS
	xassert(slot >= 0 && slot < TEXTURE_SLOTS, "Invalid slot : %d", slot);
#else
	xadvice(slot >= 0 && slot < TEXTURE_SLOTS, "Drawing on an unused texture slot.");
#endif

	x = coordinates->getX();
	y = coordinates->getY();
	w = ow = coordinates->getW();
	h = oh = coordinates->getH();

	effects = hlEffects;

	fill(slot);
}

void GraphicalEngineOGL::drawRect(Resizable* rect, unsigned int r, unsigned int g, unsigned int b, unsigned int a) {
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT | GL_ENABLE_BIT);

	glDisable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor4ub(r, g, b, a);

	glBegin(GL_QUADS);
	glVertex3f(rect->getLeftmostX(), rect->getTopmostY(), 0);
	glVertex3f(rect->getRightmostX(), rect->getTopmostY(), 0);
	glVertex3f(rect->getRightmostX(), rect->getBottommostY(), 0);
	glVertex3f(rect->getLeftmostX(), rect->getBottommostY(), 0);
	glEnd();

	glPopAttrib();
}

// QC:?
void GraphicalEngineOGL::fill(int slot) {
#ifdef GPU_IMAGEPROCESS
	if(!oglShadersSupport && slot > 0) {
		return;
	}
#endif
	beginPrimitiveDraw();
	applyPreEffects();
	if(slot == 0)
		applyCustomEffects();

#ifdef GPU_IMAGEPROCESS
	// Select render target
	if(oglFBOSupport && !rsdList.empty()) {
		setRenderingTarget(rsd.targetSurfaces[slot]);
		xassert(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT, "OpenGL FBO could not be initialized : 0x%04X", glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT));
	}
#endif

	// Disable texturing
	glDisable(GL_TEXTURE_2D);

	// Draw sprite
	glBegin(GL_QUADS);
	glVertex3f(0, 0, 0);
	glVertex3f(ow, 0, 0);
	glVertex3f(ow, oh, 0);
	glVertex3f(0, oh, 0);
	glEnd();

	// Re-enable texturing
	glEnable(GL_TEXTURE_2D);

	applyPostEffects();
	if(slot == 0)
		applyPostCustomEffects();
	endPrimitiveDraw();
}

#ifdef OPENGL_ENABLE_FBO
// QC:?
void GraphicalEngineOGL::setRenderingTarget(GLuint target) {
	xassert(oglFBOSupport, "Trying to set rendering target without FBO support.");
	if(currentRenderingTarget != target) {
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, target, 0);
		currentRenderingTarget = target;
	}
}
#endif

// QC:G
inline void GraphicalEngineOGL::draw(int slot) {
#ifdef GPU_IMAGEPROCESS
	if(!oglShadersSupport && slot > 0) {
		return;
	}
	// Select render target
	if(oglFBOSupport) {
		if(!rsdList.empty()) {
			if(!(rsd.slots & (1<<slot))) {
				return;
			}
			setRenderingTarget(rsd.targetSurfaces[slot]);
			glViewport(rsd.px, rsd.py, rsd.pw, rsd.ph);
			glLoadIdentity();
			glOrtho(rsd.vx, rsd.vw + rsd.vx, rsd.vy, rsd.vh + rsd.vy, 1, -1);
			xassert(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT, "OpenGL FBO could not be initialized : 0x%04X", glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT));
		} else {
			if(slot)
				return;
		}
	}
#else
	if(slot > 0) {
		xadvice(slot == 0, "Cannot draw on slot %d : no post processing support in the engine.", slot);
		return;
	}
#endif

	beginPrimitiveDraw();
	applyPreEffects();
	if(slot == 0)
		applyCustomEffects();

	// Select texture
	glBindTexture(GL_TEXTURE_2D, texture);

	// Draw sprite
	glBegin(GL_QUADS);
	glTexCoord2f(u1, v1);
	glVertex3f(0, 0, 0);
	glTexCoord2f(u2, v1);
	glVertex3f(ow, 0, 0);
	glTexCoord2f(u2, v2);
	glVertex3f(ow, oh, 0);
	glTexCoord2f(u1, v2);
	glVertex3f(0, oh, 0);
	glEnd();

	applyPostEffects();
	if(slot == 0)
		applyPostCustomEffects();
	endPrimitiveDraw();
}

#ifdef GPU_IMAGEPROCESS
inline void GraphicalEngineOGL::preProcess(GLuint textures[TEXTURE_SLOTS], OGLImageProcess* process) {
	xassert(oglShadersSupport, "Cannot preprocess without shaders.");

	// Create the temporary texture
	GLuint tempSurface;
	glGenTextures(1, &tempSurface);

	uint32_t slots = process->slots;

	// Select the processing shader
	glUseProgram(process->program);

	// Setup multitexturing
	for(int i=TEXTURE_SLOTS; i-- > 1; ) {
		glActiveTextureARB(GL_TEXTURE0 + i);
		if(slots & (1<<i)) {
			glBindTexture(GL_TEXTURE_2D, textures[i]);
			glUniform1i(process->textureLocations[i], i);
			glEnable(GL_TEXTURE_2D);
		} else {
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
		}
	}

	// Setup texture size
	if(process->widthLocation != -1)
		glUniform1f(process->widthLocation, w);
	if(process->heightLocation != -1)
		glUniform1f(process->heightLocation, h);

	// Copy framebuffer to the texture
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tempSurface);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, x, y, w, h, 0);

	// Setup mipmaps
	if(process->use_mipmaps) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmapEXT(GL_TEXTURE_2D);
	}
	
	// Draw the buffer using the shader
	glPushMatrix();
	glDisable(GL_BLEND);
	glTranslatef(x, y, 0);
	glBegin(GL_QUADS);

	for(int i=0; i<TEXTURE_SLOTS; i++) {
		if(slots & (1<<i)) {
			glMultiTexCoord2fARB(GL_TEXTURE0 + i, 0, 0);
		}
	}
	glVertex3f(0, 0, 0);
	for(int i=0; i<TEXTURE_SLOTS; i++) {
		if(slots & (1<<i)) {
			glMultiTexCoord2fARB(GL_TEXTURE0 + i, u2, 0);
		}
	}
	glVertex3f(ow, 0, 0);
	for(int i=0; i<TEXTURE_SLOTS; i++) {
		if(slots & (1<<i)) {
			glMultiTexCoord2fARB(GL_TEXTURE0 + i, u2, v2);
		}
	}
	glVertex3f(ow, oh, 0);
	for(int i=0; i<TEXTURE_SLOTS; i++) {
		if(slots & (1<<i)) {
			glMultiTexCoord2fARB(GL_TEXTURE0 + i, 0, v2);
		}
	}
	glVertex3f(0, oh, 0);
	glEnd();
	glEnable(GL_BLEND);
	glPopMatrix();

	// Disable the processing shader
	glUseProgram(0);
	
	// Free the texture
	glDeleteTextures(1, &tempSurface);
}
#endif

void GraphicalEngineOGL::drawWithProcessing(GLuint textures[TEXTURE_SLOTS], int tw, int th, Translatable* coordinates, GraphicalEffects* hlEffects) {
	assert(coordinates);
	assert(hlEffects);

#ifdef GPU_IMAGEPROCESS
	if(!oglShadersSupport) {
		// Draw normally the first texture slot
		texture = textures[0];
		draw(0);
	}

	// Select render target
	if(oglFBOSupport) {
		if(!rsdList.empty()) {
			xassert(rsd.slots & 1, "No primary surface to draw on.");
			setRenderingTarget(rsd.targetSurfaces[0]);
			glViewport(rsd.px, rsd.py, rsd.pw, rsd.ph);
			glLoadIdentity();
			glOrtho(rsd.vx, rsd.vw + rsd.vx, rsd.vy, rsd.vh + rsd.vy, 1, -1);
			xassert(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT, "OpenGL FBO could not be initialized : 0x%04X", glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT));
		}
	}

	x = coordinates->getX();
	y = coordinates->getY();
	ow = tw;
	oh = th;
	w = nextPowerofTwo(ow);
	h = nextPowerofTwo(oh);
	u2 = ow / w;
	v2 = oh / h;

	effects = hlEffects;

	OGLImageProcess* process = (OGLImageProcess*)effects->getPreProcess();
	if(process) {
		preProcess(textures, process);
	}

	beginPrimitiveDraw();
	applyPreEffects();

	uint32_t slots;
	// Setup GLSL post-processing
	process = (OGLImageProcess*)(effects->getPostProcess());
	if(process) {
		slots = process->slots;
		glUseProgram(process->program);
		for(int i=TEXTURE_SLOTS; i--; ) {
			glActiveTextureARB(GL_TEXTURE0 + i);
			if(slots & (1<<i)) {
				glBindTexture(GL_TEXTURE_2D, textures[i]);
				glUniform1i(process->textureLocations[i], i);
			} else {
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}

		// Setup texture size
		if(process->widthLocation != -1)
			glUniform1f(process->widthLocation, w);
		if(process->heightLocation != -1)
			glUniform1f(process->heightLocation, h);

		// Setup mipmaps
		if(process->use_mipmaps) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}
	} else {
		slots = 1;
		glActiveTextureARB(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures[0]);
	}

	// Draw multi-textured sprite
	glBegin(GL_QUADS);

	for(int i=0; i<TEXTURE_SLOTS; i++) {
		if(slots & (1<<i)) {
			glMultiTexCoord2fARB(GL_TEXTURE0 + i, 0, 0);
		}
	}
	glVertex3f(0, 0, 0);
	for(int i=0; i<TEXTURE_SLOTS; i++) {
		if(slots & (1<<i)) {
			glMultiTexCoord2fARB(GL_TEXTURE0 + i, u2, 0);
		}
	}
	glVertex3f(ow, 0, 0);
	for(int i=0; i<TEXTURE_SLOTS; i++) {
		if(slots & (1<<i)) {
			glMultiTexCoord2fARB(GL_TEXTURE0 + i, u2, v2);
		}
	}
	glVertex3f(ow, oh, 0);
	for(int i=0; i<TEXTURE_SLOTS; i++) {
		if(slots & (1<<i)) {
			glMultiTexCoord2fARB(GL_TEXTURE0 + i, 0, v2);
		}
	}
	glVertex3f(0, oh, 0);
	glEnd();

	if(process) {
		glUseProgram(0);
	}

	applyPostEffects();
	endPrimitiveDraw();
#else
	// Draw normally the first texture slot
	texture = textures[0];
	draw(0);
#endif
}

// QC:G
bool GraphicalEngineOGL::terminationRequest() {
	return false;
}

// QC:?
GLuint GraphicalEngineOGL::createRenderingSurface(int tw, int th, int format) {
	if(permanentTexUsed != (1<<PERMANENT_TEX)-1 && tw <= PERMANENT_TEX_W && th <= PERMANENT_TEX_H && format == PERMANENT_TEX_F) {
		for(int i=0; i<PERMANENT_TEX; i++) {
			if(!(permanentTexUsed & (1<<i))) {
				permanentTexUsed |= (1<<i);
				return permanentTex[i];
			}
		}
	}

	GLuint targetSurface;
	glGenTextures(1, &targetSurface);
	glBindTexture(GL_TEXTURE_2D, targetSurface);
	GLenum textureFormat;
	switch(format) {
		case IMAGE_R8G8B8:
			textureFormat = GL_RGB8;
			break;
		case IMAGE_R8G8B8A8:
			textureFormat = GL_RGBA8;
			break;
		default:
			textureFormat = IMAGE_UNKNOWN_FORMAT;
			xassert(false, "Texture format %d not supported by OpenGL FBO.", format);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, textureFormat, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	AOEVIDALLOC((void*)targetSurface, tw * th * (format == IMAGE_R8G8B8?3:4));
	glBindTexture(GL_TEXTURE_2D, 0);
	return targetSurface;
}

// QC:?
void GraphicalEngineOGL::pushRenderingSurface(Translatable* minimumSurface, uint32_t slots, void* preProcess, void* postProcess, int format) {
	// Save fullscreen matrix.
	if(rsdList.empty()) {
		glPushMatrix();
	}
	// Backup current rendering surface descriptor
	rsdList.push_front(rsd);

	int w, h;

	// Surface size
	if(minimumSurface && minimumSurface->getX() && minimumSurface->getY()) {
		w = minimumSurface->getX();
		h = minimumSurface->getY();
	} else {
		w = rsd.pw;
		h = rsd.ph;
	}

#ifdef OPENGL_ENABLE_FBO
	rsd.slots = slots;

	xassert(oglFBOSupport, "CopySubTex render to texture is not implemented.");

	// Use FBO to render directly to a texture.

	assert(w);
	assert(h);

	// Texture size
	int tw = nextPowerofTwo(w);
	int th = nextPowerofTwo(h);

	assert(tw);
	assert(th);

	// Bind the FBO
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

	// Create the textures
#ifdef GPU_IMAGEPROCESS
	for(int s=0; s<TEXTURE_SLOTS; s++) {
		if(slots & (1 << s)) {
			rsd.targetSurfaces[s] = createRenderingSurface(tw, th, format);
			setRenderingTarget(rsd.targetSurfaces[s]);
			if(postProcess) {
				OGLImageProcess* p = (OGLImageProcess*)postProcess;
				glClearColor(
					p->clearColors[s][0],
					p->clearColors[s][1],
					p->clearColors[s][2],
					p->clearColors[s][3]
				);
			} else if(preProcess) {
				OGLImageProcess* p = (OGLImageProcess*)preProcess;
				glClearColor(
					p->clearColors[s][0],
					p->clearColors[s][1],
					p->clearColors[s][2],
					p->clearColors[s][3]
				);
			}
			glClear(GL_COLOR_BUFFER_BIT);
		}
	}
	if(!oglShadersSupport) {
		setRenderingTarget(rsd.targetSurfaces[0]);
	}
#else
	rsd.targetSurfaces[0] = createRenderingSurface(tw, th, format);
	setRenderingTarget(rsd.targetSurfaces[0]);
	// Set the texture as a rendering target
	assert(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT);
	glClear(GL_COLOR_BUFFER_BIT);
#endif /* GPU_IMAGEPROCESS */

#else /* OPENGL_ENABLE_FBO */
	xerror(false, "CopySubTex render to texture is not implemented. Please enable OPENGL_ENABLE_FBO.");
#endif /* OPENGL_ENABLE_FBO */

	rsd.px = 0;
	rsd.py = 0;
	rsd.pw = w;
	rsd.ph = h;

	rsd.vx = 0;
	rsd.vy = 0;
	rsd.vw = w;
	rsd.vh = h;

	// Clear the screen and get ready for the sub-rendering.
	glViewport(rsd.px, rsd.py, rsd.pw, rsd.ph);
	glLoadIdentity();
	glOrtho(rsd.vx, rsd.vw + rsd.vx, rsd.vy, rsd.vh + rsd.vy, 1, -1);
}

// QC:A
Renderable* GraphicalEngineOGL::popRenderingSurface(State* attachedState) {
	assert(!rsdList.empty());

	Renderable* resultFrame;

#ifdef OPENGL_ENABLE_FBO
	xassert(oglFBOSupport, "CopySubTex render to texture is not implemented.");

	// Build the high-level enclosing surface.
	resultFrame = new GLFramebuffer(attachedState, this, rsd.targetSurfaces, rsd.pw, rsd.ph, rsd.slots);
#else
	xerror(false, "CopySubTex render to texture is not implemented. Please enable OPENGL_ENABLE_FBO.");
	resultFrame = NULL; // TODO : implement CopySubTex
#endif

	// Restore the previous context
	rsd = rsdList.front();
	rsdList.pop_front();

#ifdef OPENGL_ENABLE_FBO
	if(rsdList.empty()) {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glViewport(displayOffsetX, displayOffsetY, displayWidth, displayHeight);
		glPopMatrix();
	} else {
		// Restore the previous FBO
#ifndef GPU_IMAGEPROCESS
		setRenderingTarget(rsd.targetSurfaces[0]);
		assert(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT);
#endif
		glViewport(rsd.px, rsd.py, rsd.pw, rsd.ph);
		glLoadIdentity();
		glOrtho(rsd.vx, rsd.vw + rsd.vx, rsd.vy, rsd.vh + rsd.vy, 1, -1);
	}
#endif

  return resultFrame;
}

// QC:B (TODO)
void GraphicalEngineOGL::processMBLR(unsigned char amount, unsigned char r, unsigned char g, unsigned char b, unsigned char a, int angle) {}

// QC:B (TODO)
void GraphicalEngineOGL::processBLUR(coord x, coord y) {}

// QC:B (TODO)
void GraphicalEngineOGL::processRBLR(coord x, coord y) {}

