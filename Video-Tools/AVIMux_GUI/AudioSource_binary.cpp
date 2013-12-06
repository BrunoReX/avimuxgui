#include "stdafx.h"
#include "audiosource_binary.h"
#include "debug.h"


#ifdef DEBUG_NEW
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif


	///////////////////////////////////////////
	// audio source from binary input stream //
	///////////////////////////////////////////


CBinaryAudioSource::CBinaryAudioSource()
{
	source=NULL; 
	dwResync_Range = 131072; 
	bEndReached = 0;
	unstretched_duration = 0;
	SetTimecodeScale(1000);
}

CBinaryAudioSource::~CBinaryAudioSource()
{
	Close();
}

void CBinaryAudioSource::ReInit()
{
	if (!GetSource())
		return;

	GetSource()->InvalidateCache();
	Seek(0);
}

int CBinaryAudioSource::Close()
{
	return doClose();
}

int CBinaryAudioSource::Seek(__int64 qwPos)
{
	GetSource()->Seek(qwPos);
	if (GetAvgBytesPerSec()) {
		SetCurrentTimecode(qwPos * 1000000000 / GetAvgBytesPerSec(), TIMECODE_UNSCALED);
	}
	if (!qwPos) SetCurrentTimecode(0);
	bEndReached = 0;
	return STREAM_OK;
}

int CBinaryAudioSource::Open(STREAM* lpStream)
{
	 char	lpcName[256];

	 source=lpStream;
	 if (lpStream)
	 {
		 lpStream->GetName(lpcName);
		 if (lpcName && lpcName[0])
			 SetName(lpcName);
	 }

	 return (lpStream)?AS_OK:AS_ERR; 
}

int CBinaryAudioSource::doRead(MULTIMEDIA_DATA_PACKET** dataPacket)
{
	int pos = 0;

	if (GetFrameMode() == FRAMEMODE_SINGLEFRAMES)
		return ReadFrame(dataPacket);

	createMultimediaDataPacket(dataPacket);

	int numberOfPackets = GetFrameMode();
	MULTIMEDIA_DATA_PACKET** packets = new MULTIMEDIA_DATA_PACKET*[GetFrameMode()];
	memset(packets, 0, numberOfPackets * sizeof(void*));
	
	for (int j=0; j<numberOfPackets; j++) {
		if (ReadFrame(&packets[j]) < 0) {
			/* error during reading ... */
			numberOfPackets = j;
		}
	}

	MULTIMEDIA_DATA_PACKET* firstPacket = packets[0];
	MULTIMEDIA_DATA_PACKET* lastPacket = packets[numberOfPackets-1];

	(*dataPacket)->timecode = firstPacket->timecode;
	(*dataPacket)->duration = lastPacket->duration + 
		(lastPacket->timecode - firstPacket->timecode);
	(*dataPacket)->frameSizes.clear();

	for (int j=0; j<numberOfPackets; j++) {
		(*dataPacket)->totalDataSize += packets[j]->totalDataSize;
		(*dataPacket)->frameSizes.push_back(packets[j]->totalDataSize);
	}
	
	(*dataPacket)->cData = (char*)malloc((*dataPacket)->totalDataSize);
	
	for (int j=0; j<numberOfPackets; j++) {
		memcpy((*dataPacket)->cData + pos, packets[j]->cData,
			packets[j]->totalDataSize);
		pos += packets[j]->totalDataSize;
		freeMultimediaDataPacket(packets[j]);
	}
	delete[] packets;
	
	(*dataPacket)->flags = 0;
	(*dataPacket)->compressionInfo.clear();

	return 0;
}

