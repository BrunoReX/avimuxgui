#include "stdafx.h"
#include "RIFFFile.h"
#include "WAVFile.h"
#include "math.h"

WAVEFILE::WAVEFILE()
{
	InitValues();
}

void WAVEFILE::InitValues()
{
	dwAccess=NULL;
	dwStreamSize=NULL;
	SetSource(NULL);
	bOpen=false;
	dwCurrentPos=0;
}

int WAVEFILE::GetAvgBytesPerSec()
{
	if (lpwfe)
	{
		return lpwfe->nAvgBytesPerSec;
	}
	else
	{
		return 0;
	}
}

bool WAVEFILE::IsEndOfStream()
{
	return ((GetPos()>=GetSize())?true:false);
}

int WAVEFILE::GetGranularity()
{
	if (lpwfe)
		return lpwfe->nBlockAlign;
	
	return 0;
}

WAVEFILE::~WAVEFILE()
{

}

int WAVEFILE::Open(STREAM* lpStream,DWORD _dwAccess)
{
	CHUNKHEADER	chhdr;
	DWORD		dwHdrLen;
	if (bOpen) return WAV_ERR;

	if (_dwAccess==FA_READ)
	{
		if (!lpStream) return WAV_ERR;
		SetSource(lpStream);
		if (CheckRIFF_WAVE()!=WAV_OK) {
			Close();
			return WAV_ERR;
		}
		if (!(LocateData(MakeFourCC("fmt "),NULL,NULL,&chhdr,1000000,DT_CHUNK))) {
			Close();
			return WAV_ERR;
		}
		dwHdrLen=max(sizeof(WAVEFORMATEX),chhdr.dwLength);
		lpwfe=(WAVEFORMATEX*)malloc(dwHdrLen);
		ZeroMemory(lpwfe,dwHdrLen);
		GetSource()->Read(lpwfe,chhdr.dwLength);
		if (!LocateData(MakeFourCC("data"),NULL,NULL,&chhdr,1000000,DT_CHUNK))
		{
			Close();
			return WAV_ERR;
		}
		dwStreamSize=chhdr.dwLength;
		dwDataStart=(DWORD)(GetSource()->GetPos());
		dwAccess=FA_READ;
	}
	return WAV_OK;
}

int WAVEFILE::Read(void* lpDest, DWORD dwBytes)
{
	if (dwAccess!=FA_READ) return 0;
	DWORD dwRead=GetSource()->Read(lpDest,dwBytes);
	dwCurrentPos+=dwRead;
	return dwRead;
}

WAVEFORMATEX* WAVEFILE::GetStreamFormat()
{
	return lpwfe;
}

int WAVEFILE::Close()
{
	if (!bOpen) 
		return WAV_ERR;

/*	if (GetSource()) {
		GetSource()->Close();
		delete GetSource();
	}
*/
	if (lpwfe) free(lpwfe);
	InitValues();
	return WAV_OK;
}

__int64 WAVEFILE::GetSize()
{
	return dwStreamSize;
}

__int64 WAVEFILE::GetPos()
{
	return dwCurrentPos;
}

int WAVEFILE::Seek(__int64 qwPos)
{
	GetSource()->Seek(qwPos+dwDataStart);
	dwCurrentPos=(DWORD)qwPos;
	return WAV_OK;
}

int WAVEFILE::CheckRIFF_WAVE()
{
	LISTHEADER	lhdr;

	GetSource()->Seek(0);
	GetSource()->Read(&lhdr,12);
	if (lhdr.dwFourCC!=MakeFourCC("WAVE")) return WAV_ERR;
	if (lhdr.dwListID!=MakeFourCC("RIFF")) return WAV_ERR;

	return WAV_OK;
}