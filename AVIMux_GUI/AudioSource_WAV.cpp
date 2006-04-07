#include "stdafx.h"
#include "AudioSource_WAV.h"

WAVSOURCE::WAVSOURCE()
{
	wavefile = NULL;
	SetTimecodeScale(1000);
}

WAVSOURCE::~WAVSOURCE()
{
}

int WAVSOURCE::Open(WAVEFILE* source)
{
	if (!source || source->GetSize() <= 0)
		return AS_ERR;

	wavefile = source;

	Seek(0);

	return AS_OK;
}

WAVEFILE* WAVSOURCE::GetSource()
{
	return wavefile;
}

int WAVSOURCE::GetAvgBytesPerSec()
{
	if (!GetSource())
		return -1;

	WAVEFORMATEX* w = GetSource()->GetStreamFormat();

	if (!w)
		return -1;

	return w->nAvgBytesPerSec;
}

int WAVSOURCE::GetChannelCount()
{
	WAVEFORMATEX* w = GetWAVHeader();

	if (!w)
		return 0;

	return w->nChannels;
}

__int64 WAVSOURCE::GetExactSize()
{
	if (!GetSource())
		return -1;

	return GetSource()->GetSize();
}

void* WAVSOURCE::GetFormat()
{
	if (!GetSource())
		return 0;

	WAVEFORMATEX* w = GetSource()->GetStreamFormat();

	return w;
}

int WAVSOURCE::GetFormatTag()
{
	WAVEFORMATEX* w = GetWAVHeader();

	if (!w)
		return 0;

	return w->wFormatTag;
}

int WAVSOURCE::GetFrequency()
{
	WAVEFORMATEX* w = GetWAVHeader();

	if (!w)
		return 0;

	return w->nSamplesPerSec;
}

int WAVSOURCE::GetGranularity()
{
	WAVEFORMATEX* w = GetWAVHeader();

	if (!w)
		return 0;

	return w->nBlockAlign;
}

__int64 WAVSOURCE::GetUnstretchedDuration()
{
	WAVEFORMATEX* w = GetWAVHeader();

	if (!w)
		return 0;

	return (GetSize() * 1000000000 / GetAvgBytesPerSec() / GetTimecodeScale());	
}

bool WAVSOURCE::IsCBR()
{
	if (!GetSource())
		return false;

	return true;
}

int WAVSOURCE::IsCompatible(AUDIOSOURCE* a)
{
	WAVEFORMATEX* w = GetWAVHeader();

	if (!w)
		return 0;

	if (a->GetFormatTag() != GetFormatTag())
		return MMSIC_FORMATTAG;

	if (a->GetChannelCount() != GetChannelCount())
		return MMSIC_CHANNELS;

	if (a->GetFrequency() != GetFrequency())
		return MMSIC_SAMPLERATE;

	return MMS_COMPATIBLE;
}


WAVEFORMATEX* WAVSOURCE::GetWAVHeader()
{
	return (WAVEFORMATEX*)GetFormat();
}

int WAVSOURCE::GetBitDepth()
{
	WAVEFORMATEX* w = GetWAVHeader();

	if (!w)
		return -1;

	return w->wBitsPerSample;
}

int WAVSOURCE::Read(void* lpDEST, DWORD dwMicroSecDesired, DWORD* lpdwMicrosecRead,
					__int64* lpqwNanosecRead, __int64* lpiTimecode, ADVANCEDREAD_INFO* lpAARI)
{
	int bytes_requested = (int)((double)dwMicroSecDesired * (double)GetAvgBytesPerSec() / 1000000.);
	
	bytes_requested /= GetGranularity();
	bytes_requested *= GetGranularity();

	if (!bytes_requested)
		bytes_requested = GetGranularity();

	int bytes_read;

	bytes_read = GetSource()->Read(lpDEST, bytes_requested);

	__int64 nanosec = (__int64)(1000000000. * (double)bytes_read / GetAvgBytesPerSec());
	
	if (lpdwMicrosecRead)
		*lpdwMicrosecRead = (DWORD)((nanosec+500) / 1000);

	if (lpqwNanosecRead)
		*lpqwNanosecRead = nanosec;

	if (lpiTimecode)
		*lpiTimecode = GetCurrentTimecode();

	if (lpAARI) {
		lpAARI->iDuration = nanosec / GetTimecodeScale();
		lpAARI->iNextTimecode = GetCurrentTimecode() + lpAARI->iDuration;
		if (GetSource()->IsEndOfStream())
			lpAARI->iFileEnds = 1;
		else 
			lpAARI->iFileEnds = 0;
	}

	IncCurrentTimecode(nanosec);


	return bytes_read;
}

int WAVSOURCE::Seek(__int64 iPos)
{
	if (!GetSource())
		return 0;

	int datarate = GetAvgBytesPerSec();

	SetCurrentTimecode((__int64)((double)iPos * 1000000000 / datarate), TIMECODE_UNSCALED);

	return GetSource()->Seek(iPos);
}

bool WAVSOURCE::IsEndOfStream()
{
	if (!GetSource())
		return true;

	return GetSource()->IsEndOfStream();
}