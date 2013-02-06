/*-------------------------------------------------
 AURORA GAME STUDIO - AGS
 --------------------------
 Saint Seiya RPG - SSRPG
 www.saintseiyarpg.com
 --------------------------
 Authors:
 DarkSage - Jerome
 Chaperon - Jean-Matthieu
 --------------------------
 FILE : utils.cpp
 DATE : 25/07/2006
 AUTHOR : DarkSage
 --------------------------
 DESCRIPTION:

 --------------------------
 -------------------------------------------------*/

/*--- INCLUDES ---*/
#include <string.h>
#include <assert.h>
#include <cstdarg>

/*--- DEFINES ---*/

/*--- VARIABLES ---*/
AlwaysBlocking alwaysBlocking;
NeverBlocking neverBlocking;

// Default fixed-size buffer size for packed structures

const unsigned int packedStructureBufferSize = 256;

/*--- CODE ---*/

/*------------------------------------------------------------------------------*/
int nextPowerofTwo(int x) {
	int p = 1;

	if(x <= 0)
		return 0;

	while(p < x)
		p <<= 1;

	return p;
}
/*------------------------------------------------------------------------------*/
coord modc(coord c, coord m)
{
#ifdef FLOAT_COORD
	double intpart;
	return c*modf(c/m, &intpart);
#else
	return c % m;
#endif
}
/*------------------------------------------------------------------------------*/
dur modd(dur c, dur m)
{
#ifdef FLOAT_DURATION
	double intpart;
	return c*modf(c/m, &intpart);
#else
	return c % m;
#endif
}
/*------------------------------------------------------------------------------*/

// QC:S (should simplify "..", will be overridden by the VFS)
const char* relPath(const char* baseFile, const char* relFile) {
	if(relFile == NULL) {
		return NULL;
	}
	if(baseFile == NULL) {
		return relFile;
	}

	static char filename[4096];
	strncpy(filename, baseFile, 4096);
	filename[4095] = '\0';

	// Append the new file name to the base path
	char* dirSep = strrchr(filename, '/');
	if(dirSep) {
		strncpy(dirSep + 1, relFile, 4096 - (dirSep - ((char*) (&filename)) + 1));
	} else {
		strncpy(filename, relFile, 4096);
	}
	filename[4095] = '\0';

	//suppressDotPaths(filename); /*FIXME : Test this function before enabling it. */

	return (const char*) filename;
}

// QC:X
void suppressDotPaths(char* filename) {
	if(strcmp(filename, "../")) {
		syslog("Malformed path : %s", filename);
		filename[0] = '\0';
	}

	char* dirSep;
	while( (dirSep = strstr(filename, "/../")) ) {
		if(dirSep <= filename) {
			syslog("Malformed path : %s", filename);
			filename[0] = '\0';
			return;
		} else {
			char* nextDirSep = dirSep - 1;
			while(nextDirSep > filename && *nextDirSep != '/')
				nextDirSep--;
			if(*nextDirSep == '/') {
				nextDirSep++;
			}

			strcpy(nextDirSep, dirSep + 4);
		}
	}
}

// QC:F (modifies format to point on the last digit)
unsigned long parseBufferSize(const char* &format) {
	unsigned long size = 0;

	format++;
	while(*format >= '0' && *format <= '9') {
		size *= 10;
		size += *format - '0';
		format++;
	}
	format--;

	return size;
}

/** Returns a buffer packed with given values.
 *
 * Format string :
 * The format string is made of characters. Each character has a special meaning :
 *
 *  i : signed 32 bits integer.
 *  I : unsigned 32 bits integer.
 *  c : signed 8 bits integer.
 *  C : unsigned 8 bits integer.
 *  b<number> : Buffer of <number> bytes. The parameter has to be passed as a char*.
 *  b : Without number. Variable size buffer. 2 parameters must be passed : the size followed by the buffer.
 *  s : zero-terminated string. The parameter has to be passed as a char*.
 *  S : sub-structure. 2 parameters must be passed : the format followed by the buffer.
 *
 * If the format string starts with !, the buffer will be in fixed size mode.
 * In this mode, variable size buffers and strings will be treated as a fixed buffer ; the size of this buffer is set in aurora_consts.h
 * If the format string is not prefixed by !, the buffer will be in variable size mode.
 * In this mode, the function is slower because it will realloc() the buffer for each string.
 *
 * Example :
 *
 *   int a;
 *   unsigned int b;
 *   char c;
 *   unsigned char d;
 *   char e[40];
 *   int f;
 *   char g[] = "Hello World";
 *   char h[] = "EXAMPLE";
 *
 *   char* s = packStructure("iIcCb40isb", a, b, c, d, e, f, g, 8, h);
 *
 *   int h;
 *   char* t = packStructure("iS", h, "iIcCb40isb", s);
 *
 * WARNING : The returned structure is allocated by AOEMALLOC(). Remember to AOEFREE() it after use.
 *
 * @param format the format of the structure.
 * @param ... variables to store into the structure.
 */
