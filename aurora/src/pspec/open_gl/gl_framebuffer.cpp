#include "aurora_engine.h"
#include "gl_framebuffer.h"
#include "vfs.h"

Point GLFramebuffer::defaultCoords;

GLFramebuffer::~GLFramebuffer() {
#ifdef GPU_IMAGEPROCESS
	for(int s=0; s<TEXTURE_SLOTS; s++) {
		if(slots & (1<<s)) {
			glEngine->myFreeTexture(textureSlots[s]);
		}
	}
#else
	glEngine->myFreeTexture(textureSlots[0]);
#endif
}

// QC:P
void GLFramebuffer::renderGraphics() {
#ifdef GPU_IMAGEPROCESS
	if(oglShadersSupport && !mipmaps) {
		OGLImageProcess* process = (OGLImageProcess*) objectEffects.getPostProcess();
		if(process && process->use_mipmaps) {
			glBindTexture(GL_TEXTURE_2D, textureSlots[0]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 6);
			glGenerateMipmapEXT(GL_TEXTURE_2D);
			mipmaps = true;
		}
	}
#endif

	glEngine->drawWithProcessing(textureSlots, width, height, getAbsolutePosition(), getFinalEffects());
}

// QC:?
VFS::ImageSource* GLFramebuffer::renderToImageSource() {
	char* buffer = (char*)AOEMALLOC(width * height * 4);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	return VFS::createImage(buffer, IMAGE_R8G8B8A8, width, height);
}
