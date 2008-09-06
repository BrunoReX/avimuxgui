#include "stdafx.h"
#include "videosource_list.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

	//////////////////////////////////
	// joined list of video sources //
	//////////////////////////////////

VIDEOSOURCELIST::VIDEOSOURCELIST()
{
	ZeroMemory(&info,sizeof(info));
}

VIDEOSOURCELIST::~VIDEOSOURCELIST()
{
}

int VIDEOSOURCELIST::Append(VIDEOSOURCE *pNext)
{
	if (CanAppend(pNext)) {

		info.videosources = (VIDEOSOURCE**)realloc(info.videosources,(info.iCount+1)*sizeof(VIDEOSOURCE*));
		info.videosources[info.iCount] = pNext;
//		pNext->UpdateDuration(pNext->GetDuration());
		if (info.iCount) {
			pNext->SetBias(info.videosources[info.iCount-1]->GetBias()+info.videosources[info.iCount-1]->GetDuration());
		} else {
			pNext->SetBias(0);
			RESOLUTION r;
			pNext->GetOutputResolution(&r);
			SetOutputResolution(&r);
			char c[1024]; c[0]=0;
			pNext->GetName(c);
			GetTitleSet()->Import(pNext->GetTitleSet());
//			SetName(c);
			pNext->GetLanguageCode(c);
			SetLanguageCode(c);
			SetDefault(pNext->IsDefault());
		}
		info.iCount++;
		if (info.iCount==1) info.curr_source = info.videosources[0];
		UpdateDuration(GetDuration());
		pNext->ReInit();
		return true;
	}

	return false;
}

void VIDEOSOURCELIST::ReInit()
{
	for (int i=0;i<info.iCount;info.videosources[i++]->ReInit());
	info.curr_source = info.videosources[0];
	info.iActiveSource = 0;
}

int VIDEOSOURCELIST::Enable(int bEnabled)
{
	for (int i=0;i<info.iCount;info.videosources[i++]->Enable(bEnabled));
	return 0;
}

DWORD VIDEOSOURCELIST::GetFourCC()
{
	if (info.iCount)
		return info.videosources[0]->GetFourCC();

	return 0;
}

int VIDEOSOURCELIST::GetFrame(void* lpDest,DWORD* lpdwSize,__int64 *lpiTimecode, ADVANCEDREAD_INFO* lpAARI)
{
	__int64 j;
	VIDEOSOURCE* v;
	if (!info.curr_source->IsEndOfStream()) {
		int iRes = info.curr_source->GetFrame(lpDest,lpdwSize,lpiTimecode,lpAARI);
		SetCurrentTimecode(*lpiTimecode * (j=info.curr_source->GetTimecodeScale()), TIMECODE_UNSCALED);
		if (lpiTimecode) *lpiTimecode = DoStretch(GetCurrentTimecode());
		if (lpAARI) {
			lpAARI->iNextTimecode = DoStretch(lpAARI->iNextTimecode * j / GetTimecodeScale());
			lpAARI->iDuration = lpAARI->iDuration * info.curr_source->GetTimecodeScale() / GetTimecodeScale();
		}

		if (info.curr_source->IsEndOfStream()) {
			if (info.iActiveSource<info.iCount-1) {
				v = info.videosources[info.iActiveSource+1];
				if (lpAARI) lpAARI->iNextTimecode = v->GetBias(BIAS_UNSCALED) / GetTimecodeScale();
			} else {
				v = info.videosources[info.iActiveSource];
				if (lpAARI) lpAARI->iNextTimecode = (v->GetBias(BIAS_UNSCALED) + 
					v->GetDuration()*v->GetTimecodeScale())/GetTimecodeScale();
			}
		}
		return iRes;
	} else {
		if (info.iActiveSource<info.iCount-1) {
			info.curr_source = info.videosources[++info.iActiveSource];
			return GetFrame(lpDest,lpdwSize,lpiTimecode,lpAARI);
		} else return VS_ERROR;
	}
	return VS_ERROR;
}

char* VIDEOSOURCELIST::GetCodecID()
{
	if (info.iCount) {
		return info.curr_source->GetCodecID();
	} else return 0;
}

int VIDEOSOURCELIST::GetLatestReference(int* lpiCount, __int64* lpiRef1, __int64* lpiRef2)
{
	__int64 i,j,t;
	info.curr_source->GetLatestReferenceAbsolute(lpiCount,&t, &i, &j);
	if (lpiRef1) *lpiRef1 = (i*info.curr_source->GetTimecodeScale())/GetTimecodeScale() - (t*info.curr_source->GetTimecodeScale())/GetTimecodeScale();
	if (lpiRef2) *lpiRef2 = (j*info.curr_source->GetTimecodeScale())/GetTimecodeScale() - (t*info.curr_source->GetTimecodeScale())/GetTimecodeScale();

	return 0;
}

