#include "menu_item.h"
#include "state.h"

#define HEADER menu_item
#define HELPER scriptable_object_cpp
#include "include_helper.h"

MenuItem::MenuItem(State* attachedState, const char* l_upName, const char* l_downName, const char* l_leftName, const char* l_rightName) : Entity(attachedState)
{
	if( l_upName )
		upName = AOESTRDUP(l_upName);
	else
		upName = NULL;

	if( l_downName )
		downName = AOESTRDUP(l_downName);
	else
		downName = NULL;

	if( l_leftName )
		leftName = AOESTRDUP(l_leftName);
	else
		leftName = NULL;

	if( l_rightName )
		rightName = AOESTRDUP(l_rightName);
	else
		rightName = NULL;
}

MenuItem::~MenuItem()
{
	if( upName )
	{
		AOEFREE(upName);
		upName = NULL;
	}

	if( downName )
	{
		AOEFREE(downName);
		downName = NULL;
	}

	if( leftName )
	{
		AOEFREE(leftName);
		leftName = NULL;
	}

	if( rightName )
	{
		AOEFREE(rightName);
		rightName = NULL;
	}
}

const char* MenuItem::getUpName()
{
	return upName;
}

const char* MenuItem::getDownName()
{
	return downName;
}

const char* MenuItem::getLeftName()
{
	return leftName;
}

const char* MenuItem::getRightName()
{
	return rightName;
}

const char* MenuItem::getName(ControllerState::Direction dir)
{
	switch( dir )
	{
		case ControllerState::UP:
			return getUpName();
		case ControllerState::DOWN:
			return getDownName();
		case ControllerState::LEFT:
			return getLeftName();
		case ControllerState::RIGHT:
			return getRightName();
		default:
			return NULL;
	}
}

void MenuItem::setUpName(const char* name)
{
	if( upName )
		AOEFREE(upName);

	if( name )
		upName = AOESTRDUP(name);
	else
		upName = NULL;
}

void MenuItem::setDownName(const char* name)
{
	if( downName )
		AOEFREE(downName);

	if( name )
		downName = AOESTRDUP(name);
	else
		downName = NULL;
}

void MenuItem::setLeftName(const char* name)
{
	if( leftName )
		AOEFREE(leftName);

	if( name )
		leftName = AOESTRDUP(name);
	else
		leftName = NULL;
}

void MenuItem::setRightName(const char* name)
{
	if( rightName )
		AOEFREE(rightName);

	if( name )
		rightName = AOESTRDUP(name);
	else
		rightName = NULL;
}

