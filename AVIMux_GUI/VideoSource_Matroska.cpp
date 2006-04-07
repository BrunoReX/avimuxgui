#include "stdafx.h"
#include "VideoSource_Matroska.h"


	///////////////////////////////////////
	// video source from a matroska file //
	///////////////////////////////////////

VIDEOSOURCEFROMMATROSKA::VIDEOSOURCEFROMMATROSKA()
{
	ZeroMemory(&info,sizeof(info));
}

VIDEOSOURCEFROMMATROSKA::~VIDEOSOURCEFROMMATROSKA()
{
}

VIDEOSOURCEFROMMATROSKA::VIDEOSOURCEFROMMATROSKA(MATROSKA* matroska, int iStream)
{
	ZeroMemory(&info,sizeof(info));
	Open(matroska,iStream);
}

bool VIDEOSOURCEFROMMATROSKA::IsOpen()
{
	return info.iStream > -1;
}

void VIDEOSOURCEFROMMATROSKA::ReInit()
{
	Seek(-40000);
}

/*
typedef struct {
    FOURCC		fccType;
    FOURCC		fccHandler;
    DWORD		dwFlags;	
    WORD		wPriority;
    WORD		wLanguage;
    DWORD		dwInitialFrames;
    DWORD		dwScale;	
    DWORD		dwRate;	// dwRate / dwScale == samples/second 
    DWORD		dwStart;
    DWORD		dwLength;  In units above... 
    DWORD		dwSuggestedBufferSize;
    DWORD		dwQuality;
    DWORD		dwSampleSize;
    RECT		rcFrame;
} AVIStreamHeader;

*/
int VIDEOSOURCEFROMMATROSKA::Open(MATROSKA* matroska, int iStream)
{
	info.m = matroska;
	if (iStream>-1) {
		info.iStream = iStream;
	} else {
		int i=0;
		do {
			matroska->SetActiveTrack(i++);
		}
		while (matroska->GetTrackType()!=MSTRT_VIDEO && i<matroska->GetTrackCount());

		if (matroska->GetTrackType()==MSTRT_VIDEO) {
			info.iStream = i-1;
		} else {
			info.iStream = -1;
			info.m = NULL;
			return VS_ERROR;
		}
	}

	ZeroMemory(&curr_lace,sizeof(curr_lace));
	iPos = 0x7FFFFFFF;
	SetTimecodeScale(info.m->GetTimecodeScale());

	char* codecid = info.m->GetCodecID();
	if (strcmp(codecid, "V_MS/VFW/FOURCC")) {
		// can't output AVI if CodecID is not VFW
		AllowAVIOutput(0);

	} else {
		ZeroMemory(&avistreamheader, sizeof(avistreamheader));
		avistreamheader.fccType = MakeFourCC("vids");
		BITMAPINFOHEADER* lpBMI = (BITMAPINFOHEADER*)info.m->GetCodecPrivate();
		avistreamheader.fccHandler = lpBMI->biCompression;
		if (info.m->GetDefaultDuration()) {
			avistreamheader.dwRate = 1000000000;
			avistreamheader.dwScale = (DWORD)info.m->GetDefaultDuration();
			AllowAVIOutput(1);
		} else {
			// can't determine framerate if (!DefaultDuration)
			AllowAVIOutput(0);
		}
	}

	if (info.m->IsBitrateIndicated(info.iStream)) {
		return VS_OK;
	}
	UpdateDuration(info.m->GetMasterTrackDuration());
	ReInit();
	void* c = new char[1<<20];
	int	iTime = GetTickCount();
	int jMax = (int)(info.m->GetSegmentDuration() * info.m->GetTimecodeScale() / 1000000000 / 300);
	if (jMax > 50) jMax = 50;

	for (int j=0;j<=jMax && GetTickCount()-iTime < 10000;j++) {
		Seek(info.m->GetSegmentDuration() * (j) / (jMax+1));
		for (int i=0;i<50;i++) {
			GetFrame(c, NULL,NULL);
		}
	}
	
	ReInit();
	delete c;

	return VS_OK;
}

__int64 VIDEOSOURCEFROMMATROSKA::GetExactSize()
{
	if (info.m->IsBitrateIndicated()) {
		return info.m->GetTrackSize(info.iStream);
	} else {
		return 0;
	}
}
__int64 VIDEOSOURCEFROMMATROSKA::GetUnstretchedDuration()
{
	return info.m->GetMasterTrackDuration()*info.m->GetTimecodeScale()/GetTimecodeScale();
}

int VIDEOSOURCEFROMMATROSKA::GetResolution(int* lpiWidth, int* lpiHeight)
{
	info.m->SetActiveTrack(info.iStream);
	return info.m->GetResolution(lpiWidth,lpiHeight,NULL,NULL,NULL);
}

void VIDEOSOURCEFROMMATROSKA::GetOutputResolution(RESOLUTION* r)
{
	info.m->SetActiveTrack(info.iStream);
	info.m->GetResolution(NULL,NULL,&r->iWidth,&r->iHeight,NULL);
}

int VIDEOSOURCEFROMMATROSKA::Enable(int bEnabled) 
{
	info.m->EnableQueue(info.iStream, !!bEnabled);
	return 0;
}

