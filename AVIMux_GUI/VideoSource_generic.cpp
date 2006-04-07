#include "stdafx.h"
#include "videosource_generic.h"


// Videosource

VIDEOSOURCE::VIDEOSOURCE(void)
{
	lpUserData=NULL;
	bCFR = true;
	resOutput.iWidth = 0;
	resOutput.iHeight = 0;
	AllowAVIOutput(true);
	SetTimecodeScale(1000);
}

VIDEOSOURCE::~VIDEOSOURCE(void)
{
}

DWORD VIDEOSOURCE::GetMicroSecPerFrame(void)
{
	return (DWORD)round((double)GetNanoSecPerFrame()/1000);
}

__int64 VIDEOSOURCE::GetNanoSecPerFrame(void)
{
	return 0;
}

int VIDEOSOURCE::Seek(__int64 iTime)
{
	return VS_INVALIDCALL;
}

bool VIDEOSOURCE::IsKeyFrame(DWORD dwNbr)
{
	return false;
}

DWORD VIDEOSOURCE::GetPos(void)
{
	return VS_INVALIDCALL;
}

int VIDEOSOURCE::GetFrame(void* lpDest,DWORD* lpdwSize, __int64* lpiTimecode, ADVANCEDREAD_INFO* lpAARI)
{
	return VS_INVALIDCALL;
}

int VIDEOSOURCE::GetNbrOfFrames(DWORD dwKind)
{
	return 0;
}

void* VIDEOSOURCE::GetFormat(void)
{
	return NULL;
}

DWORD VIDEOSOURCE::GetFourCC(void)
{
	return 0;
}

int VIDEOSOURCE::GetType()
{
	return MMT_VIDEO;
}

AVIStreamHeader* VIDEOSOURCE::GetAVIStreamHeader(void)
{
	return NULL;
}

int VIDEOSOURCE::GetResolution(int* lpiWidth,int* lpiHeight)
{
	return VS_INVALIDCALL;
}


DWORD VIDEOSOURCE::Close(bool bCloseSource)
{
	return VS_INVALIDCALL;
}

bool VIDEOSOURCE::IsEndOfStream(void)
{
	return ((int)GetPos()>=(int)GetNbrOfFrames());
}

/*void VIDEOSOURCE::SetLatestReference(int iCount, __int64 iRef1, __int64 iRef2)
{
	last_ref.iCount = iCount;
	if (iCount) last_ref.iReferences[0] = iRef1;
	if (iCount>1) last_ref.iReferences[1] = iRef2;
}
*/

void VIDEOSOURCE::SetReferencedFramesAbsolute(int iCount, __int64 iThis, __int64 iRef1, __int64 iRef2)
{
	last_ref.iCount = iCount;
	last_ref.iThis = iThis;
	if (iCount) last_ref.iReferences[0] = iRef1;
	if (iCount>1) last_ref.iReferences[1] = iRef2;
}

int VIDEOSOURCE::GetLatestReference(int *lpiCount, __int64* lpiRef1, __int64 *lpiRef2)
{
	if (lpiCount) *lpiCount = last_ref.iCount;
	if (lpiRef1) *lpiRef1 = DoStretch(last_ref.iReferences[0])/GetTimecodeScale()-DoStretch(last_ref.iThis)/GetTimecodeScale();
	if (lpiRef2) *lpiRef2 = DoStretch(last_ref.iReferences[1])/GetTimecodeScale()-DoStretch(last_ref.iThis)/GetTimecodeScale();

	return true;
}

int VIDEOSOURCE::GetLatestReferenceAbsolute(int *lpiCount, __int64* iThis, __int64* lpiRef1, __int64 *lpiRef2)
{
	if (lpiCount) *lpiCount = last_ref.iCount;
	if (lpiRef1) *lpiRef1 = DoStretch(last_ref.iReferences[0])/GetTimecodeScale();
	if (lpiRef2) *lpiRef2 = DoStretch(last_ref.iReferences[1])/GetTimecodeScale();
	if (iThis) *iThis = DoStretch(last_ref.iThis)/GetTimecodeScale();

	return true;
}

bool VIDEOSOURCE::IsCFR()
{
	return bCFR;
}

void VIDEOSOURCE::SetCFRFlag(bool bIsCFR)
{
	bCFR = bIsCFR;
}

void VIDEOSOURCE::SetOutputResolution(RESOLUTION* r)
{
	memcpy(&resOutput,r,sizeof(*r));
}

void VIDEOSOURCE::GetOutputResolution(RESOLUTION* r)
{
	memcpy(r,&resOutput,sizeof(*r));
}

int VIDEOSOURCE::GetFormatSize()
{
	return 0;
}