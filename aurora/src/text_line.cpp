#include "text_line.h"

#include "font.h"
#include "glyph.h"
#include "state.h"
#include "bitmap_font.h"

#define HEADER text_line
#define HELPER scriptable_object_cpp
#include "include_helper.h"

// QC:?
TextLine::TextLine(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, const char* newText, Font* newFont, coord newMaxWidth, int newOverflowType, dur newDelay, dur newMaxTime, const char* newTransitionInterpolation) : Translatable(offset), Entity(attachedState), Renderable(attachedState, offset, effects, layer) {
	if(newText) {
		text = AOESTRDUP(newText);
	} else {
		text = NULL;
	}

	assert(newFont);
	font = newFont;

	metricsComputed = false;
	textFormatted = false;
	rewind = false;
	currentTime = 0;

	maxWidth = newMaxWidth;
	overflowType = newOverflowType;
	delay = newDelay;
	maxTime = newMaxTime;
	
	if(newTransitionInterpolation) {
		transitionInterpolation = newInterpolation(newTransitionInterpolation);
	} else {
		transitionInterpolation = NULL;
	}
}

// QC:P
void TextLine::resetTextFormat() {
	assert(textFormatted);

	// Unref all glyphs
	for(std::list<Glyph*>::iterator i = formattedText.begin(); i != formattedText.end(); i++) {
		if(*i) {
			mayDelete(*i);
		}
	}
	formattedText.clear();

	textFormatted = false;
}

// QC:P
void TextLine::resetMetrics() {
	assert(metricsComputed);
	metricsComputed = false;
	if(textFormatted) {
		resetTextFormat();
	}
}

// QC:A (FIXME: REMOVE_LAST_WORD algorithm is not correct. Implement other modes.)
void TextLine::computeMetrics() {
	if(!metricsComputed) {
		if(!text || !*text) {
			setW(0);
			setH(0);
			metricsComputed = true;
			return;
		}

		assert(font);
		assert(!textFormatted);

		overflowText = NULL;
		char* t = text;

		coord w = 0;
		coord h = 0;
		firstCharX = font->getGlyphMetrics(getUTF8Char(t)).getX();
		baseLineY = 0;

		characterCount = 0;
		displayedCharacterCount = 0;

		while(*t) {
			int c = getNextUTF8Char(t);

			// TODO : mark last word position
			// TODO : implement all overflow modes
			Rect m = font->getGlyphMetrics(c);
			w += m.getW();
			if(h < m.getH()) {
				h = m.getH();
			}
			baseLineY = MAX(baseLineY, m.getY());
			characterCount++;
			displayedCharacterCount++;

			if(maxWidth >= 0 && w > maxWidth) {
				switch(overflowType) {
				case OVERFLOW_LET_LAST_LETTER:
				case OVERFLOW_CROP:
					computeOverflowMetrics(t);
					return;
				break;

				case OVERFLOW_REMOVE_LAST_LETTER:
					rewindUTF8Char(t);
					characterCount--;
					displayedCharacterCount--;
					computeOverflowMetrics(t);
					return;
				break;

				case OVERFLOW_REMOVE_LAST_WORD:
					while(isWordChar(getUTF8Char(t))) {
						rewindUTF8Char(t);
						characterCount--;
						displayedCharacterCount--;
					}
					computeOverflowMetrics(t);
					return;
				}
			}
		}

		setW(w);
		setH(h);
		metricsComputed = true;
	}
}

// QC:G
void TextLine::computeOverflowMetrics(char* t) {
	overflowText = t;
	overflow = 0;
	while(*t) {
		getNextUTF8Char(t);
		characterCount++;
		overflow++;
	}
	metricsComputed = true;
}

bool TextLine::isWordChar(int c)
{
	return !(
	(c >= '0' && c <= '9')
	|| (c >= 'A' && c <= 'Z')
	|| (c >= 'a' && c <= 'z')
	);
}

// QC:?
int TextLine::rewindUTF8Char(char* &t) {
	assert(t);
	assert(t > text);
	assert(*(t-1));

	xassert(!((*t) & (1<<7)) || (*t) & (1<<6), "Invalid UTF-8 stream."); // 10xxxxxx is invalid : loss of synchronization

	do {
		t--; // Rewind one character.
	} while(*t & (1<<7)); // Loop until a starting byte has been found.

	xassert(!((*t) & (1<<7)) || (*t) & (1<<6), "Invalid UTF-8 stream."); // 10xxxxxx is invalid : loss of synchronization

	return 0;
}

