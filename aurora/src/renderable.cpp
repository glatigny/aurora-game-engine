#include "renderable.h"
#include "hardware_engine.h"
#include "renderable_interpolator.h"
#include <math.h>
#include <string.h>
#include "state.h"
#include "aurora_engine.h"
#include "vfs.h"
#include "decoration.h"
#ifdef POINTER_TRACKING
#include "game_map.h"
#endif

#define HEADER renderable
#define HELPER scriptable_object_cpp
#include "include_helper.h"

// QC:?
void Renderable::setObjectGraphicalEffects(const GraphicalEffects& effects) {
	objectEffects = effects;
	invalidateFinalCache();
}

// QC:W (TODO : finish this constructor)
Renderable::Renderable(const Renderable& src) : Translatable(src), layer(0), finalCacheValid(0), renderingCache(NULL), objectEffects(src.objectEffects), parent(NULL) {
	attachedState = src.attachedState;
	xassert(false, "Renderable copy constructor is not implemented.");
}

// QC:?
Renderable::Renderable(AuroraEngine* mainEngine, State* newAttachedState) : Translatable(), Realtime(newAttachedState), layer(0), finalCacheValid(0), parentCacheValid((unsigned int)-1), renderingCache(NULL),
#ifdef GPU_IMAGEPROCESS
hwEngine(mainEngine?mainEngine->hardware():NULL),
#endif
underlay(NULL), overlay(NULL), parent(NULL), substituteObject(NULL), substituteLifespan(NULL), interpolation(NULL) {
}

// QC:?
Renderable::Renderable(State* newAttachedState, Translatable* offset, GraphicalEffects* effects, int newLayer) : Translatable(offset), Realtime(newAttachedState), layer(newLayer), finalCacheValid(0), parentCacheValid((unsigned int)-1), renderingCache(NULL),
#ifdef GPU_IMAGEPROCESS
hwEngine(newAttachedState->engine()->hardware()),
#endif
underlay(NULL), overlay(NULL), parent(NULL), substituteObject(NULL), substituteLifespan(NULL), interpolation(NULL) {
	if(effects) {
		objectEffects = *effects;
#ifdef GPU_IMAGEPROCESS
		if(objectEffects.hasImageProcess())
			resetImageProcess();
#endif
	}
}

// QC:?
void Renderable::setParent(Renderable* newParent) {
	if(parent && newParent) {
		Point currentPos(getAbsolutePosition());
		coord currentAngle(getAbsoluteRotation());
		Point currentScale(getAbsoluteScale());

		parent = newParent;
		invalidateFinalCache();

		setAbsolutePosition(currentPos);
		setAbsoluteRotation(currentAngle);
		setAbsoluteScale(&currentScale);
	} else {
		parent = newParent;
		invalidateFinalCache();
	}
}

// QC:?
void Renderable::setTempParent(Renderable* tempParent) {
	parent = tempParent;
}

// QC:?
void Renderable::setCacheSurface(Resizable* minimumSurface) {
	renderingCacheSurface = *minimumSurface;
}

// QC:?
void Renderable::makeFinalCacheValid() {
	if(parent) {
		parent->makeFinalCacheValid();
		if(parent->finalCacheValid != parentCacheValid) {
			computeFinalPosition();
			computeFinalEffects();
			parentCacheValid = parent->finalCacheValid;
			finalCacheValid++;
		}
	} else {
		if(parentCacheValid != finalCacheValid) {
			finalPosition = this;
			finalEffects = objectEffects;
			parentCacheValid = finalCacheValid;
		}
	}
}

// QC:?
void Renderable::invalidateFinalCache() {
	// Invalidate the cache itself
	if(parent) {
		parentCacheValid--;
		parent->invalidateRenderingCache();
	}

	// Invalidate children
	finalCacheValid++;
}

// QC:?
void Renderable::invalidateRenderingCache() {
	if(renderingCache) {
		delete renderingCache;
		renderingCache = NULL;
	}
	if(parent) {
		parent->invalidateRenderingCache();
	}
}

// QC:?
bool Renderable::isFinalCacheValid() {
	if(parent) {
		return parent->finalCacheValid == parentCacheValid && parent->isFinalCacheValid();
	}
	return parentCacheValid == finalCacheValid;
}

