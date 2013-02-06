#ifndef EDIT_LINE_H
#define EDIT_LINE_H

#include "text_line.h"
#include "state.h"

/** An EditLine is a TextLine with input capabilities.
 */
class EditLine : public TextLine {
private:
	int controllerID;
	int cursorPosition;
	int editMode; // For now, a simple bool

protected:
	void handleBackspace(char* textStart);

public:
#define HEADER edit_line
#define HELPER scriptable_object_h
#include "include_helper.h"
	EditLine() {}
	EditLine(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, const char* newText, Font* font, coord maxWidth, int overflowType, dur delay, dur maxTime, const char* transitionInterpolation) : Entity(attachedState), TextLine(attachedState, offset, effects, layer, newText, font, maxWidth, overflowType, delay, maxTime, transitionInterpolation), controllerID(0), cursorPosition(newText?strlen(newText):0), editMode(0) {}
	virtual ~EditLine() {}

	/** Enable text input from a specific input controller.
	 * @param controllerID index of the input controller to use.
	 */
	void enableInput(int controllerID = 0);

	/** Disable text input. The EditLine will behave like an ordinary TextLine until input is enabled again.
	 */
	void disableInput();

	/** Set the cursor position.
	 * @param newPosition the new position of the cursor, in bytes.
	 */
	void setCursorPosition(int newPosition) {
		xerror(text && cursorPosition >= 0 && cursorPosition <= (int)strlen(text), "Invalid cursor position.");
		cursorPosition = newPosition;
	}

	/* Realtime */

	void updateObject(dur elapsedTime);

};

#endif /* EDIT_BOX_H */
