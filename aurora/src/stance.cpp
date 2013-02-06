#include "stance.h"
#include "state.h"
#include "effect_key.h"
#define HEADER stance
#define HELPER scriptable_object_cpp
#include "include_helper.h"

Stance::Stance()
{
	for(int i = 0; i < 9; i++)
	{
		animations[i] = NULL;
		translation[i].setPosition(0,0);
	}
}

Stance::Stance(
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
		Renderable* animDL,	Translatable* transDL,
		Renderable* animD,	Translatable* transD,
		Renderable* animDR,	Translatable* transDR
	      )
: Translatable(offset), Entity(attachedState), Renderable(attachedState, offset, effects, layer) // TODO : add layer
{
	animations[0] = animDL;	translation[0].setPosition(*transDL);
	animations[1] = animD;	translation[1].setPosition(*transD);
	animations[2] = animDR;	translation[2].setPosition(*transDR);
	animations[3] = animL;	translation[3].setPosition(*transL);
	animations[4] = animC;	translation[4].setPosition(*transC);
	animations[5] = animR;	translation[5].setPosition(*transR);
	animations[6] = animUL;	translation[6].setPosition(*transUL);
	animations[7] = animU;	translation[7].setPosition(*transU);
	animations[8] = animUR;	translation[8].setPosition(*transUR);
	currentDirection = ControllerState::NO_MOVE;

	for(int i = 8; i >= 0; i--) {
		if(!animations[i]) {
			Point p;
			animations[i] = new EffectKey(state(), &p, NULL, 0);
			own(animations[i]);
		}
		animations[i]->setParent(this);
	}
}

Stance::~Stance() {
	for(int i = 8; i >= 0; i--) {
		if(animations[i]) {
			delete animations[i];
		}
	}
}

void Stance::setDirection(ControllerState::Direction newDirection)
{
	currentDirection = newDirection;
}

Renderable* Stance::getCurrentAnimation()
{
	if( (currentDirection-1) >= 0 && animations[currentDirection-1] )
	{
		return animations[currentDirection-1];
	}
	else if( animations[ControllerState::NO_MOVE-1] )
	{
		return animations[ControllerState::NO_MOVE-1];
	}
	return NULL;
}

ControllerState::Direction Stance::getDirection()
{
	return currentDirection;
}

void Stance::rotateStanceCW()
{
	int val[] = {3,0,1,6,4,2,7,8,5};
	setDirection((ControllerState::Direction)val[currentDirection]);
}

void Stance::rotateStanceCCW()
{
	int val[] = {1,2,5,0,4,8,3,6,7};
	setDirection((ControllerState::Direction)val[currentDirection]);
}

void Stance::restart()
{
	if( (currentDirection-1) >= 0 && animations[currentDirection-1] )
	{
		animations[currentDirection-1]->restart();
	}
	else if( animations[ControllerState::NO_MOVE-1] )
	{
		animations[ControllerState::NO_MOVE-1]->restart();
	}
}

Translatable* Stance::getTranslation()
{
	if( (currentDirection-1) >= 0 && animations[currentDirection-1] )
	{
		return &translation[currentDirection-1];
	}
	else if( animations[ControllerState::NO_MOVE-1] )
	{
		return &translation[ControllerState::NO_MOVE-1];
	}
	static Point noTranslation;
	return &noTranslation;
}

Translatable* Stance::getPreviousTranslation()
{
	int val[] = {3,0,1,6,4,2,7,8,5};
	return &translation[ val[currentDirection-1] ];
}

Translatable* Stance::getNextTranslation()
{
	int val[] = {1,2,5,0,4,8,3,6,7};
	return &translation[ val[currentDirection-1] ];
}

void Stance::setTranslation(Translatable *p_translation, ControllerState::Direction p_direction)
{
	translation[p_direction].setPosition(*p_translation);
}

void Stance::setAnimation(Renderable *p_animation, ControllerState::Direction p_direction)
{
	if( (p_direction > 0) && (p_direction <= 10) )
	{
		animations[p_direction-1] = p_animation;
	}
}

void Stance::renderGraphics()
{
	if( (currentDirection-1) >= 0 && animations[currentDirection-1] )
	{
		animations[currentDirection-1]->render();
	}
	else if( animations[ControllerState::NO_MOVE-1] )
	{
		animations[ControllerState::NO_MOVE-1]->render();
	}
}

void Stance::updateObject(dur elapsedTime)
{
	if( (currentDirection-1) >= 0 && animations[currentDirection-1] )
	{
		animations[currentDirection-1]->update(elapsedTime);
	}
	else if( animations[ControllerState::NO_MOVE-1] )
	{
		animations[ControllerState::NO_MOVE-1]->update(elapsedTime);
	}
}

bool Stance::isFinished() {
	if(!Renderable::isFinished()) {
		return false;
	}

	if( (currentDirection-1) >= 0 && animations[currentDirection-1] ) {
		return animations[currentDirection-1]->isFinished();
	}

	return true;
}

void Stance::loadGraphics()
{
	for( int i = 0; i <= 8; i++)
	{
		if(animations[i])
			animations[i]->loadGraphics();
	}
}

void Stance::unloadGraphics()
{
	for( int i = 0; i <= 8; i++)
	{
		if(animations[i])
			animations[i]->unloadGraphics();
	}
}

bool Stance::inside(Translatable& firstCorner, Translatable& secondCorner)
{
	return false;
}
