/* preproc.h -- extension for the processed by preproc file */
/* ver. 1.01 */
#ifndef PREPROC_H
#define PREPROC_H
#include "../os_def.h"
#ifdef WINDOWS
char ppexenm[] = "preproc.exe";		// executable name windows
#else
char ppexenm[] = "./preproc.bin";	// executable name linux		
#endif	

char ppext[] = ".pp";				// output file is <original file.ext>.pp
#endif
