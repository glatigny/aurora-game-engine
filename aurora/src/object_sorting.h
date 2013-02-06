#ifndef OBJECT_SORTING_H
#define OBJECT_SORTING_H

#include "renderable.h"

/** Functor that sorts objects from furthest to nearest for rendering.
 * Objects belong to the same coordinates space, so you can rely on getX() and getY().
 */
struct ObjectSorting {
	// QC:P
	bool operator()(Renderable* left, Renderable* right) {
		// Normal display, without perspective
		int leftLayer = left->getLayer();
		int rightLayer = right->getLayer();
		if(leftLayer != rightLayer) {
			return leftLayer < rightLayer;
		}
		else {
			// Stabilize sort
			coord leftY = left->getY();
			coord rightY = right->getY();
			if(leftY == rightY)
				return left < right;
			return leftY < rightY;
		}
	}

	/* FIXME : Disabled perspective for performance improvements
	// Left has perspective points.
	// QC:S (ne gere pas a 100% les cas avec une perspective sur les 2 objets, l'algo est peut-etre optimisable)
	bool operator()(Collidable* left, Collidable* right) {
		if(left == right) {
			return false;
		}

		int leftLayer = left->getLayer();
		int rightLayer = right->getLayer();
		if(leftLayer != rightLayer) {
			return leftLayer < rightLayer;
		}

		if(right->hasPerspective() && left > right) {
			// Stabilize sort
			return !operator()(right, left);
		}
		
		Point ll;
		Point lr;
		Point rl;
		Point rr;

		if(left->hasPerspective()) {		
			ll.setPosition(*left->getLeftmostPoint());
			lr.setPosition(*left->getRightmostPoint());
		} else {
			ll.setPosition(left->getX() - 1, left->getY());
			lr.setPosition(left->getX() + 1, left->getY());
		}
		
		if(right->hasPerspective()) {
			rl.setPosition(*right->getLeftmostPoint());
			rr.setPosition(*right->getRightmostPoint());
		} else {
			rl.setPosition(right->getX() - 1, right->getY());
			rr.setPosition(right->getX() + 1, right->getY());
		}
				
		
		if(lr.getX() < rl.getX()) {
			// Too far from the left
			return lr.getY() < rl.getY();
		}
		if(rr.getX() < ll.getX()) {
			// Too far from the right
			return rr.getY() > ll.getY();
		}
		if(MAX(ll.getY(), lr.getY()) < MIN(rl.getY(), rr.getY())) {
			// Vertically out of the zone
			return true;
		}
		if(MIN(ll.getY(), lr.getY()) > MAX(rl.getY(), rr.getY())) {
			// Vertically out of the zone
			return false;
		}

		// Slopes
		coord lsn = ll.getY() - lr.getY(); // TODO : optimization possible when null numerator
		coord lsd = ll.getX() - lr.getX();
		coord rsn = rl.getY() - rr.getY();
		coord rsd = rl.getX() - rr.getX();
		
		// Find the common segment on the horizontal axis
		if(ll.getX() < rl.getX()) {
			ll.setPosition( rl.getX() ,
					ll.getY()+lsn*(rl.getX()-ll.getX())/lsd );
		} else {
			rl.setPosition( ll.getX() ,
					rl.getY()+rsn*(ll.getX()-rl.getX())/rsd );
		}

		if(lr.getX() > rr.getX()) {
			lr.setPosition( rr.getX() ,
					lr.getY()+lsn*(rr.getX()-lr.getX())/lsd );
		} else {
			rr.setPosition( lr.getX() ,
					rr.getY()+rsn*(lr.getX()-rr.getX())/rsd );
		}

		return ll.getY() < rl.getY() || lr.getY() < rr.getY();
	}*/
};

#endif

