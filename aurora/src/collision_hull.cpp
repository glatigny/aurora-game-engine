#include <iostream>
#include <png.h>

#include "collision_hull.h"
#include "collision_rect.h"
#include "collision_point.h"

#define HEADER collision_hull
#define HELPER scriptable_object_cpp
#include "include_helper.h"


/*CollisionHull::CollisionHull(State* attachedState) : Collidable(attachedState)
{
	m_map = NULL;
	contour_map = NULL;
	m_contour = false;
}*/

CollisionHull::CollisionHull(State* attachedState, Translatable* offset, int layer, int collisionLayers, VFS::File* filename, bool contour) : Entity(attachedState), Translatable(offset), Collidable(attachedState, offset, NULL, layer, collisionLayers)
{
	m_map = NULL;
	contour_map = NULL;
	m_contour = contour;
	if( filename != NULL ) {
		m_filename = AOESTRDUP(filename->name());
		load(m_filename, m_contour);
	} else {
		m_filename = NULL;
	}
}

CollisionHull::~CollisionHull()
{
	if( m_filename )
	{
		AOEFREE( m_filename );
		m_filename = NULL;
	}
	clean();
}

Collision CollisionHull::hitBy(CollisionHull* other) {
	if(!(getCollisionLayers() & other->getCollisionLayers()))
		return Collision(this, NULL);

	Collision ret(this, NULL);

	if( (contour_map == NULL ) && (m_map == NULL ))
		return ret;

	if( (other->contour_map == NULL) && (other->m_map == NULL))
		return ret;

	bool hors = false;

	// My position
	int myX = getX();//-getSize().getX()/2;
	int myY = getY();//-getSize().getY()/2;
	int myW = getSize().getX();
	int myH = getSize().getY();

	// Other's position
	int oX = other->getX();//-other->getSize().getX()/2;
	int oY = other->getY();//-other->getSize().getY()/2;
	int oW = other->getSize().getX();
	int oH = other->getSize().getY();
	
	/*-- Test RECTANGLE --*/
	if( myX > oX + oW )
		hors = true;
	if( oX > myX + myW )
		hors = true;

	if( myY > oY + oH )
		hors = true;
	if( oY > myY + myH )
		hors = true;
	
	if( hors == false ) {
		// Quatres variables pour nos tests
		int lx = MAX(myX, oX);
		int ly = MAX(myY, oY);
		int lw = MIN(myX + myW, oX + oW);
		int lh = MIN(myY + myH, oY + oH);

		/*-- Test APPROFONDI --*/

		// Test bete de mechant quand nous avons deux zones pleines
		if( (m_contour == false) && (other->m_contour == false)) 
		{

			for(int i = lx; i < lw; i++) 
			{
				for(int j = ly; j < lh; j++) 
				{
					if( (map(j - myY, i - myX)) && (other->map(j - oY, i - oX)) ) 
					{
						ret.setPosition(i, j);
						ret.setCollidedObject(other);
						ret.setCollidingHull(this);
						ret.setCollidedHull(other);
						return ret;
					}
				}
			}
		} 
		else if ( (m_contour == true) && (other->m_contour == true)) 
		{

			for(int i = lx; i < lw; i++) 
			{
				// Si le minimum de l'un est superieur au max de l'autre, il n'y a pas collision
				// Si on inverse cela..
				if( !((	
						(contour_map[i - myX][0] + myY)
						> 
						(other->contour_map[i - oX][1] + oY)
					)  || (
						(other->contour_map[i - oX][0] + oY)
						> 
						(contour_map[i - myX][1] + myY)
					))) 
				{
					ret.setPosition(i, contour_map[i - myX][0] + myY);
					ret.setCollidedObject(other);
					ret.setCollidingHull(this);
					ret.setCollidedHull(other);
					return ret;
				}
			}

		} 
		else 
		{
			if( m_contour == true ) 
			{
				for(int i = lx; i < lw; i++) 
				{
					int l_min = MAX( contour_map[i - myX][0] + myY  ,  ly );
					int l_max = MIN( contour_map[i - myX][1] + myY  ,  lh );
					for(int j = l_min; j < l_max; j++) 
					{
						if( other->map(j - oY, i - oX) ) 
						{
							ret.setPosition(i, j);
							ret.setCollidedObject(other);
							ret.setCollidingHull(this);
							ret.setCollidedHull(other);
							return ret;
						}
					}
				}
			} 
			else 
			{
				for(int i = lx; i < lw; i++) 
				{
					int l_min = MAX( other->contour_map[i - oX][0] + oY  , ly );
					int l_max = MIN( other->contour_map[i - oX][1] + oY  , lh );
					for(int j = l_min; j < l_max; j++) 
					{
						if( map(j - myY, i - myX) ) 
						{
							ret.setPosition(i, j);
							ret.setCollidedObject(other);
							ret.setCollidingHull(this);
							ret.setCollidedHull(other);
							return ret;
						}
					}
				}
			}
		}
	}

	return ret;
}

Collision CollisionHull::hitBy(CollisionPoint* otherHull) {
	if(!(getCollisionLayers() & otherHull->getCollisionLayers()))
		return Collision(this, NULL);
		
	Collision ret(this, NULL);
	return ret;
}

Collision CollisionHull::hitBy(CollisionRect* otherHull) {
	if(!(getCollisionLayers() & otherHull->getCollisionLayers()))
		return Collision(this, NULL);
		
	Collision ret(this, NULL);
	return ret;
}

