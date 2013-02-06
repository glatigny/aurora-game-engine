#include "graphical_effects.h"

#define HEADER graphical_effects
#define HELPER default_effect_value_variable_cpp
#include "include_helper.h"

#include <math.h>

const CustomGraphicalEffectsValue GraphicalEffects::defaultCustomGraphicalEffectsValue = { 0 };

// QC:?
void GraphicalEffects::mergeCustomParameters(const GraphicalEffects& other) {
	if(!other.custom.empty()) {
		for(CustomEffectMap::const_iterator oi = other.custom.begin(); oi != other.custom.end(); oi++) {
			CustomEffectMap::iterator i = custom.find(oi->first);
			if(i == custom.end()) {
				custom[oi->first] = oi->second;
				continue;
			}

			assert(i->first == oi->first);
			const CustomGraphicalEffectsValue& s = oi->second;
			CustomGraphicalEffectsValue& d = i->second;
			switch(i->first){

#define DO_NOTHING ;

#define ADD_INTEGER d.integer += s.integer;

#define ADD_AMOUNT d.color.amount += s.color.amount;

#define MODULATE_RGB \
			d.color.r = (int)d.color.r * s.color.r / 255; \
			d.color.g = (int)d.color.g * s.color.g / 255; \
			d.color.b = (int)d.color.b * s.color.b / 255;

#define MODULATE_ALPHA \
			d.color.a = (int)d.color.a * s.color.a / 255;

#define AVERAGE_RGB \
			d.color.r = (int)d.color.r + s.color.r / 2; \
			d.color.g = (int)d.color.g + s.color.g / 2; \
			d.color.b = (int)d.color.b + s.color.b / 2;

#define AVERAGE_ALPHA \
			d.color.a = (int)d.color.a + s.color.a / 2;

#define ADD_RGB \
			d.color.r = MIN((int)d.color.r + s.color.r, 255); \
			d.color.g = MIN((int)d.color.g + s.color.g, 255); \
			d.color.b = MIN((int)d.color.b + s.color.b, 255);

#define ADD_ALPHA \
			d.color.a = MIN((int)d.color.a + s.color.a, 255);

#define ADD_XY \
			d.point.x += s.point.x; \
			d.point.y += s.point.y;

#define AVERAGE_XY \
			d.point.x = d.point.x + s.point.x / 2; \
			d.point.y = d.point.y + s.point.y / 2;

#define AND_FLAGS \
			d.flags = d.flags & s.flags;

#define OR_FLAGS \
			d.flags = d.flags | s.flags;

#define XOR_FLAGS \
			d.flags = d.flags ^ s.flags;

#define HEADER graphical_effects
#define HELPER graphical_effects_merge
#include "include_helper.h"

#undef DO_NOTHING
#undef ADD_INTEGER
#undef ADD_AMOUNT
#undef MODULATE_RGBA
#undef AVERAGE_RGBA
#undef ADD_RGBA
#undef ADD_XY
#undef AVERAGE_XY
#undef AND_FLAGS
#undef OR_FLAGS
#undef XOR_FLAGS
			}
		}
	}
}

// QC:?
void GraphicalEffects::setEffects(const char* effectString) {
	while(effectString && *effectString) {
		assert(strlen(effectString) >= 5);
		assert(effectString[4] == '=');
		switch(NAME_TO_ID(effectString)){
#define BUILTIN_EFFECT_I(n, function) case n: { function(atoi(effectString + 5)); } break;
#define BUILTIN_EFFECT_C(n, function) case n: { function(atoc(effectString + 5)); } break;
			BUILTIN_EFFECT_C('HSCL', setHorizontalScale)
			BUILTIN_EFFECT_C('VSCL', setVerticalScale)
			BUILTIN_EFFECT_C('ROTA', setRotation)
			BUILTIN_EFFECT_I('OPAC', setOpacity)
			BUILTIN_EFFECT_I('BLND', setBlendMode)
#undef BUILTIN_EFFECT_I
#undef BUILTIN_EFFECT_C
			case 'SEPS': {
				Point p;
				stringToPoint(effectString, p);
				setSectionPointStart(p);
			}break;
			case 'SEWH': {
				Point p;
				stringToPoint(effectString, p);
				setSectionWidthHeight(p);
			}break;
			case 'SCAL': {
				Point p;
				stringToPoint(effectString, p);
				setScale(p);
			}break;
			case 'CROP': {
				Rect r;
				stringToRect(effectString, r);
				setSection(r);
			}break;
			case 'CACH': {
				uint32_t tSlots;
				stringToFlags(effectString, &tSlots);
				setRenderCache(tSlots);
			}break;
			default: {
				custom[NAME_TO_ID(effectString)] = decodeValueFromEffectString(effectString + 5);
			}break;
		}
		effectString = strchr(effectString, ';');

		if(effectString) {
			effectString++;
		}
	}
}

