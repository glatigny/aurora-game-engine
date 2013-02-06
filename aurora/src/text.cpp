#include "text.h"

#include "context.h"
#include "state.h"

#define HEADER text
#define HELPER scriptable_object_cpp
#include "include_helper.h"

// QC:S
Text::Text(State* attachedState) : Entity(attachedState) 
{
	m_alpha = NULL;
	m_text = NULL;
	m_text_size = 0;
	m_text_pos = 0;
	speed = 0;
	m_size = new Rect();
	playing = false;
	this->font = NULL;
}

// QC:S
/*
Text::Text(State* attachedState, Translatable* offset, GraphicalEffects* effects, SectionContainer* font) : Renderable(attachedState, offset, effects)
{
	m_alpha = NULL;

	this->font = font;

	m_text = NULL;
	m_text_size = 0;
	m_text_pos = 0;
	speed = 0;
	m_size = new Rect();
	playing = false;
}
*/

// QC:S
Text::Text(State* attachedState, Translatable* offset, GraphicalEffects* effects, const char* text, int speed, SectionContainer* font, Translatable* size) : Entity(attachedState), Renderable(attachedState, offset, effects, 0)
{
	m_alpha = NULL;

	this->font = font;

	m_size = new Rect();
	m_size->setPosition(*offset);
	m_size->setW(size->getX());
	m_size->setH(size->getY());

	playing = false;
	m_text = NULL;
	m_text_size = 0;
	this->speed = speed;
	m_text_pos = 0;
	if( text != NULL )
		setText(text);
}

// QC:S
Text::~Text()
{
	if( m_text != NULL)
		delete m_text;
	m_text_size = 0;
	m_text_pos = 0;
	
	if( m_alpha != NULL )
		delete m_alpha;
	m_alpha = NULL;

	delete m_size;
	m_size = NULL;
}

// QC:S
void Text::renderGraphics()
{
	if( (font == NULL) || (m_text == NULL) )
		return;

	Renderable *font_frame = font->getFrame();
	Point pos;
	GraphicalEffects effect;
	
	Rect r;
	char *c = new char[2];
	c[1] = '\0';

	for( unsigned int i = 0; i < m_text_size ; i++ )
	{
		c[0] = m_text[i];
		/*FIXME: Multibyte character*/
		r = font->getSection( (unsigned int)c[0] );
	
		if( (r.getW() != 0) && (r.getH() != 0 ) )
		{
			if( m_alpha != NULL )
			{
				effect.setOpacity(m_alpha[i]);
			}

			effect.setSectionPointStart( *r.getPosition() );
			effect.setSectionWidthHeight( *r.getSize() );
			
			font_frame->setPosition(pos);
			font_frame->setObjectGraphicalEffects(effect);
			font_frame->render();
		}

		if( (pos.getX() >= (m_size->getW()))
			|| (m_text[i] == '\n'))
			pos.setPosition( 0, pos.getY() + r.getH());		
	}
}

// QC:S
void Text::update(dur elapsedTime)
{
	if( elapsedTime == 0) 
		return;

	if( m_text && m_alpha && playing )
	{
		if( m_text_pos >= 0 )
		{
			int value;

			if( speed > 0 )
			//	int value = (int)((25500) / (elapsedTime * speed));
				value = elapsedTime * speed;
			else
				value = 255 * m_text_size;

			while( (value > 0) && playing )
			{
				if( (m_alpha[m_text_pos] + value) > 255 )
				{
					value -= 255 - m_alpha[m_text_pos];
					m_alpha[m_text_pos] = 255;
					m_text_pos++;
				}
				else
				{
					m_alpha[m_text_pos] += value;
					value = 0;
				}

				if( (m_text_pos >= (m_text_size -1)) && (m_alpha[m_text_size-1] == 255))
				{
					playing = false;
					return;
				}
			}
		}
	}
}

