#include "stdafx.h"
#include "VideoSource_Matroska.h"
#include "../compression.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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

MATROSKA* VIDEOSOURCEFROMMATROSKA::GetSource()
{
	return info.m;
}

int VIDEOSOURCEFROMMATROSKA::GetSourceStream()
{
	return info.iStream;
}

bool VIDEOSOURCEFROMMATROSKA::IsOpen()
{
	return info.iStream > -1;
}

void VIDEOSOURCEFROMMATROSKA::ReInit()
{
	info.m->GetSource()->InvalidateCache();
	Seek(-40000);
}

int VIDEOSOURCEFROMMATROSKA::GetStrippableHeaderBytes(void* pBuffer, int max)
{
	char* c_codecid = GetSource()->GetCodecID(GetSourceStream());

	if (!strcmp(c_codecid, "V_MPEG4/ISO/ASP")) {
		unsigned char b[] = {  0x00, 0x00, 0x01, 0xB6 };
		memcpy(pBuffer, (void*)b, max(max, 4));
		return 4;
	}
	if (!strcmp(c_codecid, "V_MPEG4/ISO/AVC")) {
		return 0;
	}

	if (GetSource()->GetTrackCompression(GetSourceStream(), 0) == COMPRESSION_HDRSTRIPING) {
		int size = GetSource()->GetTrackCompressionPrivateSize(GetSourceStream(), 0);
		void* p = malloc(size);
		GetSource()->GetTrackCompressionPrivate(GetSourceStream(), 0, p);
		int bytes_to_copy = min(size, max);
		memcpy(pBuffer, p, bytes_to_copy);
		free(p);
		return size;
	} else
		return MMS_UNKNOWN;
}

int VIDEOSOURCEFROMMATROSKA::GetLanguageCode(char* lpDest)
{
	char* c = info.m->GetLanguage(info.iStream);
	if (c && c[0]) {
		strcpy(lpDest, c);
		return strlen(c);
	} else {
		*lpDest = 0;
		return 0;
	}
}

int VIDEOSOURCEFROMMATROSKA::GetName(char* lpDest)
{
	char* c = info.m->GetTrackName(info.iStream);
	if (c && c[0]) {
		strcpy(lpDest, c);
		return strlen(c);
	} else {
		*lpDest = 0;
		return 0;
	}
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
	info.size = 0;
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
			RECT r; info.m->GetCropping(&r); int x; int y;
			info.m->GetResolution(&x, &y, NULL, NULL, NULL);
			avistreamheader.rcFrame.left   = r.left;
			avistreamheader.rcFrame.top    = r.top;
			avistreamheader.rcFrame.right  = x - r.right;
			avistreamheader.rcFrame.bottom = y - r.bottom;
			AllowAVIOutput(1);
		} else {
			// can't determine framerate if (!DefaultDuration)
			AllowAVIOutput(0);
		}
	}

	if (info.m->IsDefault())
		SetDefault(true);

	if (info.m->IsBitrateIndicated(info.iStream)) {
		return VS_OK;
	}

	if (info.m->GetTrackCount() == 1) {
		info.size = info.m->GetSize();
		return VS_OK;
	}


	UpdateDuration(info.m->GetMasterTrackDuration());
	ReInit();
	void* c = new char[1<<20];
	int	iTime = GetTickCount();
	int jMax = (int)(info.m->GetSegmentDuration() * info.m->GetTimecodeScale() / 1000000000 / 300);
	if (jMax > 10) jMax = 10;

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

DWORD VIDEOSOURCEFROMMATROSKA::GetFourCC()
{
	if (info.m) 
		return avistreamheader.fccHandler;

	return 0;
}

