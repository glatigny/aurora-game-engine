#include "savable.h"

#define WRITE_BLOCK_SIZE 4096

char* Savable::saveToString() {
	return NULL;
}

bool Savable::saveToFile(const char* filename, bool append) {
	FILE* f = fopen(filename, append?"a":"w");
	
	if(!f) {
		return false;
	}
	
	char* content = saveToString();
	char* s = content;
	int len = strlen(s);
	
	while(len > WRITE_BLOCK_SIZE) {
		fwrite(s, WRITE_BLOCK_SIZE, 1, f);
		s += WRITE_BLOCK_SIZE;
		len -= WRITE_BLOCK_SIZE;
	}
	fwrite(s, len, 1, f);
	
	AOEFREE(content);

	return true;
}
