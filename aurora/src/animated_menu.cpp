#include "animated_menu.h"


#define HEADER animated_menu
#define HELPER scriptable_object_cpp
#include "include_helper.h"

AnimatedMenu::AnimatedMenu(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, VFS::File* scriptFileName, const char* defaultItem, KeyAnimation* l_keyAnimation) : Entity(attachedState), Menu(attachedState, scriptFileName, defaultItem), Renderable(attachedState, offset, effects, layer)
{
	assert(l_keyAnimation != NULL);
	keyAnimation = l_keyAnimation;
	if( keyAnimation ) {
		keyAnimation->setParent(this);
	}
}

AnimatedMenu::~AnimatedMenu()
{
	if( keyAnimation )
		delete keyAnimation;
}

void AnimatedMenu::addItem(const char* itemName, AnimatedMenuItem* item)
{
	Menu::addItem(itemName, item);
}

void AnimatedMenu::replaceItem(const char* itemName, AnimatedMenuItem* item)
{
	Menu::addItem(itemName, item);
}

void AnimatedMenu::changeTo(char *destItem)
{
	assert( items.find( destItem ) != items.end() );

	if( destItem && currentItemName && strcmp( destItem, currentItemName ) )
	{
		if( currentItemName )
			AOEFREE( currentItemName );

		currentItemName = AOESTRDUP(destItem);
		keyAnimation->playTo( ((AnimatedMenuItem*)items[currentItemName])->getKeyName() );
	}
}

void AnimatedMenu::setCurrentItem( char *destItem )
{
	assert( items.find( destItem ) != items.end() );

	if( ( destItem && (currentItemName && strcmp( destItem, currentItemName )) ) || !currentItemName )
	{
		if( currentItemName )
			AOEFREE( currentItemName );

		currentItemName = AOESTRDUP(destItem);
		keyAnimation->setPositionByKey( ((AnimatedMenuItem*)items[this->currentItemName])->getKeyName() );
	}
}

void AnimatedMenu::setAnimation( KeyAnimation* newKeyAnimation )
{
	assert(newKeyAnimation != NULL);
	if( keyAnimation )
		delete keyAnimation;
	keyAnimation = newKeyAnimation;
}

void AnimatedMenu::renderGraphics()
{
	keyAnimation->render();
}

void AnimatedMenu::updateObject(dur elapsedTime)
{
	keyAnimation->update( elapsedTime );
}

void AnimatedMenu::loadGraphics()
{
	keyAnimation->loadGraphics();
}

void AnimatedMenu::unloadGraphics()
{
	keyAnimation->unloadGraphics();
}

