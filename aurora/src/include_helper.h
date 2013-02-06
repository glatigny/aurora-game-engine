/* Include helper
 * This file will call a helper.
 * To call a helper, define the HELPER variable with the base name of the header to include, then include this file immediatly after.
 * For example, to use the helper scriptable_object_cpp.ih on renderable.xh :
 *
 *  #define HEADER renderable // Requirement of scriptable_object_cpp
 *  #define HELPER scriptable_object_cpp
 *  #include "include_helper.h"
 *
 */

#include "macros/macro_expansion.h"

#include "macros/def_variables.h"

#define HELPER_FILE HELPER_TO_FILENAME(HELPER)
#include HELPER_FILE
#undef HELPER_FILE
#undef HELPER

#include "macros/undef_variables.h"


