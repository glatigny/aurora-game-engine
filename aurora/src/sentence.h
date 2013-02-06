#ifndef SENTENCE_H
#define SENTENCE_H

class Sentence;

#include "renderable.h"
#include "controllable.h"
#include "sub_scriptable.h"
#include "text.h"
#include "state.h"
#include <vector>
#include <map>

class Sentence : public Renderable, public Controllable, public SubScriptable
{
public:
	// Map comparator
	struct strCmp
	{
		bool operator()(const char* s1, const char* s2) const
		{ return strcmp(s1, s2) < 0; }
	};

	typedef enum
	{
		STATE_NONE,
		STATE_RENDER,
		STATE_WAITING_VALID,
		STATE_WAITING_SELECT,
		STATE_END
	} cState;

private:
	Renderable* 					m_background;
	std::vector<Text*>					m_texts;
	std::map<const char*,Text*,strCmp>	m_selects;
	const char*						m_selected;
	Renderable*						m_waitingCursor;
	Renderable*						m_selectingCursor;
	int								current_text_render;
	cState m_state;

public:
#define HEADER sentence
#define HELPER scriptable_object_h
#include "include_helper.h"
	Sentence() : m_background(NULL), m_selected(NULL), m_waitingCursor(NULL), m_selectingCursor(NULL) {}
	Sentence(State* attachedState, Translatable* offset, GraphicalEffects* effects);
	Sentence(State* attachedState, Translatable* offset, GraphicalEffects* effects, Renderable* background, Renderable* waitingCursor, Renderable* selectingCursor, VFS::File* script);
	virtual ~Sentence();

	/**
	 *
	 */
	void addText(Text* text);

	/**
	 *
	 */
	void addAnswer(const char* name, Text* text);

	/**
	 *
	 */
	const char* getIdSelected();

	/**
	 *
	 */
	bool wait();

	/**
	 *
	 */
	bool onBeginSentence();

	/**
	 *
	 */
	bool onEndSentence();

	/**
	 *
	 */
	void valid();

	/* Renderable */

	virtual void renderGraphics();
	virtual void update(dur elapsedTime);
	virtual void loadGraphics();
	virtual void unloadGraphics();

	/* Controllable */

	virtual void processInput(ControllerState* state);

	/* Translatable */
	virtual bool inside(Translatable& firstCorner, Translatable& secondCorner) {return false;}
};

#endif /* SENTENCE_H */