int VIDEOSOURCEFROMMATROSKA::GetFrame(void* lpDest,DWORD* lpdwSize,__int64* lpiTimecode, ADVANCEDREAD_INFO* lpAARI)
{
	READ_INFO*	r;

	r = &curr_lace;
	if (iPos >= curr_lace.iFrameCount || !r->pData) {
		if (r->pData) DecBufferRefCount(&r->pData);

		info.m->SetActiveTrack(info.iStream);
		ZeroMemory(r,sizeof(*r));
		if (info.m->Read(&curr_lace) != READBL_OK) {
			if (lpdwSize) *lpdwSize = 0;
			return VS_ERROR;
		}
		
		iNextTimecode = info.m->GetNextTimecode();
		iPos = 0;
		iBytePosInLace = 0;
		int iRefCount = !!(r->iReferences & RBIREF_FORWARD) + !!(r->iReferences / RBIREF_BACKWARD);
		SetCurrentTimecode(r->qwTimecode);
//		SetLatestReference(iRefCount, r->iReferencedFrames[0] * info.m->GetTimecodeScale(),
//			r->iReferencedFrames[1] * info.m->GetTimecodeScale());
		SetReferencedFramesAbsolute(iRefCount, GetCurrentTimecode() * GetTimecodeScale(),
			(r->iReferencedFrames[0] + GetCurrentTimecode()) * GetTimecodeScale(),
			(r->iReferencedFrames[1] + GetCurrentTimecode()) * GetTimecodeScale());

	}

	if (curr_lace.iFrameCount>1) {
		__int64 iDur = iNextTimecode - curr_lace.qwTimecode;
		__int64 iCurrTC = curr_lace.qwTimecode + iDur * iPos / curr_lace.iFrameCount;
		__int64 iReference = iCurrTC - GetCurrentTimecode();
		__int64 iNTC;
		
		if (curr_lace.iFrameCount > iPos + 1) {
			iNTC = curr_lace.qwTimecode + iDur * (iPos+1) / curr_lace.iFrameCount;
		} else {
			iNTC = info.m->GetNextTimecode(info.iStream);
		}

		SetCurrentTimecode(iCurrTC);
		//SetLatestReference(1, iReference * info.m->GetTimecodeScale(), 0);
		SetReferencedFramesAbsolute(1, GetCurrentTimecode(), 
			iReference * GetTimecodeScale());
		if (lpdwSize) *lpdwSize = r->iFrameSizes[iPos];
		memcpy(lpDest, ((BYTE*)r->pData->GetData()) + iBytePosInLace, r->iFrameSizes[iPos]);
		if (lpAARI) {
			lpAARI->iNextTimecode = iNTC;
			if (r->iFlags & RIF_DURATION) {
				lpAARI->iDuration = r->qwDuration * info.m->GetTimecodeScale() / GetTimecodeScale();
			} else {
				lpAARI->iDuration = info.m->GetDefaultDuration(info.iStream) / GetTimecodeScale();
			}
		}

		iBytePosInLace += r->iFrameSizes[iPos++];
	} else {

		if (lpiTimecode) *lpiTimecode = GetCurrentTimecode();
		memcpy(lpDest,r->pData->GetData(),r->pData->GetSize());
		if (lpdwSize) *lpdwSize = r->pData->GetSize();
		__int64 iNTC = info.m->GetNextTimecode() + GetBias();
		if (lpAARI) {
			lpAARI->iNextTimecode = iNTC;
			if (r->iFlags & RIF_DURATION) {
				lpAARI->iDuration = r->qwDuration * info.m->GetTimecodeScale() / GetTimecodeScale();
			} else {
				lpAARI->iDuration = info.m->GetDefaultDuration(info.iStream) / GetTimecodeScale();
			}
		}
		if (iNTC>0) AddSizeData((float)(iNTC - GetCurrentTimecode()),r->pData->GetSize());
		
		DecBufferRefCount(&r->pData);

		return VS_OK;
	}

	return VS_ERROR;
}

void* VIDEOSOURCEFROMMATROSKA::GetFormat()
{
	info.m->SetActiveTrack(info.iStream);
	return info.m->GetCodecPrivate();
}

char* VIDEOSOURCEFROMMATROSKA::GetIDString()
{
	info.m->SetActiveTrack(info.iStream);
	return info.m->GetCodecID();
}

bool VIDEOSOURCEFROMMATROSKA::IsKeyFrame(DWORD dwNbr)
{
	info.m->SetActiveTrack(info.iStream);
	return info.m->IsKeyframe();
}

int VIDEOSOURCEFROMMATROSKA::Seek(__int64 iTime)
{
	info.m->Seek(iTime);
	return 0;
}

int VIDEOSOURCEFROMMATROSKA::GetFormatSize()
{
	return info.m->GetCodecPrivateSize(info.iStream);
}

__int64 VIDEOSOURCEFROMMATROSKA::GetNanoSecPerFrame()
{
	info.m->SetActiveTrack(info.iStream);
	return DoStretch(info.m->GetDefaultDuration());
}

int VIDEOSOURCEFROMMATROSKA::GetNbrOfFrames(DWORD dwKind)
{
	return 0;
}

bool VIDEOSOURCEFROMMATROSKA::IsEndOfStream()
{
	info.m->SetActiveTrack(info.iStream);
	return ((!curr_lace.pData || (curr_lace.iFrameCount>1 && curr_lace.iFrameCount == iPos)) && info.m->IsEndOfStream());
}

AVIStreamHeader* VIDEOSOURCEFROMMATROSKA::GetAVIStreamHeader()
{
	return &avistreamheader;
}