// QC:G
int TextLine::getNextUTF8Char(char* &t) {
	assert(t);
	assert(*t);

	xerror(!((*t) & (1<<7)) || (*t) & (1<<6), "Invalid UTF-8 stream."); // 10xxxxxx is invalid : loss of synchronization

	// Quickly return one byte characters
	if(!(*t & (1<<7))) {
		return *(t++);
	}

	unsigned long c;
	t += UTF8ToCodepoint(t, c);

	return (int)c;
}

// QC:A (interpolated mode untested)
void TextLine::formatText() {
	xerror(font, "No font to draw text.");
	if(!metricsComputed) {
		xassert(!textFormatted, "Text formatted without metrics.");
		computeMetrics();
	} else if(textFormatted) {
		resetTextFormat();
	}

	if(!text || !*text) {
		textFormatted = true;
		return;
	}

	if(transitionInterpolation) {
		transitionInterpolation->set(0, maxWidth, delay, maxTime, true);
	}

	char* t = text;
	coord x = firstCharX;
	coord baseLineY = font->getBaseOffset().getY();

	if(transitionInterpolation) {
		for(int i=0; i<displayedCharacterCount; i++) {
			int c = getNextUTF8Char(t);
			Glyph* glyph = font->getGlyph(c, state(), transitionInterpolation->getInterpolatedValue(x));
			own(glyph);
			glyph->setPosition(x, baseLineY);
			x += glyph->getW();
			formattedText.push_back(glyph);
			glyph->setParent(this);
		}
	} else {
		for(int i=0; i<displayedCharacterCount; i++) {
			int c = getNextUTF8Char(t);
			Glyph* glyph = font->getGlyph(c, state(), 0);
			own(glyph);
			glyph->setPosition(x, baseLineY);
			x += glyph->getW();
			formattedText.push_back(glyph);
			glyph->setParent(this);
		}
	}

	textFormatted = true;
}

// QC:A (XXX:Only work for left to right languages).
coord TextLine::getRealW()
{
	if(!text || !*text) {
		return 0;
	}
	if(!textFormatted) {
		formatText();
	}

	return (formattedText.back())->getX() + (formattedText.back())->getW() - (formattedText.front())->getX();
}

// QC:G
void TextLine::setText(const char* newText) {
	if(metricsComputed) {
		resetMetrics();
	}

	if(text) {
		AOEFREE(text);
	}

	if(newText)
		text = AOESTRDUP(newText);
	else
		text = NULL;
}

// QC:?
void TextLine::loadGraphics() {
	xerror(font, "No font to load.");
	if(!textFormatted) {
		formatText();
		font->preload();
	}
}

// QC:?
void TextLine::unloadGraphics() {
	resetTextFormat();
}

// QC:?
void TextLine::updateObject(dur elapsedTime) {
	if(!text || !*text) {
		return;
	}

	if(rewind)
		elapsedTime = -elapsedTime;

	currentTime += elapsedTime;

	if(transitionInterpolation) {
		if(rewind) {
			if(currentTime < 0) {
				elapsedTime -= currentTime;
				currentTime = 0;
			}
		} else {
			if(currentTime > maxTime) {
				elapsedTime -= currentTime - maxTime;
				currentTime = maxTime;
			}
		}
		assert(currentTime <= maxTime && currentTime >= 0);
	}

	for(std::list<Glyph*>::iterator i = formattedText.begin(); i != formattedText.end(); i++) {
		if(*i) {
			(*i)->update(elapsedTime);
		}
	}
}

// QC:?
void TextLine::restart() {
	rewind = false;
	currentTime = 0;
	if(textFormatted) {
		// Restart all glyphs
		for(std::list<Glyph*>::iterator i = formattedText.begin(); i != formattedText.end(); i++) {
			if(*i) {
				(*i)->restart();
			}
		}
	}
}

// QC:G
void TextLine::renderGraphics() {
	// Format the text
	if(!textFormatted) {
		formatText();
	}

	// Display all formatted glyphs
	for(std::list<Glyph*>::iterator i = formattedText.begin(); i != formattedText.end(); i++) {
		if(*i) {
			(*i)->render();
		}
	}
}

// QC:A (This function is not optimized at all)
Rect TextLine::getGlyphMetrics(int glyphIndex) {
	if(!textFormatted) {
		formatText();
	}

	std::list<Glyph*>::iterator g = formattedText.begin();
	for(int c = 0; c<glyphIndex; c++) {
		if(g++ == formattedText.end()) {
			return Rect(0, 0, -1, -1); // The glyph does not exist.
		}
	}
	if(g == formattedText.end()) {
		return Rect(0, 0, -1, -1); // The glyph does not exist.
	}

	return Rect((*g)->getPosition()->getX(), (*g)->getPosition()->getY(), (*g)->getW(), (*g)->getH());
}

