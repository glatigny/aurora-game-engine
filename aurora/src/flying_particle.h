#ifndef FLYING_PARTICLE_H
#define FLYING_PARTICLE_H

class ObjectMold;

#include "game_object.h"

/** 
 *
 */
class FlyingParticle : public GameObject {
private:
	void destroy();

protected:
	Renderable* frame;
	int direction;
	int speed;
	int lifespan;
	int	signalId;
	int signalValue;
	ObjectMold* spawnWhenDead;

public:
#define HEADER flying_particle
#define HELPER scriptable_object_h
#include "include_helper.h"
	FlyingParticle() : frame(NULL), spawnWhenDead(NULL) {}
	FlyingParticle(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int collisionLayers, Collidable* collision, Renderable* framei, int direction, int speed, int lifespan, int signalId, int signalValue, ObjectMold* spawnWhenDead);
	virtual ~FlyingParticle();

	/** Returns the current direction.
	 * @return the current direction of the particle.
	 */
	virtual int getDirection() { return direction; }

	/** Change the current direction of the flying particle.
	 * @param newDirection the new direction, in degrees.
	 */
	virtual void setDirection(int newDirection) { direction = newDirection; }

	/** Rotate the direction.
	 * @param delta the delta to apply to the direction.
	 */
	virtual void rotateDirection(int delta) { direction += delta; }

	/** Change the current speed of the flying particle.
	 * @param newSpeed the new speed value.
	 */
	virtual void setSpeed(int newSpeed) { speed = newSpeed; }

	/** Retreive the current speed of the flying particle.
	 * @return the current speed value.
	 */
	virtual const int getSpeed() { return speed; }

	/** Make the particle step for one frame.
	 * This function moves the object based on its speed and angle.
	 * @param elapsedTime amount of time passed since last call.
	 */
	virtual void step(int elapsedTime);

	/** Sets the life span of the particle.
	 * When the life span reaches 0, the particle is removed from the map.
	 * If the lifespan is already 0 or negative, the particle will not be removed from the map.
	 *
	 * @param lifespan the new lifespan of the particle.
	 */
	virtual void setLifespan(int lifespan);

	/* Signalable */
	
	virtual int signal(ScriptableObject* other, SignalId signal, int data);

	/* Renderable */

	virtual void renderGraphics();
	virtual void loadGraphics();
	virtual void unloadGraphics();

	/* Realtime */

	virtual bool updateDynamics(dur elapsedTime);

};

#endif /* FLYING_PARTICLE_H */
