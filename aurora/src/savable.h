#ifndef SAVABLE_H
#define SAVABLE_H

#include "scriptable_object.h"
#include "exportable.h"

class Savable : public Exportable {
private:

public:
	virtual ~Savable();

	/** Save this object to an XML element.
	 * Once saved, loading the resulting XML structure should give an equivalent to the saved object.
	 * It will not necessarily save all inner details, but should give a good tradeoff between space, complexity and fidelity.
	 * Remember that only the parameters passed in the constructor can be saved.
	 * 
	 * This function should not output default values.
	 *  
	 * The caller is responsible for freeing the structure after use.
	 * @param destElement the XML structure that will be populated by this function.
	 */
	virtual void saveToXML(TiXmlElement* destElement) = 0;

	/** Save this object to a string.
	 * The caller is responsible for freeing the string using AOEFREE().
	 * @return a new string buffer representing the state of the object.
	 */
	char* saveToString();

	/** Save this object to a file.
	 * @param filename the name of the file to save to.
	 * @param append instead of replacing the file.
	 * @return true if the file was successfully created, false otherwise.
	 */
	bool saveToFile(const char* filename, bool append = false);

};

#endif /* SAVABLE_H */
