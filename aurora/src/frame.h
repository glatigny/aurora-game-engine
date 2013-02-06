#ifndef FRAME_H
#define FRAME_H

#include "state.h"
#include "resource_manager.h"
#include "renderable.h"

typedef ResourceManager<Texture> TextureResourceManager;
typedef ResourceManager<AudioInstance> AudioInstanceResourceManager;

/** A frame is a graphical element with exactly one texture associated.
 * It is the main kind of leaves in the rendering tree.
 * Hardware engines are hot-swappable. The initial hardware engine is
 * cached in the Frame, but if the actual hardware engine changes, the Frame
 * attempts to load the texture in the new hardware engine. This capablility is
 * provided by ResourceManager.
 */
class Frame : public Renderable, public Resizable, protected TextureResourceManager, public Audible, protected AudioInstanceResourceManager
{
private:
	int slot;
	VFS::File* sound;
	int soundLoop;

	/* ResourceManager */

	virtual bool resLoad(AudioInstance* &returnValue, VFS::File* file);
	virtual void resUnload(AudioInstance* t);

	virtual bool resLoad(Texture* &returnValue, VFS::File* file);
	virtual void resUnload(Texture* t);

public:
#define HEADER frame
#define HELPER scriptable_object_h
#include "include_helper.h"
	Frame() :
		TextureResourceManager(), Audible(), AudioInstanceResourceManager()
	{
	}

	Frame(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, VFS::File* file, int slot, VFS::File* soundFile, int soundLoop);

	Frame(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, Texture* volatileTexture);

	virtual ~Frame() {
		TextureResourceManager::freeResource();
	}

	/* Restartable */

	virtual void restart();

	/* Renderable */

	virtual void renderGraphics();

	virtual void loadGraphics();
	virtual void unloadGraphics();

	virtual void loadSounds();
	virtual void unloadSounds();
	virtual void commitSounds(int, AudioEffect* ) {}

	/* Resizable */

	void setW(coord w);
	void setH(coord h);
	coord getW();
	coord getH();

	/* Realtime */

	virtual void updateObject(dur elapsedTime) {}

	/* Translatable */

	virtual bool inside(Translatable& firstCorner, Translatable& secondCorner) {
		return false;
	}

	/* AOEObject */
	virtual VFS::File* source() {

		return Renderable::source();
	}
};

#endif

