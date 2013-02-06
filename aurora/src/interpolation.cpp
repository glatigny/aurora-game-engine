#include "interpolation.h"

// QC:P
Interpolation* newInterpolation(const char* intName) {
	if(!intName || !*intName) {
		return new DEFAULT_INTERPOLATION();
	}

	char* name;
	STACK_PARAMDUP(name, intName, 1024);

	char* params;
	if( (params = strchr(name, ':')) ) {
		*params = '\0';
		params++;
	}

	Interpolation* i = NULL;

	if(false);
#define INTERPOLATION(paramName, className) else if(strcmp(paramName, name) == 0) { i = new className(); }
	INTERPOLATION("step", StepInterpolation)
	INTERPOLATION("linear", LinearInterpolation)
	INTERPOLATION("quad", QuadInterpolation)
	INTERPOLATION("sinus", SinusInterpolation)
	INTERPOLATION("halfsinus", HalfSinusInterpolation)
	INTERPOLATION("sinc", SincInterpolation)
	INTERPOLATION("random", RandomInterpolation)
#undef INTERPOLATION

	if(!i) {
		syslog("Interpolation %s unknown.", name);
		return NULL;
	}

	if(params) {
		i->setParameters(params);
	}

	return i;
}

