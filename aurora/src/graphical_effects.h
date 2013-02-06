#ifndef GRAPHICAL_EFFECTS_H
#define GRAPHICAL_EFFECTS_H

#include "point.h"
#include "rect.h"
#include <map>
#include <vector>

class Renderable;

// Maximum number of sprites for motion blur effect
#define MOTION_BLUR_DETAIL_LEVEL 16

// Minimum distance between two sprites of the motion blur effect
#define MOTION_BLUR_MIN_STEPPING 2

#define BLEND_INHERIT       0
#define BLEND_NORMAL        1
#define BLEND_ADD           2
#define BLEND_MULTIPLY      3
#define BLEND_SUB           4
#define BLEND_ADD_ALPHA     5
#define BLEND_NOALPHA       6
#define BLEND_ADDMUL        7
#define BLEND_INVERSE       8
#define BLEND_ALPHAMASK     9
#define BLEND_ALPHALOCK     10
#define BLEND_ADD_ALPHALOCK 11


typedef struct { char processString[64]; void* pspecData; } ImageProcess;

typedef union {
		int integer;
		coord coordinate;
		struct {
			int amount;
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		} color;
		struct {
			coord x;
			coord y;
		} point;
		uint32_t flags;
} CustomGraphicalEffectsValue;

class BasicGraphicalEffects {
	friend class GraphicalEffects;
private:
	coord hScale;
	coord vScale;
	coord rotAngle;
	unsigned char opacity;
	int blendMode;
	uint32_t renderCache;
	Rect section;
#ifdef GPU_IMAGEPROCESS
	char preProcessString[64];
	void* preProcess;
	char postProcessString[64];
	void* postProcess;
#endif

	BasicGraphicalEffects() :
		hScale(100),
		vScale(100),
		rotAngle(0),
		opacity(255),
		blendMode(BLEND_INHERIT),
		renderCache(0),
		section(0,0,-1,-1)
#ifdef GPU_IMAGEPROCESS
		, preProcess(NULL)
		, postProcess(NULL)
#endif
	{
#ifdef GPU_IMAGEPROCESS
		preProcessString[0] = '\0';
		postProcessString[0] = '\0';
#endif
	}

	/** Effects combination. */
	// QC:G
	BasicGraphicalEffects operator+(BasicGraphicalEffects& other) const {
		BasicGraphicalEffects e(*this);
		e += other;
		return e;
	}

	/** Combines effects together. */
	// QC:A
	const BasicGraphicalEffects& operator+=(BasicGraphicalEffects& other) {
		hScale = hScale * other.hScale / (coord)100;
		vScale = vScale * other.vScale / (coord)100;
		rotAngle += other.rotAngle;
		rotAngle = modc(rotAngle, 360);
		opacity = (unsigned int)opacity * other.opacity / 255;
		if(blendMode == BLEND_INHERIT) {
			blendMode = other.blendMode;
		}

		if(section.getW() != -1
		&& section.getH() != -1
		) {
			if(other.section.getW() != -1
			&& other.section.getH() != -1
			) {
				section *= other.section;
			}
		} else {
			if(other.section.getW() != -1
			&& other.section.getH() != -1
			) {
				section = other.section;
			}
		}

#ifdef GPU_IMAGEPROCESS
/*#ifndef NDEBUG
		if(*preProcessString)
			xassert(!*other.preProcessString, "Combining pre processing effects has no sense.");
		if(*postProcessString)
			xassert(!*other.postProcessString, "Combining post processing effects has no sense.");
#endif
		if(*other.preProcessString && !*preProcessString) {
			preProcess = other.preProcess;
			memcpy(preProcessString, other.preProcessString, 64);
			xassert(preProcess, "Pre-processing string without pspec data.");
		}
		if(*other.postProcessString && !*postProcessString) {
			postProcess = other.postProcess;
			memcpy(postProcessString, other.postProcessString, 64);
			xassert(postProcess, "Post-processing string without pspec data.");
		}*/
#endif

		return *this;
	}

	// QC:A
	const BasicGraphicalEffects& operator-=(BasicGraphicalEffects& other) {
		hScale = hScale * 100 / other.hScale;
		vScale = vScale * 100 / other.vScale;
		rotAngle -= other.rotAngle;
		rotAngle = modc(rotAngle, 360);
		opacity = (unsigned int)opacity * 255 / other.opacity;
		// TODO : implement section, if possible
		return *this;
	}

};