// QC:G
void Renderable::render() {
	// Early rendering rejection
	if(objectEffects.getOpacity() == 0)
		return;

	// Do object rendering

	if(underlay) {
		underlay->render();
	}

	if(substituteObject) {
		substituteObject->render();
		return;
	}

	// Cache creation
	if(objectEffects.getRenderCache() && !objectEffects.getPreProcess() && !objectEffects.getPostProcess() && !renderingCache) {
		// Render the object to the cache
		renderingCache = renderToObject();
		assert(renderingCache);

		// Set cache parent to the same parent as the current object.
		renderingCache->setParent(parent);
	}
	
	// Display cache if available
	if(renderingCache) {
		renderingCache->setPosition(this);
		renderingCache->objectEffects = objectEffects; // TODO : do not do this each for each render
		renderingCache->renderGraphics();
	} else {
		renderGraphicsWithPostProcess(); // Normal rendering
	}

	if(overlay) {
		overlay->render();
	}
}

// QC:?
void Renderable::renderGraphicsWithPostProcess() {
#ifdef GPU_IMAGEPROCESS
	if(objectEffects.getPreProcess() || objectEffects.getPostProcess()) {
		// Temporarily set opacity
		int opacity = objectEffects.getOpacity();
		objectEffects.setOpacity(255);
		invalidateFinalCache();

		// Render to a temporary surface
		engine()->hardware()->pushRenderingSurface(NULL, objectEffects.getRenderCache(), objectEffects.getPreProcess(), objectEffects.getPostProcess());
		renderGraphics();
		Renderable* result = engine()->hardware()->popRenderingSurface(state());
		assert(result);

		// Render the temporary surface with displacement
		result->objectEffects.setOpacity(opacity);
		result->objectEffects.setPreProcess(objectEffects.getPreProcessString(), objectEffects.getPreProcess());
		result->objectEffects.setPostProcess(objectEffects.getPostProcessString(), objectEffects.getPostProcess());
		result->renderGraphics();
		delete result;

		// Restore opacity
		objectEffects.setOpacity(opacity);
		invalidateFinalCache();
	} else
#endif
		renderGraphics();
}

// QC:?
bool Renderable::setPreProcess(const char* processString) {
	if(!processString || !*processString)
		return false;
#ifdef GPU_IMAGEPROCESS
	xerror(strnlen(processString, 64) < 64, "PreProcess String too long (%d bytes). A process string cannot be longer than 63 bytes.", strnlen(processString, 256));
	void* pspecData;
	uint32_t neededSlots;
	bool supported = engine()->hardware()->loadImageProcess(processString, pspecData, neededSlots);

	if(!supported) {
		xwarn(false, "Process string %s not supported by this hardware engine. PreProcess ignored.", processString);
		return false;
	}
	if(neededSlots) {
		objectEffects.setRenderCache(objectEffects.getRenderCache() | neededSlots);
	}
	objectEffects.setPreProcess(processString, pspecData);
	return true;
#else
	xwarn(false, "Pre processing is not compiled in. Ignoring pre process effect");
	return false;
#endif
}

// QC:?
bool Renderable::setPostProcess(const char* processString) {
	if(!processString || !*processString)
		return false;
#ifdef GPU_IMAGEPROCESS
	xerror(strnlen(processString, 64) < 64, "PostProcess String too long (%d bytes). A process string cannot be longer than 63 bytes.", strnlen(processString, 256));
	void* pspecData;
	uint32_t neededSlots;
	bool supported = engine()->hardware()->loadImageProcess(processString, pspecData, neededSlots);

	if(!supported) {
		xwarn(false, "Process string %s not supported by this hardware engine. PostProcess ignored.", processString);
		return false;
	}
	if(neededSlots) {
		objectEffects.setRenderCache(objectEffects.getRenderCache() | neededSlots);
	}
	objectEffects.setPostProcess(processString, pspecData);
	return true;
#else
	xwarn(false, "Post processing is not compiled in. Ignoring post process effect");
	return false;
#endif
}

#ifdef GPU_IMAGEPROCESS
// QC:?
void Renderable::hardwareChanged() {
	hwEngine = engine()->hardware();
	resetImageProcess();
}

// QC:?
void Renderable::resetImageProcess() {
	setPreProcess(objectEffects.getPreProcessString());
	setPostProcess(objectEffects.getPostProcessString());
}
#endif

// QC:G
coord Renderable::getRotation() {
	return objectEffects.getRotation();
}

// QC:G
void Renderable::setRotation(coord angle) {
	objectEffects.setRotation(angle);
	invalidateFinalCache();
}

// QC:?
void Renderable::rotate(coord angle) {
	objectEffects.setRotation(objectEffects.getRotation() + angle);
	invalidateFinalCache();
}

// QC:?
void Renderable::setScale(Translatable* newScale) {
	objectEffects.setHorizontalScale(newScale->getX());
	objectEffects.setVerticalScale(newScale->getY());
	invalidateFinalCache();
}

