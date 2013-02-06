#ifndef SPRITE_H
#define SPRITE_H

#include <map>

#include "renderable.h"
#include "stance.h"

/** A sprite is an independantly animated element on the screen.
 * To improve extensibility, animations have litteral names.
 * Sprites can be flipped.
 */
class Sprite : public Renderable {
public:
	// Map comparator
	struct strCmp
	{
		bool operator()(const char* s1, const char* s2) const
		{ return strcmp(s1, s2) < 0; }
	};
private:
	std::map<const char*, Stance*, strCmp> stances;
	Stance* currentStance;

public:
#define HEADER sprite
#define HELPER scriptable_object_h
#include "include_helper.h"
	Sprite() {}
	Sprite(State* attachedState, Translatable* offset, GraphicalEffects* effects);
	virtual ~Sprite();

	/** Set current animation.
	 * Don't forget to set the direction of the stance just after.
	 * @param newAnimation the name of the animation to display.
	 */
	void setStance(const char* newStance);

	/** Get currently played animation.
	 * @return a pointer to the currently playing animation.
	 */
	Stance* getCurrentStance();

	/** Tells if the the sprite contains a particular stance.
	 * @param stance the name of the stance to test.
	 * @return true if the sprite contains the stance, false otherwise.
	 */
	bool exist(const char* stanceName);

	/** Tells if the the sprite is in a particular stance.
	 * @param stance the name of the stance to test.
	 * @return true if the sprite is in the given stance, false otherwise.
	 */
	bool isIn(const char* stanceName);

	/** Add a new stance.
	 * @param stanceName the name of the stance to add.
	 * @param stance the stance to add.
	 */
	void addStance(const char* stanceName, Stance* stance);

	/* Renderable */

	virtual void renderGraphics();
	virtual void loadGraphics();
	virtual void unloadGraphics();

	/* Blocking */

	virtual bool isFinished() {
		return Renderable::isFinished() && (currentStance ? currentStance->isFinished():true);
	}

	/* Realtime */

	virtual void updateObject(dur elapsedTime);

	/* Translatable */

	virtual bool inside(Translatable& firstCorner, Translatable& secondCorner);
};

#endif