class GraphicalEffects {
friend class RenderableInterpolator;
friend class InterpolationDescriptor;
friend class EffectInterpolator;
public:
	typedef std::map<uint32_t, CustomGraphicalEffectsValue> CustomEffectMap;
private:
	BasicGraphicalEffects* basic;

	CustomEffectMap custom;

	static const CustomGraphicalEffectsValue defaultCustomGraphicalEffectsValue;

	void makeBasicStructure() {
		if(!basic) {
			basic = new BasicGraphicalEffects();
		}
	}

	static CustomGraphicalEffectsValue decodeValueFromEffectString(const char* string);

	void mergeCustomParameters(const GraphicalEffects& other);

public:
	// QC:G
	GraphicalEffects() :
		basic(NULL)
	{
	}

	~GraphicalEffects() {
		if(basic) {
			delete basic;
		}
	}

	// QC:G
	GraphicalEffects(const GraphicalEffects& other) :
		custom(other.custom)
	{
		if(other.basic) {
			basic = new BasicGraphicalEffects(*(other.basic));
		} else {
			basic = NULL;
		}
	}

	const GraphicalEffects& operator=(const GraphicalEffects& other) {
		BasicGraphicalEffects* otherBasicEffects = other.basic;
		if(otherBasicEffects) {
			if(!basic)
				basic = new BasicGraphicalEffects(*otherBasicEffects);
			*basic = *otherBasicEffects;
		} else {
			if(basic) {
				delete basic;
				basic = NULL;
			}
		}

		custom = other.custom;

		return *this;
	}

	/** Effects combination. */
	GraphicalEffects operator+(const GraphicalEffects& other) const {
		GraphicalEffects e(*this);
		e += other;
		return e;
	}

	/** Combines effects together. */
	const GraphicalEffects& operator+=(const GraphicalEffects& other) {
		if(other.basic) {
			if(basic) {
				*basic += *(other.basic);
			} else {
				basic = new BasicGraphicalEffects(*(other.basic));
			}
		}

		mergeCustomParameters(other);

		return *this;
	}

	const GraphicalEffects& operator-=(const GraphicalEffects& other) {
		if(other.basic) {
			if(basic) {
				*basic -= *(other.basic);
			}
		}

		return *this;
	}

	void reset() {
		if(basic) {
			delete basic;
			basic = NULL;
		}

		custom.clear();
	}

	void resetCustomEffects() {
		custom.clear();
	}

	/* Getters */

	bool hasBasicEffects() {
		return basic?true:false;
	}

	bool hasCustomEffects() {
		return (bool)custom.size();
	}

	bool hasImageProcess() {
#ifdef GPU_IMAGEPROCESS
		if(!basic)
			return false;
		return *(basic->preProcessString) || *(basic->postProcessString);
#else
		return false;
#endif
	}

	bool hasSection() {
		assert(hasBasicEffects());
		return (bool)(basic->section.getW()) || (bool)(basic->section.getH());
	}

	/** Gets the raw value of an effect. */
	const CustomGraphicalEffectsValue& getValue(const char* effectId);

	/** Puts the transform result in result. */
	void transformCoordinates(Translatable* result, Translatable* position);

	/** Puts the transform result in result. */
	void inverseTransformCoordinates(Translatable* result, Translatable* position);

	Point getScale() {
		if(basic) {
			return Point(basic->hScale, basic->vScale);
		}
		return Point(100, 100);
	}

	int getHorizontalScale() {
		if(basic) {
			return basic->hScale;
		}
		return 100;
	}

	int getVerticalScale() {
		if(basic) {
			return basic->vScale;
		}
		return 100;
	}

	coord getRotation() {
		if(basic) {
			return basic->rotAngle;
		}
		return 0;
	}

	unsigned char getOpacity() {
		if(basic) {
			return basic->opacity;
		}
		return 255;
	}

	int getBlendMode() {
		if(basic) {
			return basic->blendMode;
		}

		return BLEND_INHERIT;
	}

	uint32_t getRenderCache() {
		if(basic) {
			return basic->renderCache;
		}
		return 0;
	}

	bool getRenderCache(int slot) {
		if(basic) {
			return basic->renderCache & (1<<slot);
		}
		return false;
	}

