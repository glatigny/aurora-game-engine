#include "xml_source.h"

namespace VFS {


XMLSource::XMLSource(File* file) {
	doc = new TiXmlDocument;
	sourceFile = file;
	char* data;
	file->mmapWholeFile(data);
	if(data)
		doc->Parse(data);
	file->munmap(data);
}

XMLSource::~XMLSource() {
	assert(sourceFile);
	delete sourceFile;

	assert(doc);
	delete doc;
}

XMLSource* openXML(File* f) {
	if(!f) {
		return NULL;
	}
	return new XMLSource(f);
}

size_t XMLSource::getText(const char*& outputBuffer) {
	TiXmlPrinter printer;
	printer.SetStreamPrinting();
	xassert(doc, "No document to save.");
	doc->Accept(&printer);
	outputBuffer = printer.CStr();
	return printer.Size();
}

// QC:?
bool XMLSource::save() {
	xassert(sourceFile, "No file to save XML data to.");

	const char* data;
	size_t size = getText(data);
	sourceFile->truncate();
	return sourceFile->write((char*)data, size) == size;
}

}
