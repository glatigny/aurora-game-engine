#ifndef GRAPHICAL_ENGINE_H
#define GRAPHICAL_ENGINE_H

#include "graphical_effects.h"
#include "translatable.h"
#include "texture.h"
#include "state.h"
#include "rect.h"
#include <map>

class HardwareEngine;
namespace VFS {
	class ImageSource;
}

class GraphicalEngineInterface {
	public:
		virtual ~GraphicalEngineInterface() {}
		virtual void init(HardwareEngine* hw) {}

		virtual dur getNativeFrameRate() = 0;
		virtual Resizable* getDisplaySize() = 0;

		virtual void preDraw() = 0;
		virtual void postDraw() = 0;

		virtual void draw(Texture* texture, Translatable* coordinates, GraphicalEffects* effects, int targetTextureSlot = 0) = 0;
		virtual void fill(Resizable* coordinates, GraphicalEffects* effects, int targetTextureSlot = 0) = 0;

		virtual void drawRect(Resizable* rect, unsigned int r, unsigned int g, unsigned int b, unsigned int a) = 0;

		bool isTheEnd() { xwarn(0,"Remove isTheEnd"); return terminationRequest(); }
		virtual bool terminationRequest() = 0;

		virtual Texture* loadTexture(VFS::ImageSource* p_file) = 0;
		virtual bool freeTexture(Texture* p_texture) = 0;

		virtual void pushRenderingSurface(Translatable* minimumSurfaceSize = NULL, uint32_t slots = 1, void* preProcess = NULL, void* postProcess = NULL, int format = IMAGE_R8G8B8A8) = 0;
		virtual Renderable* popRenderingSurface(State* attachedState) = 0;
		virtual bool loadImageProcess(const char* processString, void* &pspecData, uint32_t &neededSlots) = 0;
		virtual void unloadImageProcess(void* &pspecData) = 0;

		virtual void setViewport(Resizable* physicalViewport = NULL, Resizable* virtualViewport = NULL) = 0;

		virtual void resizeWindow(Resizable* newSize) = 0;
};

class GraphicalEngine : public GraphicalEngineInterface {
	public:
		struct strCmp
		{
			bool operator()(VFS::ImageSource* s1, VFS::ImageSource* s2) const
			{ return strcmp(s1->source()->name(), s2->source()->name()) < 0; }
		};

	/*private:*/
	protected:
		struct struct_Texture
		{
			Texture*	l_texture;
			int		usageCount;
		};

		std::map<VFS::ImageSource*, struct_Texture*, strCmp> m_textures;
	
	protected:		
		virtual Texture*	myLoadTexture(VFS::ImageSource* p_file) = 0;
		virtual bool		myFreeTexture(Texture* p_texture) = 0;
	
	public:
		virtual ~GraphicalEngine();

		virtual void preDraw() {}
		virtual void postDraw() {}
	
		Texture* loadTexture(VFS::ImageSource* p_file);

		bool hasTexture(Texture* p_texture);
		bool freeTexture(Texture* p_texture);
};

#endif /* GRAPHICAL_ENGINE_H */
