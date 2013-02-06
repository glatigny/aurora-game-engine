#include "text_box.h"
#include "text_line.h"
#include "font.h"
#include "factory.h"

#define HEADER text_box
#define HELPER scriptable_object_cpp
#include "include_helper.h"


TextBox::TextBox(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, TiXmlNode* newText, Font* newDefaultFont, int newLineHeightMode, Translatable* newMaxSize, int newOverflowType, dur newDelay, dur newSpeed, const char* newTransitionInterpolation, Renderable* newTextEffects) : Translatable(offset), Entity(attachedState), Renderable(attachedState, offset, effects, layer), defaultFont(newDefaultFont), lineHeightMode(newLineHeightMode), maxSize(newMaxSize), overflowType(newOverflowType), delay(newDelay), speed(newSpeed), lineHeight(-1), metricsComputed(false) {
	xassert(newText, "No text tag for this TextBox.");
	text = newText->Clone()->ToElement();

	xassert(defaultFont, "You must specify a default font for this TextBox.");
	
	
	if(newTransitionInterpolation) {
		transitionInterpolation = AOESTRDUP(newTransitionInterpolation);
	} else {
		transitionInterpolation = NULL;
	}

	if(newTextEffects)
	{
		textEffects = *newTextEffects->getObjectGraphicalEffects();
	}

}

// QC:?
TextBox::~TextBox()
{
	delete text;
	if(transitionInterpolation)
		AOEFREE(transitionInterpolation);
}

// QC:?
void TextBox::resetTextFormat()
{
	if(metricsComputed)
	{
		for(std::list<TextLine*>::iterator i = formattedText.begin(); i != formattedText.end(); i++)
		{
			mayDelete(*i);
		}
		metricsComputed = false;
	}
}

// Helper for computeMetrics
struct TextAttributes {
	Font* font;
	dur relSpeed; // Relative speed in %
	const char* interpolation;
	GraphicalEffects effects;
};

// QC:?
void TextBox::computeMetrics() {

	std::vector<TextAttributes> attribStack(4);
	TextAttributes attribs;

	attribs.font = defaultFont;
	attribs.relSpeed = 100;
	attribs.interpolation = transitionInterpolation;
	attribs.effects = textEffects;

	dur delay = this->delay;
	Point position;

	assert(!metricsComputed);

	TiXmlElement* currentNode = text;
	TiXmlElement* rootNode = currentNode;
	int currentLineHeight = lineHeight == -1 ? 0 : lineHeight;
	child = 0;
	bool hasMaxSize = maxSize.getX() > 0 && maxSize.getY() > 0;

	do {
		child = currentNode->IterateChildren(child);
		if(child) {
			switch(child->Type()) {
				case TiXmlNode::TEXT: {
					const char* text = child->Value();

					while(text) {
						// Generate new text strip.
						coord maxWidth;
						if(hasMaxSize)
							maxWidth = (maxSize.getX() - position.getX())*100/attribs.effects.getHorizontalScale();
						else
							maxWidth = -1;

						TextLine* line = new TextLine(state(), &position, &(attribs.effects), 0, text, attribs.font, maxWidth, OVERFLOW_LET_LAST_LETTER, delay, delay, attribs.interpolation); // TODO : implement other modes than LAST_LETTER
						own(line);
						line->setParent(this);
						line->formatText();
						formattedText.push_back(line);

						coord lineWidth = line->getRealW();

						// Compute real display time
						if(speed) {
							dur displayTime = lineWidth / (speed * (attribs.relSpeed / 100));
							line->setMaxTime(delay + displayTime);
							delay += displayTime;
						}
	
						// Compute new cursor position.
						position.translate(Point(lineWidth, 0));

						// Update currentLineHeight.
						currentLineHeight = MAX(currentLineHeight, line->getH());
	
						// If overflown
						if( (text = line->getOverflowText()) ) {
							// Go to next line.

							// Reset cursor position.
							position.setPosition(0, position.getY() + currentLineHeight);

							// Reset currentLineHeight.
							if(lineHeight != -1) {
								currentLineHeight = lineHeight;
							}
						}
					}
				}
				break;

				case TiXmlNode::ELEMENT: {
					const char* tagName = child->Value();
					if(strcmp(tagName, "br") == 0) {
						// Apply forced newLine

						coord maxHeight = maxSize.getY();
						coord positionY = position.getY();
						if(maxHeight > 0 && positionY + currentLineHeight >= maxHeight) {
							// Vertical overflow.
							// TODO : implement vertical overflow
							return;
						}

						// Reset cursor position.
						position.setPosition(0, positionY + currentLineHeight);

						// Reset currentLineHeight.
						if(lineHeight != -1) {
							currentLineHeight = lineHeight;
						}
						
					} else {
						// Descend into the child.
						attribStack.push_back(attribs);
						currentNode = (TiXmlElement*)child;
						child = 0;

						// Read custom node attributes.
						const char* attribValue = currentNode->Attribute("font");
						if(attribValue) {
							Font* newFont = dynamic_cast<Font*>(state()->script()->getobj(attribValue));
							if(newFont) {
								attribs.font = newFont;
							}
						}

						attribValue = currentNode->Attribute("speed");
						if(attribValue) {
							attribs.relSpeed = atoc(attribValue);
						}

						attribValue = currentNode->Attribute("interpolation");
						if(attribValue) {
							attribs.interpolation = attribValue;
						}

						GraphicalEffects nodeEffects = Factory::loadEffects(currentNode);
						attribs.effects += nodeEffects;
					}
				}
				break;
			}
		} else if(currentNode != rootNode) {
			// Go up to the parent.
			child = currentNode;
			currentNode = (TiXmlElement*)currentNode->Parent();
			attribs = attribStack.back();
			attribStack.pop_back();
		}
	} while(child || currentNode != rootNode);

	metricsComputed = true;
}

// QC:?
void TextBox::renderGraphics()
{
	if(!metricsComputed)
	{
		computeMetrics();
	}

	for(std::list<TextLine*>::iterator i = formattedText.begin(); i != formattedText.end(); i++)
	{
		(*i)->render();
	}
}

// QC:?
void TextBox::updateObject(dur elapsedTime)
{
	if(!metricsComputed)
	{
		computeMetrics();
	}

	for(std::list<TextLine*>::iterator i = formattedText.begin(); i != formattedText.end(); i++)
	{
		(*i)->update(elapsedTime);
	}
}

// QC:?
void TextBox::restart()
{
	if(!metricsComputed)
	{
		return;
	}

	for(std::list<TextLine*>::iterator i = formattedText.begin(); i != formattedText.end(); i++)
	{
		(*i)->restart();
	}
}

// QC:?
void TextBox::loadGraphics()
{
	if(!metricsComputed)
	{
		computeMetrics();
	}

	for(std::list<TextLine*>::iterator i = formattedText.begin(); i != formattedText.end(); i++)
	{
		(*i)->loadGraphics();
	}
}

// QC:?
void TextBox::unloadGraphics()
{
	if(!metricsComputed)
	{
		return;
	}

	for(std::list<TextLine*>::iterator i = formattedText.begin(); i != formattedText.end(); i++)
	{
		(*i)->unloadGraphics();
	}
}