__int64 VIDEOSOURCEFROMMATROSKA::GetExactSize()
{
	if (info.m->IsBitrateIndicated()) {
		return info.m->GetTrackSize(info.iStream);
	} else {
		return info.size;
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
	info.m->GetCropping(&r->rcCrop);
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
	if ((size_t)iPos >= curr_lace.frame_sizes.size() || !r->pData) {
		if (r->pData) DecBufferRefCount(&r->pData);

		info.m->SetActiveTrack(info.iStream);
		ZeroMemory(r,sizeof(*r));
		if (info.m->Read(&curr_lace) != READBL_OK) {
			if (lpdwSize) *lpdwSize = 0;
			return VS_ERROR;
		}
		if (curr_lace.pData->GetSize() == 0) {
			Sleep(1);
		}

		iNextTimecode = info.m->GetNextTimecode();
		iPos = 0;
		iBytePosInLace = 0;

		int iRefCount = r->references.size();
		SetCurrentTimecode(r->qwTimecode);
		std::vector<__int64> refs;
		for (size_t i=0;i<r->references.size();i++)
			refs.push_back((r->references[i] + GetCurrentTimecode()) * GetTimecodeScale());
		SetReferencedFramesAbsolute(GetCurrentTimecode() * GetTimecodeScale(), 
			refs);

	}

	if (curr_lace.frame_sizes.size() > 1) {
		__int64 iDur = iNextTimecode - curr_lace.qwTimecode;
		__int64 iCurrTC = curr_lace.qwTimecode + iDur * iPos / curr_lace.frame_sizes.size();
		__int64 iReference = iCurrTC - GetCurrentTimecode();
		__int64 iNTC;
		
		if ((int)curr_lace.frame_sizes.size() > iPos + 1) {
			iNTC = curr_lace.qwTimecode + iDur * (iPos+1) / curr_lace.frame_sizes.size();
		} else {
			iNTC = info.m->GetNextTimecode(info.iStream);
		}

		SetCurrentTimecode(iCurrTC);
		//SetLatestReference(1, iReference * info.m->GetTimecodeScale(), 0);
		SetReferencedFramesAbsolute(1, GetCurrentTimecode(), 
			iReference * GetTimecodeScale());
		
		if (lpdwSize) 
			*lpdwSize = r->frame_sizes[iPos];

		memcpy(lpDest, ((BYTE*)r->pData->GetData()) + iBytePosInLace, r->frame_sizes[iPos]);
		if (lpAARI) {
			lpAARI->iNextTimecode = iNTC;
			if (r->iFlags & RBIF_DURATION) {
				lpAARI->iDuration = r->qwDuration * info.m->GetTimecodeScale() / GetTimecodeScale();
			} else {
				lpAARI->iDuration = info.m->GetDefaultDuration(info.iStream) / GetTimecodeScale();
			}
		}

		iBytePosInLace += r->frame_sizes[iPos++];//iFrameSizes[iPos++];
	} else {
		if (lpiTimecode) 
			*lpiTimecode = GetCurrentTimecode();

		memcpy(lpDest,r->pData->GetData(),r->pData->GetSize());
		if (lpdwSize) 
			*lpdwSize = r->pData->GetSize();

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
		iPos++;
		return VS_OK;
	}

	return VS_ERROR;
}

void* VIDEOSOURCEFROMMATROSKA::GetFormat()
{
	info.m->SetActiveTrack(info.iStream);
	return info.m->GetCodecPrivate();
}

char* VIDEOSOURCEFROMMATROSKA::GetCodecID()
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
	return -1;
}

bool VIDEOSOURCEFROMMATROSKA::IsEndOfStream()
{
	info.m->SetActiveTrack(info.iStream);
	return ((!curr_lace.pData || (int)curr_lace.frame_sizes.size() <= iPos) && info.m->IsEndOfStream());
}

AVIStreamHeader* VIDEOSOURCEFROMMATROSKA::GetAVIStreamHeader()
{
	return &avistreamheader;
}

void VIDEOSOURCEFROMMATROSKA::GetCropping(RECT* r)
{
	info.m->SetActiveTrack(info.iStream);
	info.m->GetCropping(r);
}