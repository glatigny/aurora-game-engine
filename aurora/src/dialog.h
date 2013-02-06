#ifndef DIALOG_H
#define DIALOG_H

class Dialog;

#include "renderable.h"
#include "controllable.h"
#include "text.h"
#include <map>

class Dialog : public Renderable, public Controllable
{
public:
	// Map comparator
	struct strCmp
	{
		bool operator()(const char* s1, const char* s2) const
		{ return strcmp(s1, s2) < 0; }
	};

private:
	map<const char*, Text*, strCmp>  m_texts;

public:
#define HEADER dialog
#define HELPER scriptable_object_h
#include "include_helper.h"

	Dialog(State* attachedState, Translatable* offset, GraphicalEffects* effects);
	virtual ~Dialog();

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

#endif /* DIALOG_H */
