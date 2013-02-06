#include "renderable_menu_item.h"
#include "state.h"

#define HEADER renderable_menu_item
#define HELPER scriptable_object_cpp
#include "include_helper.h"

RenderableMenuItem::RenderableMenuItem(const char* upName, const char* downName, const char* leftName, const char* rightName, Renderable* l_attachedRenderable) : MenuItem(upName, downName, leftName, rightName)
{
	attachedRenderable = l_attachedRenderable;
	if( attachedRenderable)
	{
//FIXME : RenderableMenuItem does not seem to be Renderable ? !		attachedRenderable->setParent(this);
	}
}

// QC:?
RenderableMenuItem::~RenderableMenuItem()
{
	// TODO : May leak
}

void RenderableMenuItem::setRenderable(Renderable* newAttachedRenderable)
{
	attachedRenderable = newAttachedRenderable;
//FIXME : RenderableMenuItem does not seem to be Renderable ? !		attachedRenderable->setParent(this);
	}
}
