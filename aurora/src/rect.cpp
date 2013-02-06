#include "rect.h"

// QC:?
bool stringToRect(const char* string, Rect& r) {
	coord x;
	coord y;
	coord w;
	coord h;
	
	int count = stringToCoordList(string, &x, &y, &w, &h);

	if(count != 4) {
		return false;
	}

	r.setX(x);
	r.setY(y);
	r.setW(w);
	r.setH(h);

	return true;
}

