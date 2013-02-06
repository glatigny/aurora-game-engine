#ifndef SECTION_CONTAINER_H
#define SECTION_CONTAINER_H

class SectionContainer;

#include "scriptable_object.h"
#include "renderable.h"
#include "rect.h"
#include "vfs.h"
#include <map>

class SectionContainer : public ScriptableObject, virtual public Entity {
private:
	Renderable* m_frame;
	std::map<unsigned int, Rect*> m_map;

public:
#define HEADER section_container
#define HELPER scriptable_object_h
#include "include_helper.h"

	SectionContainer();
	SectionContainer(State* attachedState, Renderable* newFrame);
	virtual ~SectionContainer();

	void addSection(const char* c, Rect* rect);

	Renderable* getFrame() { return m_frame; }
	Rect getSection(unsigned int c);

	/* AOEObject */

	VFS::File* source() { return m_frame->source(); }
};

#endif /* SECTION_CONTAINER_H */
