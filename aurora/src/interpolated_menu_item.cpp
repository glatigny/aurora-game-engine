#include "interpolated_menu_item.h"

#define HEADER interpolated_menu_item
#define HELPER scriptable_object_cpp
#include "include_helper.h"

InterpolatedMenuItem::InterpolatedMenuItem(const char* upName, const char* downName, const char* leftName, const char* rightName, Renderable* render, InterpolationDescriptor* l_interpolationDesc) : RenderableMenuItem(upName, downName, leftName, rightName, render)
{
	interpolationDesc = l_interpolationDesc;
}

InterpolatedMenuItem::~InterpolatedMenuItem()
{
	if( interpolationDesc )
	{
		delete interpolationDesc;
	}
}

void InterpolatedMenuItem::setInterpolationDesc(InterpolationDescriptor* newInterpolationDesc)
{
	if( interpolationDesc )
	{
		delete interpolationDesc;
	}
	
	interpolationDesc = newInterpolationDesc;
}
