#include "time.h"
#include <windows.h>

void waitMillis(unsigned long dwMilliseconds)
{
	Sleep(dwMilliseconds);
}