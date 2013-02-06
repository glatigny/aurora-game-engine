#ifndef COLLISION_HULL_H
#define COLLISION_HULL_H

#include "collidable.h"
#include "state.h"

/** A CollisionHull is a zone which can collide with another.
 * Unlike most other Collidables, CollisionHulls have no dependencies on other classes to compute a collision.
 * Most other Collidables rely on CollisionHulls for their own collisions.
 */
class CollisionHull : public Collidable {
private:
	bool m_contour;
	int wp;
	Point m_size;
	Point p1, p2;
	char* m_filename;
	unsigned char** m_map;
	unsigned short (**contour_map);

	/** Internal function in order to load a RAW image Collision
	 * @param filename of image you want to load
	 * @return special png structure with image data
	 */
	void* loadPng(const char* p_file);
	bool map(int p_x, int p_y);
	void clean();

protected:
	/** Computes a collision between two CollisionHulls.
	 * This method is where collision detection is actually computed.
	 * @param otherHull the other collision hull to compare with this one.
	 * @return otherHull if a collision happened, NULL otherwise.
	 */
	Collision hitBy(CollisionHull* otherHull);
	Collision hitBy(CollisionPoint* otherHull);
	Collision hitBy(CollisionRect* otherHull);
public:
#define HEADER collision_hull
#define HELPER scriptable_object_h
#include "include_helper.h"
	CollisionHull() : Collidable(), m_contour(NULL), m_filename(NULL), m_map(NULL), contour_map(NULL) {}
	CollisionHull(State* attachedState, Translatable* offset, int layer, int collisionLayers, VFS::File* filename, bool contour);
	virtual ~CollisionHull();

	void load(const char* p_file, bool p_contour);

	/* Collidable */
	
	virtual Translatable* getLeftmostPoint() {
		return &p1;
	}
	
	virtual Translatable* getRightmostPoint() {
		return &p2;
	}
	
	virtual inline int getCollisionType() { return COLLISION_TYPE_HULL; }

	virtual Point getSize();	
	virtual Collision hitBy(Collidable* other);

	/* Renderable */
	
	/** CollisionHulls are not renderable, so this function does nothing. */
	void renderGraphics() { }

	/** CollisionHulls are static, so this function does nothing. */
	void updateObject(dur elapsedTime) { }

	virtual void loadGraphics();
	virtual void unloadGraphics();
};

#endif

