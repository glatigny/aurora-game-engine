#ifndef COLLIDABLE_H
#define COLLIDABLE_H

#include "collision.h"
#include "renderable.h"
#include "signalable.h"
#include "context.h"
#include "vfs_file.h"

class CollisionPoint;
class CollisionRect;
class CollisionHull;

#define COLLISION_TYPE_NONE			(00)
#define COLLISION_TYPE_POINT		(01)
#define COLLISION_TYPE_RECT			(02)
#define COLLISION_TYPE_HULL			(03)

/** Object sensitive to collisions.
 */
class Collidable : public Renderable, public Signalable {
private:
	int collisionLayers;
protected:
	bool collisionsEnabled;
public:
	static const int COLLISION_PLATFORM = 1;
	static const int COLLISION_BACK_PLATFORM = 2;
	static const int COLLISION_PLAYER_DAMAGE = 4;
	static const int COLLISION_ENEMY_DAMAGE = 8;

#define HEADER collidable
#define HELPER scriptable_object_h
#include "include_helper.h"
	Collidable() : collisionLayers(-1), collisionsEnabled(false) { }
	Collidable(AuroraEngine* mainEngine, State* attachedState) : Renderable(mainEngine, attachedState) { collisionsEnabled = true; collisionLayers = -1; }
	Collidable(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int newCollisionLayers) : Translatable(offset), Renderable(attachedState, offset, effects, layer) { collisionsEnabled = true; collisionLayers = newCollisionLayers; }

	/** Enables collisions.
	 */
	void enableCollisions();

	/** Disables collisions.
	 */
	void disableCollisions();

	/** Telles whether collisions are enabled.
	 */
	bool getCollisionsEnabled();

	/** Sets collision layers bitmask.
	 * @param newLayers the layers to set.
	 */
	void setCollisionLayers(int newLayers);

	/** Sets collision layer.
	 * @param layer the layer to alter.
	 * @param newLayerValue the state of the layer : true if the object is present in the layer, false otherwise.
	 */
	void setCollisionLayer(int layer, bool newLayerValue);

	/** Returns collision layers.
	 * Collision layers is a bitmask that indicates on which layers this collidable is present.
	 * For example : 3 means that the collidable is present on both layer 0 and layer 1. -1 means all layers, 0 means no layer.
	 * @return the bitmask of collision layers.
	 */
	int getCollisionLayers() { return collisionLayers; }
	
	/** Returns a collision layer status.
	 * @param layer the layer to test.
	 * @return true if the object is present on this layer, false otherwise.
	 */
	bool getCollisionLayer(int layer) { xassert(layer >= 0 && layer <= 32, "Invalid collision layer : %d", layer); return collisionLayers & 1 << layer; }
	
	/** Get the collision type (none, point, rect, hull...)
	 */
	virtual inline int getCollisionType() { return COLLISION_TYPE_NONE; }

	/** Get the size of the object.
	 * This function returns the size of a rectangle that fits entirely the object. It is not necessarily the size on screen nor the size of the texture to be displayed, but such behaviour is recommended.
	 * @return the size of a rectangle that bounds this object.
	 */
	virtual Point getSize() = 0;

	/** Test which object hits another.
	 * The collision test can offer details about the collision by telling which sub-object was hit.
	 * Remember that only CollisionHulls can actually hit together : please avoid endless loops when using sub-Collidables.
	 * @param other the other object of the collision test.
	 * @return a collision.
	 */
	virtual Collision hitBy(Collidable* other) = 0;
	
	/** Called whenever this object collided another.
	 * @param collision the collision descriptor.
	 */
	virtual void collided(Collision* collision) {}

	/** Get the leftmost point of the collision zone of an object.
	 * This function returns the point which is on the left side of an object.
	 * This point is then used for object ordering.
	 * @return the leftmost point of the collision zone or NULL if the object is not known as convex.
	 */
	virtual Translatable* getLeftmostPoint() = 0;

	/** Get the rightmost point of the collision zone of an object.
	 * This function returns the point which is on the right side of an object.
	 * This point is then used for object ordering.
	 * @return the rightmost point of the collision zone or NULL if the object is not known as convex.
	 */
	virtual Translatable* getRightmostPoint() = 0;

	/** Tells if this collidable has perspective.
	 * A collidable has a perspective if it has a leftmostPoint and a rightmostPoint.
	 * @return true if the object has perspective, false otherwise.
	 */
	bool hasPerspective();
	
	/** Put a translatable on the top surface point.
	 * @param t the translatable to move.
	 * @return true if the point exists, false if outside of the collidable.
	 */
	virtual bool getDistanceFromBorders(Collidable* c, Resizable* dist) { return false; }
	
	/** Callback to signal that collision detection is starting.
	 * @param elapsedTime time since last call.
	 */
	virtual void preCollisions(dur elapsedTime) {}

	/** Callback to signal that collision detection is finished.
	 * @param elapsedTime time since last call.
	 */
	virtual void postCollisions(dur elapsedTime) {}

	/* Signalable */
	
	virtual int signal(ScriptableObject* other, SignalId signalId, int data);

	/* Translatable */

	virtual bool inside(Translatable& firstCorner, Translatable& secondCorner);
	
};

#endif
