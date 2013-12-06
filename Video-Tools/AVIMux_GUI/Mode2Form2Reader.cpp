#include "stdafx.h"
#include "Mode2Form2Reader.h"
#include "ecc.h"

int	MODE2FORM2SOURCE::Open(STREAM* lpSource)
{
	LISTHEADER		lhHdr;
	CHUNKHEADER		chHdr;

	if (!lpSource) 
		return STREAM_ERR;
	
	dwPosition=0;
	dwSectors=0;
	CheckCRC(false);
	SetSource(lpSource);

	lpSource->Seek(0);
	lpSource->Read(&lhHdr,12);

	if ((lhHdr.dwListID!=MakeFourCC("RIFF"))||(lhHdr.dwFourCC!=MakeFourCC("CDXA")))
	{
		lpSource->Seek(0);
		SetSource(NULL);
		return STREAM_ERR;
	}
	if (!LocateData(MakeFourCC("data"),NULL,NULL,&chHdr,2000,DT_CHUNK))
	{
		lpSource->Seek(0);
		SetSource(NULL);
		return STREAM_ERR;
	};
	dwSectors=chHdr.dwLength/2352;

	dwDataStart=44;
	Seek(0);
	return STREAM_OK;
}

__int64 MODE2FORM2SOURCE::GetSize()
{
	return dwSectors*2324;
}

__int64 MODE2FORM2SOURCE::GetPos()
{
	return dwPosition-dwDataStart;
}

int MODE2FORM2SOURCE::Seek(__int64 qwPos)
{
	if (qwPos>=GetSize()) return STREAM_ERR;
	dwPosition=(DWORD)qwPos+dwDataStart;
	return STREAM_OK;
}

int MODE2FORM2SOURCE::GetGranularity()
{
	return 1;
}

int MODE2FORM2SOURCE::CheckCRC(bool bCCRC)
{
	bCheckCRC=bCCRC;
	return STREAM_OK;
}

int MODE2FORM2SOURCE::ReadRAWSector(RAWSECTOR* lpDest,DWORD* lpdwCRC_OK)
{
	DWORD	dwRead;
	DWORD	result;
	BYTE*	lpbDest=(BYTE*)lpDest;

	if (GetSource())
	{
		GetSource()->Seek(dwDataStart+2352*(GetPos()/2324));
		dwRead=GetSource()->Read(lpDest,2352);

		if (dwRead!=2352) return 0;

		if (lpdwCRC_OK) {
			if (bCheckCRC) {
				result = build_edc((unsigned char*)lpDest, 16, 16+8+2324-1);
				*lpdwCRC_OK=(result==(*(DWORD*)&(lpbDest[2348])))?1:0;
			} else {
				*lpdwCRC_OK=1;
			}
		}
	}
	 
	return dwRead;
}

int MODE2FORM2SOURCE::Close()
{
	STREAM* s = NULL;

	if (s = GetSource()) {
		s->Close();
		delete s;
	}

	SetSource(NULL);
	return STREAM_OK;
}

int MODE2FORM2SOURCE::ReadPartialSector(DWORD dwNbr,DWORD dwOffset,DWORD dwLength,void* lpDest)
{
	RAWSECTOR	raw;
	BYTE*		lpbDest=(BYTE*)lpDest;
	DWORD		dwCRC_OK;

	dwCRC_OK=1;
	if (dwLength+dwOffset>=2324) dwLength=2324-dwOffset;
	if (!ReadRAWSector(&raw,(bCheckCRC)?&dwCRC_OK:NULL)) return 0;

	if (dwCRC_OK)
	{
		memcpy(lpDest,&(raw.data[dwOffset]),dwLength);
		dwPosition+=dwLength;
	}
	else
	{
		dwLength=0;
	}
	
	return dwLength;
}

int MODE2FORM2SOURCE::Read(void* lpDest,DWORD dwBytes)
{
	BYTE*	lpbDest=(BYTE*)lpDest;
	DWORD	dwTotal=0;
	DWORD	dwRead=1;

	while ((dwRead)&&(dwTotal<dwBytes))
	{
		DWORD	dwSector=(DWORD)(GetPos()/2324);
		DWORD	dwOffset=(DWORD)(GetPos()%2324);
		dwTotal+=(dwRead=ReadPartialSector(dwSector++,dwOffset,dwBytes-dwTotal,&(lpbDest[dwTotal])));
//		dwOffset=0;
	}
	
	return dwTotal;
}