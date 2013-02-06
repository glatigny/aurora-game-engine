#ifndef GL_FRAMEBUFFER_H
#define GL_FRAMEBUFFER_H

#include "open_gl.h"
#include "renderable.h"
#include "point.h"


class GraphicalEngineOGL;

class GLFramebuffer : public Renderable {
private:
	bool freeTextures;
	GraphicalEngineOGL* glEngine;
	uint32_t slots;
	GLuint textureSlots[TEXTURE_SLOTS];
	int width;
	int height;

#ifdef GPU_IMAGEPROCESS
	bool mipmaps;
#endif

	static Point defaultCoords;
public:
	GLFramebuffer(State* newAttachedState, GraphicalEngineOGL* newGLEngine, GLuint newTextureSlots[TEXTURE_SLOTS], int w, int h, uint32_t newSlots) : Entity(newAttachedState), Renderable(newAttachedState, &defaultCoords, NULL, 0), glEngine(newGLEngine), slots(newSlots), width(w), height(h)
#ifdef GPU_IMAGEPROCESS
		, mipmaps(false)
#endif
	{
		memcpy(textureSlots, newTextureSlots, TEXTURE_SLOTS * sizeof(GLuint));
	}
	~GLFramebuffer();

	/* Renderable */

	virtual void renderGraphics();
	virtual void loadGraphics() {}
	virtual void unloadGraphics() {}
	virtual VFS::ImageSource* renderToImageSource();

	/* Translatable */

	virtual bool inside(Translatable& p1, Translatable& p2) { return false; }
};

#endif /* GL_FRAMEBUFFER_H */