// QC:P
void Text::setText(const char* text)
{
	if( m_text != NULL )
		delete m_text;
	m_text = AOESTRDUP(text);
	m_text_size = strlen(m_text);
	
	if( speed != 0 )
	{
		m_text_pos = 0;

		if( m_alpha )
			AOEFREE(m_alpha);
		m_alpha = new unsigned char[m_text_size];
		for( unsigned int i = 0; i < m_text_size; i++ )
		{
			m_alpha[i] = 0;
		}
	}
}

// QC:P
char* Text::getText()
{
	return m_text;
}

// QC:S
void Text::setFont(SectionContainer* font)
{
	this->font = font;
}
	
// QC:P
SectionContainer* Text::getFont()
{
	return this->font;
}

// QC:P
void Text::setSpeed(int speed)
{
	this->speed = speed;
}

// QC:P
int Text::getSpeed()
{
	return this->speed;
}

// QC:S
void Text::setOpacity(int alpha, unsigned int firstChar)
{
	if( firstChar >= 0 )
	{
		if( m_alpha == NULL )
		{
			m_alpha = new unsigned char[m_text_size];
			for( unsigned int i = 0; (i < firstChar) && (i < m_text_size); i++ )
			{
				m_alpha[i] = 255; //objectEffects->opacity;
			}
		}
		for( unsigned int i = firstChar; i < m_text_size; i++ )
		{
			m_alpha[i] = alpha;
		}
	}
	else
	{
		if( m_alpha != NULL )
			delete m_alpha;
		m_alpha = NULL;
		Renderable::setOpacity(alpha);
	}
}

// QC:S
void Text::setOpacity(int alpha, unsigned int firstChar, unsigned int lastChar)
{
	if( m_alpha == NULL )
	{
		m_alpha = new unsigned char[m_text_size];
		for( unsigned int i = 0; i < m_text_size; i++ )
		{
			m_alpha[i] = 255; //objectEffects->opacity;
		}
	}

	for( unsigned int i = firstChar; (i < m_text_size) && (i < lastChar); i++ )
	{
		m_alpha[i] = alpha;
	}
}
	
// QC:P
void Text::setSize(Point size)
{
	m_size->setPosition(*getPosition());
	m_size->setH(size.getY());
	m_size->setW(size.getX());
}

// QC:P
void Text::setSize(Rect size)
{
	m_size->setPosition(*size.getPosition());
	m_size->setH( size.getH() );
	m_size->setW( size.getW() );
}

// QC:P
void Text::setSize(Point pos, Point size)
{
	m_size->setPosition(pos);
	m_size->setH(size.getY());
	m_size->setW(size.getX());
}

// QC:P
Translatable* Text::getSize()
{
	return m_size->getSize();
}

// QC:P
Point Text::getEndPoint()
{
	Point pos;
	Rect r;
	pos.setPosition(*getPosition());
	char *c = new char[2];
	c[1] = '\0';

	pos.translate( -(m_size->getX()/2), -(m_size->getY()/2) );

	for( unsigned int i = 0; i < m_text_size ; i++ )
	{
		c[0] = m_text[i];
		/*FIXME: Multibyte character*/
		r = font->getSection( (unsigned int)c[0] );
	
		if( (r.getW() != 0) && (r.getH() != 0 ) )			
			pos.translate( r.getW(), 0 );

		if( (pos.getX() > (getPosition()->getX() + (m_size->getX()/2)))
			|| (m_text[i] == '\n'))
			pos.setPosition( getPosition()->getX()-(m_size->getX()/2), 
							pos.getY() + r.getH());		
	}

	return pos;
}

// QC:P
void Text::play()
{
	playing = true;
}

// QC:P
void Text::stop()
{
	playing = false;
}

void Text::rewind()
{
	m_text_pos = 0;
	if( m_alpha != NULL )
	{
		for( unsigned int i = 0; i < m_text_size; i++ )
		{
			m_alpha[i] = 0;
		}
	}
}

// QC:P
bool Text::wait()
{
	return playing;
}

