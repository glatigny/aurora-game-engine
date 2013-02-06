#ifndef INTERPOLATED_MENU_H
#define INTERPOLATED_MENU_H

#include "renderable_menu.h"
#include "renderable_menu_item.h"
#include "renderable_interpolator.h"

class InterpolatedMenu : public RenderableMenu
{
	private:
		InterpolationDescriptor* interpolationDesc;
		Renderable*	currentRenderable;
		int duration;
		bool inInterpolation;

	protected:

	public:
#define HEADER interpolated_menu
#define HELPER scriptable_object_h
#include "include_helper.h"
		InterpolatedMenu(State* attachedState, Translatable* offset, GraphicalEffects* effects, const char* scriptFileName, const char* defaultItem, InterpolationDescriptor* interpolationDesc, int duration);
		virtual ~InterpolatedMenu();

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

#endif /* INTERPOLATED_MENU_H */
