#include "animated_menu_item.h"
#include "state.h"

#define HEADER animated_menu_item
#define HELPER scriptable_object_cpp
#include "include_helper.h"

AnimatedMenuItem::AnimatedMenuItem(State* attachedState, const char* upName, const char* downName, const char* leftName, const char* rightName, const char* l_keyName) : MenuItem(attachedState, upName, downName, leftName, rightName)
{
	if( l_keyName )
		keyName = AOESTRDUP(l_keyName);
	else
		keyName = NULL;

	syslog("Item name=%s, up=%s, down=%s, left=%s, right=%s", keyName, upName, downName, leftName, rightName);
}

AnimatedMenuItem::~AnimatedMenuItem()
{
	if( keyName )
	{
		AOEFREE(keyName);
		keyName = NULL;
	}
}

void AnimatedMenuItem::setKeyName(const char* name)
{
	if( keyName )
	{
		AOEFREE( keyName );
	}
	
	if( name )
	{
		keyName = AOESTRDUP( name );
	}
	else
	{
		keyName = NULL;
	}
}

