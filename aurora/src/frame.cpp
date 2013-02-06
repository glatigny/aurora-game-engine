#include "frame.h"
#include "vfs_file.h"
#include "vfs.h"
#include <string.h>
#define HEADER frame
#define HELPER scriptable_object_cpp
#include "include_helper.h"

// QC:P
Frame::Frame(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, VFS::File* textureFile, int newSlot, VFS::File* soundFile, int newSoundLoop) :
	Translatable(offset), Entity(attachedState), Renderable(attachedState, offset, effects, layer), TextureResourceManager(attachedState, textureFile),
	Audible(attachedState), AudioInstanceResourceManager(attachedState, soundFile), slot(newSlot), soundLoop(newSoundLoop)
{
}

// QC:P
Frame::Frame(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, Texture* volatileTexture) :
	Translatable(offset), Entity(attachedState), Renderable(attachedState, offset, effects, layer), TextureResourceManager(attachedState, volatileTexture),
	Audible(attachedState), AudioInstanceResourceManager(attachedState, (VFS::File*)NULL), soundLoop(false)
{
}

// QC:P
void Frame::renderGraphics() {
	Texture* texture;
	TextureResourceManager::getResource(texture);
	if(texture != NULL) {
		engine()->hardware()->draw(texture, getAbsolutePosition(), getFinalEffects(), slot);
	}
}

// QC:?
bool Frame::resLoad(Texture* &returnValue, VFS::File* file) {
	VFS::ImageSource* source = VFS::openImage(file);
	returnValue = engine()->hardware()->loadTexture(source);
	delete source;
	return true;
}

// QC:P
void Frame::resUnload(Texture* t) {
	engine()->hardware()->freeTexture(t);
}

// QC:P
inline void Frame::loadGraphics() {
	TextureResourceManager::preloadResource();
}

// QC:P
inline void Frame::unloadGraphics() {
	TextureResourceManager::freeResource();
}

// QC:?
void Frame::loadSounds() {
	AudioInstanceResourceManager::preloadResource();
}

// QC:?
void Frame::unloadSounds() {
	AudioInstanceResourceManager::freeResource();
}

// QC:?
bool Frame::resLoad(AudioInstance* &returnValue, VFS::File* soundFile) { // TODO : use openPCM
	//VFS::PCMSource* pcmSource = VFS::openPCM(soundFile);
	returnValue = engine()->hardware()->instantiateSound(soundFile, AUDIO_FILE_PRELOAD, 10000, 0, true, soundLoop, getAbsolutePosition());
	return true;
}

// QC:?
void Frame::resUnload(AudioInstance* ai) {
	engine()->hardware()->freeAudioInstance(ai);
}

// QC:?
void Frame::restart() {
	AudioInstance* soundInstance;
	AudioInstanceResourceManager::getResource(soundInstance);

	if(soundInstance) {
		engine()->hardware()->pause(soundInstance);
		engine()->hardware()->rewind(soundInstance);
		engine()->hardware()->play(soundInstance);
	}
}

// QC:?
void Frame::setW(coord newW) {
	Texture* texture;
	TextureResourceManager::getResource(texture);
	if(texture != NULL) {
		setHorizontalScale(newW * 100 / texture->ow);
	}
}

// QC:?
void Frame::setH(coord newH) {
	Texture* texture;
	TextureResourceManager::getResource(texture);
	if(texture != NULL) {
		setVerticalScale(newH * 100 / texture->oh);
	}
}

// QC:?
coord Frame::getW() {
	Texture* texture;
	TextureResourceManager::getResource(texture);
	if(texture != NULL) {
		return texture->ow * getScale().getX() / 100;
	} else {
		return 0;
	}
}

// QC:?
coord Frame::getH() {
	Texture* texture;
	TextureResourceManager::getResource(texture);
	if(texture != NULL) {
		return texture->oh * getScale().getY() / 100;
	} else {
		return 0;
	}
}
