#include "stdafx.h"
#include "videosource_avi.h"

	//////////////////////////
	// Videosource from AVI //
	//////////////////////////

VIDEOSOURCEFROMAVI::VIDEOSOURCEFROMAVI()
{
	ZeroMemory(&info,sizeof(info));
}

VIDEOSOURCEFROMAVI::~VIDEOSOURCEFROMAVI()
{

}

bool VIDEOSOURCEFROMAVI::IsCFR()
{
	return true;
}


__int64 VIDEOSOURCEFROMAVI::GetUnstretchedDuration()
{
	return (info.avifile[0]->GetNanoSecPerFrame()*info.avifile[0]->GetFrameCount()/GetTimecodeScale());
}

int VIDEOSOURCEFROMAVI::Open(AVIFILEEX* avifile)
{
	info.avifile = (AVIFILEEX**)realloc(info.avifile,(info.iCount++)*sizeof(AVIFILEEX*));
	info.avifile[info.iCount-1] = avifile;
	SetCurrentTimecode(-avifile->GetNanoSecPerFrame(),TIMECODE_UNSCALED);
	return VS_OK;
}


void* VIDEOSOURCEFROMAVI::GetFormat()
{
	return info.avifile[0]->GetStreamFormat(0);
}

void VIDEOSOURCEFROMAVI::ReInit()
{
	Seek(0);
}

int VIDEOSOURCEFROMAVI::GetFrame(void* lpDest, DWORD *lpdwSize, __int64 *lpiTimecode, ADVANCEDREAD_INFO* lpAARI)
{
	AVIFILEEX* a = info.avifile[info.iActiveFile];
	if (!info.avifile) return VS_INVALIDCALL;
	__int64		iOldTimecode = GetCurrentTimecode();

	if (!a->IsEndOfStream(0)) {
		bool bKeyframe = a->IsKeyFrame(CN_CURRENT_CHUNK);

		IncCurrentTimecode(a->GetNanoSecPerFrame());

		if (lpiTimecode) *lpiTimecode = GetCurrentTimecode();
		if (lpAARI) {
			lpAARI->iNextTimecode = GetCurrentTimecode() + a->GetNanoSecPerFrame()/GetTimecodeScale();
			lpAARI->iDuration = a->GetNanoSecPerFrame() / GetTimecodeScale();
		}

		if (!bKeyframe) {
			SetReferencedFramesAbsolute(1, GetCurrentTimecode() * GetTimecodeScale(),
				iOldTimecode * GetTimecodeScale());
		} else SetReferencedFramesAbsolute(0,0);
		
		return (a->GetVideoChunk(CN_NEXT_CHUNK,lpDest,lpdwSize)==AFE_OK)?VS_OK:VS_ERROR;
	} else {
		 return VS_ERROR;
	}
}



int VIDEOSOURCEFROMAVI::GetResolution(int *lpiWidth,int* lpiHeight)
{
	return info.avifile[info.iActiveFile]->GetVideoResolution(lpiWidth,lpiHeight);
}

void VIDEOSOURCEFROMAVI::GetOutputResolution(RESOLUTION* r)
{
	GetResolution(&r->iWidth,&r->iHeight);
}

__int64 VIDEOSOURCEFROMAVI::GetExactSize()
{
	return info.avifile[info.iActiveFile]->GetStreamSize(0);
}

AVIStreamHeader* VIDEOSOURCEFROMAVI::GetAVIStreamHeader()
{
	return info.avifile[info.iActiveFile]->GetStreamHeader(0);
}

int VIDEOSOURCEFROMAVI::GetNbrOfFrames(DWORD dwKind)
{
	return info.avifile[info.iActiveFile]->GetNbrOfFrames(dwKind);
}

DWORD VIDEOSOURCEFROMAVI::GetPos()
{
	return info.avifile[info.iActiveFile]->GetCurrChunk(0);
}

int VIDEOSOURCEFROMAVI::Seek(__int64 iTime)
{
	__int64 iFrame = iTime / GetNanoSecPerFrame();
	info.avifile[info.iActiveFile]->SeekVideoStream(DWORD(iFrame));
	SetCurrentTimecode(iTime - GetNanoSecPerFrame(), TIMECODE_UNSCALED);
	SetReferencedFramesAbsolute(0,0);
	return VS_OK;
}

bool VIDEOSOURCEFROMAVI::IsEndOfStream()
{
	return (info.avifile[info.iActiveFile]->IsEndOfStream(0));
}

bool VIDEOSOURCEFROMAVI::IsKeyFrame(DWORD dwNbr)
{
	return info.avifile[info.iActiveFile]->IsKeyFrame(dwNbr);
}

__int64 VIDEOSOURCEFROMAVI::GetNanoSecPerFrame()
{
	return DoStretch(info.avifile[info.iActiveFile]->GetNanoSecPerFrame());
}

int VIDEOSOURCEFROMAVI::GetFormatSize()
{
	return sizeof(BITMAPINFOHEADER);
}