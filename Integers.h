#ifndef I_INTEGERS
#define I_INTEGERS

#include "buffers.h"

extern "C" {
    int _stdcall Int2VSUInt_asm(__int64 *x, char* y, int iLen = 0);
    __int64 _stdcall FSSInt2Int (char* x, int iLen);
    __int64 _stdcall swap_byteorder (__int64 x, int iLen);
}


int _stdcall Int2VSUInt(__int64 *x, char* y, int iLen = 0);
int Int2VSSInt(__int64 *x, char* y, int iLen = 0);
int  VSUInt2Int (char* x, __int64* y);
int  VSSInt2Int (char* x, __int64* y);
__int64 FSSInt2Int (CBuffer* x);

__int64 rdtsc();

typedef struct
{
	int  iLength;
	char* cData;
} INT2VSUINT_DESCR;

extern int VSizeInt_Len[256];

#endif