// QC:?
void Renderable::setHorizontalScale(coord newHorizontalScale) {
	objectEffects.setHorizontalScale(newHorizontalScale);
	invalidateFinalCache();
}

// QC:?
void Renderable::setVerticalScale(coord newVerticalScale) {
	objectEffects.setVerticalScale(newVerticalScale);
	invalidateFinalCache();
}

// QC:?
void Renderable::flipHorizontally() {
	objectEffects.setHorizontalScale(-objectEffects.getHorizontalScale());
	invalidateFinalCache();
}

// QC:?
void Renderable::flipVertically() {
	objectEffects.setVerticalScale(-objectEffects.getVerticalScale());
	invalidateFinalCache();
}

// QC:?
void Renderable::setOpacity(unsigned char globalOpacity) {
	objectEffects.setOpacity(globalOpacity);
	invalidateFinalCache();
}

// QC:?
void Renderable::setBlendMode(int mode)
{
	objectEffects.setBlendMode(mode);
	invalidateFinalCache();
}

// QC:?
Renderable* Renderable::renderToObject() {
	// Create the rendering surface and place the object over it.
#ifdef GPU_IMAGEPROCESS
	engine()->hardware()->pushRenderingSurface(renderingCacheSurface.getSize(), objectEffects.getRenderCache(), objectEffects.getPreProcess(), objectEffects.getPostProcess());
#else
	engine()->hardware()->pushRenderingSurface(renderingCacheSurface.getSize());
#endif

	// Remember object state
	Renderable* parentSave = parent;
	Point offsetSave(*this);
	GraphicalEffects effectsSave(objectEffects);

	// Draw the object in its own space.
	parent = NULL;
	setPosition(Point());
	objectEffects.reset();

	renderGraphics();

	// Grab the resulting rendering.
	Renderable* rendered = engine()->hardware()->popRenderingSurface(state());

	// Verify the object generated by the hardware engine.
	assert(rendered);

	// The position and effect cache have been destroyed by reparent.
	invalidateFinalCache();

	// Restore object state
	setPosition(offsetSave);
	objectEffects = effectsSave;
	parent = parentSave;

	return rendered;
}

// QC:?
void Renderable::computeFinalPosition() {
	assert(parent && parent->isFinalCacheValid());

	parent->getFinalEffects()->transformCoordinates(&finalPosition, this);
	finalPosition.translate(parent->getAbsolutePosition());
}

// QC:?
void Renderable::computeFinalEffects() {
	assert(parent && parent->isFinalCacheValid());

	finalEffects = parent->finalEffects;
	finalEffects += objectEffects;
}

// QC:G
Translatable* Renderable::getAbsolutePosition() {
	if(!parent) {
		return this;
	}

	if(!isFinalCacheValid()) {
		makeFinalCacheValid();
	}

	return &finalPosition;
}

// QC:?
Point Renderable::getAbsolutePosition(Translatable* pos) {
	Point result;
	getFinalEffects()->transformCoordinates(&result, pos);
	result.translate(getAbsolutePosition());
	return result;
}

// QC:?
GraphicalEffects* Renderable::getFinalEffects() {
	if(!parent) {
		return &objectEffects;
	}

	if(!isFinalCacheValid()) {
		makeFinalCacheValid();
	}

	return &finalEffects;
}

// QC:?
Point Renderable::getRelativePosition(Translatable* absoluteCoordinates) {
	Point result(absoluteCoordinates);
	result.invertTranslate(getAbsolutePosition());
	getFinalEffects()->inverseTransformCoordinates(&result, &result);
	return result;
}

// QC:G (can be optimized if parent == other->parent)
Point Renderable::getRelativePosition(Renderable* other) {
	Point result(other->getAbsolutePosition());
	result.invertTranslate(getAbsolutePosition());
	getFinalEffects()->inverseTransformCoordinates(&result, &result);
	return result;
}

// QC:?
Point Renderable::getRelativePositionFrom(Renderable* other, Translatable* pos) {
	Point result(other->getAbsolutePosition(pos));
	result.invertTranslate(getAbsolutePosition());
	getFinalEffects()->inverseTransformCoordinates(&result, &result);
	return result;
}

// QC:?
coord Renderable::getRelativeRotation(Renderable* other) {
	xwarn(false, "Unimplemented");
	return 0;
}

// QC:?
Point Renderable::getRelativeScale(Renderable* other) {
	xwarn(false, "Unimplemented");
	return 0;
}

// QC:?
coord Renderable::getRelativeRotationFrom(Renderable* other, coord angle) {
	xwarn(false, "Unimplemented");
	return 0;
}

