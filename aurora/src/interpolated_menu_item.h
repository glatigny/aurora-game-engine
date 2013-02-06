#ifndef INTERPOLATED_MENU_ITEM_H
#define INTERPOLATED_MENU_ITEM_H

#include "renderable_menu_item.h"
#include "renderable_interpolator.h"

class InterpolatedMenuItem : public RenderableMenuItem 
{
	private:
		InterpolationDescriptor* interpolationDesc;

	public:
#define HEADER animated_menu_item
#define HELPER scriptable_object_h
#include "include_helper.h"
		InterpolatedMenuItem(const char* upName, const char* downName, const char* leftName, const char* rightName, Renderable* render, InterpolationDescriptor* interpolationDesc);
		virtual ~InterpolatedMenuItem();

		/**
		 * @return the key name.
		 */
		InterpolationDescriptor* getInterpolationDesc() { return interpolationDesc; }

		/**
		 * @param name the new key name.
		 */
		void setInterpolationDesc(InterpolationDescriptor* newInterpolationDesc);
};

#endif /* INTERPOLATED_MENU_ITEM_H */
