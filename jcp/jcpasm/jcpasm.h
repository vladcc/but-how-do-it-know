/* jcpasm.h -- header for the jcpasm exporting information needed by the lang compiler */
/* ver. 1.123 */
#ifndef JCPASM_H
#define JCPASM_H
#include "../os_def.h"
#ifdef WINDOWS
char jasm_exenm[] = "jcpasm.exe";	// windows executable name
#else
char jasm_exenm[] = "./jcpasm.bin";	// linux executable name
#endif

char jasm_ext[] = ".jasm";			// output file is <original file.ext>.jasm
#endif