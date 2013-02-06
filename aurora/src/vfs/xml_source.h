#ifndef VFS__XML_SOURCE_H
#define VFS__XML_SOURCE_H

#include "tinyxml.h"
#include "file.h"

namespace VFS {


class XMLSource {
protected:
	File* sourceFile;
	TiXmlDocument* doc;
public:
	XMLSource(File* file);
	virtual ~XMLSource();

	TiXmlDocument* getDocument();
	TiXmlElement* getRoot();
	void setRoot(TiXmlElement* newRoot);

	/** Get the UTF-8 text version of the XML document.
	 * @param outputBuffer OUTPUT : will point to the XML string.
	 * @return the size of the XML string.
	 */
	size_t getText(const char*& outputBuffer);

	/** Saves the document on the disk. The file needs to be writable to do this.
	 * @return true if the document was successfully saved, false otherwise.
	 */
	bool save();
};

#ifdef XML_SUPPORT
TiXmlElement* openXML(FILE* sourceFile);
#else
TiXmlElement* openXML(FILE* sourceFile) { xassert(false, "XML support is not compiled in VFS."); return NULL; }
#endif

}

#endif /* VFS__XML_SOURCE_H */
