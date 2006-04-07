#include "stdafx.h"
#include "audiosource_generic.h"

	//////////////////////////
	// generic audio source //
	//////////////////////////

AUDIOSOURCE::AUDIOSOURCE()
{
	bSeamless = false;
}

AUDIOSOURCE::~AUDIOSOURCE()
{
}

int AUDIOSOURCE::doClose()
{
	return 0;
}

__int64 AUDIOSOURCE::GetExactSize()
{
	return 0;
}

int AUDIOSOURCE::GetFormatTag()
{
	return 0;
}

int AUDIOSOURCE::Read(void* lpDest, DWORD dwMicrosecDesired, DWORD* lpdwMicrosecRead,
					  __int64* lpqwNanosecRead, __int64* lpiTimeocode, ADVANCEDREAD_INFO* lpAARI) 
{
	return 0;
}

int AUDIOSOURCE::Seek(__int64 iPos)
{
	return 0;
}

int AUDIOSOURCE::SetFrameMode(DWORD dwMode)
{
	iFrameMode=dwMode;
	return 1;
}

int AUDIOSOURCE::GetFrameMode(void)
{
	return iFrameMode;
}

int AUDIOSOURCE::GetFrequency()
{
	return 0;
}

int AUDIOSOURCE::GetOutputFrequency()
{
	return GetFrequency();
}

int AUDIOSOURCE::GetChannelCount()
{
	return 0;
}

int AUDIOSOURCE::GetGranularity()
{
	return 0;
}

int AUDIOSOURCE::GetAvgBytesPerSec()
{
	return 0;
}

int AUDIOSOURCE::GetOffset()
{
	return 0;
}

int AUDIOSOURCE::GetType()
{
	return MMT_AUDIO;
}

int AUDIOSOURCE::JoinSeamless(bool _bSeamless)
{
	bSeamless = _bSeamless;
	return AS_OK;
}

bool AUDIOSOURCE::IsSeamless()
{
	return bSeamless;
}

__int64 AUDIOSOURCE::GetUnstretchedDuration()
{
	/*if (GetMaxLength())*/ return GetMaxLength();

//	return 0;
}

char* AUDIOSOURCE::GetIDString()
{
	return NULL;
}

int AUDIOSOURCE::GetBitDepth()
{
	return 0;
}

void* AUDIOSOURCE::GetFormat()
{
	return NULL;
}

bool AUDIOSOURCE::IsCBR()
{
	return false;
}

int AUDIOSOURCE::IsCompatible(AUDIOSOURCE* a)
{
	int i, j;
	__int64 k,l;

	k = FormatSpecific(MMSGFS_VORBIS_FRAMEDURATIONS);
	l = a->FormatSpecific(MMSGFS_VORBIS_FRAMEDURATIONS);

	if ((i=GetFormatTag()) && (j=a->GetFormatTag())) {
		if (i!=j || k!=l) return MMSIC_FORMATTAG;
	} else {
		if (strcmp(GetIDString(),a->GetIDString())) {
			return MMSIC_IDSTRING;
		}
	}

	if (a->GetFrequency() != GetFrequency()) return MMSIC_SAMPLERATE;

	return MMS_COMPATIBLE;
}

__int64 AUDIOSOURCE::GetFeature(int iFeature) 
{
	switch (iFeature)
	{
		case FEATURE_EXTRACTBIN: return 1; break;
		default: return MULTIMEDIASOURCE::GetFeature(iFeature);
	}
}