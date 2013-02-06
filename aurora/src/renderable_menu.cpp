#include "renderable_menu.h"

#define HEADER renderable_menu
#define HELPER scriptable_object_cpp
#include "include_helper.h"

RenderableMenu::RenderableMenu(State* attachedState, Translatable* offset, GraphicalEffects* effects, const char* scriptFileName, const char* defaultItem) : Menu(attachedState, scriptFileName, defaultItem), Renderable(attachedState, offset, effects)
{
	currentRenderable = NULL;
}

void RenderableMenu::addItem(const char* itemName, RenderableMenuItem* item)
{
	Menu::addItem(itemName, item);
}

void RenderableMenu::replaceItem(const char* itemName, RenderableMenuItem* item)
{
	Menu::addItem(itemName, item);
}

void RenderableMenu::changeTo(char *destItem)
{
	assert( items.find( destItem ) != items.end() );

	if( destItem && strcmp( destItem, currentItemName ) )
	{
		if( currentItemName )
			AOEFREE( currentItemName );

		currentItemName = AOESTRDUP(destItem);

		if( currentRenderable )
		{
			// FIXME : should delete currentRenderable ?
			//currentRenderable->objUnref(this);
		}
		currentRenderable = ((RenderableMenuItem*)items[currentItemName])->getRenderable();
		currentRenderable->setParent(this);
	}
}

void RenderableMenu::setCurrentItem( char *destItem )
{
	assert( items.find( destItem ) != items.end() );

	if( destItem && strcmp( destItem, currentItemName ) )
	{
		if( currentItemName )
			AOEFREE( currentItemName );

		currentItemName = AOESTRDUP(destItem);

		if( currentRenderable )
		{
			//currentRenderable->objUnref(this);
		}
		currentRenderable = ((RenderableMenuItem*)items[currentItemName])->getRenderable();
		currentRenderable->setParent(this);
	}
}

void RenderableMenu::renderGraphics()
{
	if( currentRenderable )
	{
		currentRenderable->render();
	}
}

void RenderableMenu::update(dur elapsedTime)
{
	if( currentRenderable )
	{
		currentRenderable->update( elapsedTime );
	}
}

void RenderableMenu::loadGraphics()
{
	for(map<const char*, MenuItem*, strCmp>::iterator item = items.begin(); item != items.end(); item++) 
	{
		assert( dynamic_cast<RenderableMenuItem*>(item->second) );
		((RenderableMenuItem*)item->second)->getRenderable()->loadGraphics();
	}
}

void RenderableMenu::unloadGraphics()
{
	for(map<const char*, MenuItem*, strCmp>::iterator item = items.begin(); item != items.end(); item++) 
	{
		assert( dynamic_cast<RenderableMenuItem*>(item->second) );
		((RenderableMenuItem*)item->second)->getRenderable()->unloadGraphics();
	}
}

