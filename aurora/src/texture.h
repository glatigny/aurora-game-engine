#ifndef TEXTURE_H
#define TEXTURE_H

/*--- INCLUDES ---*/

/*--- DEFINES ---*/

/*--- VARIABLES ---*/

/*--- DEFINITION ---*/
typedef struct {
	void* vData; // Video RAM data
	char* sData; // System RAM image
	int format; // System RAM data format
	size_t bufferLen;
	size_t pitch;
	int w;
	int h;
	int ow; // original image size 
	int oh;
} Texture;

/*-EOF-*/
#endif /* TEXTURE_H */
