#include "bitmap_glyph.h"

#define HEADER bitmap_glyph
#define HELPER scriptable_object_cpp
#include "include_helper.h"

BitmapGlyph::BitmapGlyph(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int newWidth, int newHeight, int newDelay, Renderable* newGlyphImage) : Translatable(offset), Entity(attachedState), Glyph(attachedState, offset, effects, layer, newWidth, newHeight, newDelay), glyphImage(newGlyphImage) {
	xassert(newGlyphImage, "Glyph image missing.");
	newGlyphImage->setParent(this);
}

// QC:P
BitmapGlyph::~BitmapGlyph() {
}

// QC:P
void BitmapGlyph::renderGraphics() {
	glyphImage->render();
}

// QC:G
void BitmapGlyph::updateObject(dur elapsedTime) {
	Glyph::updateObject(elapsedTime);
	if(currentTime >= delay) {
		glyphImage->update(elapsedTime);
	}
}

// QC:G
void BitmapGlyph::restart() {
	glyphImage->restart();
	currentTime = 0;
}