int CBinaryAudioSource::Read(void* lpDest, DWORD dwMicroSecDesired,
								DWORD* lpdwMicrosecRead,
								__int64* lpqwNanosecRead, 
								__int64* lpiTimecode, 
								ADVANCEDREAD_INFO* lpAARI)
{
	__int64	iNanosecRead, iCTC;
	
	if (lpiTimecode) *lpiTimecode = iCTC = GetCurrentTimecode();

	int	iRead = -1;
	
	if (!IsEndOfStream())
		iRead = doRead(lpDest,dwMicroSecDesired,lpdwMicrosecRead,&iNanosecRead);
	if (iRead <= 0) bEndReached = 1;

	if (lpqwNanosecRead) *lpqwNanosecRead = iNanosecRead;

	__int64 iLTC = -1000;
	
	IncCurrentTimecode(iNanosecRead);
	if (lpAARI) lpAARI->iNextTimecode = GetCurrentTimecode();

	if (IsEndOfStream())
		unstretched_duration = GetCurrentTimecode() - GetBias();

	return iRead;
}

int CBinaryAudioSource::Read(MULTIMEDIA_DATA_PACKET** dataPacket)
{
	return doRead(dataPacket);
}

__int64 CBinaryAudioSource::GetUnstretchedDuration()
{
	if (GetMaxLength())
		return GetMaxLength();

	return unstretched_duration;
}

int CBinaryAudioSource::doClose()
{ 
	source=NULL; 
	if (lpcName) {
		delete lpcName;
		lpcName = NULL;
	}

	return AS_OK; 
}

__int64 CBinaryAudioSource::GetExactSize()
{
	return GetSource()->GetSize();
}

bool CBinaryAudioSource::IsEndOfStream()
{
	return (/*GetMaxLength() <= GetCurrentTimecode()-GetBias() || */bEndReached || source->IsEndOfStream()); 
}

int CBinaryAudioSource::GetAvgBytesPerSec()
{
	int i = source->GetAvgBytesPerSec();
	if (i)
		return i;

	__int64 duration = GetDuration() * GetTimecodeScale() / 1000000000;

	return (int)((double)GetSize()/(double)duration);

}

int CBinaryAudioSource::GetChannelCount()
{
	 return GetSource()->GetChannels();
}

int CBinaryAudioSource::GetFrequency()
{
	 return GetSource()->GetFrequency();
}

void CBinaryAudioSource::SetResyncRange(DWORD dwRange)
{
	dwResync_Range = dwRange;
}

int CBinaryAudioSource::GetResyncRange()
{ 
	return dwResync_Range; 
}

int CBinaryAudioSource::GetOffset()
{
	return (GetSource())?GetSource()->GetOffset():0;
}

bool CBinaryAudioSource::IsCBR()
{
	return false;
}

	//////////////////////////////
	// general CBR audio source //
	//////////////////////////////
/*
int CBRAUDIOSOURCE::doRead(void* lpDest,DWORD dwMicroSecDesired,DWORD* lpdwMicroSecRead,__int64* lpqwNanoSecRead)
{
	DWORD	dwBytes,dwAdd;

	if (GetGranularity==0) 
	{
		// variable Granularität bei CBR-Audio nicht zulässig!
		*lpdwMicroSecRead=0;
		return 0;
	}
	dwBytes=(DWORD)((__int64)dwMicroSecDesired*(__int64)GetAvgBytesPerSec()/1000000);
	dwAdd=(dwBytes%GetGranularity())?1:0;
	dwBytes/=GetGranularity();
	dwBytes+=dwAdd;
	dwBytes*=GetGranularity();

	dwBytes=GetSource()->Read(lpDest,dwBytes);
	if (lpdwMicroSecRead) 
	{
		*lpdwMicroSecRead=(DWORD)round(1000000*d_div(dwBytes,GetAvgBytesPerSec(),"CBRAUDIOSOURCE::Read: GetAvgBytesPerSec()"));
	}
	if (lpqwNanoSecRead) 
	{
		*lpqwNanoSecRead=round(1000000000*d_div(dwBytes,GetAvgBytesPerSec(),"CBRAUDIOSOURCE::Read: GetAvgBytesPerSec()"));
	}

	return dwBytes;
}

int CBRAUDIOSOURCE::doClose()
{
	 return AUDIOSOURCE::doClose();
}

*/