// QC:?
void GraphicalEffects::setEffect(const char* effectId, const char* effectValue) {
	assert(strlen(effectId) == 4);
	assert(!strchr(effectId, ';'));
	assert(!strchr(effectValue, ';'));
	switch(NAME_TO_ID(effectId)){
#define BUILTIN_EFFECT_I(n, function) case n: { function(atoi(effectValue)); } break;
#define BUILTIN_EFFECT_C(n, function) case n: { function(atoc(effectValue)); } break;
	BUILTIN_EFFECT_C('HSCL', setHorizontalScale)
	BUILTIN_EFFECT_C('VSCL', setVerticalScale)
	BUILTIN_EFFECT_C('ROTA', setRotation)
	BUILTIN_EFFECT_I('OPAC', setOpacity)
	BUILTIN_EFFECT_I('BLND', setBlendMode)
#undef BUILTIN_EFFECT_I
#undef BUILTIN_EFFECT_C
	case 'SEPS': {
		Point p;
		stringToPoint(effectValue, p);
		setSectionPointStart(p);
	}break;
	case 'SEWH': {
		Point p;
		stringToPoint(effectValue, p);
		setSectionWidthHeight(p);
	}break;
	case 'SCAL': {
		Point p;
		stringToPoint(effectValue, p);
		setScale(p);
	}break;
	case 'CROP': {
		Rect r;
		stringToRect(effectValue, r);
		setSection(r);
	}break;
	case 'CACH': {
		uint32_t tSlots;
		stringToFlags(effectValue, &tSlots);
		setRenderCache(tSlots);
	}break;
	default: {
		custom[NAME_TO_ID(effectId)] = decodeValueFromEffectString(effectValue);
	}break;
}
}

// QC:?
void GraphicalEffects::setIntegerEffect(const char* effectId, int effectValue) {
	assert(strlen(effectId) == 4);
	assert(!strchr(effectId, ';'));
	switch(NAME_TO_ID(effectId)){
#define BUILTIN_EFFECT(n, function) case n: { function(effectValue); } break;
	BUILTIN_EFFECT('HSCL', setHorizontalScale)
	BUILTIN_EFFECT('VSCL', setVerticalScale)
	BUILTIN_EFFECT('ROTA', setRotation)
	BUILTIN_EFFECT('OPAC', setOpacity)
	BUILTIN_EFFECT('BLND', setBlendMode)
	BUILTIN_EFFECT('CACH', setRenderCache)
#undef BUILTIN_EFFECT
	case 'SEPS':
	case 'SEWH':
	case 'SCAL':
	case 'CROP': {
		assert(false); // These effects are not integers.
	}break;
	default: {
		// TODO: custom[NAME_TO_ID(effectId)] = ??? ;
	}break;
}
}

// QC:?
const CustomGraphicalEffectsValue& GraphicalEffects::getDefaultValue(uint32_t effect) {
	switch(effect) {
#define HEADER graphical_effects
#define HELPER default_effect_value
#include "include_helper.h"
	}

	syslog("GraphicalEffects::getDefaultValue: effect %s is unknown.", effect);
	assert(false);
	return defaultCustomGraphicalEffectsValue;
}

const CustomGraphicalEffectsValue& GraphicalEffects::getCustomEffectValue(uint32_t effect) {
	if(custom.empty()) {
		return getDefaultValue(effect);
	}

	CustomEffectMap::iterator it = custom.find(effect);
	if(it == custom.end()) {
		return getDefaultValue(effect);
	}

	return it->second;
}

