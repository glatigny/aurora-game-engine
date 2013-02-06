#ifndef TEXT_LINE_H
#define TEXT_LINE_H

#include <list>

#define OVERFLOW_NONE 0 // Totally ignore maxWidth.
#define OVERFLOW_LET_LAST_WORD 1 // Let the last word overflow beyond maxWidth
#define OVERFLOW_LET_LAST_LETTER 2 // Let the last letter overflow beyond maxWidth
#define OVERFLOW_CROP 3 // Graphically crop the last character
#define OVERFLOW_REMOVE_LAST_LETTER 4 // Remove the last letter to fit in maxWidth
#define OVERFLOW_REMOVE_LAST_WORD 5 // Remove the last word to fit in maxWidth

#include "renderable.h"
#include "resizable.h"
#include "interpolation.h"
#include "rect.h"

class Glyph;
class Font;

/** This class displays a line of latin characters.
 * It is designed to display text in lines from left to right, with one font.
 * It can use any font, and renders text using the result of getGlyph and aligning glyphs using their metrics.
 */
class TextLine : public Renderable, public Resizable {
friend class TextBox;
private:
	Font* font;
	coord maxWidth;
	dur delay;
	int overflowType;
	Interpolation* transitionInterpolation; // Not fully valid until textFormatted == true
	dur maxTime; // Maximum time for rewind.


	/** Aborts metrics computing by setting characterCount and overflow.
	 * width, height, characterCount and displayedCharactercount must be already set
	 * with values corresponding to the last displayed character.
	 * @param t the pointer at which the overflown text starts.
	 */
	void computeOverflowMetrics(char* t);

	void computeMetrics();
	void formatText();
	void resetMetrics();
	void resetTextFormat();

	bool isWordChar(int c);
	int rewindUTF8Char(char* &t);
	int getNextUTF8Char(char* &t);
	int getUTF8Char(char* t) {return getNextUTF8Char(t);}

protected:
	char* text; // UTF-8
	bool rewind;
	bool textFormatted;
	// Cached text format. The cache is valid only when textFormatted == true.
	std::list<Glyph*> formattedText;

	// Cached metrics
	bool metricsComputed;
	int overflow; // Number of characters (not bytes) that overflow. In crop mode, the cropped character counts as displayed.
	dur transitionDuration; // Transition duration.
	char* overflowText;
	int characterCount;
	int displayedCharacterCount;
	coord firstCharX;
	coord baseLineY;
	dur currentTime; 

public:
#define HEADER text_line
#define HELPER scriptable_object_h
#include "include_helper.h"
	TextLine() : transitionInterpolation(NULL), text(NULL), textFormatted(false), metricsComputed(false) {}
	TextLine(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, const char* text, Font* font, coord maxWidth, int overflowType, dur delay, dur maxTime, const char* transitionInterpolation);

	virtual ~TextLine() {
		if(text) {
			AOEFREE(text);
		}

		if(transitionInterpolation) {
			delete transitionInterpolation;
		}
	}

	/** Sets total time of the transition.
	 * This function sets total time of transition. THIS IS NOT the duration of text apparition.
	 * Rather, it helps to use rewind : set maxTime to the appear time of the whole text box :
	 * interpolators are clipped, so the text will remain still once displayed.
	 * @param newMaxTime maximum time of the transition.
	 */
	void setMaxTime(dur newMaxTime) {
		assert(maxTime >= delay);
		if(textFormatted) {
			resetTextFormat();
		}
		maxTime = newMaxTime;
	}

	/** Returns the real width of this line.
	 * @return the real, formatted width of this line, in pixels.
	 **/
	coord getRealW();

	/** Return the code point of this glyph.
	 * @return the code point of the glyph;
	 */
	virtual coord getMaxWidth() { return maxWidth; }

	/** Gets the number of characters that could not be displayed because of overflow.
	 * @return the number of characters not displayed.
	 */
	int getOverflowLength() { if(!metricsComputed) { computeMetrics(); } return overflow; }

	/** Gets the text that could not be dsplayed.
	 * @return a pointer inside the internal text buffer pointing to the first character that could not be displayed because of overflow.
	 */
	char* getOverflowText() { if(!metricsComputed) { computeMetrics(); } return overflowText; }

	/** Sets the maximum width of the line.
	 * Exceeding text will behave as specified by the over flow rule.
	 * This function resets metrics cache.
	 * @param newMaxWidth the maximum width in pixels.
	 */
	virtual void setMaxWidth(coord newMaxWidth) { maxWidth = newMaxWidth; metricsComputed = false; textFormatted = false; }

	/** Returns the metrics of an individual glyph.
	 * @param glyphIndex position of the glyph in the string.
	 * @return glyph metrics, in the standard metrics representation.
	 */
	Rect getGlyphMetrics(int glyphIndex);

	/** Changes the displayed text.
	 * Changing the text obviously resets metrics cache and rendering cache,
	 * so please use this function carefully.
	 * @param text the new text to be displayed. Old text is discarded.
	 */
	void setText(const char* text);

	/** Returns the total time needed to do the transition.
	 * @return time needed to display the text.
	 */
	dur getTransitionTime() {
		return maxTime - delay;
	}

	/** Hides or shows text by rewinding playback direction.
	 * @param hidingText true to rewind the animation and hide the text, false to play it the normal way.
	 */
	void hideText(bool hidingText) {
		rewind = hidingText;
	}

	/** Tells whether text transition has finished.
	 * @return true if the transition is finished, false otherwise.
	 */
	bool finished() { return rewind?currentTime==0:currentTime==maxTime; }

	/* Renderable */

	virtual void renderGraphics();
	virtual bool inside(Translatable& firstCorner, Translatable& secondCorner) { return false; }
	virtual void loadGraphics();
	virtual void unloadGraphics();

	/* Realtime */

	virtual void updateObject(dur elapsedTime);

	/* Resizable */

	virtual coord getW() {
		if(!metricsComputed)
			computeMetrics();
		return Resizable::getW();
	}

	virtual coord getH() {
		if(!metricsComputed)
			computeMetrics();
		return Resizable::getH();
	}

	/* Restartable */

	virtual void restart();
};

#endif /* TEXT_LINE */

