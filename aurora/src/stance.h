#ifndef STANCE_H
#define STANCE_H

#include <vector>

#include "renderable.h"
#include "controller_state.h"

/** A sprite is an independantly animated element on the screen.
 * To improve extensibility, animations have litteral names.
 * Sprites can be flipped.
 */
class Stance : public Renderable {
private:
	Renderable* animations[9];
	ControllerState::Direction currentDirection;
	Point translation[9];

public:
#define HEADER stance
#define HELPER scriptable_object_h
#include "include_helper.h"
	Stance();
	Stance(
		State* attachedState,
		Translatable* offset,
		GraphicalEffects* effects,
		int layer,
		Renderable* animUL,	Translatable* transUL,
		Renderable* animU,	Translatable* transU,
		Renderable* animUR,	Translatable* transUR,
		Renderable* animL,	Translatable* transL,
		Renderable* animC,	Translatable* transC,
		Renderable* animR,	Translatable* transR,
		Renderable* animBL,	Translatable* transBL,
		Renderable* animB,	Translatable* transB,
		Renderable* animBR,	Translatable* transBR
	      );
	virtual ~Stance();

	/** Set current direction.
	 * @param newDirection the new direction.
	 */
	void setDirection(ControllerState::Direction newDirection);

	/** Retreive the current animation.
	 * @return currAnimation the current animation.
	 */
	Renderable* getCurrentAnimation();

	/** Get current direction
	 * @return currDirection the current direction.
	 */
	ControllerState::Direction getDirection();

	/** Rotate the stance by 45 degrees clockwise.
	 */
	void rotateStanceCW();

	/** Rotate the stance by 45 degrees counterclockwise.
	 */
	void rotateStanceCCW();

	/** Restart the current animation.
	 */
	void restart();

	/** Get the current translation
	 * @return the pointer of the translation
	 */
	Translatable* getTranslation();
	/** Get the translation of the "previous" direction
	 * @return the pointer of the translation
	 */
	Translatable* getPreviousTranslation();
	/** Get the translation of the "next" direction
	 * @return the pointer of the translation
	 */
	Translatable* getNextTranslation();

	/** Modify the translation of a direction
	 * @param a translation
	 */
	void setTranslation(Translatable *p_translation, ControllerState::Direction p_direction);

	/** Set an animation
	 * @param p_animation, a renderable (Animation in general)
	 * @param p_direction, the direction of this animation
	 */
	void setAnimation(Renderable *p_animation, ControllerState::Direction p_direction);

	/* Blocking */

	virtual bool isFinished();

	/* Renderable */

	virtual void renderGraphics();
	virtual void loadGraphics();
	virtual void unloadGraphics();

	/* Realtime */

	virtual void updateObject(dur elapsedTime);

	/* Translatable */

	virtual bool inside(Translatable& firstCorner, Translatable& secondCorner);
};

#endif /* STANCE_H */