// QC:G
char* packStructure(const char* format, ...) {
	assert(format && *format);
	unsigned long bufferLen = computePackedStructureLength(format);
	char* buffer = (char*) AOEMALLOC(bufferLen);
	char* currentValue = buffer;
	bool isSized = false;

	if(*format != '!') {
		currentValue += 4;
		isSized = true;

		syslog("packStructure : emitting sized structure.");
		syslog("%c", *format);
	} else {
		format++;
	}

	va_list pv;
	va_start(pv, format);

	do {
		switch(*format){
		case 'i': {
			*((int32_t*) (currentValue)) = HTON_INT32(va_arg(pv, int));
			currentValue += 4;
		}
			break;

		case 'I': {
			*((uint32_t*) (currentValue)) = HTON_UINT32(va_arg(pv, unsigned int));
			currentValue += 4;
		}
			break;

		case 'c': {
			*((int8_t*) (currentValue)) = HTON_INT8((char) va_arg(pv, int));
			currentValue += 1;
		}
			break;

		case 'C': {
			*((uint8_t*) (currentValue)) = HTON_UINT8((unsigned char) va_arg(pv, unsigned int));
			currentValue += 1;
		}
			break;

		case 'b': {
			unsigned int size;
			unsigned int bufsize;
			if(format[1] < '0' && format[1] > '9') {
				size = (unsigned int) va_arg(pv, unsigned int);
				if(isSized) {
					bufsize = size;

					unsigned int sizePrefix = (format[1]) ? 4 : 0;

					// Reallocate larger buffer
					bufferLen += size + sizePrefix;
					unsigned long currentPosition = currentValue - buffer;
					buffer = (char*) realloc(buffer, bufferLen);
					currentValue = buffer + currentPosition;

					if(sizePrefix) {
						*((uint32_t*) (currentValue)) = HTON_UINT32(size);
						currentValue += 4;
					}
				} else {
					bufsize = packedStructureBufferSize;
				}
			} else {
				bufsize = size = parseBufferSize(format);
			}

			xassert(bufsize > size, "Payload is too big to fit in the buffer. Payload is %d and buffer is %d bytes long.", size, bufsize);

			memcpy(currentValue, (char*) va_arg(pv, char*), size);
			if(size < bufsize) {
				// Zero out remaining part of the buffer to avoid propagating uninitialized memory.
				bzero(currentValue + size, bufsize - size);
			}
			currentValue += bufsize;
		}
			break;

		case 's': {
			char* srcString = (char*) va_arg(pv, char*);
			assert(srcString);

			if(isSized) {
				unsigned long stringLen = strlen(srcString) + 1;
				bufferLen += stringLen;
				unsigned long currentPosition = currentValue - buffer;
				buffer = (char*) realloc(buffer, bufferLen);
				currentValue = buffer + currentPosition;
				memcpy(currentValue, srcString, stringLen);
				currentValue += stringLen;
			} else {
				strncpy(currentValue, srcString, packedStructureBufferSize);
				currentValue += packedStructureBufferSize;
			}
		}
			break;

		case 'S': {
			const char* subFormat = (const char*) va_arg(pv, const char*);
			const char* subBuffer = (const char*) va_arg(pv, const char*);
			assert(subFormat);
			assert(subBuffer);

			bool subSized = false;
			if(!subFormat || *subFormat != '!') {
				subSized = true;
			}

			xerror(isSized, "You cannot put a sub structure inside an unsized structure.");

			unsigned long size = getPackedStructureLength(subFormat, subBuffer);
			if(format[1] == '\0' && subSized) {
				// The sub structure is the last member.
				// Storing size in the sub structure is not needed because we can deduce it easily while unpacking.
				size -= 4;
				subBuffer += 4;
			}

			bufferLen += size;
			unsigned long currentPosition = currentValue - buffer;
			buffer = (char*) realloc(buffer, bufferLen);
			currentValue = buffer + currentPosition;
			memcpy(currentValue, subBuffer, size);
			currentValue += size;
		}
		}

	} while(*(++format));

	va_end(pv);

	if(isSized) {
		*((uint32_t*) (buffer)) = HTON_UINT32(currentValue - buffer);
	}

	return buffer;
}

