#ifndef DECORATION_H
#define DECORATION_H

#include <vector>

#include "resizable.h"
#include "game_object.h"

/** A decoration is a graphical element made of
 * one or more than one frames (renderables). All the frames
 * are displayed at the same time. Frames can be arranged
 * independently by adjusting their relative position.
 *
 * Decorations can also have parallax.
 *
 * VIRTUAL CONSTRUCTORS:
 *  - Translatable(offset)
 */
class Decoration : public GameObject, public Resizable {
private:
	std::vector<Renderable*> graphics;
	Point parallax;

protected:
	/* Renderable */

	virtual void computeFinalPosition();

public:
#define HEADER decoration
#define HELPER scriptable_object_h
#include "include_helper.h"
	Decoration() {
	}

//	Decoration(State* attachedState);

	Decoration(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int collisionLayers, Translatable* newParallax, Collidable* collision);

	virtual ~Decoration();

	/** Returns current parallax.
	 * @return current decoration's parallax.
	 */
	Translatable* getParallax();

	/** Sets this object's parallax.
	 * Parallax is defined as a percentage of camera movement.
	 * @param newParallax the Translatable which indicates x and y proportions to this object's coordinates.
	 */
	void setParallax(Translatable* newParallax);

	/** Appends a frame to this decoration's graphics.
	 * Frames are all displayed at the same time. Adding more fames can be used to make a composition (and sometimes save memory).
	 * @param newFrame the frame to be added to this decoration.
	 */
	void addFrame(Renderable* newFrame);

	/** Get a frame from this decoration.
	 * Bound checking is forced by an assertion.
	 * @param position the index of the frame. Frames are in their insertion order.
	 * @return the frame at the given position.
	 */
	Renderable* getFrame(unsigned int position);

	/* Renderable */

	virtual void renderGraphics();
	virtual void loadGraphics();
	virtual void unloadGraphics();

	/* Realtime */

	virtual void updateObject(dur elapsedTime);
};

#endif
