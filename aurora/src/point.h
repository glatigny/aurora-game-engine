#ifndef POINT_H
#define POINT_H

class Point;
#include "translatable.h"

/** Point is a group of two coordinates, X and Y.
 * @see Translatable
 */
class Point : public Translatable {
public:
	Point() { x = 0; y = 0; }
	Point(coord newX, coord newY) { x = newX; y = newY; }
	Point(Translatable& t) { x = t.getX(); y = t.getY(); }
	Point(Translatable* t) { x = t->getX(); y = t->getY(); }
	virtual ~Point();

	const Translatable* operator=(Translatable* t) {
		setX(t->getX());
		setY(t->getY());

		return this;
	}

	bool operator==(const Point& p) const {
		return x == p.x && y == p.y;
	}

	bool operator!=(const Point& p) const {
		return x != p.x && y != p.y;
	}

	const Point& operator=(const Point& p) {
		x = p.x;
		y = p.y;

		return *this;
	}

	Point operator+(const Point& p) {
		return Point(x+p.x, y+p.y);
	}

	Point operator-(const Point& p) {
		return Point(x-p.x, y-p.y);
	}

	Point operator-() {
		return Point(-x, -y);
	}
	
	Point operator+(Translatable* p) {
		return Point(x+p->getX(), y+p->getY());
	}

	Point operator-(Translatable* p) {
		return Point(x-p->getX(), y-p->getY());
	}

	const Point& operator+=(const Point& p) {
		x += p.x;
		y += p.y;

		return *this;
	}

	const Point& operator-=(const Point& p) {
		x -= p.x;
		y -= p.y;

		return *this;
	}

	const Point& operator+=(Translatable* p) {
		x += p->getX();
		y += p->getY();

		return *this;
	}

	const Point& operator-=(Translatable* p) {
		x -= p->getX();
		y -= p->getY();

		return *this;
	}
	
	/* Translatable */
	
	virtual bool inside(Translatable& firstCorner, Translatable& secondCorner);
};

bool stringToPoint(const char* string, Point& p);

#endif

