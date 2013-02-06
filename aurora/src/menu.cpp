#include "menu.h"

#define HEADER menu
#define HELPER scriptable_object_cpp
#include "include_helper.h"

// QC:?
Menu::Menu(State* attachedState, VFS::File* scriptFileName, const char* l_defaultItem) :  Controllable(attachedState), SubScriptable(attachedState->script(), scriptFileName)
{
	if( l_defaultItem && *l_defaultItem )
	{
		defaultItem = AOESTRDUP(l_defaultItem);
	}
	else
	{
		defaultItem = NULL;
	}

	currentItemName = NULL;
	lastKeyPressed = 0;
}


// QC:?
Menu::~Menu()
{
	for(std::map<const char*, MenuItem*, strCmp>::iterator item = items.begin(); item != items.end(); item++) {
		script()->resetval(item->first);
		AOEFREE((void*)(item->first));
		delete item->second;
	}

	if( currentItemName )
		AOEFREE(currentItemName);

	if( defaultItem )
		AOEFREE(defaultItem);
}


// QC:?
void Menu::addItem(const char* itemName, MenuItem* item)
{
	assert( itemName );
	assert( *itemName );
	assert( item );

	items[AOESTRDUP(itemName)] = item;
	script()->setval(itemName, item);

	if( !defaultItem ) {
		defaultItem = AOESTRDUP(itemName);
	}

	if( !currentItemName )
	{
		if( !strcmp(itemName, defaultItem ) )
		{
			setCurrentItem( defaultItem );
		}
	}

}

void Menu::replaceItem(const char* itemName, MenuItem* item)
{
	assert( itemName );
	assert( *itemName );
	assert( item );
	assert( items.find(itemName) != items.end() );

	// Replace item
	delete items[itemName];
	items[itemName] = item;
	script()->setval(itemName, item);
}

void Menu::removeItem(const char* itemName)
{
	assert( itemName );
	assert( *itemName );
	assert( items.find(itemName) != items.end() );

	// Get iterator of the item
	std::map<const char*, MenuItem*, strCmp>::iterator item = items.find(itemName);

	// Destroy it
	script()->resetval(item->first);
	AOEFREE((void*)(item->first));
	delete item->second;
}

/* Controllable */

void Menu::processInput(ControllerState* state)
{
	char* destItem = NULL;

	if(!defaultItem) return;
	if(!currentItemName) {
		setCurrentItem(defaultItem);
	}
#define MENU_DIRECTION_PRESS(KEY, FCT, DIR) \
	if( state->isButtonPressed( KEY ) ) \
	{ \
		MenuItem* item = items[currentItemName]; \
		if(item) { \
			if( (char*)item->FCT() ) \
			{ \
				destItem = AOESTRDUP((char*)item->FCT()); \
				if( destItem[0] != '\0' ) \
				{ \
					char* tmpItem = AOESTRDUP(destItem); \
					do { \
						AOEFREE(tmpItem);\
						tmpItem = destItem; \
						syslog("calling onMove with currentItem=%s and destItem=%s (direction: %d)", currentItemName, destItem, DIR); \
						ScriptVariable sv = script()->callFunction(*this, (char*)"onMove", (char*)"ssi", currentItemName, tmpItem, DIR); \
						if(sv.gettype() == VAR_STR) \
						{ \
							destItem = AOESTRDUP((char*)sv.getstr()); \
							syslog("script answered destItem=%s", destItem); \
						} \
					} while( (strcmp(destItem, currentItemName)) && (strcmp(tmpItem, destItem))); \
				} \
			} \
		} \
	}

	MENU_DIRECTION_PRESS( ControllerState::BTN_Up, getUpName, ControllerState::UP )
	MENU_DIRECTION_PRESS( ControllerState::BTN_Down, getDownName, ControllerState::DOWN )
	MENU_DIRECTION_PRESS( ControllerState::BTN_Left, getLeftName, ControllerState::LEFT )
	MENU_DIRECTION_PRESS( ControllerState::BTN_Right, getRightName, ControllerState::RIGHT )

#undef DIRECTION_PRESS

	if(destItem) {
		changeTo( destItem );
		AOEFREE( destItem );
	}

#define MENU_KEY_PRESS( KEY ) \
	if( state->isButtonPressed( KEY ) ) \
	{ \
		script()->callFunction(*this, (char*)"onKey", (char*)"si", currentItemName, KEY ); \
		lastKeyPressed = KEY; \
	}

	MENU_KEY_PRESS( ControllerState::BTN_A )
	MENU_KEY_PRESS( ControllerState::BTN_B )
	MENU_KEY_PRESS( ControllerState::BTN_X )
	MENU_KEY_PRESS( ControllerState::BTN_Y )
	MENU_KEY_PRESS( ControllerState::BTN_L )
	MENU_KEY_PRESS( ControllerState::BTN_R )
	MENU_KEY_PRESS( ControllerState::BTN_Start )
	MENU_KEY_PRESS( ControllerState::BTN_Select )


#undef MENU_KEY_PRESS

	// Make script run
	framestep();
}

int Menu::getLastKeyPressed() {
	return lastKeyPressed;
}

void Menu::ackWait() {
	lastKeyPressed = 0;
}

void Menu::changeTo(char *destItem)
{
	if( destItem && strcmp( destItem, currentItemName ) )
	{
		if( currentItemName )
			AOEFREE( currentItemName );

		currentItemName = AOESTRDUP(destItem);
	}
}

void Menu::setCurrentItem( char *itemName )
{
	if( itemName && strcmp( itemName, currentItemName ) )
	{
		if( currentItemName )
			AOEFREE( currentItemName );

		currentItemName = AOESTRDUP(itemName);
	}
}

char* Menu::getCurrentItem() {
	return currentItemName;
}

char* Menu::onMove(char* startItem, char* destItem, int direction)
{
	return destItem;
}

void Menu::onKey(char* itemName, int key)
{
}
