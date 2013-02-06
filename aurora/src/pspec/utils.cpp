#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#endif

bool isDirectory(const char* filename) {
	DIR* d = opendir(filename);
	if(d) {
		closedir(d);
		return true;
	}
	return false;
}

bool checkByteOrder() {
	int i = 0x4F4B;
#ifdef LE
	syslog("Compiled for Little Endian plateform.");
#else
	syslog("Compiled for Big Endian plateform.");
#endif

	if(strncmp("OK", (char*)&i, 2) != 0) {
#ifdef LE
		syslog("Plateform is little endian.");
		return true;
#else
		syslog("Plateform is not big endian.");
		return false;
#endif
	}

#ifdef LE
	syslog("Plateform is big endian.");
	return false;
#else
	syslog("Plateform is not little endian.");
	return true;
#endif
}

#ifdef WIN32
// QC:S
char* strchrnul(const char* s, int c) {
	char* chr = (char*)strchr((char*)s, c);
	if(!chr) {
		return (char*)(s + strlen(s));
	}

	return chr;
}
#endif

