#ifndef BORDERED_FRAME_H
#define BORDERED_FRAME_H

#include "frame.h"

class BorderedFrame : public Frame {
private:
	int topBorder;
	int bottomBorder;
	int leftBorder;
	int rightBorder;

public:
#define HEADER bordered_frame
#define HELPER scriptable_object_h
#include "include_helper.h"
	BorderedFrame(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, const char* filename, int topBorder, int bottomBorder, int leftBorder, int rightBorder);
	virtual ~BorderedFrame() {}

	/* Renderable */

	virtual void renderGraphics();
};

#endif