	const Rect* getSection() {
		static Rect defaultRect(0, 0, 0, 0);
		if(basic) {
			return &(basic->section);
		}
		return &defaultRect;
	}

	unsigned char getBlur() {
		// TODO
		return 0;
	}

	unsigned char getRadialBlur() {
		// TODO
		return 0;
	}

	/* Setters */
	void setScale(const Point& newScale) {
		makeBasicStructure();
		basic->hScale = newScale.getX();
		basic->vScale = newScale.getY();
	}

	void setScale(int h, int v) {
		makeBasicStructure();
		basic->hScale = h;
		basic->vScale = v;
	}

	void setHorizontalScale(int newHScale) {
		makeBasicStructure();
		basic->hScale = newHScale;
	}

	void setVerticalScale(int newVScale) {
		makeBasicStructure();
		basic->vScale = newVScale;
	}

	void setRotation(coord newRotAngle) {
		makeBasicStructure();
		basic->rotAngle = newRotAngle;
	}

	void setOpacity(unsigned char newOpacity) {
		makeBasicStructure();
		basic->opacity = newOpacity;
	}

	void setBlendMode(int newBlendMode) {
		makeBasicStructure();
		basic->blendMode = newBlendMode;
	}

	void setBlendMode(const char* s) {
		makeBasicStructure();
		basic->blendMode = stringToBlendMode(s);
	}

	void setRenderCache(uint32_t c) {
		makeBasicStructure();
		basic->renderCache = c;
	}

	void enableRenderingCache(int slot) {
		makeBasicStructure();
		basic->renderCache |= (1<<slot);
	}

	void disableRenderingCache(int slot) {
		makeBasicStructure();
		basic->renderCache &=~ (1<<slot);
	}

	void setSection(const Rect& newSection) {
		makeBasicStructure();
		basic->section = newSection;
	}

	void setSectionPointStart(const Point& p) {
		makeBasicStructure();
		basic->section.setPosition(p);
	}

	void setSectionWidthHeight(const Point& p) {
		makeBasicStructure();
		basic->section.setSize(p);
	}

	void setBlur(int blur) {
		CustomGraphicalEffectsValue v;
		v.integer = blur;
		custom[3] = v; // 3 == BLUR
	}

	void setRadialBlur(int blur) {
		CustomGraphicalEffectsValue v;
		v.integer = blur;
		custom[4] = v; // 4 == RBLR
	}

	/** Parse an effect string and put it into the effect.
	 * Effect string format :
	 * "NAME=value;NAME=value:value;NAME=value;..."
	 * NAME is the 4 character code of the effect, value is an integer or a char depending on the needed parameter type.
	 * There are 5 possibilities for values :
	 * int => integer
	 * int:int => point.x:point.y
	 * byte:byte:byte => color.r:color.g:color.b
	 * byte:byte:byte:byte => color.r:color.g:color.b:color.a
	 * int:byte:byte:byte:byte => color.amount:color.r:color.g:color.b:color.a
	 *
	 * Internal effects can also be set via this method. They are mapped to constant names.
	 */
	void setEffects(const char* effectString);

	/** Return the content of this effect as an effect string.
	 * The builtin effects are also output.
	 * WARNING: you must AOEFREE() the returned string yourself.
	 * @return the content of this effect as an effect string.
	 */
	char* getEffectString();

	/** Return the effect string for custom effects.
	 * The builtin effects are not output.
	 * WARNING: you must AOEFREE() the returned string yourself.
	 * @return the content of this effect as an effect string.
	 */
	char* getCustomEffectString();

	/** Sets a custom effect's value.
	 * @param effectId the ID of the effect to alter.
	 * @param effectValue the new value formatted like an effectString.
	 */
	void setEffect(const char* effectId, const char* effectValue);

	/** Sets a custom effect's value.
	 * This function only works for single-valued effects.
	 * @param effectId the ID of the effect to alter.
	 * @param effectValue the new value of the effect.
	 */
	void setIntegerEffect(const char* effectId, int effectValue);

	/** Reset a custom effect.
	 * @param effectId the ID of the effect to reset to its default value.
	 */
	void resetEffect(const char* effectId);

	/** Return the default value of an effect.
	 * @param effect the name of the effect.
	 * @return the default vaue for the requested effect.
	 */
	static const CustomGraphicalEffectsValue& getDefaultValue(const char* effect) { assert(strlen(effect) >= 4); return getDefaultValue(NAME_TO_ID(effect)); }

