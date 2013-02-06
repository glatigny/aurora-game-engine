#ifndef TRANSLATABLE_H
#define TRANSLATABLE_H

#include "interface.h"
class RenderableInterpolator;



/** A translatable is an object which can be positioned in space.
 * It has X and Y coordinates.
 * @see Point the simplest form of translatable object.
 */
class Translatable : public Interface { friend class RenderableInterpolator;
protected:
	coord x;
	coord y;
public:
	Translatable() { // QC:P
		x = 0;
		y = 0;
	}

	Translatable(coord newX, coord newY) { // QC:P
		x = newX;
		y = newY;
	}
	
	Translatable(Translatable* offset) { // QC:P
		if(offset != NULL) {
			x = offset->getX();
			y = offset->getY();
		} else {
			x = 0;
			y = 0;
		}
	}
	
	virtual ~Translatable() { }

	const Translatable* operator=(Translatable* t) {
		setX(t->getX());
		setY(t->getY());
		return this;
	}

	bool operator==(Translatable* other) {
		return getX() == other->getX() && getY() == other->getY();
	}

	/** Get the X coordinate of this object.
	 * @return the X coordinate of this object.
	 */
	virtual coord getX() const { return x; } // QC:P

	/** Get the Y coordinate of this object.
	 * @return the Y coordinate of this object.
	 */
	virtual coord getY() const { return y; } // QC:P

	/** Return the translatable itself.
	 * This function is useful for the script engine.
	 * @return the translatable as a Translatable.
	 */
	Translatable* getPosition() { return this; } // QC:P
	
	/** Set the position of this object.
	 * @param newX the new x coordinate of this object.
	 * @param newY the new y coordinate of this object.
	 */
	void setPosition(coord newX, coord newY) { // QC:P
		setX(newX);
		setY(newY);
	}

	/** Set the position of this object.
	 * @param position the new position of this object.
	 */
	void setPosition(const Translatable& position) { // QC:P
		setPosition(position.getX(), position.getY());
	}

	void setPosition(Translatable* position) { setPosition(*position); }
	
	/** Set the X coordinate of this object.
	 * @param x the X coordinate of this object.
	 */
	virtual void setX(coord newX) { x = newX; } // QC:P

	/** Set the Y coordinate of this object.
	 * @param y the Y coordinate of this object.
	 */
	virtual void setY(coord newY) { y = newY; } // QC:P

	/** Translate the position of this object.
	 * @param delta offset to add to this object.
	 */
	void translate(const Translatable& delta) { // QC:P
		translate(delta.getX(), delta.getY());
	}

	/** Translate the position of this object.
	 * @param delta offset to add to this object.
	 */
	void translate(const Translatable* delta) {
		translate(delta->getX(), delta->getY());
	}

	/** Translate the position of this object.
	 * @param deltaX offset to aff to the X coordinate of this object.
	 * @param deltaY offset to aff to the Y coordinate of this object.
	 */
	void translate(coord deltaX, coord deltaY) { // QC:P
		setX(x + deltaX);
		setY(y + deltaY);
	}

	/** Translate the position of this object.
	 * @param delta offset to add to this object.
	 */
	void invertTranslate(const Translatable& delta) { // QC:P
		invertTranslate(delta.getX(), delta.getY());
	}

	/** Translate the position of this object.
	 * @param delta offset to add to this object.
	 */
	void invertTranslate(const Translatable* delta) { // QC:P
		invertTranslate(delta->getX(), delta->getY());
	}

	/** Translate the position of this object.
	 * @param deltaX offset to aff to the X coordinate of this object.
	 * @param deltaY offset to aff to the Y coordinate of this object.
	 */
	void invertTranslate(coord deltaX, coord deltaY) { // QC:P
		setX(x - deltaX);
		setY(y - deltaY);
	}

	/** Get the absolute position of this element.
	 * @return the absolute position of this object.
	 */
	Translatable* getAbsolutePosition() {
		return this;
	}

	/** Tests whether this object is inside a rectangle.
	 * @param firstCorner the first corner of the rectangle.
	 * @param secondCorner the second corner of the rectangle.
	 * @return true if the object is inside the rectangle (or on a bound), false otherwise.
	 */
	virtual bool inside(Translatable& firstCorner, Translatable& secondCorner) = 0;
};
#endif