/** Sets values based on the content of the buffer.
 * @param buffer the buffer with stored values.
 * @param format the format of the buffer.
 * @param ... pointers to the variables to be modified. Buffers must be pre-allocated and passed as char*.
 *        Strings are passed as pointers to char* and must be AOEFREE()d by the caller.
 *
 *        The format of sub-structures is not passed as a char** but as a const char*.
 */
// QC:G
void unpackStructure(const char* format, const char* buffer, ...) {
	xerror(buffer, "NULL packed structure buffer.");
	const char* currentValue = buffer;
	unsigned long bufferLen;
	bool isSized;

	va_list pv;
	va_start(pv, buffer);

	if(*format != '!') {
		bufferLen = NTOH_UINT32(*((uint32_t*) (currentValue)));
		currentValue += 4;
		isSized = true;
	} else {
		format++;
		(void)bufferLen;
	}

	do {
		switch(*format){
		case 'i': {
			*((int*) (va_arg(pv, int*))) = NTOH_INT32(*((int32_t*) (currentValue)));
			currentValue += 4;
		}
			break;

		case 'I': {
			*((unsigned int*) (va_arg(pv, unsigned int*))) = NTOH_UINT32(*((uint32_t*) (currentValue)));
			currentValue += 4;
		}
			break;

		case 'c': {
			*((char*) (va_arg(pv, char*))) = NTOH_INT8(*((int8_t*) (currentValue)));
			currentValue += 4;
		}
			break;

		case 'C': {
			*((unsigned char*) (va_arg(pv, unsigned char*))) = NTOH_UINT8(*((uint8_t*) (currentValue)));
			currentValue += 1;
		}
			break;

		case 'b': {
			unsigned int size;
			if(format[1] < '0' && format[1] > '9') {
				if(isSized) {
					unsigned int sizePrefix = (format[1]) ? 4 : 0;
		
					if(sizePrefix) {
						size = NTOH_UINT32(*((uint32_t*) (currentValue)));
						currentValue += 4;
					} else {
						size = bufferLen - (currentValue - buffer); // Deduce the size of the sub structure
					}
					*((unsigned int*) (va_arg(pv, unsigned int*))) = size;
				} else {
					size = packedStructureBufferSize;
				}
			} else {
				size = parseBufferSize(format);
			}

			memcpy((char*) va_arg(pv, char*), currentValue, size);
			currentValue += size;
		}
			break;

		case 's': {
			char** dstBuf = (char**) va_arg(pv, char**);
			assert(dstBuf);

			unsigned long size;
			if(isSized) {
				// Variable-length sizes are only valid with sized structures.
				size = strnlen(currentValue, bufferLen - (currentValue - buffer)) + 1;
			} else {
				size = packedStructureBufferSize;
			}
			*dstBuf = (char*) AOEMALLOC(size);
			memcpy(*dstBuf, currentValue, size);
			(*dstBuf)[size - 1] = '\0'; // Ensure that the string is terminated properly.
			currentValue += size;
		}
			break;

		case 'S': {
			char** dstStruct = (char**) va_arg(pv, char**);
			const char* subFormat = (const char*) va_arg(pv, const char*);

			assert(dstStruct);
			assert(subFormat);
			assert(isSized);

			unsigned long size;
			if(format[1] == '\0' && *subFormat == '!') {
				size = bufferLen - (currentValue - buffer); // Deduce the size of the sub structure
				*dstStruct = (char*) AOEMALLOC(size + 4);
				memcpy((*dstStruct) + 4, currentValue, size);
				*((uint32_t*) (*dstStruct)) = HTON_UINT32(size + 4); // Restore the size header of the sub structure
			} else {
				size = getPackedStructureLength(subFormat, currentValue);
				*dstStruct = (char*) AOEMALLOC(size);
				memcpy((*dstStruct), currentValue, size);
			}

			currentValue += size;
			assert(format[1] != '\0' || (currentValue == buffer+bufferLen));
		}
			break;
		}
	} while(*(++format));

	va_end(pv);

}

