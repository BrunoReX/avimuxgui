#ifndef I_FORMATI64
#define I_FORMATI64

#include <string>

enum FormatSizeModes 
{
	BYTES,

	BYTES_PER_SECOND,

	BITS_PER_SECOND
};

enum FormatSizeKSizes
{
	SIZE_1000,

	SIZE_1024
};

void QW2Str(__int64 qwX,char*	lpDest,int  dwLen);
void FormatSize(char* d, unsigned __int64 qwSize, FormatSizeModes Mode = BYTES, FormatSizeKSizes KSize = SIZE_1024);
std::string FormatSize(unsigned __int64 qwSize, FormatSizeModes Mode, FormatSizeKSizes KSize);

#endif