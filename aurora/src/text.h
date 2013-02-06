#ifndef TEXT_H
#define TEXT_H

class Text;

#include "renderable.h"
#include "section_container.h"

/** A text is a graphical element 
 * to render text to screen
 * it use a font
 */
class Text : public Renderable {
protected:
	SectionContainer*	font;
	int					speed;
	bool				playing;
	char*				m_text;
	unsigned int		m_text_size;
	unsigned int		m_text_pos;
	Rect*				m_size;			// Height and Width for drawing section
	unsigned char*		m_alpha;		// same lenght of m_text. Contains value of alpha for the character on same position

public:
#define HEADER text
#define HELPER scriptable_object_h
#include "include_helper.h"
	Text() : font(NULL), m_text(NULL), m_size(NULL), m_alpha(NULL) {}
	Text(State* attachedState);
	//Text(State* attachedState, Translatable* offset, GraphicalEffects* effects, SectionContainer* font);
	Text(State* attachedState, Translatable* offset, GraphicalEffects* effects, const char* text, int speed, SectionContainer* font, Translatable* size);
	virtual ~Text();
	
	/** setText set text 
	 * @param text 
	 */
	void setText(const char* text);
	
	/** getText return current text
	 *  @return text
	 */
	char* getText();
	
	/** setFont set the font used for text
	 * @param font
	 */
	void setFont(SectionContainer* font);
	
	/** getFont return font for text
	 * @return font
	 */
	SectionContainer* getFont();
	
	/** Set the current speed of the text
	 * @param the new value for the speed
	 */
	void setSpeed(int speed);

	/** Get current speed
	 * @return the current speed
	 */
	int getSpeed();

	/** setAlpha set alpha for the specified printable char
	 * @param alpha level of alpha
	 * @param firstChar printable character to set alpha
	 */
	virtual void setOpacity(int alpha, unsigned int firstChar);
	
	/** setAlpha set alpha for in range printable char
	 * @param alpha level of alpha
	 * @param firstChar first printable character to set alpha
	 * @param lastChar last printable character to set alpha
	 */
	virtual void setOpacity(int alpha, unsigned int firstChar, unsigned int lastChar);
		
	/** Set the current Size
	 * @param a Point with the size
	 */
	void setSize(Point size);
	void setSize(Rect size);
	void setSize(Point pos, Point size);

	/** Get the current Size
	 * @return the size in a Point
	 */
	Translatable* getSize();

	/** Get end point
	 * @return the point where the text will end
	 */
	Point getEndPoint();

	/** Start the text animation
	 */
	void play();
	
	/** Stop the text animation
	 */
	void stop();


	void rewind();

	/** Wait until the text animation finishes.
	 * @return true if the animation finished, false otherwise.
	 */
	bool wait();

	/* Renderable */
	virtual void renderGraphics();
	virtual void update(dur elapsedTime) ;
	virtual void loadGraphics() {}
	virtual void unloadGraphics() {}

	/* Translatable */
	virtual bool inside(Translatable& firstCorner, Translatable& secondCorner) {return false;}
};

#endif