// QC:P
unsigned long getPackedStructureLength(const char* format, const char* buffer) {
	if(!format || *format != '!') {
		xerror(buffer, "NULL packed structure buffer.");
		xwarn(NTOH_INT32(*((uint32_t*) (buffer))) < 4, "Malformed packed structure : size < 4");
		return NTOH_INT32(*((uint32_t*) (buffer)));
	} else {
		return computePackedStructureLength(format);
	}
}

/** Duplicates a structure.
 * This function returns a duplicate of a packed structure, like strdup for strings.
 * You must provide the format for the size to be computed properly.
 *
 * Because the size of the structure is known in all cases, this function can be used to extract a sub-structure from
 * a larger buffer.
 */
// QC:P
char* packedStructureDup(const char* format, const char* buffer) {
	assert(format);
	xerror(buffer, "NULL packed structure buffer.");

	unsigned long size = getPackedStructureLength(format, buffer);
	char* out = (char*) AOEMALLOC(size);
	memcpy(out, buffer, size);
	return out;
}

// QC:P
unsigned long computePackedStructureLength(const char* format) {
	unsigned long size = 0;
	bool isSized = false;

	if(*format != '!') {
		size += 4;
		isSized = true;
	} else {
		format++;
	}

	do {
		switch(*format){
		case 'I':
		case 'i':
			size += 4;
			break;

		case 'c':
		case 'C':
			size += 1;
			break;

		case 'b': {
			unsigned int bufsize;
			bufsize = parseBufferSize(format);
			if(bufsize > 0) {
				size += bufsize;
			} else {
				if(!isSized) {
					size += packedStructureBufferSize;
				}
			}
		}
			break;

		case 's': {
			if(!isSized) {
				size += packedStructureBufferSize;
			}
		}

		case 'S': {
			assert(isSized);
		}
		}
	} while(*(++format));

	return size;
}

// QC:?
bool stringToInt(const char* string, int* i) {
	if(i && string && ((*string >= '0' && *string <= '9') || *string == '-')) {
		*i = atoi(string);
		return true;
	}

	return false;
}

// QC:?
bool stringToBool(const char* string, bool* b) {
	if(b && string) {
		if(*string == 'y' || *string == 't' || *string == '1' || *string == 'e') {
			*b = true;
			return true;
		}
		if(*string == 'n' || *string == 'f' || *string == '0' || *string == 'd') {
			*b = false;
			return true;
		}
	}

	return false;
}

// QC:?
bool stringToFlags(const char* string, uint32_t* f) {
	if(f && string) {
		*f = 0;
		if(*string == 'n' || *string == 'f' || *string == 'd') {
			return true;
		}
		if(*string == 'y' || *string == 't' || *string == 'e') {
			*f = 1;
			return true;
		}
		while(string && string != (const char*)1 && *string >= 'O' && *string <= '9') {
			int flagBit = atoi(string);
			xassert(flagBit >= 0, "Trying to set a negative bit in a flag.");
			xerror(flagBit < 32, "Trying to set %dth bit of a flag. Flags can only have 32 bits.", flagBit);
			*f |= (1 << flagBit);
			string = strchr(string, ':') + 1;
		}
		return true;
	}

	return false;
}

// QC:?
bool stringToCoord(const char* string, coord* i) {
	if(i && string && ((*string >= '0' && *string <= '9') || *string == '-')) {
		*i = atoc(string);
		return true;
	}

	return false;
}

// QC:P
int stringToIntList(const char* string, int* v0, int* v1, int* v2, int* v3) {
	if(v0 && string && ((*string >= '0' && *string <= '9') || *string == '-')) {
		*v0 = atoi(string);
		string = strchr(string, ':') + 1;
		if(v1 && string!=(const char*)1 && ((*string >= '0' && *string <= '9') || *string == '-')) {
			*v1 = atoi(string);
			string = strchr(string, ':') + 1;
			if(v2 && string!=(const char*)1 && ((*string >= '0' && *string <= '9') || *string == '-')) {
				*v2 = atoi(string);
				string = strchr(string, ':') + 1;
				if(v3 && string!=(const char*)1 && ((*string >= '0' && *string <= '9') || *string == '-')) {
					*v3 = atoi(string);
					return 4;
				} else {
					return 3;
				}
			} else {
				return 2;
			}
		} else {
			return 1;
		}
	} else {
		return 0;
	}
}