Collision CollisionHull::hitBy(Collidable* other) {
	if(!(getCollisionLayers() & other->getCollisionLayers()))
		return Collision(this, NULL);
		
	switch( other->getCollisionType() ) {
		case COLLISION_TYPE_HULL:
			return hitBy((CollisionHull*)other);
		case COLLISION_TYPE_POINT:
			return hitBy((CollisionPoint*)other);
		case COLLISION_TYPE_RECT:
			return hitBy((CollisionRect*)other);
	}

	// The other one is not a known Collision.
	// We need to test it against a CollisionHull.
	// This call gives us a reversed Collision, so let's swap it.
	Collision reversedCol = other->hitBy(this);
	reversedCol.swap();
	return reversedCol;
}

Point CollisionHull::getSize() {
	return m_size;
}

void CollisionHull::clean()
{
	if( m_map != NULL )
	{
		for(int i = 0; i < m_size.getY(); i++)
			AOEFREE(m_map[i]);
		AOEFREE(contour_map);
	}
	m_map = NULL;

	if( contour_map != NULL ) {
		for(int i = 0; i < m_size.getX(); i++) {
			delete[] contour_map[i];
		}
		delete[] contour_map;
	}
	contour_map = NULL;
}

// QC:A
void* CollisionHull::loadPng(const char* p_file)
{
	char header[8];	// 8 is the maximum size that can be checked
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep * row_pointers;

	/* open file and test for it being a png */
	FILE *fp = fopen(p_file, "rb");
	if (!fp)
		return NULL;

	if(!fread(header, 1, 8, fp))
		return NULL;

	if (png_sig_cmp((png_bytep)header, 0, 8))
		return NULL;

	/* initialize stuff */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL); // FIXME : Memory leak here
	
	if (!png_ptr)
		return NULL;

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		return NULL;

	if (setjmp(png_jmpbuf(png_ptr)))
		return NULL;

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	m_size.setX(info_ptr->width);
	m_size.setY(info_ptr->height);
	wp = info_ptr->rowbytes;

	if( (info_ptr->bit_depth != 1) || (info_ptr->color_type != PNG_COLOR_TYPE_PALETTE) )
		return NULL;

	png_read_update_info(png_ptr, info_ptr);

	/* read file */
	if (setjmp(png_jmpbuf(png_ptr)))
		return NULL;

	row_pointers = (png_bytep*) AOEMALLOC(sizeof(png_bytep) * m_size.getY());
	for(int i = 0; i < m_size.getY(); i++)
		row_pointers[i] = (png_byte*) AOEMALLOC(info_ptr->rowbytes);

	png_read_image(png_ptr, row_pointers);
	png_read_end(png_ptr, info_ptr);
	fclose(fp);

	return row_pointers;
}

void CollisionHull::load(const char* p_file, bool p_contour) 
{
	// Chargement de la texture
	int i, j;
	
	clean();

	m_map = (unsigned char**)loadPng(p_file);
	if( m_map == NULL ) {
		syslog("Cannot load %s", p_file);
		return;
	}

	p1.setX( 0 );
	p1.setY( 0 );
	p2.setX( 0 );
	p2.setY( 0 );

	// Affectation du type
	m_contour = p_contour;

#if 0
			// les objets sans contour n'ont pas de perspective
	if( !m_contour ) 
	{
		
		// Parcours en hauteur
		for(i = 0; i < m_size.getY(); i++) 
		{
			png_byte* row = (png_byte*)m_map[i];
			// Parcours en largeur
			for(j = 0; j < m_size.getX(); j++) 
			{
				// Test sur le pixel
				for(int a = 0; (a < 8) && (a < (m_size.getX()-j*8)); a++)
				{
					if((row[j]&(1<<(7-a))) == 0)
					{
						int p = (j*8+a);

						// p1 et p2
						if( p <= p1.getX() ) 
						{
							p1.setY(i);
							p1.setX(p);
						}
						if( p > p2.getX() ) 
						{
							p2.setY(i);
							p2.setX(p);
						}
					}
				}
			}
		}

	} 
	else 
#endif
	if(m_contour) {
#ifdef FLOAT_COORD
//#warning FIXME : Collision hulls do not work correctly in float mode and will leak memory.
#endif
		int min, max;
		int pos, mask;
		// Init du tableau
		contour_map = new unsigned short* [(int)m_size.getX()];

		p1.setX( m_size.getX() );
		
		// Creation du tableau
		for(j = 0; j < m_size.getX(); j++) 
		{
			contour_map[j] = new unsigned short[2];
			pos = (int)j/8;
			mask = 1 << (7-(j%8));
			min = -1;
			max = -1;
			// On recupere le point min et le point max
			for(i = 0; i < (int)m_size.getY(); i++) 
			{
				if( (m_map[i][pos] & mask )== 0 )
				{
					if( min == -1 )
						min = i;
					if( i > max )
						max = i;

					// p1 et p2
					if( j < p1.getX() ) 
					{
						p1.setY(i);
						p1.setX(j);
					}
					if( j > p2.getX() ) 
					{
						p2.setY(i);
						p2.setX(j);
					}
				}
			}
			// On affecte
			contour_map[j][0] = min;
			contour_map[j][1] = max;
		}

		// On a le contour on vide la m_map
		for(int i = 0; i < (int)m_size.getY(); i++)
			AOEFREE(m_map[i]);
		AOEFREE(m_map);
		m_map = NULL;

		p1.translate(-m_size.getX()/2, -m_size.getY()/2);
		p2.translate(-m_size.getX()/2, -m_size.getY()/2);
	}
}

bool CollisionHull::map(int p_y, int p_x)
{
	return !(m_map[p_y][(int)p_x/8] & (1 << (7-(p_x%8))));
}

void CollisionHull::loadGraphics() {
	if( m_filename != NULL )
		load(m_filename, m_contour);
}

void CollisionHull::unloadGraphics() {
	clean();
}