int VIDEOSOURCELIST::GetNbrOfFrames(DWORD dwKind)
{
	int		iRes = 0;
	for (int i=0;i<info.iCount;iRes+=info.videosources[i++]->GetNbrOfFrames(dwKind));
	return iRes;
}

void* VIDEOSOURCELIST::GetFormat()
{
	return info.videosources[0]->GetFormat();
}

int VIDEOSOURCELIST::GetResolution(int *lpiWidth,int* lpiHeight)
{
	return info.videosources[0]->GetResolution(lpiWidth,lpiHeight);
}

int VIDEOSOURCELIST::GetFormatSize()
{
	return (info.iCount)?info.curr_source->GetFormatSize():0;
}

AVIStreamHeader* VIDEOSOURCELIST::GetAVIStreamHeader()
{
	return info.videosources[0]->GetAVIStreamHeader();
}

__int64 VIDEOSOURCELIST::GetNanoSecPerFrame()
{
	return info.videosources[0]->GetNanoSecPerFrame();
}

bool VIDEOSOURCELIST::IsAVIOutputPossible()
{
	for (int i=0;i<info.iCount;i++) {
		if (!info.videosources[i]->IsAVIOutputPossible())  {
			return false;
		}
	} 
	return true;
}

void VIDEOSOURCELIST::AllowAVIOutput(bool bAllow)
{
	info.videosources[0]->AllowAVIOutput(bAllow);
}

bool VIDEOSOURCELIST::IsKeyFrame(DWORD dwNbr)
{
	if (!info.curr_source->IsEndOfStream()) {
		return info.curr_source->IsKeyFrame(dwNbr);
	} else {
		if (info.iActiveSource>=info.iCount-1) {
			return true;
		} else {
			return info.videosources[info.iActiveSource+1]->IsKeyFrame(dwNbr);
		}
	}
}

__int64 VIDEOSOURCELIST::GetSize()
{
	__int64 iRes = 0;
	for (int i=0;i<info.iCount;iRes+=info.videosources[i++]->GetSize());
	return iRes;
}

int VIDEOSOURCELIST::Seek(__int64 iTime)
{
	int i = 0;
	__int64 iMax = 0;//info.videosources[i]->GetDurationUnscaled();
	__int64 iMaxes[100];
	ZeroMemory(iMaxes,sizeof(iMaxes));

	while (iMax<iTime && i<info.iCount-1) {
		iMax+=info.videosources[i]->GetDurationUnscaled();
		iMaxes[i+1] = iMaxes[i] + info.videosources[i]->GetDurationUnscaled();
		i++;
	}

	if (i>0) i--;

	if (iTime-iMaxes[i] < info.videosources[i]->GetDurationUnscaled()) {
		info.curr_source = info.videosources[i];
		info.iActiveSource = i;
		info.curr_source->Seek(iTime-iMaxes[i]);
	}

	return VS_OK;
}

DWORD VIDEOSOURCELIST::Close(bool bCloseSource)
{
	for (int i=0;i<info.iCount;i++) {
		info.videosources[i]->Close(false);
		delete info.videosources[i];
	}
	delete info.videosources;
	return VS_OK;
}

__int64 VIDEOSOURCELIST::GetUnstretchedDuration()
{
	__int64 iRes = 0;
	for (int i=0;i<info.iCount;i++) {
		iRes+=info.videosources[i]->GetUnstretchedDuration()*info.videosources[i]->GetTimecodeScale()/GetTimecodeScale();
	}
	return iRes;
}

bool VIDEOSOURCELIST::IsEndOfStream()
{
	return (info.curr_source->IsEndOfStream() && !(info.iActiveSource<info.iCount-1));
}

/*
void VIDEOSOURCELIST::GetCropping(RECT* r)
{
	if (info.iCount) info.videosources[0]->GetCropping(r);
}

void VIDEOSOURCELIST::GetOutputResolution(RESOLUTION* r)
{
	if (info.iCount)
		info.videosources[0]->GetOutputResolution(r);
	*r = re
}
*/

int VIDEOSOURCELIST::GetStrippableHeaderBytes(void* pBuffer, int max)
{
	int res = min(max, 64);
	unsigned char bytes_result[64];
	unsigned char bytes[64];

	if (!info.iCount)
		return 0;

	res = info.videosources[0]->GetStrippableHeaderBytes(bytes_result, res);
	if (res < 0)
		res = 0;

	if (res)	for (int i=1;i<info.iCount;i++) {
		int z = info.videosources[i]->GetStrippableHeaderBytes(bytes, res);

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