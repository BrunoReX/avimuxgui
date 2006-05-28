#include "stdafx.h"
#include "bitstream.h"


int BITSTREAM::Open(STREAM* lpStream)
{
	 source=lpStream;
	 dwCurrBitPos=16;
	 if (GetSource()) GetSource()->Seek(0);
	 return (lpStream)?STREAM_OK:STREAM_ERR; 
}

int BITSTREAM::ReadBit(int iFlag)
{
	if (dwCurrBitPos>7)
	{
		dwCurrBitPos=7;
		LoadWord();
	}

	int iRes=!!(wData&(1<<(iFlag?(7-dwCurrBitPos--):dwCurrBitPos--)));
	
	return iRes;
}

int BITSTREAM::Seek(__int64 qwPos)
{
	_ASSERT(qwPos >= 0);
	_ASSERT(GetSource());

	if (!GetSource()) 
		return 0;

	if (GetSource()->Seek(qwPos>>3)==STREAM_OK)
	{
		GetSource()->Read(&wData,1);
		dwCurrBitPos=(DWORD)(qwPos&0x7);
		return 1;
	}
	else
		return 0;
}

void BITSTREAM::LoadWord(void)
{
	if (GetSource())
	{
		GetSource()->Read(&wData,1);
	}
}

int BITSTREAM::ReadBits(int n, int iFlag)
{
	return (int)ReadBits64(n, iFlag);
}

__int64 BITSTREAM::GetPos()
{
	return GetSource()->GetPos();
}

__int64 BITSTREAM::ReadBits64(int n, int iFlag)
{
	__int64	iRes=0;
	__int64 iMul=1;
	if (!GetSource()) return 0;

	for (int i=0;i<n;i++)
	{
		if (!iFlag) {
			iRes*=2;
			iRes|=ReadBit(iFlag);
		} else {
			iRes += iMul*ReadBit(iFlag);
			iMul *= 2;
		}
	}

	return iRes;
}