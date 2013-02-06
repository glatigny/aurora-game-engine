#ifndef KEY_ANIMATION_H
#define KEY_ANIMATION_H

#include "animation.h"
#include <map>

/** The list of types of animations an animation can take. */
#define KEY_ANIMATION_LINEAR	(0)
#define KEY_ANIMATION_FORWARD	(1)
#define KEY_ANIMATION_BACKWARD	(2)
#define KEY_ANIMATION_CIRCULAR	(3)

/** An animation is a succession of images played at a constant rate.
 */
class KeyAnimation : public Animation {
public:
	struct strCmp
	{
		bool operator()(const char* s1, const char* s2) const
		{ return strcmp(s1, s2) < 0; }
	};
private:
	std::map<const char *, int, strCmp> keys;
	int mode;

public:
#define HEADER key_animation
#define HELPER scriptable_object_h
#include "include_helper.h"
	KeyAnimation() {}
	KeyAnimation(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int frameDuration, int animSpeed, bool reversePlayback, int loopType, int mode, int frameUpdateMode, const char* interpolations);
	virtual ~KeyAnimation();

	/** Plays animation to the key.
	 * @param keyName the name of the key.
	 */
	void playTo(const char* keyName);

	/** Returns the frame under the given key.
	 * @param keyName the name of the key to query.
	 * @return the frame under the key. If the key is between two frames, the frame before the pointer is returned.
	 */
	Renderable* getFrameByKey(const char* keyName);

	/** Sets the animation at the position of the key.
	 * @param keyName the name of the key.
	 */
	void setPositionByKey(const char* keyName);

	/** Sets the playback mode.
	 * @param mode the selected mode.
	 */
	void setPlaybackMode(int mode);

	/** Add a Key to this animation.
	 * @param frame the frame to add to this animation.
	 * @param pos the position of the key
	 */
	void addKey(const char* keyName, int pos);

	/* Animation */

	void setPlaybackPosition(int newPosition);
};

#endif
