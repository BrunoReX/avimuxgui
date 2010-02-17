#ifndef I_FORMATTIME
#define I_FORMATTIME

/* convert a certain number of milliseconds into hours, minutes,
   seconds and milliseconds */
void	Millisec2HMSF(__int64 qwMillisec, 
					  int* lpdwH, 
					  int* lpdwM,
					  int* lpdwS,
					  int* lpdwF);

/* convert a certain number of milliseconds into a string like
   hh:mm:ss.fff */
void	Millisec2Str(__int64 qwMillisec, 
					 char* lpcDest);



/* convert a string into milliseconds. The string can be given in 
   formats like hh:mm:ss.fff or xxhxxmxxsxxx with no restrictions
   to hh etc, so it does support strings like 73m91s */
__int64	Str2Millisec(char* c);

#endif