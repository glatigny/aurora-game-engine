#ifndef RESIZABLE_H
#define RESIZABLE_H

#include "translatable.h"
#include "point.h"

/** A resizable is an object which has a bounding box.
 * It has a width and a height, which are independant from the actual display size.
 * @see Rect the simplest form of resizable object.
 */
class Resizable : virtual public Translatable { friend class RenderableInterpolator;
protected:
	Point size;

public:
	Resizable() : size(0, 0) {}
	Resizable(Translatable* offset) : Translatable(offset), size(0, 0) {}
	Resizable(Resizable* area) : Translatable(area), size(area->size) {}
	Resizable(coord x, coord y, coord w, coord h) : Translatable(x, y), size(w, h) {}
	Resizable(coord w, coord h) : size(w, h) {
	}
	
	virtual ~Resizable() { }

	/** Get the width of this object.
	 * @return the width of the object.
	 */
	virtual coord getW() const { return size.getX(); }

	/** Get the height of this object.
	 * @return the height of the object.
	 */
	virtual coord getH() const { return size.getY(); }

	/** Get the position of the left border.
	 * @return the X coordinate of the leftmost point.
	 */
	virtual coord getLeftmostX() const { return getX(); }

	/** Get the position of the right border.
	 * @return the X coordinate of the rightmost point.
	 */
	virtual coord getRightmostX() const { return getX() + getW(); }

	/** Get the position of the top border.
	 * @return the Y coordinate of the topmost point.
	 */
	virtual coord getTopmostY() const { return getY(); }

	/** Get the position of the bottom border.
	 * @return the Y coordinate of the bottommost point.
	 */
	virtual coord getBottommostY() const { return getY() + getH(); }

	/** Get the coordinates of the point at the top left-hand corner of the area.
	 * @return the coordinates of the point at the top left-hand corner of this area.
	 */
	virtual Point getTopLeftPoint() const { return Point(getLeftmostX(), getTopmostY()); }

	/** Get the coordinates of the point at the top right-hand corner of the area.
	 * @return the coordinates of the point at the top right-hand corner of this area.
	 */
	virtual Point getTopRightPoint() const { return Point(getRightmostX(), getTopmostY()); }

	/** Get the coordinates of the point at the bottom left-hand corner of the area.
	 * @return the coordinates of the point at the bottom left-hand corner of this area.
	 */
	virtual Point getBottomLeftPoint() const { return Point(getLeftmostX(), getBottommostY()); }

	/** Get the coordinates of the point at the bottom right-hand corner of the area.
	 * @return the coordinates of the point at the bottom right-hand corner of this area.
	 */
	virtual Point getBottomRightPoint() const { return Point(getRightmostX(), getBottommostY()); }

	/** Return the size as a vector.
	 * This function is useful for the script engine.
	 * @return the size of the object as a Translatable.
	 */
	Translatable* getSize() { return &size; } // QC:P

	/** Return the area as a resizable.
	 * This function is useful for the script engine.
	 * @return the area represented as a pointer to a resizable.
	 */
	Resizable* getArea() { return this; } // QC:P
	
	/** Set the size of this object.
	 * WARNING : It must not be assumed that the new size will be
	 * applied as-is. Objects may have constrains and thus change their
	 * size afterwards. Please use getSize or getArea to query the object
	 * for its final size after calling this function.
	 * @param newW the new width of this object.
	 * @param newH the new height of this object.
	 */
	void setSize(coord newW, coord newH) { // QC:P
		setW(newW);
		setH(newH);
	}

	/** Set the position of this object.
	 * WARNING : It must not be assumed that the new size will be
	 * applied as-is. Objects may have constrains and thus change their
	 * size afterwards. Please use getSize or getArea to query the object
	 * for its final size after calling this function.
	 * @param newSize the new size of the object.
	 */
	void setSize(const Translatable& newSize) { // QC:P
		setW(newSize.getX());
		setH(newSize.getY());
	}
	
	/** Set the width of this object.
	 * WARNING : It must not be assumed that the new size will be
	 * applied as-is. Objects may have constrains and thus change their
	 * size afterwards. Please use getSize or getArea to query the object
	 * for its final size after calling this function.
	 * @param newW the new width of this object.
	 */
	virtual void setW(coord newW) { size.setX(newW); } // QC:P

	/** Set the height of this object.
	 * WARNING : It must not be assumed that the new size will be
	 * applied as-is. Objects may have constrains and thus change their
	 * size afterwards. Please use getSize or getArea to query the object
	 * for its final size after calling this function.
	 * @param newH the new height of this object.
	 */
	virtual void setH(coord newH) { size.setY(newH); } // QC:P
	
	virtual void setRightmostX(coord newX) { setW(newX - getX()); }

	virtual void setTopmostY(coord newY) { setH(newY - getY()); }

	/** Set the whole area.
	 * WARNING : It must not be assumed that the new size will be
	 * applied as-is. Objects may have constrains and thus change their
	 * size afterwards. Please use getSize or getArea to query the object
	 * for its final size after calling this function.
	 * @param newArea the object to copy offset and size from.
	 */
	void setArea(Resizable& newArea) { // QC:?
		setPosition(*newArea.getPosition());
		setSize(*newArea.getSize());
	}

	/** Set the whole area.
	 * WARNING : It must not be assumed that the new size will be
	 * applied as-is. Objects may have constrains and thus change their
	 * size afterwards. Please use getSize or getArea to query the object
	 * for its final size after calling this function.
	 * @param newArea the object to copy offset and size from.
	 */
	void setArea(Resizable* newArea) { // QC:?
		assert(newArea);
		setArea(*newArea);
	}
};
#endif /* RESIZABLE_H */

