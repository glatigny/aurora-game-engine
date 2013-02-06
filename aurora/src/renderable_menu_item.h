#ifndef RENDERABLE_MENU_ITEM_H
#define RENDERABLE_MENU_ITEM_H

#include "menu_item.h"
#include "renderable.h"

class RenderableMenuItem : public MenuItem 
{
	private:
		Renderable* attachedRenderable;

	public:
#define HEADER renderable_menu_item
#define HELPER scriptable_object_h
#include "include_helper.h"
		RenderableMenuItem(): attachedRenderable(NULL) {}
		RenderableMenuItem(const char* upName, const char* downName, const char* leftName, const char* rightName, Renderable* attachedRenderable);
		virtual ~RenderableMenuItem();

		/**
		 * @return the key name.
		 */
		Renderable* getRenderable() { return attachedRenderable; }

		/**
		 * @param name the new key name.
		 */
		void setRenderable(Renderable* newAttachedRenderable);
};

#endif /* RENDERABLE_MENU_ITEM_H */
