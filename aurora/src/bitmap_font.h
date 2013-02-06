#ifndef BITMAP_FONT_H
#define BITMAP_FONT_H

#include "font.h"
#include <vector>
#include <map>
#include "rect.h"

class Renderable;

/** This class implements a simple bitmap font.
 * The font crops images to extract bitmap glyphs. It also provides a default image to
 * avoid repeating <image> tags.
 */
class BitmapFont : public Font {
protected:
	ObjectMold* defaultImage; // FIXME : Should be a mold
	std::vector<ObjectMold*> glyphImages;
	std::vector<Rect> glyphOffsets;
	std::vector<Rect> glyphMetrics;
	std::map<int, int> codePoints;
	int fontHeight;
	coord baseY;
	Rect baseOffset;

	Glyph* getGlyphById(int glyphId, State* attachedState, int delay);

public:
#define HEADER bitmap_font
#define HELPER scriptable_object_h
#include "include_helper.h"
	BitmapFont() : defaultImage(NULL), fontHeight(0) {}
	BitmapFont(AuroraEngine* parentEngine, ObjectMold* defaultImage, int fontHeight, const Rect& baseOffset);
	virtual ~BitmapFont() {}

	/** Add an image providing glyphs.
	 * Please do not use this fonction until you need to have textual data.
	 * The image may be added more than once via the ID system.
	 * Glyphs will use parts of this image to display the font.
	 * @param glyphId the ID of the glyph to add. It must be a number.
	 * @param image the image to use for the given glyph.
	 */
	void addGlyphImage(const char* glyphId, ObjectMold* image);

	/** Add an image providing glyphs.
	 * The image may be added more than once via the ID system.
	 * Glyphs will use parts of this image to display the font.
	 * @param glyphId the ID of the glyph to add. It must be a number.
	 * @param image the image to use for the given glyph.
	 */
	void addGlyphImage(int glyphId, ObjectMold* image);

	/** Add an offset to the font.
	 * Please do not use this fonction until you need to have textual data.
	 * An offset defines the zone to crop in the image to extract the glyph.
	 * @param glyphId the ID of the glyph to add. It must be a number.
	 * @param glyphOffsetValues the zone to crop to extract the glyph.
	 */
	void addGlyphOffset(const char* glyphId, const char* glyphOffsetValues);

	/** Add an offset to the font.
	 * An offset defines the zone to crop in the image to extract the glyph.
	 * @param glyphId the ID of the glyph to add.
	 * @param glyphOffsetValues the zone to crop to extract the glyph.
	 */
	void addGlyphOffset(int glyphId, Rect glyphOffsetValues);

	/** Add a glyph metric to this font.
	 * Please do not use this fonction until you need to have text IDs. In all cases, the ID must be a number.
	 * @param glyphId the ID of the glyph to add.
	 * @param glyphMetricValue The metric of the font. First point is the anchor of the glyph, second is its typographical size.
	 */
	void addGlyphMetric(const char* glyphId, const char* glyphMetricValues);

	/** Add a glyph metric to this font.
	 * @param glyphId the ID of the glyph to add.
	 * @param glyphMetricValue The metric of the font. First point is the anchor of the glyph, second is its typographical size.
	 */
	void addGlyphMetric(int glyphId, Rect glyphMetricValues);

	/** Define a glyph for a given code point.
	 * Please do not use this fonction until you need to have text IDs. In all cases, the ID must be a number.
	 * @param codePoint the code point to associate.
	 * @param the glyph ID to associate to the code point.
	 */
	void addCodePoint(const char* codePoint, int glyphId);

	/** Define a glyph for a given code point.
	 * @param codePoint the code point to associate.
	 * @param the glyph ID to associate to the code point.
	 */
	void addCodePoint(int codePoint, int glyphId);

	/* Font */

	virtual Glyph* getGlyph(int codePoint, State* attachedState, int delay);
	virtual const Rect& getGlyphMetrics(int codePoint);
	virtual const Rect& getBaseOffset() { return baseOffset; }
	virtual void preload();

};

#endif /* BITMAP_FONT_H */