// QC:?
void GraphicalEffects::transformCoordinates(Translatable* result, Translatable* position) {
	if(basic) {
		// Translate the object.
		floatcoord finalX = position->getX();
		floatcoord finalY = position->getY();

		// Rotate the base point.
		floatcoord r;
		if((r = basic->rotAngle)) {
#ifdef DOUBLE_COORD
			// Use double for internal coordinates
			floatcoord cosangle = cos(-r*((floatcoord)M_PI)/180.0);
			floatcoord sinangle = sin(-r*((floatcoord)M_PI)/180.0);
#else
			// Use float for internal coordinates
			floatcoord cosangle = cosf(-r*((floatcoord)M_PI)/180.0);
			floatcoord sinangle = sinf(-r*((floatcoord)M_PI)/180.0);
#endif

			floatcoord dX = finalX;
			floatcoord dY = finalY;

			finalX = dX*cosangle - dY*sinangle;
			finalY = dY*cosangle + dX*sinangle;
		}
		
		// Scale the base point.
		finalX *= basic->hScale / (floatcoord)100;
		finalY *= basic->vScale / (floatcoord)100;

		result->setPosition((coord)finalX, (coord)finalY);
	} else {
		// Simple effect propagation.
		result->setPosition(position);
	}
}

// QC:?
void GraphicalEffects::inverseTransformCoordinates(Translatable* result, Translatable* position) {
	if(basic) {
		// Translate the object.
		floatcoord finalX = position->getX();
		floatcoord finalY = position->getY();

		// Scale the base point.
		finalX /= basic->hScale / (floatcoord)100;
		finalY /= basic->vScale / (floatcoord)100;
		
		// Rotate the base point.
		floatcoord r;
		if((r = -basic->rotAngle)) {
#ifdef DOUBLE_COORD
			// Use double for internal coordinates
			floatcoord cosangle = cos(-r*((floatcoord)M_PI)/180.0);
			floatcoord sinangle = sin(-r*((floatcoord)M_PI)/180.0);
#else
			// Use float for internal coordinates
			floatcoord cosangle = cosf(-r*((floatcoord)M_PI)/180.0);
			floatcoord sinangle = sinf(-r*((floatcoord)M_PI)/180.0);
#endif

			floatcoord dX = finalX;
			floatcoord dY = finalY;

			finalX = dX*cosangle - dY*sinangle;
			finalY = dY*cosangle + dX*sinangle;
		}

		result->setPosition((coord)finalX, (coord)finalY);
	} else {
		// Simple effect propagation.
		result->setPosition(position);
	}
}

// QC:?
const CustomGraphicalEffectsValue& GraphicalEffects::getValue(const char* effectId) {
#ifndef NDEBUG
	switch(NAME_TO_ID(effectId)) {
		case 'HSCL':
		case 'VSCL':
		case 'ROTA':
		case 'OPAC':
		case 'BLND':
		case 'CACH':
		case 'SEPS':
		case 'SEWH':
		case 'SCAL':
		case 'CROP':
			xerror(false, "Effect name reserved for basic effect.");
			break;

		default:
#endif
			CustomEffectMap::iterator i;
			if((i = custom.find(NAME_TO_ID(effectId))) != custom.end())
				return i->second;
#ifndef NDEBUG
	}
#endif
	return getDefaultValue(effectId);
}

// QC:?
CustomGraphicalEffectsValue GraphicalEffects::decodeValueFromEffectString(const char* string) {
	int vCount;
	int v[5];

	for(vCount = 0; string && vCount < 5; vCount++) {
		v[vCount] = atoi(string);
		string = strchr(string, ':');
		if(string) {
			string++;
		}
	}

	CustomGraphicalEffectsValue ev;

	switch(vCount){
	case 1:
		ev.integer = v[0];
		break;

	case 2:
		ev.point.x = v[0];
		ev.point.y = v[1];
		break;

	case 5:
		ev.color.amount = v[4];
	case 4:
		ev.color.a = (unsigned char) v[3];
	case 3:
		ev.color.b = (unsigned char) v[2];
		ev.color.g = (unsigned char) v[1];
		ev.color.r = (unsigned char) v[0];
		break;

	default:
		break;
	}

	return ev;
}
