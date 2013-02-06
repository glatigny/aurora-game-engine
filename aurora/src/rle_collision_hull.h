
class RLECollisionHull: public Collidable {
private:
	/** This inner class defines one line of a collision mask.
	 * It simplifies implementation of RLECollisionHull : you only have to find the right
	 * lines and test whether they overlap.
	 */
	class RLELine {
	private:
		int zoneCount;		// Number of zones in this line
		unsigned short first*;	// First pixel in the zone (array)
		unsigned short last*;	// Last pixel in the zone (array)
		
		/** Builds a line based on a buffer.
		 * @param buffer the buffer holding uncompressed data.
		 * @param size the size of the buffer.
		 * @param first the table which will receive zone starts. It must be allocated and big enough to hold data.
		 * @param last the table which will receive zone ends. It must be allocated and big enough to hold data.
		 * @param smoothing the number of consecutive pixels that must be white to be considered as a zone separator (reduces memory usage).
		 * @return the number of zones in the line.
		 */
		int buildLine(char* buffer, int size, unsigned short* first, unsigned short* last, int smoothing);

		/** Tells whether this line overlaps a zone.
		 * @param first the first pixel of the zone to test.
		 * @param last the last pixel of the zone to test.
		 * @return true if two zones overlap, false otherwise.
		 */
		bool overlaps(int first, int last);
	public:
		~RLELine();
		RLELine();
		RLELine(const RLELine& l);

		/** Builds a line from one line of an image.
		 * Pixel format can be 1 to many bytes per pixel. One bit per pixel can also be used. Maximum number of bytes per pixel is undefined.
		 * Decoding is done by taking binary 1's as not colliding and any other value as colliding.
		 * This way, alpha channels are treated correctly (fully opaque is 0xFF). Be aware that not fully opaque pixels
		 * will be considered as colliding.
		 * @param buffer the buffer holding one line of image.
		 * @param size the size of the buffer in pixels (WARNING : not in bytes).
		 * @param bpp number of Bytes Per Pixel. If bpp == 0, one bit per pixel is used.
		 */
		RLELine(char* buffer, int size, int bpp);

		/** Returns the position of the leftmost colliding pixel.
		 * @return the position of the leftmost colliding pixel.
		 */
		unsigned short getLeftmostPixel();
		
		/** Returns the position of the rightmost colliding pixel.
		 * @return the position of the rightmost colliding pixel.
		 */
		unsigned short getRightmostPixel();

		/** Tells whether two lines overlap.
		 * TODO : an algorithm using dichotomy for O(log(n)) efficiency !
		 * @param other the line to compare with this one.
		 * @param offset the horizontal offset between the two lines.
		 * @return a point where the two zones overlap
		 */
		int overlaps(RLELine* other, int offset);
	};

	int lines;	// Number of lines
	RLECollisionHull::RLELine** line;	// Pointers to each line
public:
	virtual ~RLECollisionHull();

	/** Builds an empty mask.
	 */
	RLECollisionHull();

	/** Builds a copy of a RLECollisionHull.
	 */
	RLECollisionHull(const RLECollisionHull& m);

	/** Builds a mask from a PNG file.
	 * @param filename the name of the PNG file to be converted to a mask.
	 */
	RLECollisionHull(const char* filename);

	/** Tells whether this mask touches another.
	 * @param other the other mask to test.
	 * @return true if masks touch each other, false otherwise.
	 */
	Collision hitBy(RLECollisionHull* other);
};

