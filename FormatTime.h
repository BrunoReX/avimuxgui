#ifndef I_FORMATTIME
#define I_FORMATTIME

void	Millisec2HMSF(__int64 qwMillisec,int* lpdwH,int* lpdwM,int* lpdwS,int* lpdwF);
void	Millisec2Str(__int64 qwMillisec, char* lpcDest);
__int64	Str2Millisec(char* c);

#endif