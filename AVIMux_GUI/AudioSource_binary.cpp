#include "stdafx.h"
#include "audiosource_binary.h"
#include "debug.h"

	//////////////////////
	// audio cue points //
	//////////////////////

CUEPOINTS::CUEPOINTS()
{
	points = new CDynIntArray;
}

void CUEPOINTS::AddPoint(__int64 iTimecode, __int64 iPosition)
{
	AUDIO_CUEPOINT* p = new AUDIO_CUEPOINT;
	p->iTimecode = iTimecode;
	p->iStreamPosition = iPosition;

	points->Insert((int)p);
}

int CUEPOINTS::FindClosestPoint(__int64 iTimecode, __int64* piTimecode, __int64* piPosition)
{
	AUDIO_CUEPOINT* p[3];
	
	if (!points->GetCount()) {
		return -1;
	}

	if (!piTimecode && !piPosition) {
		return -1;
	}

	p[2] = (AUDIO_CUEPOINT*)points->At(points->GetCount()-1);
	if (p[2]->iTimecode<iTimecode) {
		if (piTimecode) *piTimecode = p[2]->iTimecode;
		if (piPosition) *piPosition = p[2]->iStreamPosition;
		return 1;
	}

	p[0] = (AUDIO_CUEPOINT*)points->At(0);
	if (p[0]->iTimecode>iTimecode) {
		return -1;
	}

	int  iMin, iMax, iMid;
	iMin = 0;
	iMax = points->GetCount()-1;

	while (iMax - iMin > 1) {
		iMid = (iMin + iMax) >> 1;
		p[1] = (AUDIO_CUEPOINT*)points->At(iMid);

		if (p[0]->iTimecode > iTimecode && p[1]->iTimecode < iTimecode) {
			p[2] = p[1];
			iMax = iMid;
		} else {
			p[0] = p[1];
			iMin = iMid;
		}
	}

	if (piTimecode) *piTimecode = p[0]->iTimecode;
	if (piPosition) *piPosition = p[0]->iStreamPosition;

	return 0;
}

void CUEPOINTS::Delete()
{
	for (int i=points->GetCount()-1;i>=0;i--) {
		delete ((AUDIO_CUEPOINT*)points->At(i));
	}

	points->DeleteAll();
	delete points;
}


	///////////////////////////////////////////
	// audio source from binary input stream //
	///////////////////////////////////////////


AUDIOSOURCEFROMBINARY::AUDIOSOURCEFROMBINARY()
{
	source=NULL; 
	SetName(NULL); 
	dwResync_Range = 131072; 
	cues = NULL;
	bEndReached = 0;
	SetTimecodeScale(1000);
}

int AUDIOSOURCEFROMBINARY::Seek(__int64 qwPos)
{
	GetSource()->Seek(qwPos);
	if (GetAvgBytesPerSec()) {
		SetCurrentTimecode(qwPos * 1000000000 / GetAvgBytesPerSec(), TIMECODE_UNSCALED);
	}
	if (!qwPos) SetCurrentTimecode(0);
	bEndReached = 0;
	return STREAM_OK;
}

int AUDIOSOURCEFROMBINARY::Open(STREAM* lpStream)
{
	 char	lpcName[256];

	 source=lpStream;
	 if (lpStream)
	 {
		 lpStream->GetName(lpcName);
		 SetName(lpcName);
	 }

	 cues = new CUEPOINTS;

	 return (lpStream)?AS_OK:AS_ERR; 
}


int AUDIOSOURCEFROMBINARY::Read(void* lpDest,DWORD dwMicroSecDesired,DWORD* lpdwMicrosecRead,
					  __int64* lpqwNanosecRead, __int64* lpiTimecode, ADVANCEDREAD_INFO* lpAARI)
{
	__int64	iNanosecRead, iCTC;
	
	if (lpiTimecode) *lpiTimecode = iCTC = GetCurrentTimecode();
	int	iRead = doRead(lpDest,dwMicroSecDesired,lpdwMicrosecRead,&iNanosecRead);
	if (!iRead) bEndReached = 1;

	if (lpqwNanosecRead) *lpqwNanosecRead = iNanosecRead;

	__int64 iLTC = -1000;
	
/*	if (cues && (cues->FindClosestPoint(iCTC,&iLTC,NULL)==-1 || iCTC - iLTC > 1000)) {
		cues->AddPoint(iCTC,GetSource()->GetPos());
	}
*/
	IncCurrentTimecode(iNanosecRead);
	if (lpAARI) lpAARI->iNextTimecode = GetCurrentTimecode();

	return iRead;
}

int AUDIOSOURCEFROMBINARY::doClose()
{ 
	source=NULL; 
	if (lpcName) {
		delete lpcName;
		lpcName = NULL;
	}

	if (cues) {
		cues->Delete();
		delete cues;
	}

	return AS_OK; 
}

__int64 AUDIOSOURCEFROMBINARY::GetExactSize()
{
	return GetSource()->GetSize();
}

bool AUDIOSOURCEFROMBINARY::IsEndOfStream()
{
	return (/*GetMaxLength() <= GetCurrentTimecode()-GetBias() || */bEndReached || source->IsEndOfStream()); 
}

int AUDIOSOURCEFROMBINARY::GetAvgBytesPerSec()
{
	return source->GetAvgBytesPerSec();
}

int AUDIOSOURCEFROMBINARY::GetChannelCount()
{
	 return GetSource()->GetChannels();
}

int AUDIOSOURCEFROMBINARY::GetFrequency()
{
	 return GetSource()->GetFrequency();
}

void AUDIOSOURCEFROMBINARY::SetResyncRange(DWORD dwRange)
{
	dwResync_Range = dwRange;
}

int AUDIOSOURCEFROMBINARY::GetResyncRange()
{ 
	return dwResync_Range; 
}

int AUDIOSOURCEFROMBINARY::GetOffset()
{
	return (GetSource())?GetSource()->GetOffset():0;
}

bool AUDIOSOURCEFROMBINARY::IsCBR()
{
	return false;
}

	//////////////////////////////
	// general CBR audio source //
	//////////////////////////////

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

