#ifndef MENU_H
#define MENU_H

#include <map>

#include "renderable.h"
#include "controllable.h"
#include "sub_scriptable.h"
#include "state.h"

#include "menu_item.h"

class Menu : public Controllable, public SubScriptable
{
	public:
		// Map comparator
		struct strCmp
		{
			bool operator()(const char* s1, const char* s2) const
			{ return strcmp(s1, s2) < 0; }
		};

	private:

	protected:
		std::map<const char*, MenuItem*, strCmp> items;
		char* currentItemName;
		char* defaultItem;
		int lastKeyPressed;

	public:
#define HEADER menu
#define HELPER scriptable_object_h
#include "include_helper.h"
		Menu():currentItemName(NULL), defaultItem(NULL) {}
		Menu(State* attachedState, VFS::File* scriptFileName, const char* defaultItem);
		virtual ~Menu();

		/** Add a new item.
		 * @param itemName the name of the item to add.
		 * @param item the item to add.
		 */
		virtual void addItem(const char* itemName, MenuItem* item);

		/** Set a item.
		 * @param itemName the name of the item to set.
		 * @param item the new item for the given name.
		 */
		virtual void replaceItem(const char* itemName, MenuItem* item);

		/** Remove a item.
		 * @param itemName the name of the item to remove.
		 */
		virtual void removeItem(const char* itemName);

		/** Event on Move.
		 * Ask to the destItem if the cursor could go on it.
		 * item will answer with a itemName. Itself if it's ok, the source item if not or
		 * the name of another item if cursor is redirect.
		 * @param startItem name of the source item.
		 * @param destItem name of the destination item.
		 * @param direction current direction of the movement.
		 * @return Name of the item which will be pointed.
		 */
		virtual char* onMove(char* startItem, char* destItem, int direction);

		/** Change current item.
		 * @param destItem the next item.
		 */
		virtual void changeTo(char *destItem);

		/** Set the current item.
		 * The change will be instantanous.
		 * @param destItem the new current item.
		 */
		virtual void setCurrentItem( char *itemName );

		/** Return the current item name.
		 * @return the name of the current active item.
		 */
		virtual char* getCurrentItem();

		/** Event on Key.
		 * @param itemName the name of the item which receive the event.
		 * @param key the pressed key.
		 */
		virtual void onKey(char* itemName, int key);

		/** Returns the last key pressed.
		 * @return the scancode of the last pressed key.
		 */
		virtual int getLastKeyPressed();

		/** Acknowledge Wait
		 * Reset the last key pressed in order to use the menu again.
		 */
		virtual void ackWait();

		/* Controllable */

		virtual void processInput(ControllerState* state);

		/* SubScriptable */

		virtual void framestep() {}

		/* Blocking */

		virtual bool isFinished() { return lastKeyPressed != 0; }

		/* */

		virtual VFS::File* source() { return SubScriptable::source(); }
};

#endif

