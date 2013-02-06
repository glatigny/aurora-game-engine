#include "glyph.h"

#define HEADER glyph
#define HELPER scriptable_object_cpp
#include "include_helper.h"

Glyph::Glyph(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int newWidth, int newHeight, int newDelay) : Translatable(offset), Renderable(attachedState, offset, effects, layer), Resizable(newWidth, newHeight), delay(newDelay), currentTime(0) {}


void Glyph::render() {
	if(currentTime >= delay) {
		Renderable::render();
	}
}


void Glyph::updateObject(dur elapsedTime) {
	currentTime += elapsedTime;
}


void Glyph::restart() {
	currentTime = 0;
}

