#include "decoration.h"

#define HEADER decoration
#define HELPER scriptable_object_cpp
#include "include_helper.h"

/*Decoration::Decoration(State* attachedState) :
	GameObject(attachedState)
{
}*/

// QC:P
Decoration::Decoration(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int collisionLayers, Translatable* newParallax,
                       Collidable* collision) :
	Translatable(offset), Entity(attachedState), GameObject(attachedState, offset, effects, layer, collisionLayers, collision), Resizable(offset)
{
	parallax.setPosition(*newParallax);
}

Decoration::~Decoration() {
	for (std::vector<Renderable*>::iterator it = graphics.begin(); it != graphics.end(); it++) {
		delete *it;
	}
}

// QC:P
Translatable* Decoration::getParallax() {
	return &parallax;
}

// QC:P
void Decoration::setParallax(Translatable* newParallax) {
	if (newParallax) {
		parallax.setPosition(*newParallax);
	} else {
		parallax.setPosition(100, 100);
	}
}

// QC:P
void Decoration::addFrame(Renderable* newFrame) {
	assert(newFrame);

	graphics.insert(graphics.end(), newFrame);
	newFrame->setParent(this);
}

// QC:G
Renderable* Decoration::getFrame(unsigned int position) {
	assert( position < graphics.size() );

	return graphics[position];
}

// QC:?
void Decoration::computeFinalPosition() {
	Renderable::computeFinalPosition();
		
	if(parallax.getX() != 100 || parallax.getY() != 100) {
		finalPosition.setPosition(finalPosition.getX()*parallax.getX()/100, finalPosition.getY()*parallax.getY()/100);
	}
}

// QC:P
void Decoration::renderGraphics() {
	// Render all subgraphics
	for (std::vector<Renderable*>::iterator it = graphics.begin(); it != graphics.end(); it++) {
		(*it)->render();
	}
}

// QC:P
void Decoration::updateObject(dur elapsedTime) {
	for (std::vector<Renderable*>::iterator it = graphics.begin(); it != graphics.end(); it++) {
		(*it)->update(elapsedTime);
	}
}

// QC:P
void Decoration::loadGraphics() {
	for (std::vector<Renderable*>::iterator it = graphics.begin(); it != graphics.end(); it++) {
		(*it)->loadGraphics();
	}
}

// QC:P
void Decoration::unloadGraphics() {
	for (std::vector<Renderable*>::iterator it = graphics.begin(); it != graphics.end(); it++) {
		(*it)->unloadGraphics();
	}
}
