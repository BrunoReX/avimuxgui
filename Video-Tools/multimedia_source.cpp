#include "stdafx.h"
#include "multimedia_source.h"
#include "math.h"
#include "memory.h"
#include "stdlib.h"
#include "string.h"


#ifdef DEBUG_NEW
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

MULTIMEDIASOURCE::MULTIMEDIASOURCE()
{
	memset(&info,0,sizeof(info));
	lpcName = NULL;
	lpcLangCode = NULL;
	info.iTimecodeScale = 1000000;
	info.iMaxLength = 0;
	info.dStretchFactor = 1;
	size_guesser = new CSizeGuesser;
	info.bAVIOutputPossible = true;
	accumulatedDelay = 0;
	delayMMS = NULL;
}

__int64 MULTIMEDIASOURCE::GetAccumulatedDelay()
{
	if (delayMMS)
		return delayMMS->GetAccumulatedDelay();
	else
		return accumulatedDelay;
}

void MULTIMEDIASOURCE::AddToAccumulatedDelay(__int64 valueToAdd)
{
	if (delayMMS)
		delayMMS->AddToAccumulatedDelay(valueToAdd);
	else
		accumulatedDelay += valueToAdd;
}

void MULTIMEDIASOURCE::SetDelayMMS(MULTIMEDIASOURCE* delayMMS)
{
	this->delayMMS = delayMMS;
}

MULTIMEDIASOURCE::~MULTIMEDIASOURCE()
{
	delete size_guesser;
//	if (lpcName)
//		free (lpcName);
}

void MULTIMEDIASOURCE::SetCompressionAlgo(int algo)
{
	info.iCompression = algo;
}

int MULTIMEDIASOURCE::GetCompressionAlgo()
{
	return info.iCompression;
}

__int64 MULTIMEDIASOURCE::GetCurrentTimecode() // scaled
{
	return (DoStretch(info.iCurrentTimecode + info.iBias)/GetTimecodeScale());
}

void MULTIMEDIASOURCE::IncCurrentTimecode(__int64 iTime)  // not scaled!!!
{
	info.iCurrentTimecode += iTime;
}

void MULTIMEDIASOURCE::SetCurrentTimecode(__int64 iTime, int iFlags)
{
	info.iCurrentTimecode = iTime*((iFlags & TIMECODE_UNSCALED)?1:GetTimecodeScale()); // scaled
}

void MULTIMEDIASOURCE::AllowAVIOutput(bool bAllow)
{
	info.bAVIOutputPossible = bAllow;
}

bool MULTIMEDIASOURCE::IsAVIOutputPossible()
{
	return info.bAVIOutputPossible;
}

void MULTIMEDIASOURCE::SetBias(__int64 iBias, int iFlags)
{
	__int64 iScale = (iFlags & BIAS_UNSCALED)?1:info.iTimecodeScale;
	if (iFlags & BIAS_ABSOLUTE) {
		info.iBias = iBias * iScale;
	} else 
	if (iFlags & BIAS_RELATIVE) {
		info.iBias += iBias * iScale;
	} else {
		info.iBias = iBias * iScale;
	}
}

void MULTIMEDIASOURCE::SetTimecodeScale(__int64 iScale)
{
	info.iTimecodeScale = iScale;
}

__int64 MULTIMEDIASOURCE::GetTimecodeScale()
{
	return info.iTimecodeScale;
}

void MULTIMEDIASOURCE::SetMaxLength(__int64 iLength, int iFlags)
{
	if (iFlags & TIMECODE_UNSCALED) {
		info.iMaxLength = iLength;
	} else info.iMaxLength = iLength * GetTimecodeScale();
}

__int64 MULTIMEDIASOURCE::GetMaxLength()
{
	return info.iMaxLength / GetTimecodeScale();
}

void MULTIMEDIASOURCE::SetDefault(int bDefault)
{
	info.bDefault = !!bDefault;
}

int MULTIMEDIASOURCE::IsDefault()
{
	return info.bDefault;
}

bool MULTIMEDIASOURCE::IsEndOfStream()
{
	return true;
}

int MULTIMEDIASOURCE::GetSourceType()
{
	return MMS_INVALID;
}

void MULTIMEDIASOURCE::SetName(char* _lpcName)
{
/*	if (lpcName) free(lpcName);
	lpcName=NULL;
	if (_lpcName && *_lpcName)
	{
		lpcName=(char*)malloc(1+strlen(_lpcName));
		strcpy(lpcName,_lpcName);
	}
	return;
	*/
	GetTitleSet()->SetTitle(_lpcName);
}

void MULTIMEDIASOURCE::SetLanguageCode(char* _lpcName)
{
	if (lpcLangCode) free(lpcLangCode);
	lpcLangCode=NULL;
	if (_lpcName && *_lpcName) {
		lpcLangCode = new char[1+strlen(_lpcName)];
		strcpy(lpcLangCode,_lpcName);
	}
}

int MULTIMEDIASOURCE::GetName(char* lpDest)
{
	/*
	unsigned char*	lpbDest=(unsigned char*)lpDest;
	if (!lpcName) {
		if (lpDest) *lpbDest=0;
	} else {
		if (lpDest) memcpy(lpDest,lpcName,1+strlen(lpcName));
	}
	return (lpcName)?strlen(lpcName):0;
	*/

	char* pTitle = NULL;
	GetTitleSet()->GetTitle(&pTitle);
	if (lpDest) {
		if (pTitle)
			strcpy(lpDest, pTitle);
		else
			strcpy(lpDest, "");
	}

	return (int)strlen(lpDest?lpDest:pTitle?pTitle:"");
}

