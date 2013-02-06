#include "edit_line.h"

#include "state.h"
#include "font.h"
#include "bitmap_font.h"
#include "aurora_engine.h"

#define HEADER edit_line
#define HELPER scriptable_object_cpp
#include "include_helper.h"

#define BUFSIZE 1024

// QC:?
void EditLine::enableInput(int controllerID) {
	this->controllerID = controllerID;
	engine()->hardware()->enableTextInput(controllerID);
	editMode = 1;
	cursorPosition = strlen(text);
}

// QC:?
void EditLine::disableInput() {
	engine()->hardware()->disableTextInput(controllerID);
	editMode = 0;
}

// QC:A (Heavily suboptimal, but is it a problem ?)
void EditLine::updateObject(dur elapsedTime) {
	if(editMode) {
		char c[BUFSIZE];

		// Poll hardware buffer
		size_t byteCount = engine()->hardware()->readTextInput(c, BUFSIZE, controllerID);
		c[byteCount] = '\0';
		while(*c) {
			if(!text) {
				if(*c == 0x08)
					*c = '\0';
				setText(c);
				cursorPosition += strnlen(c, BUFSIZE);
			} else {
				int newTextLen = strnlen(c, BUFSIZE);
				int textLen = strlen(text);
				xerror(textLen + newTextLen < BUFSIZE - 1, "EditLine too long. Limited to %d bytes.", BUFSIZE);

				char newText[BUFSIZE];

				xassert(cursorPosition >=0 && cursorPosition <= (text?(int)strlen(text):0), "Cursor outside string : position = %d, string length = %d.", cursorPosition, (text?strlen(text):0));

				// Assemble text
				if(text)
					memcpy(newText, text, cursorPosition);
				memcpy(&newText[cursorPosition], c, newTextLen);
				if(text && textLen < cursorPosition)
					memcpy(&newText[cursorPosition + newTextLen], text + cursorPosition, textLen - cursorPosition);
				cursorPosition += newTextLen;

				// Properly end string
				newText[textLen + newTextLen] = '\0';

				// Set new text
				handleBackspace(newText);
				setText(newText);
			}
			byteCount = engine()->hardware()->readTextInput(c, BUFSIZE, controllerID);
			c[byteCount] = '\0';
		}
	}
	TextLine::updateObject(elapsedTime);
}

// QC:? (very suboptimal. totally unsafe.)
// FIXME : handle cursor position correctly
void EditLine::handleBackspace(char* textStart) {
	char* t = textStart;
	while(*t) {
		if(*t == 0x08) {
			char* src = t+1; // Start after the backspace code
			char* dst;
			if(t != textStart) {
				// Find character before t
				dst = t-1;

				// Seek previous character in the UTF-8 string
				while(dst > textStart && (*dst & 0xC0) == 0x80)
					--dst;
			} else {
				dst = textStart;
			}
			// Overwrite backspace and its previous code with the rest of the string.
			strcpy(dst, src);
			t = dst;
			cursorPosition = strlen(textStart); // FIXME : UGLY, SLOW AND WRONG
			if(!*t)
				return;
		}

		t++;
	}
}
