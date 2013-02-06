#ifndef GUI_WIDGET_H
#define GUI_WIDGET_H

namespace GUI {

class Widget : public Resizable, public Renderable {
public:
#define HEADER gui_widget
#define HELPER scriptable_object_h
#include "include_helper.h"
	Widget(State* attachedState, Translatable* offset, Translatable* size, Widget* upWidget, Widget* downWidget, Widget* leftWidget, Widget* rightWidget, Widget* nextWidget, Widget* prevWidget);

	Widget* upWidget;
	Widget* downWidget;
	Widget* leftWidget;
	Widget* rightWidget;
	Widget* nextWidget;
	Widget* prevWidget;

};

}

#endif
