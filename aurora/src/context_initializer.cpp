#include "context.h"
#include "context_initializer.h"
#include "h_list.h"

// QC:P
void ContextInitializer::registerAllNativeClasses(Context* c) {
#define LIST_HELPER function_registration
#include "xh_list.xh"
}
