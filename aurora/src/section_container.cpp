#include "section_container.h"

#include "context.h"
#include "state.h"

#define HEADER section_container
#define HELPER scriptable_object_cpp
#include "include_helper.h"

SectionContainer::SectionContainer()
{

}

SectionContainer::SectionContainer(State* attachedState, Renderable* newFrame)
{
	m_frame = newFrame;
}

SectionContainer::~SectionContainer()
{
	for(std::map<unsigned int, Rect*>::iterator l_map = m_map.begin(); l_map != m_map.end(); l_map++) {
		AOEFREE((void*)(l_map->second));
		AOEFREE((void*)(l_map->first));
	}
}

void SectionContainer::addSection(const char* c, Rect* rect)
{
	/*FIXME: multibyte characters*/	
	m_map[(unsigned int)c[0]] = new Rect(*rect);
}

Rect SectionContainer::getSection(unsigned int c)
{
	if( m_map.find(c) != m_map.end() )
	{
		return *(m_map.find(c)->second);
	}
	Rect r(0,0,0,0);
	return r;
}

