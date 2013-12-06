#include "stdafx.h"
#include "audiosource_avi.h"
#include "..\utf-8.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

	/////////////////////////////////////
	// audio source from an AVI stream //
	/////////////////////////////////////

AUDIOSOURCEFROMAVI::AUDIOSOURCEFROMAVI()
{
	ZeroMemory(&info,sizeof(info));
}

AUDIOSOURCEFROMAVI::AUDIOSOURCEFROMAVI(AVIFILEEX* avifile, int iStream)
{
	ZeroMemory(&info,sizeof(info));
	Open(avifile,iStream);
}

AUDIOSOURCEFROMAVI::~AUDIOSOURCEFROMAVI()
{
}

int AUDIOSOURCEFROMAVI::Open(AVIFILEEX* avifile, int iStream)
{
	info.avifile = avifile;
	info.iStream = iStream;

	if (!info.avifile->IsAudioStream(info.iStream)) return AS_ERR;
	SetMaxLength(info.avifile->GetNanoSecPerFrame() * info.avifile->GetFrameCount());
	SetDefault(!(info.avifile->GetStreamHeader(info.iStream)->dwFlags & AVISF_DISABLED));

	return AS_OK;
}

int AUDIOSOURCEFROMAVI::GetAvgBytesPerSec()
{
	return info.avifile->GetAvgBytesPerSec(info.iStream);
}

bool AUDIOSOURCEFROMAVI::IsEndOfStream()
{
	return info.avifile->IsEndOfStream(info.iStream);
}

int AUDIOSOURCEFROMAVI::GetChannelCount()
{
	return info.avifile->GetChannels(info.iStream);
}

int AUDIOSOURCEFROMAVI::GetFrequency()
{
	return ((WAVEFORMATEX*)info.avifile->GetStreamFormat(info.iStream))->nSamplesPerSec;
}

int AUDIOSOURCEFROMAVI::GetBitDepth()
{
	return ((WAVEFORMATEX*)info.avifile->GetStreamFormat(info.iStream))->wBitsPerSample;
}

int AUDIOSOURCEFROMAVI::GetFormatTag()
{
	return info.avifile->GetFormatTag(info.iStream);
}

int AUDIOSOURCEFROMAVI::GetGranularity()
{
	return ((WAVEFORMATEX*)info.avifile->GetStreamFormat(info.iStream))->nBlockAlign;
}

int AUDIOSOURCEFROMAVI::GetOffset()
{
	return 0;
}

__int64 AUDIOSOURCEFROMAVI::GetExactSize()
{
	return info.avifile->GetStreamSize(info.iStream);
}

int AUDIOSOURCEFROMAVI::Seek(__int64 iPos)
{
	return info.avifile->SeekByteStream(info.iStream,iPos);
}

void AUDIOSOURCEFROMAVI::ReInit()
{
	info.avifile->GetSource()->InvalidateCache();
	Seek(0);
}

int AUDIOSOURCEFROMAVI::Read(void* lpDest, DWORD dwMicrosecDesired, DWORD* lpdwMicrosecRead, __int64* lpqwNanosecRead,
							 __int64* lpiTimecode, ADVANCEDREAD_INFO* lpAARI)
{
	__int64 iNanosec;
	DWORD	dwMS;
	DWORD	dwRead;
	
	if (GetFrameMode() == FRAMEMODE_SINGLEFRAMES) {
		dwMS = 1000*GetGranularity()/GetAvgBytesPerSec();
	} else dwMS = dwMicrosecDesired/1000;

	dwRead = info.avifile->GetAudioData(info.iStream,dwMS,lpDest);

	iNanosec = (1000000000*(__int64)(dwRead)/GetAvgBytesPerSec());
	if (lpdwMicrosecRead) *lpdwMicrosecRead=(DWORD)(1000000*(__int64)(dwRead)/GetAvgBytesPerSec());
	if (lpqwNanosecRead) *lpqwNanosecRead=iNanosec;

	if (lpiTimecode) *lpiTimecode = GetCurrentTimecode();
	IncCurrentTimecode(iNanosec);
	if (lpAARI) lpAARI->iNextTimecode = GetCurrentTimecode();

	return dwRead;
}

int AUDIOSOURCEFROMAVI::doClose()
{
	info.avifile = 0;
	info.iStream = -1;

	return AS_OK;
}

int AUDIOSOURCEFROMAVI::GetName(char* lpDest)
{
	char cTemp[256]; cTemp[0] = 0;
	info.avifile->GetStreamName(info.iStream,cTemp);
	Str2UTF8(cTemp, cTemp);
	strcpy(cTemp, lpDest);
	return (strlen(cTemp));
}

bool AUDIOSOURCEFROMAVI::IsCBR()
{
	return true;
}
