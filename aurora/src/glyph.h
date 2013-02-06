#ifndef GLYPH_H
#define GLYPH_H

#include "renderable.h"
#include "state.h"

class Glyph : public Renderable, public Resizable {
protected:
	int delay;
	int currentTime;

public:
#define HEADER glyph
#define HELPER scriptable_object_h
#include "include_helper.h"
	Glyph() {}
	Glyph(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int width, int height, int delay);

	virtual ~Glyph() {}

	/* Renderable */

	virtual void render();

	/* Realtime */

	virtual void updateObject(dur elapsedTime);

	/* Restartable */

	virtual void restart();
};

#endif /* GLYPH_H */

