#ifndef RENDERABLE_MENU_H
#define RENDERABLE_MENU_H

#include "menu.h"
#include "renderable_menu_item.h"
#include "renderable.h"

class RenderableMenu : public Menu, public Renderable
{
	private:
		Renderable*	currentRenderable;
	protected:

	public:
#define HEADER renderable_menu
#define HELPER scriptable_object_h
#include "include_helper.h"
		RenderableMenu(): currentRenderable(NULL) {}
		RenderableMenu(State* attachedState, Translatable* offset, GraphicalEffects* effects, const char* scriptFileName, const char* defaultItem);
		~RenderableMenu() {}

		/** Add a new item.
		 * @param itemName the name of the item to add.
		 * @param item the item to add.
		 */
		virtual void addItem(const char* itemName, RenderableMenuItem* item);

		/** Set a item.
		 * @param itemName the name of the item to set.
		 * @param item the new item for the given name.
		 */
		virtual void replaceItem(const char* itemName, RenderableMenuItem* item);

		/* Menu */

		virtual void changeTo(char *destItem);
		virtual void setCurrentItem( char *destItem );

		/* Renderable */

		virtual void renderGraphics();
		virtual void update(dur elapsedTime);
		virtual void loadGraphics();
		virtual void unloadGraphics();

		/* Translatable */

		virtual bool inside(Translatable& firstCorner, Translatable& secondCorner) { return false; }
};

#endif /* RENDERABLE_MENU_H */
