/* os_def.h -- define if you want to compile for Windows or Linux here */
#ifndef OS_DEF_H
#define OS_DEF_H
// #define WINDOWS / LINUX
#ifdef _WIN32
# define WINDOWS
#elif defined (__linux__)
# define LINUX
#endif

#ifdef WINDOWS
	#include <windows.h>
#endif

#endif
