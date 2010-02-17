#include "stdafx.h"
#include "avifile.h"
#include "avistream.h"

// AVISTREAM

AVISTREAM::AVISTREAM(AVIFILEEX* avifile,int iStreamNbr)
{
	STREAM::Open(StreamMode::Read);
	AVIFile = avifile; 
	dwStreamNbr = iStreamNbr;
}

int AVISTREAM::Open(AVIFILEEX* _AVIFile,DWORD _dwStreamNbr)
{
	STREAM::Open(StreamMode::Read);

	if (!_dwStreamNbr) return STREAM_ERR;
	AVIFile=_AVIFile;
	dwStreamNbr=_dwStreamNbr;
//	SetDefault(!(AVIFile->GetStreamHeader(_dwStreamNbr)->dwFlags & AVISF_DISABLED));

	return STREAM_OK;
}

bool AVISTREAM::IsEndOfStream()
{
	return AVIFile->IsEndOfStream(dwStreamNbr);
}

int AVISTREAM::Close(void)
{
	dwStreamNbr=0;
	AVIFile=NULL;
	return STREAM_OK;
}

__int64 AVISTREAM::GetSize(void)
{
	return AVIFile->GetStreamSize(dwStreamNbr);
}

int AVISTREAM::Read(void* lpDest,DWORD dwBytes)
{
	if (GetMode()!=StreamMode::Read) return STREAM_ERR;
	return (AVIFile->LoadAudioData(dwStreamNbr,dwBytes,lpDest));
}

int AVISTREAM::Seek(__int64 qwPos)
{
	return (AVIFile->SeekByteStream(dwStreamNbr,qwPos + GetOffset())==AFE_OK)?STREAM_OK:STREAM_ERR;
}

int AVISTREAM::GetAvgBytesPerSec(void)
{
	return (((WAVEFORMATEX*)(AVIFile->GetStreamFormat(dwStreamNbr)))->nAvgBytesPerSec);
}

__int64 AVISTREAM::GetPos(void)
{
	return AVIFile->GetByteStreamPos(dwStreamNbr)-GetOffset();
}

int AVISTREAM::SetName(char* lpcName)
{
	return AVIFile->SetStreamName(dwStreamNbr,lpcName);
}

int AVISTREAM::GetName(char* lpcName)
{
	char cTemp[256]; cTemp[0] = 0;
	AVIFile->GetStreamName(dwStreamNbr,lpcName);
	return 1;
}

