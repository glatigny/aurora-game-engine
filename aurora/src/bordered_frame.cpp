#include "bordered_frame.h"

#define HEADER bordered_frame
#define HELPER scriptable_object_cpp
#include "include_helper.h"

void BorderFrame::renderGraphics() {
	Texture* texture = getResource();
	GraphicalEffects borderEffect;
	if(finalEffects) {
		memcpy(&borderEffect);
	} else {
		Renderable::setNeutralEffect(&borderEffect);
	}

	// FIXME : Border sizes are ignored for now.
	
	borderEffect.horizontalScale = borderEffect.horizontalScale*getW()/texture->ow;
	borderEffect.verticalScale = borderEffect.verticalScale*getH()/texture->oh;

	if(texture != NULL) {
		state()->manager()->hardware()->draw(texture, &finalPosition, &finalEffects);
	}
}

