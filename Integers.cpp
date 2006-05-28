#include "stdafx.h"
#include "integers.h"

#ifdef DEBUG_NEW
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/*

  use lookup tables for variable size integer encoding. Might be faster or not...

*/

int VSizeInt_Len[256] = 
	{ 
	9,8,7,7,6,6,6,6,5,5,5,5,5,5,5,5,
	4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
 };

INT2VSUINT_DESCR*	int2vsuint_table = NULL;

const int int2vsuint_table_length = 131072;


int  VSUInt2Int (char* x, __int64* y)
{
	int i = 1; // length
	int l;
	__int64 res = 0;
	// get length
	char d = *x;
	if (!d) return 0;
	while (!(d & 0x80)) {
		i++;
		d<<=1;
	}

	l = i;
	if (!y) return l;

	res = (*(unsigned char*)x++)&(~(1<<(8-l)));

	while (--i) {
		res = res*256;
		res = res + *(unsigned char*)x;
		x++;
	}

	*y = res;

	return l; 
}

int _stdcall Int2VSUInt(__int64 *x, char* y, int iLen)
{
	if (!int2vsuint_table) {
		int2vsuint_table = new INT2VSUINT_DESCR[int2vsuint_table_length];
		ZeroMemory(int2vsuint_table,int2vsuint_table_length*sizeof(INT2VSUINT_DESCR));
	}

	if (*x > int2vsuint_table_length || iLen) {
		return Int2VSUInt_asm(x,y,iLen);
	}

	if (int2vsuint_table[*x].iLength) {
		memcpy(y,int2vsuint_table[*x].cData,int2vsuint_table[*x].iLength);
		return int2vsuint_table[*x].iLength;
	}

	int i = Int2VSUInt_asm(x,y,iLen);
	int2vsuint_table[*x].iLength = i;
	int2vsuint_table[*x].cData = new char[i];
	memcpy(int2vsuint_table[*x].cData,y,i);
	return i;
}

__int64 int_max [] = { 0x3F, 0x1FFF, 0x0FFFFF, 0x07FFFFFF, 
                       0x03FFFFFFFF, 0x01FFFFFFFFFF, 0x00FFFFFFFFFFFFFF,
					   0x007FFFFFFFFFFFFF };
int int_max_len = sizeof(int_max)/sizeof(__int64);

int Int2VSSInt(__int64 *x, char* y, int iLen)
{
	int i;
	
	for (i=0;i<int_max_len;i++) {
		if (*x >= -int_max[i] && *x <= int_max[i]) {
			__int64 j = *x + int_max[i];
			return Int2VSUInt(&j, y, i+1);
		}
	}

	return 0;
}


int  VSSInt2Int (char* x, __int64* y)
{
	int l = VSUInt2Int(x,y);
	*y-=int_max[l-1];

	return l; 
}

__int64  FSSInt2Int (CBuffer* x)
{
	return FSSInt2Int(x->AsString(),x->GetSize());
}

__int64 rdtsc()
{
	_asm rdtsc;
}