	/** Return the default value of an effect.
	 * @param effect the id of the effect.
	 * @return the default vaue for the requested effect.
	 */
	static const CustomGraphicalEffectsValue& getDefaultValue(uint32_t effect);

	const CustomGraphicalEffectsValue& getCustomEffectValue(const char* effect) { assert(strlen(effect) >= 4); return getCustomEffectValue(NAME_TO_ID(effect)); }
	const CustomGraphicalEffectsValue& getCustomEffectValue(uint32_t effect);

	/** Set the post processing pass.
	 * @param processString the string describing post process.
	 * @param pspecData the opaque pointer passed by the hardware engine.
	 */
	void setPostProcess(const char* processString, void* pspecData) {
#ifdef GPU_IMAGEPROCESS
		if(!processString || !*processString) {
			if(basic) {
				basic->postProcessString[0] = '\0';
				basic->postProcess = NULL;
			}
			return;
		}
		makeBasicStructure();
		if(processString != basic->postProcessString)
			strncpy(basic->postProcessString, processString, 64);
		basic->postProcess = pspecData;
#endif
	}

	/** Set the pre processing pass.
	 * @param processString the string describing pre process.
	 * @param pspecData the opaque pointer passed by the hardware engine.
	 */
	void setPreProcess(const char* processString, void* pspecData) {
#ifdef GPU_IMAGEPROCESS
		if(!processString || !*processString) {
			if(basic) {
				basic->preProcessString[0] = '\0';
				basic->preProcess = NULL;
			}
			return;
		}
		makeBasicStructure();
		if(processString != basic->preProcessString)
			strncpy(basic->preProcessString, processString, 64);
		basic->preProcess = pspecData;
#endif
	}

	/** Return the post process to apply to the image.
	 * @return the process to apply to the image while drawing.
	 */
	void* getPostProcess() const {
#ifdef GPU_IMAGEPROCESS
		if(basic)
			return basic->postProcess;
#else
		xassert(false, "Trying to get a post process but post process is not compiled in.");
#endif
		return NULL;
	}

	const char* getPostProcessString() {
#ifdef GPU_IMAGEPROCESS
		if(basic)
			return basic->postProcessString;
#else
		xassert(false, "Trying to get a post process but post process is not compiled in.");
#endif
		return NULL;
	}

	/** Return the pre process to apply to the image's background.
	 * @return the process to apply to the background of the image before drawing.
	 */
	void* getPreProcess() {
#ifdef GPU_IMAGEPROCESS
		if(basic)
			return basic->preProcess;
#else
		xassert(false, "Trying to get a pre process but post process is not compiled in.");
#endif
		return NULL;
	}

	const char* getPreProcessString() {
#ifdef GPU_IMAGEPROCESS
		if(basic)
			return basic->preProcessString;
#else
		xassert(false, "Trying to get a pre process but pre process is not compiled in.");
#endif
		return NULL;
	}

	/** Returns a blend mode given its name.
	 * @param s the name of the blend mode as a string.
	 * @return the identifier of the blend mode.
	 */
	static int stringToBlendMode(const char* s) {
		if(!s)
			return BLEND_INHERIT;

	#define BLENDMODE(name, value) if(strcmp(s, #name) == 0) return value;
		BLENDMODE(inherit, BLEND_INHERIT)
		BLENDMODE(normal, BLEND_NORMAL)
		BLENDMODE(add, BLEND_ADD)
		BLENDMODE(mul, BLEND_MULTIPLY)
		BLENDMODE(sub, BLEND_SUB)
		BLENDMODE(addalpha, BLEND_ADD_ALPHA)
		BLENDMODE(noalpha, BLEND_NOALPHA)
		BLENDMODE(addmul, BLEND_ADDMUL)
		BLENDMODE(inverse, BLEND_INVERSE)
		BLENDMODE(alphaMask, BLEND_ALPHAMASK)
		BLENDMODE(alphaLock, BLEND_ALPHALOCK)
		BLENDMODE(addAlphaLock, BLEND_ADD_ALPHALOCK)
	#undef BLENDMODE

		syslog("WARNING: %s is not a valid blend mode.", s);
		return BLEND_INHERIT;
	}

#define HEADER graphical_effects
#define HELPER default_effect_value_variable_h
#include "include_helper.h"

};


#endif
