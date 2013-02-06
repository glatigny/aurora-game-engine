#ifndef FONT_H
#define FONT_H

#include "abstract.h"
#include "rect.h"
#include "state.h"

class Glyph;
class State;

/** This is an interface to query glyphs from code points.
 * It is suitable for languages without ligatures or complex character
 * combinations.
 *
 * For codepoint values, please have a look at the official unicode table :
 *  http://www.unicode.org/charts/
 */
class Font : public Abstract, public ScriptableObject { // FIXME : could work with simpler GC model.
public:
#define HEADER font
#define HELPER scriptable_object_h
#include "include_helper.h"
	Font() {}
	Font(AuroraEngine* parentEngine) : Abstract(parentEngine) {}
	virtual ~Font() {}

	/** Generates a glyph for a specific codepoint.
	 * @param codePoint the unicode codepoint to generate a character for.
	 * @param attachedState the state which will contain this new glyph.
	 * @param delay the delay to set in the glyph.
	 * @return the glyph corresponding to the code point.
	 */
	virtual Glyph* getGlyph(int codePoint, State* attachedState, int delay) = 0;

	/** Returns the metrics of a glyph.
	 * Metrics are returned as a Rect, where offset is the base point of the glyph and size is the typographical size.
	 * @return the metrics of a glyph.
	 */
	virtual const Rect& getGlyphMetrics(int codePoint) = 0;

	/** Returns the base offset of the font.
	 * The base offset is the offset to apply to glyphs so they will fit in a top-left based box.
	 */
	virtual const Rect& getBaseOffset() = 0;

	/** Preloads the font in memory.
	 * Only the most important graphics are loaded or generated.
	 */
	virtual void preload() = 0;
};

#endif /* FONT_H */

