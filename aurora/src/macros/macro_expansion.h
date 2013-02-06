/* Macro expansion for helpers.
 * These function-like macros do common taks for manipulating preprocessor variables.
 */

#ifndef MACRO_EXPANSION_HELPER_INCLUDED
#define MACRO_EXPANSION_HELPER_INCLUDED

#define EXPAND_MACRO(X) X

#define STRINGIFY_(X) #X
#define STRINGIFY(X) STRINGIFY_(X)

#define JOIN_(X,Y) X ## Y
#define JOIN(X,Y) JOIN_(X,Y)

#define HELPER_TO_FILENAME_(HELPER_NAME) STRINGIFY(helpers/HELPER_NAME.ih)
#define HELPER_TO_FILENAME(HELPER_NAME) HELPER_TO_FILENAME_(HELPER_NAME)
#define HEADER_TO_FILENAME_(HEADER_NAME,HEADER_EXTENSION) STRINGIFY(HEADER_NAME.HEADER_EXTENSION)
#define HEADER_TO_FILENAME(HEADER_NAME,HEADER_EXTENSION) HEADER_TO_FILENAME_(HEADER_NAME,HEADER_EXTENSION)

#endif /* MACRO_EXPANSION_HELPER_INCLUDED */

