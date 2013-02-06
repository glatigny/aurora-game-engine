#ifndef ANIMATED_MENU_H
#define ANIMATED_MENU_H

#include "menu.h"
#include "animated_menu_item.h"
#include "key_animation.h"

class AnimatedMenu : public Menu, public Renderable
{
	private:
		KeyAnimation*	keyAnimation;
	protected:

	public:
#define HEADER animated_menu
#define HELPER scriptable_object_h
#include "include_helper.h"
		AnimatedMenu(): keyAnimation(NULL) {}
		AnimatedMenu(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, VFS::File* scriptFileName, const char* defaultItem, KeyAnimation* keyAnimation);
		virtual ~AnimatedMenu();

		/** Add a new item.
		 * @param itemName the name of the item to add.
		 * @param item the item to add.
		 */
		virtual void addItem(const char* itemName, AnimatedMenuItem* item);

		/** Set a item.
		 * @param itemName the name of the item to set.
		 * @param item the new item for the given name.
		 */
		virtual void replaceItem(const char* itemName, AnimatedMenuItem* item);

		/** Change current key animation.
		 * @param newKeyAnimation the new key animation.
		 */
		virtual void setAnimation( KeyAnimation* newKeyAnimation );

		/* Menu */

		virtual void changeTo(char *destItem);
		virtual void setCurrentItem( char *destItem );

		/* Renderable */

		virtual void renderGraphics();
		virtual void updateObject(dur elapsedTime);
		virtual void loadGraphics();
		virtual void unloadGraphics();

		/* Blocking */

		virtual bool isFinished() {
			return Menu::isFinished() || Renderable::isFinished();
		}

		/* Translatable */

		virtual bool inside(Translatable& firstCorner, Translatable& secondCorner) { return false; }

		/*  */

		virtual AuroraEngine* engine() { return Renderable::engine(); }
		virtual VFS::File* source() { return Renderable::source(); }

};

#endif /* ANIMATED_MENU_H */
