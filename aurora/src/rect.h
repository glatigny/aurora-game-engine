#ifndef RECT_H
#define RECT_H

class Rect;

#include "resizable.h"

/** Rect is a group of four coordinates, X, Y, W and H.
 * @see Resizable
 */
class Rect : public Resizable {
public:
	Rect() : Resizable(0, 0, 0, 0) {}
	Rect(const Rect& r) {
		x = r.x;
		y = r.y;
		size = r.size;
	}

	Rect(Translatable* offset) : Resizable(offset) {}
	Rect(Resizable* area) : Resizable(area) {}
	Rect(Translatable& p, Translatable& s) : Resizable(p.getX(), p.getY(), s.getX(), s.getY()) {}
	Rect(int nX, int nY, int nW, int nH) {
		x = nX;
		y = nY;
		size = Point(nW, nH);
	}
	
	~Rect() {}

	const Rect& operator=(const Rect& r) {
		x = r.x;
		y = r.y;
		size = r.size;

		return *this;
	}

	const Rect& operator=(Resizable* r) {
		x = r->getX();
		y = r->getY();
		size = *(r->getSize());

		return *this;
	}

	const Rect& operator=(Resizable& r) {
		setPosition(r);
		size = *(r.getSize());

		return *this;
	}

	const Rect& operator+=(const Rect& r) {
		x += r.x;
		y += r.y;
		size += r.size;

		return *this;
	}

	Rect operator+(const Rect& r) {
		return Rect(x+r.x, y+r.y, size.getX()+r.size.getX(), size.getY()+r.size.getY());
	}

	const Rect& operator*=(const Rect& r) {
		if(r.x > x)
			x = r.x;

		if(r.y > y)
			y = r.y;

		if(r.size.getX() < size.getX())
			size.setX(r.size.getX());

		if(r.size.getY() < size.getY())
			size.setY(r.size.getY());

		return *this;
	}

	Rect operator*(const Rect& r) {
		return Rect(MAX(x, r.x), MAX(y, r.y), MIN(size.getX(), r.size.getX()), MIN(size.getY(), r.size.getY()));
	}

	const Rect& operator/=(const Rect& r) {
		if(r.x < x)
			x = r.x;

		if(r.y < y)
			y = r.y;

		if(r.size.getX() > size.getX())
			size.setX(r.size.getX());

		if(r.size.getY() > size.getY())
			size.setY(r.size.getY());

		return *this;
	}

	Rect operator/(const Rect& r) {
		return Rect(MIN(x, r.x), MIN(y, r.y), MAX(size.getX(), r.size.getX()), MAX(size.getY(), r.size.getY()));
	}
	/* Translatable */

	bool inside(Translatable& firstCorner, Translatable& secondCorner) { return false; } // QC:W
};

bool stringToRect(const char* string, Rect& r);

#endif

