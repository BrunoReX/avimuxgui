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
	Close();
}

int WAVEFILE::Open(STREAM* lpStream,DWORD _dwAccess)
{
	CHUNKHEADER	chhdr;
	DWORD		dwHdrLen;
	if (bOpen) return WAVE_OPEN_ERROR;

	if (_dwAccess != WAVE_OPEN_READ && _dwAccess != WAVE_OPEN_ERROR)
		return WAVE_OPEN_ERROR;

	if (_dwAccess == WAVE_OPEN_READ)
	{
		if (!lpStream) 
			return WAVE_OPEN_ERROR;

		SetSource(lpStream);

		if (CheckRIFF_WAVE() <= WAVE_GENERIC_ERROR) {
			Close();
			return WAVE_OPEN_ERROR;
		}
		if (!(LocateData(MakeFourCC("fmt "),NULL,NULL,&chhdr,1000000,DT_CHUNK))) {
			Close();
			return WAVE_OPEN_ERROR;
		}
		dwHdrLen=max(sizeof(WAVEFORMATEX),chhdr.dwLength);
		lpwfe=(WAVEFORMATEX*)malloc(dwHdrLen);
		ZeroMemory(lpwfe,dwHdrLen);
		GetSource()->Read(lpwfe,chhdr.dwLength);
		if (!LocateData(MakeFourCC("data"),NULL,NULL,&chhdr,1000000,DT_CHUNK)) {
			Close();
			return WAVE_OPEN_ERROR;
		}
		dwStreamSize=chhdr.dwLength;
		dwDataStart=(DWORD)(GetSource()->GetPos());
		//dwAccess=WAVE_OPEN_ERROR;
		dwAccess = _dwAccess;
	}

	return WAVE_GENERIC_OK;
}

int WAVEFILE::Read(void* lpDest, DWORD dwBytes)
{
	if (dwAccess != WAVE_OPEN_READ) 
		return WAVE_READ_ERROR;
	
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
		return WAVE_GENERIC_ERROR;

	if (lpwfe) {
		free(lpwfe);
		lpwfe = NULL;
	}

	InitValues();

	return WAVE_GENERIC_OK;
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
	return WAVE_GENERIC_OK;
}

int WAVEFILE::CheckRIFF_WAVE()
{
	LISTHEADER	lhdr;

	GetSource()->Seek(0);
	GetSource()->Read(&lhdr,12);
	if (lhdr.dwFourCC!=MakeFourCC("WAVE")) return WAVE_HEADER_NOT_FOUND;
	if (lhdr.dwListID!=MakeFourCC("RIFF")) return WAVE_HEADER_NOT_FOUND;

	return WAVE_GENERIC_OK;
}