int MULTIMEDIASOURCE::GetLanguageCode(char* lpDest)
{
	unsigned char*	lpbDest=(unsigned char*)lpDest;
	if (!lpcLangCode) {
		if (lpDest) *lpbDest=0;
	} else {
		if (lpDest) memcpy(lpDest,lpcLangCode,1+strlen(lpcLangCode));
	}
	return (lpcLangCode)?(int)strlen(lpcLangCode):0;
}

char* MULTIMEDIASOURCE::GetCodecID()
{
	return NULL;
}

int MULTIMEDIASOURCE::Append(MULTIMEDIASOURCE *_pNext)
{
	return true;	
}
bool MULTIMEDIASOURCE::CanAppend(MULTIMEDIASOURCE* _pNext)
{
	return true;
}

__int64 MULTIMEDIASOURCE::GetBias(int iFlags)
{
	return info.iBias / ((iFlags & BIAS_UNSCALED)?1:info.iTimecodeScale);
}

__int64 MULTIMEDIASOURCE::DoStretch(__int64 iValue)
{
	if (info.bDoStretch) return (__int64)(info.dStretchFactor*iValue); else return iValue;
}

void MULTIMEDIASOURCE::SetStretchFactor(double dFactor)
{
	info.dStretchFactor = dFactor;

	if (fabs(dFactor - 1) > 0.0000000001) {
		info.bDoStretch = true;
	} else {
		info.bDoStretch = false;
	}
}

__int64 MULTIMEDIASOURCE::GetDuration()
{
	return DoStretch(GetUnstretchedDuration());
}

__int64 MULTIMEDIASOURCE::GetDurationUnscaled()
{
	return DoStretch(GetUnstretchedDuration() * GetTimecodeScale());
}

__int64 MULTIMEDIASOURCE::GetUnstretchedDuration()
{
	return 0;
}

void MULTIMEDIASOURCE::UpdateDuration(__int64 iDuration)
{
	size_guesser->SetTotalDuration((float)iDuration);
}

__int64 MULTIMEDIASOURCE::GuessTotalSize()
{
	return size_guesser->GuessSize();
}

int MULTIMEDIASOURCE::Close()
{
	return doClose();
}

__int64 MULTIMEDIASOURCE::GetExactSize()
{
	return 0;
}

__int64 MULTIMEDIASOURCE::GetSize()
{
	__int64 i = GetExactSize();

	return (i)?i:GuessTotalSize();
}

int MULTIMEDIASOURCE::doClose()
{
	return 0;
}

int MULTIMEDIASOURCE::Enable(int bEnabled)
{
	return 0;
}

__int64 MULTIMEDIASOURCE::GetFrameDuration()
{
	return FRAMEDURATION_UNKNOWN;
}

void MULTIMEDIASOURCE::AddSizeData(float f, __int64 i)
{
	size_guesser->AddData(f, i);
}

__int64 MULTIMEDIASOURCE::FormatSpecific(__int64 iCode, __int64 iValue)
{
	return 0;
}

int MULTIMEDIASOURCE::Seek(__int64 iTime)
{
	return 0;
}

void MULTIMEDIASOURCE::ReInit()
{
	Seek(0);
}

int MULTIMEDIASOURCE::IsCompatible(MULTIMEDIASOURCE* m)
{
	if (m->GetType() != GetType()) return MMSIC_TYPE;
	return MMS_COMPATIBLE;
}

int MULTIMEDIASOURCE::GetType()
{
	return MMT_UNDEFINED;
}

CSizeGuesser::CSizeGuesser()
{
	fTotal = 1;
	fDuration = 1;
	iSize = 0;
}

void CSizeGuesser::AddData(float f, __int64 i)
{
	fDuration += f;
	iSize += i;
}

__int64 CSizeGuesser::GuessSize()
{
	return (__int64)(fTotal * iSize / fDuration);
}

void CSizeGuesser::SetTotalDuration(float f)
{
	fTotal = f;
}

__int64 MULTIMEDIASOURCE::GetFeature(__int64 iFeature)
{
	return 0;
}

int MULTIMEDIASOURCE::GetStrippableHeaderBytes(void* pBuffer, int max)
{
	return MMS_UNKNOWN;
}



/* multimedia data packet stuff */
void createMultimediaDataPacket(MULTIMEDIA_DATA_PACKET** packet)
{
	*packet = new MULTIMEDIA_DATA_PACKET;
	(*packet)->timecode = -INT64_MAX;
	(*packet)->duration = -INT64_MAX;
	(*packet)->flags = 0;
	(*packet)->nextTimecode = -INT64_MAX;
	(*packet)->totalDataSize = 0;
	(*packet)->usageCounter = 1;
}

void useMultimediaDataPacket(MULTIMEDIA_DATA_PACKET* packet)
{
	if (packet->usageCounter > 0)
		packet->usageCounter++;
}

void freeMultimediaDataPacket(MULTIMEDIA_DATA_PACKET* packet)
{
	if (!packet)
		return;

	if (!packet->usageCounter)
		return;

	if (!--packet->usageCounter) {
		if (packet->data)
			free(packet->cData);
		packet->data = NULL;
	}

	delete (packet);
}

int MULTIMEDIASOURCE::GetPreferredTitle(char **pDest)
{
	char lngcode[16]; memset(lngcode, 0, sizeof(lngcode));
	GetLanguageCode(lngcode);

	char* title;
	GetTitleSet()->GetTitleStringFromLanguage(lngcode, &title);

	if (!title)
	{
		GetTitleSet()->GetTitleString(0, &title);
	}

	if (pDest)
	{
		*pDest = title;
	}

	return !!title;
}