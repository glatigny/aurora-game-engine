#include "sentence.h"

#define HEADER sentence
#define HELPER scriptable_object_cpp
#include "include_helper.h"

Sentence::Sentence(State *attachedState, Translatable *offset, GraphicalEffects *effects) : Entity(attachedState), Renderable(attachedState, offset, effects, 0)
{
	m_background = NULL;
	m_texts.clear();
	m_selects.clear();
	m_selected = NULL;
	m_waitingCursor = NULL;
	m_selectingCursor = NULL;
	m_state = STATE_NONE;
	current_text_render = 0;
}

Sentence::Sentence(State* attachedState, Translatable* offset, GraphicalEffects* effects, Renderable* background, Renderable* waitingCursor, Renderable* selectingCursor, VFS::File* script) : Entity(attachedState), Renderable(attachedState, offset, effects, 0), SubScriptable(state()->script(), script)
{
	m_background = background;
	m_texts.clear();
	m_selects.clear();
	m_selected = NULL;
	m_waitingCursor = waitingCursor;
	m_selectingCursor = selectingCursor;
	m_state = STATE_NONE;
	current_text_render = 0;

}

Sentence::~Sentence()
{
	for(std::map<const char*, Text*, strCmp>::iterator l_map = m_selects.begin(); l_map != m_selects.end(); l_map++) {
		AOEFREE((void*)(l_map->first));
	}
}

void Sentence::addText(Text* text)
{
	m_texts.insert(m_texts.end(), text);
}

void Sentence::addAnswer(const char* name, Text* text)
{
	const char* dup = AOESTRDUP(name);
	m_selects[dup] = text;
}

const char* Sentence::getIdSelected()
{
	return m_selected;
}

bool Sentence::wait()
{
	if( m_state == STATE_END )
		return false;
	return true;
}

bool Sentence::onBeginSentence()
{
	return false;
}

bool Sentence::onEndSentence()
{
	return false;
}

void Sentence::renderGraphics()
{
	if( m_state == STATE_END )
		return;

	if( m_background )
		m_background->render();
	
	if( m_state == STATE_WAITING_VALID )
	{
		if( m_waitingCursor )
			m_waitingCursor->render();
	}

	for(std::vector<Text*>::iterator vec = m_texts.begin(); vec != m_texts.end(); vec++)
	{
		(*vec)->render();
	}
}

void Sentence::update(dur elapsedTime)
{
	if( m_state == STATE_END )
		return;

	if( m_background )
		m_background->update(elapsedTime);
	
	if( m_state == STATE_NONE )
	{
		m_texts.at(current_text_render)->play();
		script()->callFunction(*this, (char*)"onBeginSentence");
		m_state = STATE_RENDER;
	}
	else if( m_state == STATE_RENDER )
	{
		if( !m_texts.at(current_text_render)->wait() )
		{
			current_text_render++;
			if( (current_text_render - (int)m_texts.size()) == 0 )
			{
				m_state = STATE_WAITING_VALID;
				script()->callFunction(*this, (char*)"onEndSentence");
			}
			else
				m_texts.at(current_text_render)->play();
		}
	}
	else if( m_state == STATE_WAITING_VALID )
	{
		if( m_waitingCursor )
			m_waitingCursor->update(elapsedTime);
	}

	for(std::vector<Text*>::iterator vec = m_texts.begin(); vec != m_texts.end(); vec++)
	{
		(*vec)->update(elapsedTime);
	}

	if(script()->threadsRunning()) {
		script()->framestep();
	}
}

void Sentence::loadGraphics() {
	if( m_background != NULL )
		m_background->loadGraphics();
	if( m_waitingCursor != NULL )
		m_waitingCursor->loadGraphics();
	if( m_selectingCursor != NULL )
		m_selectingCursor->loadGraphics();
}

void Sentence::unloadGraphics() {
	if( m_background != NULL )
		m_background->unloadGraphics();
	if( m_waitingCursor != NULL )
		m_waitingCursor->unloadGraphics();
	if( m_selectingCursor != NULL )
		m_selectingCursor->unloadGraphics();
}

void Sentence::processInput(ControllerState *state)
{
	if( state->isButtonPressed(ControllerState::BTN_A) )
	{
		valid();
	}
}

void Sentence::valid() 
{
		if( m_state == STATE_RENDER )
		{
			for(unsigned int i = 0; i < m_texts.size(); i++)
				m_texts.at(i)->setOpacity(255,0);
			m_state = STATE_WAITING_VALID;
			script()->callFunction(*this, (char*)"onEndSentence");
		}
		else if( m_state == STATE_WAITING_VALID )
		{
			m_state = STATE_END;
		}
}
