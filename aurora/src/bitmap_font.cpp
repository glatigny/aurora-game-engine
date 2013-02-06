#include "bitmap_font.h"

#include "decoration.h"
#include "bitmap_glyph.h"

#define HEADER bitmap_font
#define HELPER scriptable_object_cpp
#include "include_helper.h"

BitmapFont::BitmapFont(AuroraEngine* parentEngine, ObjectMold* newDefaultImage, int newFontHeight, const Rect& newBaseOffset) : Font(parentEngine), defaultImage(newDefaultImage), fontHeight(newFontHeight), baseOffset(newBaseOffset) {}

// QC:?
Glyph* BitmapFont::getGlyph(int codePoint, State* attachedState, int delay)
{
	std::map<int, int>::iterator cpi = codePoints.find(codePoint);
	if(cpi != codePoints.end()) {
		return getGlyphById(cpi->second, attachedState, delay);
	}

	xerror(false, "BitmapFont::getGlyph : glyph with codepoint %d not found.", codePoint);
	return NULL;
}

// QC:?
Glyph* BitmapFont::getGlyphById(int glyphId, State* attachedState, int delay)
{
	xassert(glyphId < (int)glyphOffsets.size(), "Glyph offset : Glyph number %d is above font size (%d)", glyphId, (int)glyphOffsets.size());
	xassert(glyphId < (int)glyphMetrics.size(), "Glyph metric : Glyph number %d is above font size (%d)", glyphId, (int)glyphOffsets.size());

	GraphicalEffects* effects = new GraphicalEffects;
	effects->setSection(glyphOffsets[glyphId]);
	Point offset(*(glyphMetrics[glyphId].getPosition()));

	ObjectMold* glyphMold = defaultImage;
	if(glyphId < (int)glyphImages.size()) {
		glyphMold = glyphImages[glyphId];
		if(!glyphMold) {
			glyphMold = defaultImage;
		}
	}

	BitmapGlyph* glyph = new BitmapGlyph(attachedState, &offset, effects, 0, glyphMetrics[glyphId].getW(), glyphMetrics[glyphId].getH(), delay, dynamic_cast<Renderable*>(glyphMold->create(attachedState)));

	return glyph;
}

// QC:A
const Rect& BitmapFont::getGlyphMetrics(int codePoint)
{
	std::map<int, int>::iterator cpi = codePoints.find(codePoint);
	if(cpi != codePoints.end()) {
		return glyphMetrics[cpi->second];
	}

	xerror(false, "BitmapFont::getGlyphMetrics : glyph with codepoint %d not found.", codePoint);
	return glyphMetrics[0];
}

// QC:A
void BitmapFont::addGlyphImage(const char* glyphId, ObjectMold* image)
{
	assert(glyphId);
	assert(*glyphId >= '0' && *glyphId <= '9');
	assert(image);

	addGlyphImage(atoi(glyphId), image);
}

// QC:A
void BitmapFont::addGlyphOffset(const char* glyphId, const char* glyphOffsetValues)
{
	assert(glyphId);
	assert(*glyphId >= '0' && *glyphId <= '9');
	assert(glyphOffsetValues);

	int offsetX, offsetY, offsetW, offsetH;
#ifndef NDEBUG
	int valueCount = 
#endif
	stringToIntList(glyphOffsetValues, &offsetX, &offsetY, &offsetW, &offsetH);
	assert(valueCount == 4);

	addGlyphOffset(atoi(glyphId), Rect(offsetX, offsetY, offsetW, offsetH));
}

// QC:A
void BitmapFont::addGlyphMetric(const char* glyphId, const char* glyphMetricValues)
{
	assert(glyphId);
	assert(*glyphId >= '0' && *glyphId <= '9');
	assert(glyphMetricValues);

	int metricX, metricY, metricW, metricH;
#ifndef NDEBUG
	int valueCount = 
#endif
	stringToIntList(glyphMetricValues, &metricX, &metricY, &metricW, &metricH);
	assert(valueCount == 4);

	addGlyphMetric(atoi(glyphId), Rect(metricX, metricY, metricW, metricH));
}

// QC:A
void BitmapFont::addCodePoint(const char* codePoint, int glyphId)
{
	assert(codePoint);
	assert(*codePoint >= '0' && *codePoint <= '9');

	addCodePoint(atoi(codePoint), glyphId);
}

// QC:?
void BitmapFont::addGlyphImage(int glyphId, ObjectMold* image)
{
	if(glyphId >= (int)glyphImages.size())
		glyphImages.resize(glyphId+1, NULL);

	glyphImages[glyphId] = image;
}

// QC:?
void BitmapFont::addGlyphOffset(int glyphId, Rect glyphOffsetValues)
{
	if(glyphId >= (int)glyphOffsets.size()) {
		glyphOffsets.resize(glyphId+1);
	}

	glyphOffsets[glyphId] = glyphOffsetValues;
}

// QC:?
void BitmapFont::addGlyphMetric(int glyphId, Rect glyphMetricValues)
{
	if(glyphId >= (int)glyphMetrics.size()) {
		glyphMetrics.resize(glyphId+1);
	}

	glyphMetrics[glyphId] = glyphMetricValues;
}

// QC:?
void BitmapFont::addCodePoint(int codePoint, int glyphId)
{
	codePoints[codePoint] = glyphId;
}


// QC:?

void BitmapFont::preload()
{
}

