#include "interpolated_menu.h"

#define HEADER interpolated_menu
#define HELPER scriptable_object_cpp
#include "include_helper.h"

InterpolatedMenu::InterpolatedMenu(State* attachedState, Translatable* offset, GraphicalEffects* effects, const char* scriptFileName, const char* defaultItem, InterpolationDescriptor* l_interpolationDesc, int l_duration) : RenderableMenu(attachedState, offset, effects, scriptFileName, defaultItem)
{
	currentRenderable = NULL;
	interpolationDesc = l_interpolationDesc;
	duration = l_duration;
	inInterpolation = false;
}

InterpolatedMenu::~InterpolatedMenu()
{
	if( interpolationDesc )
		delete interpolationDesc;
	if( currentRenderable )
		delete currentRenderable;
}

void InterpolatedMenu::addItem(const char* itemName, RenderableMenuItem* item)
{
	Menu::addItem(itemName, item);
}

void InterpolatedMenu::replaceItem(const char* itemName, RenderableMenuItem* item)
{
	Menu::addItem(itemName, item);
}

void InterpolatedMenu::changeTo(char *destItem)
{
	assert( items.find( destItem ) != items.end() );

	if( !inInterpolation )
	{
		if( destItem && strcmp( destItem, currentItemName ) )
		{
			if( currentItemName )
				AOEFREE( currentItemName );

			currentItemName = AOESTRDUP(destItem);

			if( currentRenderable )
			{
				Renderable *src = currentRenderable;
				Renderable *dest = ((RenderableMenuItem*)items[currentItemName])->getRenderable();

				currentRenderable = new RenderableInterpolator(src, dest, 0, duration, interpolationDesc);
				inInterpolation = true;
			}
			else
			{
				currentRenderable = ((RenderableMenuItem*)items[currentItemName])->getRenderable();
			}
			currentRenderable->setParent(this);
		}
	}
}

void InterpolatedMenu::setCurrentItem( char *destItem )
{
	assert( items.find( destItem ) != items.end() );

	if( destItem && strcmp( destItem, currentItemName ) )
	{
		if( currentItemName )
			AOEFREE( currentItemName );

		currentItemName = AOESTRDUP(destItem);

		currentRenderable = ((RenderableMenuItem*)items[currentItemName])->getRenderable();
		currentRenderable->setParent(this);
		inInterpolation = false;
	}
}

void InterpolatedMenu::renderGraphics()
{
	if( currentRenderable )
	{
		currentRenderable->render();
	}
}

void InterpolatedMenu::update(dur elapsedTime)
{
	if( currentRenderable )
	{
		currentRenderable->update( elapsedTime );
	}

	if( inInterpolation )
	{
		assert( currentRenderable );

		inInterpolation = ((RenderableInterpolator*)currentRenderable)->isFinished();
		if( !inInterpolation )
		{
			currentRenderable = ((RenderableMenuItem*)items[currentItemName])->getRenderable();
			currentRenderable->setParent(this);
		}
	}
}

void InterpolatedMenu::loadGraphics()
{
	for(map<const char*, MenuItem*, strCmp>::iterator item = items.begin(); item != items.end(); item++) 
	{
		assert( dynamic_cast<RenderableMenuItem*>(item->second) );
		((RenderableMenuItem*)item->second)->getRenderable()->loadGraphics();
	}
}

void InterpolatedMenu::unloadGraphics()
{
	for(map<const char*, MenuItem*, strCmp>::iterator item = items.begin(); item != items.end(); item++) 
	{
		assert( dynamic_cast<RenderableMenuItem*>(item->second) );
		((RenderableMenuItem*)item->second)->getRenderable()->unloadGraphics();
	}
}
