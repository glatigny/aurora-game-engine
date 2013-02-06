/* This file defines platform-specific memory allocation.
 * It allows to reroute object creation and memory allocator.
 */

namespace Aurora {
	namespace Mem {
		enum Hint {
			NORMAL, // Normal, unmodified allocator.
			FRAME, // Frame-limited allocator. Emptied each frame.
			STATIC, // Static, unfragmented allocator, which ignores delete.
			CHUNK, // Chunk-based allocator, for small allocations.
			PTRPOOL, // Aligned void* pool
			C16, // Aligned 16 bytes chunks
			C64, // Aligned 64 bytes chunks
			PERFILE, // Per-file allocator : one pool per CPP file.
			PERLINE, // Per-line allocator : one pool for each file and each line of code.
		};
	}
}

/*#define AOENEW new(Aurora::Mem::NORMAL, __FILE__, __LINE__)
#define AOENEW2(hint) new(hint, __FILE__, __LINE__)*/
#define AOENEW new
#define AOENEW2(hint) new

#define AOEDEL delete
