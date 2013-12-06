#include "stdafx.h"
#include "audiosource_list.h"
#include "..\matroska.h"
#include "TraceFile.h"
#include "..\FormatTime.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

	//////////////////////////////////
	// list of joined audio sources //
	//////////////////////////////////

AUDIOSOURCELIST::AUDIOSOURCELIST()
{
	ZeroMemory(&info,sizeof(info));
}

AUDIOSOURCELIST::~AUDIOSOURCELIST()
{
}

int AUDIOSOURCELIST::Append(AUDIOSOURCE* pNext)
{
	info.audiosources = (AUDIOSOURCE**)realloc(info.audiosources,(info.iCount+1)*sizeof(AUDIOSOURCE*));
	info.audiosources[info.iCount] = pNext;

	if (info.iCount) {
		pNext->SetBias(info.audiosources[info.iCount-1]->GetBias()+info.audiosources[info.iCount-1]->GetDuration());
	} else {
		pNext->SetBias(0);
		char cBuffer[2048];
		ZeroMemory(cBuffer,sizeof(cBuffer));
	//	pNext->GetName(cBuffer);
	//	SetName(cBuffer);
		GetTitleSet()->Import(pNext->GetTitleSet());

		ZeroMemory(cBuffer,sizeof(cBuffer));
		pNext->GetLanguageCode(cBuffer);
		SetLanguageCode(cBuffer);
		SetTimecodeScale(1000);
		SetDefault(pNext->IsDefault());
	}

	info.iCount++;
	if (info.iCount == 1) {
		info.active_source = info.audiosources[0];
	}

	for (int i=0;i<info.iCount;i++) {
		if (!info.audiosources[i]->IsAVIOutputPossible()) {
			AllowAVIOutput(false);
		}
	}

	pNext->SetDelayMMS(this);
	pNext->ReInit();

	return AS_OK;
}

__int64 AUDIOSOURCELIST::GetUnstretchedDuration()
{
	__int64 res = 0;
	for (int i=0; i<info.iCount; i++) {
		res+=info.audiosources[i]->GetUnstretchedDuration() * info.audiosources[i]->GetTimecodeScale() / GetTimecodeScale();
	}

	return res;
}

int AUDIOSOURCELIST::GetFrameMode()
{
	return info.active_source->GetFrameMode();
}

int AUDIOSOURCELIST::GetGranularity()
{
	return info.active_source->GetGranularity();
}

int AUDIOSOURCELIST::GetChannelCount()
{
	return info.active_source->GetChannelCount();
}

int AUDIOSOURCELIST::GetAvgBytesPerSec()
{
	return info.active_source->GetAvgBytesPerSec();
}

int AUDIOSOURCELIST::GetOffset()
{
	return info.audiosources[0]->GetOffset();
}

int AUDIOSOURCELIST::SetFrameMode(DWORD dwMode)
{
	for (int i=0;i<info.iCount;info.audiosources[i++]->SetFrameMode(dwMode));
	return AS_OK;
}

void AUDIOSOURCELIST::AssumeCBR()
{
	for (int i=0;i<info.iCount;info.audiosources[i++]->AssumeCBR());
}

void AUDIOSOURCELIST::AssumeVBR()
{
	for (int i=0;i<info.iCount;info.audiosources[i++]->AssumeVBR());
}

int AUDIOSOURCELIST::Read(void* lpDest, DWORD dwMicrosecDesired, DWORD* lpdwMicrosecRead,
						  __int64* lpqwNanosecRead, __int64* lpiTimecode, ADVANCEDREAD_INFO* lpAARI)
{
	__int64 j;
	if (!lpqwNanosecRead) lpqwNanosecRead = &j;

	// can read sth from current source?
	if (!info.active_source->IsEndOfStream()) {
		int res = info.active_source->Read(lpDest,dwMicrosecDesired,
			lpdwMicrosecRead,lpqwNanosecRead,lpiTimecode,lpAARI);

		/* if no real data was read and suddently the end of the stream
		   has been reached, the end was crap */
		if (res == 0 && info.active_source->IsEndOfStream()) {
			return Read(lpDest, dwMicrosecDesired, lpdwMicrosecRead,
				lpqwNanosecRead, lpiTimecode, lpAARI);
		}

		if (lpiTimecode) {
			SetCurrentTimecode(*lpiTimecode * info.active_source->GetTimecodeScale(), TIMECODE_UNSCALED);
			*lpiTimecode = GetCurrentTimecode();
		}
		if (lpAARI) {
			if (lpAARI->iNextTimecode != TIMECODE_UNKNOWN) {
	//			lpAARI->iNextTimecode = lpAARI->iNextTimecode*info.active_source->GetTimecodeScale() / GetTimecodeScale() + GetBias();
			}
		}
		if (lpAARI) {
			// end reached -> current audio frame was last one of active source
			if (info.active_source->IsEndOfStream()) {
				// is there another audio source?
				if (info.iActiveSource < info.iCount-1) {
					AUDIOSOURCE* next = info.audiosources[info.iActiveSource+1];
					// join seemless? -> set BIAS for next source accordingly
					if (IsSeamless()) {
						next->SetBias(GetCurrentTimecode() * GetTimecodeScale()+
							*lpqwNanosecRead, BIAS_UNSCALED);
					}
					lpAARI->iNextTimecode = next->GetBias(BIAS_UNSCALED) / GetTimecodeScale();
				}
			} else {
				if (lpAARI->iNextTimecode != TIMECODE_UNKNOWN) {
					lpAARI->iNextTimecode *= info.active_source->GetTimecodeScale();
					lpAARI->iNextTimecode /= GetTimecodeScale();
					lpAARI->iNextTimecode += GetBias();
				}
			}
		}

		return res;
	} else {
		// end of list?
		if (info.iActiveSource >= info.iCount-1) {
			return AS_ERR;
		} else {
		// one more file available

			/* create log entry */
			char cTime[64]; memset(cTime, 0, sizeof(cTime));
			Millisec2Str(info.active_source->GetCurrentTimecode() *
				info.active_source->GetTimecodeScale() / 1000000, cTime);

			char cName[1024]; memset(cName, 0, sizeof(cName));
			GetName(cName);

			char cMsg[2048]; memset(cMsg, 0, sizeof(cMsg));
			sprintf(cMsg, "End of stream encountered\nName: %s\nTimecode: %s",
				cName, cTime);

		//	GetApplicationTraceFile()->Trace(TRACE_LEVEL_NOTE, "End of stream", cMsg);


			info.active_source = info.audiosources[++info.iActiveSource];
			if (IsSeamless()) {
				info.active_source->SetBias(info.audiosources[info.iActiveSource-1]->GetCurrentTimecode());
			}
			return Read(lpDest,dwMicrosecDesired,lpdwMicrosecRead,lpqwNanosecRead,lpiTimecode,lpAARI);
		}
	}
}

