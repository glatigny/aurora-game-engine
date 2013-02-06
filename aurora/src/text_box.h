#ifndef TEXT_BOX_H
#define TEXT_BOX_H

#include "state.h"
#include "renderable.h"
class TextLine;
class Font;

/** This class displays a zone of formatted text, with automatic line feed.
 *
 * text zone example :
 * ---
 * This is some text. Spaces    are  collapsed.<br />
 * <text font="big">This text is bigger.</text><br>
 * <text delay="30" speed="60">This text will be slowed down and displayed at 60% speed</text>
 * ---
 */
class TextBox : public Renderable, public Resizable {
private:
	TiXmlElement* text;
	Font* defaultFont;
	int lineHeightMode;
	Point maxSize;
	int overflowType;
	dur delay;
	dur speed; // Speed in pixels per second
	char* transitionInterpolation;
	char* verticalTransitionInterpolation;
	GraphicalEffects textEffects;

	int lineHeight;
	bool metricsComputed;

	TiXmlNode* child;
	int lineCount;

protected:
	std::list<TextLine*> formattedText;
	const char* currentText;

	void computeMetrics();
	void resetMetrics();

public:
#define HEADER text_box
#define HELPER scriptable_object_h
#include "include_helper.h"
	TextBox(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, TiXmlNode* text, Font* defaultFont, int lineHeightMode, Translatable* maxSize, int overflowType, dur delay, dur speed, const char* transitionInterpolation, Renderable* textEffects);

	TextBox() : text(NULL), defaultFont(NULL), transitionInterpolation(NULL), verticalTransitionInterpolation(NULL), child(NULL), currentText(NULL) {}
	virtual ~TextBox();


	/** Gets the number of characters that could not be displayed because of overflow.
	 * @return the number of characters that could not be displayed because of overflow.
	 */
	int getOverflowLength();

	/** Gets the text that could not be displayed.
	 * @return the text that could not be displayed because of overflow, as a new TiXmlElement. WARNING : The caller must delete this object.
	 */
	TiXmlElement* getOverflowText();

	/** Resets all text formatting.
	 **/
	void resetTextFormat();

	/* Renderable */

	virtual void renderGraphics();
	virtual bool inside(Translatable& firstCorner, Translatable& secondCorner) { return false; }
	virtual void loadGraphics();
	virtual void unloadGraphics();

	/* Realtime */

	virtual void updateObject(dur elapsedTime);

	/* Restartable */

	virtual void restart();

};

#endif /* TEXT_BOX_H */
