#ifndef ANIMATED_MENU_ITEM_H
#define ANIMATED_MENU_ITEM_H

#include "menu_item.h"

class AnimatedMenuItem : public MenuItem 
{
	private:
		char* keyName;

	public:
#define HEADER animated_menu_item
#define HELPER scriptable_object_h
#include "include_helper.h"
		AnimatedMenuItem(): keyName(NULL) {}
		AnimatedMenuItem(State* attachedState, const char* upName, const char* downName, const char* leftName, const char* rightName, const char* keyName);
		virtual ~AnimatedMenuItem();

		/**
		 * @return the key name.
		 */
		const char* getKeyName() { return keyName; }

		/**
		 * @param name the new key name.
		 */
		void setKeyName(const char* name);
};

#endif /* ANIMATED_MENU_ITEM_H */
