#include "flying_particle.h"
#include <math.h>
#include "state.h"
#include "game_map.h"
#include "object_mold.h"
#define HEADER flying_particle
#define HELPER scriptable_object_cpp
#include "include_helper.h"

FlyingParticle::FlyingParticle(State *attachedState, Translatable *offset, GraphicalEffects *effects, int layer, int collisionLayers, Collidable *collision, Renderable *newFrame, int newDirection, int newSpeed, int newLifespan, int newSignalId, int newSignalValue, ObjectMold* newSpawnWhenDead) : 
	Entity(attachedState), 
	GameObject(attachedState, offset, effects, layer, collisionLayers, collision),
	speed(newSpeed),
	lifespan(newLifespan),
	direction(newDirection),
	signalId(newSignalId),
	signalValue(newSignalValue),
	spawnWhenDead(newSpawnWhenDead)
{
	assert( newFrame != NULL );

	frame = newFrame;
	frame->setParent(this);
}

FlyingParticle::~FlyingParticle() {
	if(spawnWhenDead)
		delete spawnWhenDead;
	if(frame)
		delete frame;
}

void FlyingParticle::setLifespan(int newLifespan) {
	lifespan = newLifespan;
}

void FlyingParticle::renderGraphics()
{
	assert( frame != NULL );
	frame->render();
}

void FlyingParticle::step(int elapsedTime)
{
	/*
		La flying particle doit pouvoir se mouvoir en utilisant les données suivantes : posX posY Angle Speed
	*/

	int newX = getX() + (int)(speed * cosf(-direction*M_PI/180));
	int newY = getY() + (int)(speed * sinf(-direction*M_PI/180));

	setPosition(newX, newY);
}

bool FlyingParticle::updateDynamics(dur elapsedTime)
{
	assert( frame != NULL );
	step(elapsedTime);
	frame->update(elapsedTime);

	GameMap* map = dynamic_cast<GameMap*>(parent);
	if(!map || !map->isInMap(this))
		return true;

	if( getCollisionHull() )
	{
		Collision c = map->hitBy(this);
		if( c.collided() && !c.getCollidingObject()->isInstance("FlyingParticle") )
		{
			c.getCollidingObject()->signal(this, (enum SignalId)signalId, signalValue);
			if( !map->isInMap(this) ) {
				return false;
			}
		}
	}
	
	if(lifespan) {
		lifespan -= elapsedTime;
		if(lifespan <= 0) {
			destroy();
			return false;
		}
	}
	return true;
}

void FlyingParticle::loadGraphics()
{
	assert( frame != NULL );
	frame->loadGraphics();
}

void FlyingParticle::unloadGraphics()
{
	assert( frame != NULL );
	frame->unloadGraphics();
}

int FlyingParticle::signal(ScriptableObject *other, SignalId signal, int data)
{
	if(signal == DAMAGE) {
		lifespan = -1;
	}

	return 0;
}

void FlyingParticle::destroy() {
	assert(this > (FlyingParticle*)0x1000);
	GameMap* map = dynamic_cast<GameMap*>(parent);
	if(!map || !map->isInMap(this))
		return;

	if(spawnWhenDead) {
		Collidable* spawnedObject = scriptable_object_cast<Collidable*>(spawnWhenDead->create(state()));
		spawnedObject->setPosition(this);
		map->addObject(spawnedObject);
		map->own(spawnedObject);
		spawnWhenDead = NULL;
	}

	map->removeObject(this);
}
