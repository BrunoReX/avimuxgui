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
//	_ASSERT(qwPos >= 0);
//	_ASSERT(GetSource());

	if (!GetSource()) 
		return 0;

	if (GetSource()->Seek(qwPos>>3)==STREAM_OK)
	{
		GetSource()->Read(&wData,1);
		dwCurrBitPos=(DWORD)(7 - (qwPos&0x7));
		return 1;
	}
	else
		return 0;
}

void BITSTREAM::LoadWord(void)
{
	if (GetSource())
	{
		wData = 0;
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

CBitStream2::CBitStream2() : 
	CBitStream2::BITSTREAM(),
	m_PrebufferSize(16)
{
}

CBitStream2::~CBitStream2()
{
}


void CBitStream2::LoadWord(void)
{
	if (GetSource())
	{
		if (m_InputBuffer.empty())
		{
			if (m_PrebufferSize > 64)
				m_PrebufferSize = 64;

			char buffer[64];
			int result = GetSource()->Read(buffer, m_PrebufferSize);

			for (int j=0; j<result; j++)
			{
				m_InputBuffer.push_back(buffer[j]);
			}
		}

		wData = (WORD) m_InputBuffer.front();
		m_InputBuffer.pop_front();
		dwCurrBitPos = 7;
	}

}

int CBitStream2::Seek(__int64 qwPos)
{
	m_InputBuffer.clear();
	return BITSTREAM::Seek(qwPos);
}

static uint8 shift_table_fl[] =
{
	17	
};

static uint8 relevance_map_fl[] = {
	0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF 
};

static uint8 relevance_map_fr[] = {
	0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF 
};

__int64 CBitStream2::ReadBits64(int n, int iFlag)
{
	__int64	iRes=0;
	__int64 iMul=1;
	if (!GetSource()) return 0;

	if (dwCurrBitPos>7)
	{
		dwCurrBitPos=7;
		LoadWord();
	}

	if (n == 8 && dwCurrBitPos == 7)
	{
		iRes = wData;
		LoadWord();
	} else
	if (n <= dwCurrBitPos+1)
	{
		int bitsLeftAfterwards = dwCurrBitPos - n + 1;
		if (iFlag == 0)
		{
			uint8 current_relevance_map = relevance_map_fl[dwCurrBitPos] & relevance_map_fr[7 - bitsLeftAfterwards];
			iRes = (wData & current_relevance_map) >> (bitsLeftAfterwards);
		}
		else
		{
			uint8 current_relevance_map = relevance_map_fr[dwCurrBitPos] & relevance_map_fl[7 - bitsLeftAfterwards];
			iRes = (wData & current_relevance_map) >> (7-dwCurrBitPos);
		}
		dwCurrBitPos -= n;
	} 
	else 
	{
		DWORD firstPart = dwCurrBitPos+1;
		DWORD secondPart = n-dwCurrBitPos-1;
		DWORD shiftCount = firstPart;

		uint64 firstResult = ReadBits64(firstPart, iFlag);
		uint64 secondResult = ReadBits64(secondPart, iFlag);

		if (iFlag == 1)
		{
			shiftCount = firstPart;
			return (secondResult << shiftCount) + firstResult; 
		}
		else
		{
			shiftCount = secondPart;
			return (secondResult) + (firstResult << shiftCount); 
		}

	}


	return iRes;
}