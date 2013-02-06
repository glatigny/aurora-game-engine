#ifndef BITMAP_GLYPH_H
#define BITMAP_GLYPH_H

#include "renderable.h"
#include "glyph.h"

class BitmapGlyph : public Glyph {
private:
	Renderable* glyphImage;

protected:

	/* Renderable */

	virtual void renderGraphics();

public:
#define HEADER bitmap_glyph
#define HELPER scriptable_object_h
#include "include_helper.h"
	BitmapGlyph() : glyphImage(NULL) {}
	BitmapGlyph(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int width, int height, int delay, Renderable* glyphImage);

	virtual ~BitmapGlyph();

	/* Renderable */

	virtual void updateObject(dur elapsedTime);
	virtual void loadGraphics() { glyphImage->loadGraphics(); }
	virtual void unloadGraphics() { glyphImage->unloadGraphics(); }

	/* Restartable */

	virtual void restart();

	/* Translatable */

	virtual bool inside(Translatable& topLeft, Translatable& bottomRight) { return false; }
};

#endif /* BITMAP_GLYPH */