// QC:?
Point Renderable::getRelativeScaleFrom(Renderable* other, Translatable* scale) {
	xwarn(false, "Unimplemented");
	return 0;
}


// QC:?
void Renderable::setAbsolutePosition(Translatable* pos) {
	if(parent) {
		Point relativePos;
		parent->getFinalEffects()->inverseTransformCoordinates(&relativePos, pos);
		setPosition(relativePos);
	} else {
		setPosition(pos);
	}
}

// QC:?
void Renderable::setAbsoluteRotation(coord angle) {
	if(parent) {
		setRotation(angle - parent->getAbsoluteRotation());
	} else {
		setRotation(angle);
	}
}

// QC:?
void Renderable::setAbsoluteScale(Translatable* scale) {
	if(parent) {
		Point parentScale(parent->getAbsoluteScale());
		Point finalScale(scale->getX()*100/parentScale.getX(), scale->getY()*100/parentScale.getY());
		setScale(&finalScale);
	} else {
		setScale(scale);
	}
}

// QC:?
void Renderable::setOverlay(Renderable* newOverlay)
{
	overlay = newOverlay;
	if(overlay)
		overlay->setParent(this);
}

// QC:?
Renderable* Renderable::getOverlay() {
	return overlay;
}

// QC:?
void Renderable::setUnderlay(Renderable* newUnderlay)
{
	underlay = newUnderlay;
	if(underlay)
		underlay->setParent(this);
}

// QC:?
Renderable* Renderable::getUnderlay() {
	return underlay;
}

// QC:?
void Renderable::updateOverlay(dur elapsedTime)
{
	if(overlay)
		overlay->update(elapsedTime);
}

// QC:?
void Renderable::updateUnderlay(dur elapsedTime)
{
	if(underlay)
		underlay->update(elapsedTime);
}

// QC:G
void Renderable::substitute(Renderable* newSubstituteObject, Blocking* until)
{
	if(!until)
	{
		until = dynamic_cast<Blocking*>(newSubstituteObject);
	}

	xerror(until, "No blocking object given to substitute or object is not blocking.");

	substituteObject = newSubstituteObject;
	substituteObject->setParent(this);
	
	substituteLifespan = until;
}

// QC:G
bool Renderable::updateSubstitute(dur elapsedTime)
{
	if(!substituteObject)
	{
		updateInterpolation(elapsedTime);
		return true;
	}

	x = substituteObject->getX();
	y = substituteObject->getY();

	if(substituteLifespan->hasFinished())
	{
		cancelSubstitution();
		return true;
	}

	substituteObject->update(elapsedTime);

	return false;
}

// QC:?
void Renderable::cancelSubstitution()
{
	xassert(substituteObject, "No substitute to cancel");
	substituteObject = NULL;
}

// QC:?
void Renderable::interpolate(Renderable* target, const char* interpolationString, dur duration)
{
	if(interpolation)
		delete interpolation;

	xwarn(parent, "Interpolating an object which has no parent : you should add the object to the state.map() ?");

	interpolation = new InterpolationDescriptor(interpolationString);
	interpolation->setInterpolationBoundaries(duration, 0, getPosition(), target->getPosition(), getObjectGraphicalEffects(), target->getObjectGraphicalEffects());
	interpolationPosition = duration;
}

// QC:?
void Renderable::updateInterpolation(dur elapsedTime)
{
	if(interpolation)
	{
		interpolationPosition -= elapsedTime;
		if(interpolationPosition > 0) {
			interpolation->applyTo(this, interpolationPosition);
		} else {
			interpolation->applyTo(this, 0);
			delete interpolation;
			interpolation = 0;
		}
		invalidateFinalCache();
	}
}

VFS::File* Renderable::source() {
	VFS::File* s;
	if((s = ScriptableObject::source()))
		return s;
	return Realtime::source();
}

#ifdef POINTER_TRACKING
void Renderable::reload() {
	Renderable* o = this;
	Renderable* p = parent;

	while(p) {
		Collidable* c = dynamic_cast<Collidable*>(o);
		if(c && p == state()->map()) {
			GameMap* m = dynamic_cast<GameMap*>(p);
			xassert(m, "Could not cast state map to GameMap.");

			// Replace the object by a new version.
			m->removeObject(c);
			ScriptableObject* newObj = state()->load(c->source());
			replacePointer(c, newObj);
			c = dynamic_cast<Collidable*>(newObj);
			assert(c);
			m->addObject(c);

			return;
		}

		o = p;
		p = p->parent;
	}

	ScriptableObject* newObj = state()->load(o->source());
	replacePointer(o, newObj);
	xwarn(false, "Tried to replace an object that is not in the current map. Replaced only in script engine.");
}
#endif