int AUDIOSOURCELIST::Seek(__int64 iPos)
{
	if (!iPos) {
		info.active_source = info.audiosources[0];
		info.active_source->Seek(iPos);
		info.iActiveSource = 0;
	}
	return 0;
}

void AUDIOSOURCELIST::ReInit()
{
	for (int i=0;i<info.iCount;info.audiosources[i++]->ReInit());
	info.active_source = info.audiosources[0];
	info.iActiveSource = 0;
}

int AUDIOSOURCELIST::Enable(int bEnabled)
{
	for (int i=0;i<info.iCount;info.audiosources[i++]->Enable(bEnabled));
	return 0;
}

__int64 AUDIOSOURCELIST::GetSize()
{
	__int64 iRes=0;
	for (int i=0;i<info.iCount;iRes+=info.audiosources[i++]->GetSize());
	return iRes;
}

int AUDIOSOURCELIST::GetFrequency()
{
	return info.active_source->GetFrequency();
}

int AUDIOSOURCELIST::GetOutputFrequency()
{
	return info.active_source->GetOutputFrequency();
}

int AUDIOSOURCELIST::doClose()
{
	for (int i=0;i<info.iCount;i++) {
		info.audiosources[i]->Close();
		delete info.audiosources[i];
	}
	delete info.audiosources;
	return AS_OK;
}

__int64 AUDIOSOURCELIST::FormatSpecific(__int64 iCode, __int64 iValue)
{
	__int64 j;
	for (int i=0;i<info.iCount;i++) if (j=info.audiosources[i]->FormatSpecific(iCode, iValue)) return j;
	return 0;
}

int AUDIOSOURCELIST::GetBitDepth()
{
	return info.active_source->GetBitDepth();
}

char* AUDIOSOURCELIST::GetCodecID()
{
	return info.audiosources[0]->GetCodecID();
}

int AUDIOSOURCELIST::IsCompatible(AUDIOSOURCE* a)
{
	if (info.iCount) {
		return info.audiosources[0]->IsCompatible(a);
	} else {
		return MMS_COMPATIBLE;
	}
}

bool AUDIOSOURCELIST::IsEndOfStream()
{
	return (info.active_source->IsEndOfStream() && (info.iActiveSource >= info.iCount-1));
}

void* AUDIOSOURCELIST::GetFormat()
{
	return info.active_source->GetFormat();
}

bool AUDIOSOURCELIST::IsCBR()
{
	bool bRes = true;

	for (int i=0;i<info.iCount;i++) {
		if (!info.audiosources[i]->IsCBR() || 
			(i<info.iCount-1 && !info.audiosources[i+1]->IsCBR()) ||
			(i<info.iCount-1 && info.audiosources[i]->GetAvgBytesPerSec()!=info.audiosources[i+1]->GetAvgBytesPerSec()))
		{
			bRes = false;
		}
	}

	return bRes;
}

__int64 AUDIOSOURCELIST::GetFrameDuration()
{
	__int64 iDur = info.audiosources[0]->GetFrameDuration();
	if (iDur == FRAMEDURATION_UNKNOWN) return iDur;

	for (int i=1;i<info.iCount;i++) {
		if (info.audiosources[i]->GetFrameDuration() != iDur) return FRAMEDURATION_UNKNOWN;
	}

	return iDur;
}

int AUDIOSOURCELIST::GetFormatTag()
{
	return info.active_source->GetFormatTag();
}

int AUDIOSOURCELIST::GetStrippableHeaderBytes(void* pBuffer, int max)
{
	int res = min(max, 64);
	unsigned char bytes_result[64];
	unsigned char bytes[64];

	if (!info.iCount)
		return 0;

	res = info.audiosources[0]->GetStrippableHeaderBytes(bytes_result, res);
	if (res < 0)
		res = 0;

	if (res)	for (int i=1;i<info.iCount;i++) {
		int z = info.audiosources[i]->GetStrippableHeaderBytes(bytes, res);

		res = min(res, z);
		
		for (int j=0;j<res;j++) {
			if (bytes_result[j] != bytes[j])
				res = j;
		}
	}

	if (res)
		memcpy(pBuffer, bytes_result, res);

	return res;

}