// QC:?
int stringToCoordList(const char* string, coord* v0, coord* v1, coord* v2, coord* v3) {
	if(v0 && string && ((*string >= '0' && *string <= '9') || *string == '-')) {
		*v0 = atoc(string);
		string = strchr(string, ':') + 1;
		if(v1 && string!=(const char*)1 && ((*string >= '0' && *string <= '9') || *string == '-')) {
			*v1 = atoc(string);
			string = strchr(string, ':') + 1;
			if(v2 && string!=(const char*)1 && ((*string >= '0' && *string <= '9') || *string == '-')) {
				*v2 = atoc(string);
				string = strchr(string, ':') + 1;
				if(v3 && string!=(const char*)1 && ((*string >= '0' && *string <= '9') || *string == '-')) {
					*v3 = atoc(string);
					return 4;
				} else {
					return 3;
				}
			} else {
				return 2;
			}
		} else {
			return 1;
		}
	} else {
		return 0;
	}
}

// QC:?
int codepointToUTF8(char* outbuf, unsigned long codepoint) {
	xerror(codepoint < (1<<21), "Invalid unicode codepoint : out of range.");
	if(codepoint < (1<<7)) {
		*outbuf = (char)codepoint;
		return 1;
	}
	if(codepoint >= (1<<16)) {
		outbuf[0] = 0xF0 | (codepoint >> 18);
		outbuf[1] = 0x80 | ((codepoint >> 12) & 0x3F);
		outbuf[2] = 0x80 | ((codepoint >> 6) & 0x3F);
		outbuf[3] = 0x80 | (codepoint & 0x3F);
		return 4;
	} else if(codepoint >= (1<<12)) {
		outbuf[0] = 0xE0 | (codepoint >> 12);
		outbuf[1] = 0x80 | ((codepoint >> 6) & 0x3F);
		outbuf[2] = 0x80 | (codepoint & 0x3F) ;
		return 3;
	} else {
		outbuf[0] = 0xC0 | (codepoint >> 6);
		outbuf[1] = 0x80 | (codepoint & 0x3F);
		return 2;
	}
	
	xassert(false, "Invalid unicode codepoint : out of range.");
	return 0;
}

// QC:?
int UTF8ToCodepoint(char* t, unsigned long& codepoint) {
	unsigned long c = (unsigned char)*(t++);

	// Compute the number of supplementary bytes
	int suppByteCount = getUTF8CharLen(c) - 1;
	if(!suppByteCount) {
		codepoint = c;
		return 1;
	}

	xerror(suppByteCount >= 1 && suppByteCount <= 4, "Invalid UTF-8 stream."); // UTF-8 characters are encoded in 1 to 4 bytes

	// Decode the first byte
	c &= (1<<(7-suppByteCount)) - 1;
	
	int i = suppByteCount;
	while(i--) {
		xerror(*t & (1<<7) && !(*t & (1<<6)), "Invalid UTF-8 stream."); // Must be prefixed by 10xxxxxx

		c <<= 6;
		c |= *(t++) & ((1<<6) - 1);
	}

	codepoint = c;
	return suppByteCount + 1;
}

