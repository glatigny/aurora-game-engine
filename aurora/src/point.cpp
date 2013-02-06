#include "point.h"
#include <string>
#include <iostream>

Point::~Point() {
}

bool Point::inside(Translatable& firstCorner, Translatable& secondCorner) {
	int minX = (firstCorner.getX() < secondCorner.getX()) ? firstCorner.getX() : secondCorner.getX();
	int maxX = (firstCorner.getX() > secondCorner.getX()) ? firstCorner.getX() : secondCorner.getX();
	int minY = (firstCorner.getY() < secondCorner.getY()) ? firstCorner.getY() : secondCorner.getY();
	int maxY = (firstCorner.getY() > secondCorner.getY()) ? firstCorner.getY() : secondCorner.getY();

	return (x >= minX)
		&& (x <= maxX)
		&& (y >= minY)
		&& (y <= maxY);

}

// QC:?
bool stringToPoint(const char* string, Point& p) {
	coord x;
	coord y;

	int count = stringToCoordList(string, &x, &y, NULL, NULL);

	if(count != 2) {
		return false;
	}

	p.setX(x);
	p.setY(y);

	return true;
}

