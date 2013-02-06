#ifndef RENDERABLE_H
#define RENDERABLE_H

class Renderable;
class InterpolationDescriptor;
class HardwareEngine;
namespace VFS {
	class ImageSource;
}

#include "graphical_effects.h"
#include "translatable.h"
#include "realtime.h"
#include "blocking.h"
#include "scriptable_object.h"

// Render to all slots
#define ALL_SLOTS -1

/** An object that can be drawn on screen.
 */
class Renderable :
	virtual public ScriptableObject,
	virtual public Translatable,
	virtual public Realtime
{ friend class RenderableInterpolator; friend class EffectInterpolator;

private:
	int layer;

	/* Rendering cache */
	unsigned int finalCacheValid;
	unsigned int parentCacheValid;
	Renderable* renderingCache; // The rendering cache itself. Can be NULL even if finalCacheValid is true : it means that finalPosition and finalEffects are OK, but not the rendering cache, which is optional.
	Rect renderingCacheSurface; // The surface covered by renderingCache.

#ifdef GPU_IMAGEPROCESS
	HardwareEngine* hwEngine; // Used to trigger hardwareChanged
	void resetImageProcess();
#endif

	Renderable* underlay;
	Renderable* overlay;

protected:
	Point finalPosition;
	GraphicalEffects finalEffects;
	GraphicalEffects objectEffects;
	Renderable* parent;
	Renderable* substituteObject;
	Blocking* substituteLifespan;
	InterpolationDescriptor* interpolation;
	dur interpolationPosition;

	/** Do the actual rendering of the graphics.
	 * This function is called by render(), to do the actual rendering after computing final position and effects.
	 */
	virtual void renderGraphics() = 0;

#ifdef GPU_IMAGEPROCESS
	/** Called when the hardware engine pointer changed.
	 */
	void hardwareChanged();
#endif

	void renderGraphicsWithPostProcess();

	void substitutionExpired();

	void makeFinalCacheValid();

	/** Invalidates the final position and final effects cache.
	 */
	void invalidateFinalCache();

	/** Ensures that the final rendering is freed.
	 */
	void invalidateRenderingCache();

	/** Tells whether the finalCache is valid.
	 * @return true if the current state of the final cache is valid, false otherwise.
	 */
	bool isFinalCacheValid();

	/** Computes the final position depending on the parent's properties.
	 */
	virtual void computeFinalPosition();

	/** Computes the final effects depending on the parent's properties.
	 */
	void computeFinalEffects() ;

	void updateInterpolation(dur elapsedTime);

public:
#define HEADER renderable
#define HELPER scriptable_object_h
#include "include_helper.h"
	// QC:?
	Renderable() : Translatable(), layer(0), finalCacheValid(0), parentCacheValid((unsigned int)-1), renderingCache(NULL),
#ifdef GPU_IMAGEPROCESS
	hwEngine(NULL),
#endif
	underlay(NULL), overlay(NULL), parent(NULL), substituteObject(NULL), substituteLifespan(NULL), interpolation(NULL) {
	}

	Renderable(const Renderable& src);

	Renderable(AuroraEngine* mainEngine, State* newAttachedState);
	Renderable(State* newAttachedState, Translatable* offset, GraphicalEffects* effects, int newLayer);

	virtual ~Renderable() { // QC:P
		if(substituteObject)
			cancelSubstitution(); 
		if(renderingCache)
			delete renderingCache;
	}

	/** Set the parent object.
	 * If the object already has a parent, it will be translated, rotated and scaled to keep the same absolute position, rotation and scale.
	 * Other effects will be inherited from the new parent.
	 * This way, reparenting an object will have no visual impact if no special effect is used.
	 * @param parent the new parent object of this renderable.
	 */
	virtual void setParent(Renderable* parent);

	/** Temporarily set the parent.
	 * The object is not reparented like with setParent.
	 * @param tempParent the parent to set.
	 */
	void setTempParent(Renderable* tempParent);


	/** Sets the cache surface rect.
	 * Because computing the position and the size of the cache surface can be tricky, it can be manually set to fine tune optimization.
	 *
	 * The parameter only defines the minimum size of the surface to cache : the surface may be bigger and cropping may not be precise.
	 * Please do not use rendering cache on a small surface for cropping effects.
	 *
	 * @param minimumSurface a rect enclosing the minimum surface to cache. If the surface is too small, cropping may appear.
	 */
	virtual void setCacheSurface(Resizable* minimumSurface);

	/** Render this object on screen or in its parent cache.
	 * @param slot The texture slot to render. To render to all the texture slots, use ALL_SLOTS.
	 * @see GraphicalEffect
	 * @see setObjectGraphicalEffect
	 */
	virtual void render();

	/** Renders the current object to another object.
	 * Remember that a rendering consume a lot of video memory, so use this function wisely.
	 *
	 * A neat trick is to render to an object, then unloadGraphics() to free original textures from memory.
	 * This allows to scale down graphics or to render complex objects with great memory and CPU savings.
	 * @return a platform-specific object that contains the image of this renderable.
	 */
	virtual Renderable* renderToObject();

	/** Load the data of the renderable object.
	 * Used for preloading textures.
	 *
	 * This function does not affect rendering cache.
	 */
	virtual void loadGraphics() = 0;

	/** Unload the data of the renderable object.
	 * Used for unloading textures to free memory.
	 *
	 * This function does NOT affect rendering cache, which allows to render-then-free-source for great memory usage optimization.
	 */
	virtual void unloadGraphics() = 0;

	/** Get the current effects linked to this object.
	 * @return the current effects applied to this object.
	 */
	virtual GraphicalEffects* getObjectGraphicalEffects() {
		return &objectEffects;
	}

	/** Set the graphical effect linked to this object.
	 * @param effects the structure holding information about effects to apply to this object.
	 * @see GraphicalEffects
	 */
	virtual void setObjectGraphicalEffects(const GraphicalEffects& effects);

	/** Set the graphical effects of thos object using an effect string.
	 * @param effectString the string that holds effects and their parameters.
	 * @see GraphicalEffects::setEffects
	 */
	virtual void setEffects(const char* effectString) { objectEffects.setEffects(effectString); }

	/** Set the value of a graphical effect.
	 * The format is the same as an effect string, except that it takes only one effect.
	 * @param effectId the ID of the effect.
	 * @param effectValue the value to apply to the effect.
	 */
	void setEffect(const char* effectId, const char* effectValue) { objectEffects.setEffect(effectId, effectValue); }

	/** Sets a custom effect's value.
	 * This function only works for single-valued effects.
	 * @param effectId the ID of the effect to alter.
	 * @param effectValue the new value of the effect.
	 */
	void setIntegerEffect(const char* effectId, int effectValue) { objectEffects.setIntegerEffect(effectId, effectValue); }

	/** Resets the graphical effects.
	 */
	virtual void resetEffects() { objectEffects.reset(); }

	/** Reset an extended effect.
	 * @param effectId the ID of the effect to reset to its default value.
	 */
	void resetEffect(const char* effectId) { objectEffects.resetEffect(effectId); }

	/** Attach a pre-processing pass to this Renderable.
	 * Pre-processing applies to the background of the sprite, just before blitting the sprite.
	 * Pre-processing string format :
	 * <shader name>:<constant1>:<constant2>:...
	 * Example :
	 * "displacement_blur:5.0:8.0:12.0" // Will do a displacement map with gaussian blur, with X coefficient to 5.0, Y coefficient to 8.0 and blur coefficient to 12.0
	 * @param preProcessString the string describing the pass.
	 * @return true if the effect is supported, false otherwise.
	 */
	bool setPreProcess(const char* preProcessString);

	/** Attach a post processing pass to this Renderable.
	 * Post processing string format :
	 * <shader name>:<constant1>:<constant2>:...
	 * Example :
	 * "displacement_blur:5.0:8.0:12.0" // Will do a displacement map with gaussian blur, with X coefficient to 5.0, Y coefficient to 8.0 and blur coefficient to 12.0
	 * @param postProcessString the string describing the pass.
	 * @return true if the effect is supported, false otherwise.
	 */
	bool setPostProcess(const char* postProcessString);

	/** Change this object's owner.
	 * @param newState the new owner of this object.
	 */
	void attachToState(State* newState) { assert(newState != NULL); attachedState = newState; }

	/* Graphical effects */

	/** Sets the rotation angle of this object.
	 * @param angle the new angle of the object.
	 */
	void setRotation(coord angle);

	/** Rotates the object by a certain angle.
	 * @param angle the angle to add to current rotation value.
	 */
	void rotate(coord angle);

	/** Returns the current rotation angle of the object.
	 */
	coord getRotation();

	/** Returns the current scale of this object.
	 */
	Point getScale() { return objectEffects.getScale(); }

	/** Set this object's scale.
	 * @param newScale sets the scale of this object, in percents.
	 */
	void setScale(Translatable* newScale);

	/** Set this object's scale.
	 * @param newScale sets the scale of this object, in percents.
	 */
	void setScale(Point newScale) { setScale(&newScale); }

	/** Set this object's horizontal scale.
	 * @param newHorizontalScale the new horizontal scale of the object.
	 */
	void setHorizontalScale(coord newHorizontalScale);

	/** Set this object's vertical scale.
	 * @param newVerticalScale the new vertical scale of the object.
	 */
	void setVerticalScale(coord newVerticalScale);

	/** Flips this object horizontally.
	 * The object is flipped if it was not and turned back to normal display if it was flipped.
	 */
	void flipHorizontally();

	/** Flips this object vertically.
	 * The object is flipped if it was not and turned back to normal display if it was flipped.
	 */
	void flipVertically();

	/** Sets object's global opacity.
	 * @param globalOpacity the opacity to apply to the object. 0 means fully transparent and 255 means fully opaque.
	 */
	void setOpacity(unsigned char globalOpacity);

	/** Get the absolute position of an element in this object's coordinates system.
	 * @param pos the position to evaluate, relative to this object.
	 * @return the absolute position of the translatable.
	 */
	Point getAbsolutePosition(Translatable* pos);

	/** Get the absolute rotation angle of this element.
	 * The angle is updated when the render() function is called.
	 * @return the last absolute rotation angle of this object.
	 */
	coord getAbsoluteRotation() {
		return getFinalEffects()->getRotation();
	}

	/** Get the absolute scale of this element.
	 * @return the absolute scale of this object.
	 */
	Point getAbsoluteScale() {
		return getFinalEffects()->getScale();
	}

	/** Get the final effects of this object.
	 * @return the final effects, with all parent effects accumulated.
	 */
	GraphicalEffects* getFinalEffects();
	
	/** Returns the position of another object in this renderable's space.
	 * @param absoluteCoordinates the position to evaluate.
	 * @return the position of that point relative to the member.
	 */
	Point getRelativePosition(Translatable* absoluteCoordinates);

	/** Returns the position of another renderable in this renderable's space.
	 * @param other the object to evaluate.
	 * @return the position of that renderable relative to the member.
	 */
	Point getRelativePosition(Renderable* other);

	/** Returns the position of a point inside another renderable's space.
	 * @param other the renderable to use as source space.
	 * @param cordinates the position to evaluate in the member's coordinate system.
	 * @return the position of that renderable relative to the member.
	 */
	Point getRelativePositionFrom(Renderable* other, Translatable* coordinates);

	coord getRelativeRotation(Renderable* other);
	Point getRelativeScale(Renderable* other);

	coord getRelativeRotationFrom(Renderable* other, coord angle);
	Point getRelativeScaleFrom(Renderable* other, Translatable* scale);

	/** Sets the absolute position of the object.
	 * @param coordinates the place to put object to. These coordinates are in screen space.
	 */
	void setAbsolutePosition(Translatable* coordinates);
	void setAbsolutePosition(Translatable& coordinates) { setAbsolutePosition(&coordinates); }

	/** Sets the absolute rotation angle of the object.
	 * @param angle the new angle of the object, relative to screen space.
	 */
	void setAbsoluteRotation(coord angle);

	/** Sets the absolute scale of the object.
	 * @param scale the scale of the object, relative to screen space.
	 */
	void setAbsoluteScale(Translatable* scale);
	void setAbsoluteScale(Translatable& scale) { setAbsoluteScale(&scale); }

	/** Sets the position of the object relatively to another object's space.
	 * @param other the coordinate reference.
	 * @param coordinates the coordinates to put the object to.
	 */
	void setRelativePosition(Renderable* other, Translatable* coordinates) {
		setPosition(getRelativePositionFrom(other, coordinates));
	}

	/** Sets the rotation angle of the object relatively to another object's space.
	 * @param other the coordinate reference.
	 * @param angle the relative angle of this object to the reference.
	 */
	void setRelativeRotation(Renderable* other, coord angle) {
		setRotation(getRelativeRotationFrom(other, angle));
	}

	/** Sets the scale of the object relatively to another object's space.
	 * @param other the coordinate reference.
	 * @param scale the relative scale of the object to the reference.
	 */
	void setRelativeScale(Renderable* other, Translatable* scale) {
		setScale(getRelativeScaleFrom(other, scale));
	}

	/** Adds an overlay to this object.
	 * @param overlay the object to overlay to this one. The overlay will be reparented.
	 */
	void setOverlay(Renderable* overlay);

	/** Returns the current overlay of this object.
	 * The overlay will be rendered after the actual object.
	 * @return the current overlay of this object.
	 */
	Renderable* getOverlay();

	/** Adds an underlay to this object.
	 * The underlay will be rendered before the actual object.
	 * @param underlay the object to underlay to this one. The underlay will be reparented.
	 */
	void setUnderlay(Renderable* underlay);

	/** Returns the current underlay of this object.
	 * @return the current underlay of this object.
	 */
	Renderable* getUnderlay();

	/** Temporarily substitutes this renderable by another.
	 * This function sets another renderable to be updated and rendered in place of this one.
	 * It is mainly used for transitions.
	 *
	 * @param newSubstituteObject the object to substitute to this Renderable.
	 * @param until a blocking object indicating when to release the substitute and return to normal operation. This pointer must be valid during the substitute's lifetime since no memory management will be done on this object. Pass Blocking::alwaysBlocking for a permanent substitute.
	 */
	void substitute(Renderable* newSubstituteObject, Blocking* until = &alwaysBlocking);
	void substitute(Renderable* newSubstituteObject, ScriptableObject* until) {
		substitute(newSubstituteObject, dynamic_cast<Blocking*>(until));
	}

	/** Waits until substitution has finished.
	 * This is a blocking function.
	 *
	 * @return true if the substitution is currently active, false otherwise.
	 */
	bool waitSubstitute() {
		return substituteObject;
	}

	/** Cancel this object's substitution.
	 * The object will be turned back to its normal operation.
	 */
	void cancelSubstitution();

	/** Interpolates this renderable with another.
	 * @param targetPosition the destination to reach.
	 * @param duration the duration of the transition, in milliseconds.
	 */
	virtual void interpolate(Renderable* target, const char* interpolationString, dur duration);

	bool interpolate_block(Renderable* target, const char* interpolationString, dur duration) {
		return interpolation;
	}

	/** Waits for interpolation to finish.
	 * Blocking function for scripts.
	 * @return true if interpolation is active, false otherwise.
	 */
	bool waitInterpolation() { return interpolation; }

	/** Return the current layer of this object.
	 * Layers are used to position objects in the Z axis.
	 * @return the current layer of the object.
	 */
	virtual int getLayer() {
		return layer;
	}

	/** Return an ImageSource from this renderable.
	 * @param file the file to save image to.
	 * @return the ImageSource of the file.
	 */
	virtual VFS::ImageSource* renderToImageSource() {
		Renderable* cache = renderToObject();
		VFS::ImageSource* source = cache->renderToImageSource();
		delete cache;
		return source;
	}

	/** Enable the rendering cache on this object.
	 * If enabled, the object will be drawn to a temporary surface, enabling backing store
	 * and full effects support.
	 * @param minimumSurface the surface to allocate to the temporary surface.
	 */
	void enableRenderingCache(const Rect& minimumSurface, int slot = 0) {
		renderingCacheSurface = minimumSurface;
		objectEffects.enableRenderingCache(slot);
		invalidateRenderingCache();
	}

	/** Disable the rendering cache.
	 * This function disables the rendering cache for this object.
	 */
	void disableRenderingCache(int slot = 0) {
		objectEffects.disableRenderingCache(slot);
	}

	/** Change the layer of the object.
	 * @param layer the new layer.
	 */
	virtual void setLayer(int newLayer) {
		layer = newLayer;
		invalidateFinalCache();
	}

	/** Change the blending mode.
	 * @param mode the blending mode.
	 */
	virtual void setBlendMode(int mode);

	/* Realtime */

	virtual bool updateSubstitute(dur elapsedTime);
	virtual void updateOverlay(dur elapsedTime);
	virtual void updateUnderlay(dur elapsedTime);

	/* Translatable */

	// QC:?
	virtual void setX(coord newX) {
		x = newX;
		invalidateFinalCache();

		if(substituteObject)
			substituteObject->setX(newX);
	}

	// QC:?
	virtual void setY(coord newY) {
		y = newY;
		invalidateFinalCache();

		if(substituteObject)
			substituteObject->setY(newY);
	}

	virtual Translatable* getAbsolutePosition();

	/* Blocking */

	virtual bool isFinished() {
		return !interpolation && (!substituteLifespan || substituteLifespan->isFinished());
	}
	
	/* AOEObject */
	virtual VFS::File* source();

#ifdef POINTER_TRACKING
	virtual void reload();
#endif
};
#endif