// QC:?
int getUTF8CharLen(unsigned long c) {
	// UTF-8 :
	// 0xxxxxxx => bytesPerChar == 1
	// 110xxxxx => bytesPerChar == 2
	// 1110xxxx => bytesPerChar == 3
	// 11110xxx => bytesPerChar == 4

	if(!(c & (1<<7)))
		return 1;
	xerror((c & 0xC0) != 0x80, "Byte is in mid-character. The stream may nopt be synchronized.");
	if((c & 0xE0) == 0xC0)
		return 2;
	if((c & 0xF0) == 0xE0)
		return 3;
	xerror((c & 0xF8) == 0xF8, "Invalid UTF-8 value.");
	return 4;
}
#ifdef MEMTRACKING
#include <map>
#include <pspec/thread.h>
/*------------------------------------------------------------------------------*/
std::map<void*, size_t> aoeMemoryAllocations;
Mutex aoeMallocMutex;
size_t aoeMallocTotal = 0;
size_t aoeMallocPeak = 0;
size_t aoeMallocCount = 0;
void* aoeMalloc(size_t size) {
	aoeMallocTotal += size;
	if(aoeMallocPeak < aoeMallocTotal)
		aoeMallocPeak = aoeMallocTotal;
	aoeMallocCount++;
#ifdef AOE_MAX_MEMORY
	xerror(aoeMallocTotal < AOE_MAX_MEMORY, "Memory full (reached %d MB, limit is %d MB).", aoeMallocTotal / 1048576, AOE_MAX_MEMORY / 1048576);
#endif
	void* result = malloc(size);
	xassert(result, "Could not allocate memory.");
	aoeMallocMutex.p();
	aoeMemoryAllocations[result] = size;
	aoeMallocMutex.v();
	return result;
}
/*------------------------------------------------------------------------------*/
void aoeFree(void* ptr) {
	aoeMallocMutex.p();
	xassert(ptr, "Trying to free a NULL pointer.");
	std::map<void*, size_t>::iterator it = aoeMemoryAllocations.find(ptr);
	xassert(it != aoeMemoryAllocations.end(), "Trying to free an invalid pointer.");
	size_t size = it->second;
	aoeMallocTotal -= size;
	aoeMallocCount--;
	aoeMemoryAllocations.erase(it);
	if(size >= 4) {
		((uint32_t*)ptr)[0] = 0xdeadbeef;
	} else if(size >= 2) {
		((uint16_t*)ptr)[0] = 0xcafe;
	} else if(size >= 1) {
		((unsigned char*)ptr)[0] = 0xfb;
	}
	free(ptr);
	aoeMallocMutex.v();
}
/*------------------------------------------------------------------------------*/
char* aoeStrdup(const char* str) {
	xassert(str, "Trying to duplicate a NULL string.");
	size_t size = strlen(str) + 1;
	char* newString = (char*)aoeMalloc(size);
	memcpy(newString, str, size);
	return newString;
}
/*------------------------------------------------------------------------------*/
std::map<void*, size_t> aoeVideoMemoryAllocations;
Mutex aoeVidAllocMutex;
size_t aoeVidAllocTotal = 0;
size_t aoeVidAllocPeak = 0;
size_t aoeVidAllocCount = 0;
void aoeVidAlloc(void* ptr, size_t size) {
	aoeVidAllocTotal += size;
	if(aoeVidAllocPeak < aoeVidAllocTotal)
		aoeVidAllocPeak = aoeVidAllocTotal;
	aoeVidAllocCount++;
#ifdef AOE_MAX_VIDEO_MEMORY
	xerror(aoeVidAllocTotal < AOE_MAX_VIDEO_MEMORY, "Video memory full (reached %d MB, limit is %d MB).", aoeVidAllocTotal / 1048576, AOE_MAX_VIDEO_MEMORY / 1048576);
#endif
	aoeVidAllocMutex.p();
	aoeVideoMemoryAllocations[ptr] = size;
	aoeVidAllocMutex.v();
}
/*------------------------------------------------------------------------------*/
void aoeVidFree(void* ptr) {
	aoeVidAllocMutex.p();
	std::map<void*, size_t>::iterator it = aoeVideoMemoryAllocations.find(ptr);
	xassert(it != aoeVideoMemoryAllocations.end(), "Trying to free an invalid pointer.");
	size_t size = it->second;
	aoeVidAllocTotal -= size;
	aoeVidAllocCount--;
	aoeVideoMemoryAllocations.erase(it);
	aoeVidAllocMutex.v();
}
#endif /* MEMTRACKING */
/*------------------------------------------------------------------------------*/

#ifdef POINTER_TRACKING
PointerMap pointerMap;

void trackPointer(ScriptableObject* &ptr) {
	if(ptr) {
		pointerMap[&ptr] = ptr;
	} else {
		untrackPointer(ptr);
	}
}

void untrackPointer(ScriptableObject* &ptr) {
	PointerMap::iterator it = pointerMap.find(&ptr);
	if(it != pointerMap.end()) {
		pointerMap.erase(it);
	}
}

/** Replace the address of an object in all registered pointers.
 */
void replacePointer(ScriptableObject* oldValue, ScriptableObject* newValue) {
	// Replace value in all pointers that point to the object.
	for(PointerMap::iterator it = pointerMap.begin(); it != pointerMap.end(); it++) {
		if(it->second == oldValue) {
			// Replace the pointer in memory
			*(it->first) = newValue;

			// Store the new value in the map
			it->second = newValue;
		}
	}
}

void trackPointer(char* &ptr) {
	if(ptr) {
		pointerMap[(ScriptableObject**)&ptr] = (ScriptableObject*)ptr;
	} else {
		untrackPointer(ptr);
	}
}

void untrackPointer(char* &ptr) {
	PointerMap::iterator it = pointerMap.find((ScriptableObject**)&ptr);
	if(it != pointerMap.end()) {
		pointerMap.erase(it);
	}
}

#endif
