#include "png_image_source.h"

void png_user_read_data(png_structp png_ptr, png_bytep data, png_size_t length) {
	VFS::File* f = (VFS::File*)png_ptr->io_ptr;
	f->read(data, length);
}

void png_user_write_data(png_structp png_ptr, png_bytep data, png_size_t length) {
	VFS::File* f = (VFS::File*)png_ptr->io_ptr;
	f->write(data, length);
}

void png_user_flush_data(png_structp png_ptr) {}

namespace VFS {

PNGImageSource::PNGImageSource(VFS::File* f) : ImageSource(f) {}

PNGImageSource::~PNGImageSource() {}

PNGImageSource* PNGImageSource::copy() {
	return new PNGImageSource(file);
}

bool PNGImageSource::handles(VFS::File* f) {
	return strstr(f->name(), ".png") != NULL;
}

// QC:B (need to implement image formats)
void PNGImageSource::load(char* &buffer, Rect &bufferSize, size_t& bufferLen, size_t& pitch, int &format, Rect &imageSize, bool forcePOT, bool allowConversion) {
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep *row_pointers;

	int iw, ih; // Image size
	int bw, bh; // Buffer size

	format = IMAGE_R8G8B8A8; // TODO : implement other formats

	xwarn(!allowConversion, "Conversion for PNG files is not supported yet.");

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL);
	xerror(png_ptr, "Cannot create libpng structure for PNG file %s", file->name());

	info_ptr = png_create_info_struct(png_ptr);
	xerror(info_ptr, "Cannot create info pointer for PNG file %s", file->name());

	file->switchMode(MODE_READ);
	if(file->canSeek()) {
		file->seek(0, SEEK_SET);
	}

	png_set_read_fn(png_ptr, (void*)file, png_user_read_data);
	png_read_info(png_ptr, info_ptr);

	int components;
	switch(info_ptr->color_type) {
		case PNG_COLOR_TYPE_GRAY :
			components = 1;
			switch(info_ptr->bit_depth) {
				case 1:
					format = IMAGE_MONO;
					break;
			}
			break;
		case PNG_COLOR_TYPE_PALETTE :
			components = 1;
			switch(info_ptr->bit_depth) {
				case 8:
					format = IMAGE_P8_RGBA8;
					break;
				case 4:
					format = IMAGE_P4_RGBA8;
					break;
				case 2:
					format = IMAGE_P2_RGBA8;
					break;
				case 1:
					format = IMAGE_MONO;
					break;
				default:
					format = IMAGE_UNKNOWN_FORMAT;
					xerror(false, "Image has unknown format : palette with bit depth %d.", info_ptr->bit_depth);
					break;
			}
			break;
		case PNG_COLOR_TYPE_RGB :
			components = 3;
			switch(info_ptr->bit_depth) {
				case 8:
					format = IMAGE_R8G8B8;
					break;
				case 16:
					format = IMAGE_R16G16B16;
					break;
				default:
					format = IMAGE_UNKNOWN_FORMAT;
					xerror(false, "Image has unknown format : RGB with bit depth %d.", info_ptr->bit_depth);
					break;
			}
			break;
		case PNG_COLOR_TYPE_RGBA :
			components = 4;
			switch(info_ptr->bit_depth) {
				case 8:
					format = IMAGE_R8G8B8A8;
					break;
				case 16:
					format = IMAGE_R16G16B16A16;
					break;
				default:
					format = IMAGE_UNKNOWN_FORMAT;
					xerror(false, "Image has unknown format : RGBA with bit depth %d.", info_ptr->bit_depth);
					break;
			}
			break;
		case PNG_COLOR_TYPE_GA :
			components = 2;
			switch(info_ptr->bit_depth) {
				case 8:
					format = IMAGE_G8A8;
					break;
				case 16:
					format = IMAGE_G16A16;
					break;
				default:
					format = IMAGE_UNKNOWN_FORMAT;
					xerror(false, "Image has unknown format : Alpha+Gray with bit depth %d.", info_ptr->bit_depth);
					break;
			}
			break;
		default:
			components = 0;
			format = IMAGE_UNKNOWN_FORMAT;
			xerror(false, "Image has unknown color type %d.", info_ptr->color_type);
			break;
	}

	if(forcePOT) {
		iw = info_ptr->width;
		ih = info_ptr->height;
		bw = nextPowerofTwo(iw);
		bh = nextPowerofTwo(ih);
	} else {
		bw = iw = info_ptr->width;
		bh = ih = info_ptr->height;
	}
	pitch = bw * components * info_ptr->bit_depth / 8;
	bufferSize = Rect(0, 0, bw, bh);
	imageSize = Rect(0, 0, iw, ih);
	bufferLen = bh * pitch;
	buffer = (char*)AOEMALLOC(bufferLen);
	if(forcePOT) {
		bzero(buffer, bh * pitch);
	}
	row_pointers = (png_bytep*) alloca(ih * sizeof(png_bytep));
	for(int y=0; y<ih; y++) {
		row_pointers[y] = (png_byte*)(buffer + (y * pitch));
	}

	png_read_image(png_ptr, row_pointers);

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	file->switchMode(MODE_CLOSED);
}

#ifdef PNG_SAVE_SUPPORT
bool PNGImageSource::handlesWriting(VFS::File* f, int format) {
	return (strstr(f->name(), ".png") && (
		format == IMAGE_R8G8B8 ||
		format == IMAGE_R8G8B8A8 ||
		format == IMAGE_G8 ||
		format == IMAGE_G8A8));
}

bool PNGImageSource::save(VFS::File* f, const char* buffer, int format, int width, int height) {
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep *row_pointers;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr)
		return false;

	info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr)
		return false;

	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

	int color_type;
	int pitch = ImageSource::getPitch(format, width);
	switch(format) {
		case IMAGE_R8G8B8:
			color_type = PNG_COLOR_TYPE_RGB;
			break;
		case IMAGE_R8G8B8A8:
			color_type = PNG_COLOR_TYPE_RGB_ALPHA;
			break;
		case IMAGE_G8:
			color_type = PNG_COLOR_TYPE_GRAY;
			break;
		case IMAGE_G8A8:
			color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
			break;
		default:
			xwarn(false, "PNG ImageSource could not save %s : unhandled format %d.", f->name(), format);
			return false;
			break;
	}

	row_pointers = (png_bytep*) alloca(height * sizeof(png_bytep));
	for(int y=0; y<height; y++) {
		row_pointers[y] = (png_byte*)(buffer + (y * pitch));
	}

	png_set_IHDR(png_ptr, info_ptr, width, height, 8, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_set_write_fn(png_ptr, (void*)f, png_user_write_data, png_user_flush_data);
	png_write_info(png_ptr, info_ptr);
	png_write_image(png_ptr, row_pointers);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	return true;
}
#endif

}
