#ifndef MENU_ITEM_H
#define MENU_ITEM_H

#include <string.h>
#include "context.h"
#include "controllable.h"
#include "entity.h"

class MenuItem : public ScriptableObject, public Entity
{
	private:
		char* upName;
		char* downName;
		char* leftName;
		char* rightName;

	public:
#define HEADER menu_item
#define HELPER scriptable_object_h
#include "include_helper.h"
		MenuItem():upName(NULL), downName(NULL), leftName(NULL), rightName(NULL) {}
		MenuItem(State* attachedState, const char* upName, const char* downName, const char* leftName, const char* rightName);
		virtual ~MenuItem();

		/** Get up item name.
		 * @return name of the up item.
		 */
		const char* getUpName();

		/** Get down item name.
		 * @return name of the down item.
		 */
		const char* getDownName();

		/** Get left item name.
		 * @return name of the left item.
		 */
		const char* getLeftName();

		/** Get right item name.
		 * @return name of the right item.
		 */
		const char* getRightName();

		/** Get item name by giving a direction.
		 * @param dir Direction
		 * @return name of the right item.
		 */
		const char* getName(ControllerState::Direction dir);

		/** Set the name of the upper item
		 * @param name Name of the upper item
		 */
		void setUpName(const char* name);

		/** Set the name of the lower item
		 * @param name Name of the lower item
		 */
		void setDownName(const char* name);

		/** Set the name of the left item
		 * @param name Name of the left item
		 */
		void setLeftName(const char* name);

		/** Set the name of the right item
		 * @param name Name of the right item
		 */
		void setRightName(const char* name);

		virtual VFS::File* source() { return ScriptableObject::source(); }
};

#endif /* MENU_ITEM_H */
