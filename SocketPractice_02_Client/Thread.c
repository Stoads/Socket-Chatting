#include "Thread.h"
#include <Windows.h>
void synchronize(CRITICAL_SECTION* crt) {
	EnterCriticalSection(crt);
}
void synchronized(CRITICAL_SECTION* crt) {
	LeaveCriticalSection(crt);
}