#include "stdafx.h"
#include "audiosource_matroska.h"
#include "audiosource_aac.h"
#include "audiosource_mp3.h"
#include "../compression.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

	/////////////////////////////////////////
	// audio source from a matroska stream //
	/////////////////////////////////////////

AUDIOSOURCEFROMMATROSKA::AUDIOSOURCEFROMMATROSKA()
{
	ZeroMemory(&info,sizeof(info));
	info.iStream = -1;
	bDelace = false;
}

AUDIOSOURCEFROMMATROSKA::~AUDIOSOURCEFROMMATROSKA()
{
}

AUDIOSOURCEFROMMATROSKA::AUDIOSOURCEFROMMATROSKA(MATROSKA* matroska, int iStream)
{
	ZeroMemory(&info,sizeof(info));
	bDelace = false;
	Open(matroska,iStream);
}

void AUDIOSOURCEFROMMATROSKA::ReInit()
{
	info.m->GetSource()->InvalidateCache();
	Seek(-33000);
}

int AUDIOSOURCEFROMMATROSKA::Open(MATROSKA* matroska, int iStream)
{
	info.m = matroska;
	info.m->SetActiveTrack(iStream);
	if (info.m->GetTrackType()!=MSTRT_AUDIO) {
		info.m = NULL;
		info.iStream = -1;
		return AS_ERR;
	}
	info.iStream = iStream;
	if (info.m->GetDefaultDuration(info.iStream) > 0) bDelace = true;
	iPos = 0x7FFFFFFF;
	iBytePosInLace = 0;
	ZeroMemory(&curr_lace,sizeof(curr_lace));
	SetTimecodeScale(info.m->GetTimecodeScale());
	SetDefault(info.m->IsDefault(info.iStream));

	char* c_codecid = info.m->GetCodecID(info.iStream);

	if (!strncmp(c_codecid, "A_MPEG/L",8)) {
		info.mpeg.iValid = 1;
		info.mpeg.iCodecIDLayer = c_codecid[8] - '0';
		MP3FRAMEHEADER* mp3f = new MP3FRAMEHEADER;
		char b[10240]; Read(b, 1, NULL, NULL, NULL);
		mp3f->SetFrameHeader(*((DWORD*)&b[0]));
		info.mpeg.iRealLayer = mp3f->GetLayerVersion();
		info.mpeg.iMPEGVersion = mp3f->GetMPEGVersion();
		delete mp3f;
		Seek(0);
	} else

	if (!strcmp(c_codecid, "A_AC3")) {
		info.ac3 = 1;
	} else
	if (!strcmp(c_codecid, "A_DTS")) {
		info.dts = 1;
	} else
	if (!strncmp(c_codecid, "A_AAC/", 6)) {
		// is aac
		info.aac.iValid = 1;
		c_codecid += 6;
		if (!strncmp(c_codecid, "MPEG2/", 6)) {
			info.aac.iMPEGVersion = 2;
		} else 
		if (!strncmp(c_codecid, "MPEG4/", 6)) {
			info.aac.iMPEGVersion = 4;
		}
		
		c_codecid += 6;
		if (!strncmp(c_codecid, "LC", 2)) {
			info.aac.iProfile = AAC_ADTS_PROFILE_LC;
			c_codecid += 3;
		} else
		if (!strncmp(c_codecid, "MAIN", 4)) {
			info.aac.iProfile = AAC_ADTS_PROFILE_MAIN;
			c_codecid += 3;
		} else
		if (!strncmp(c_codecid, "SSR", 3)) {
			info.aac.iProfile = AAC_ADTS_PROFILE_SSR;
			c_codecid += 3;
		} else
		if (!strncmp(c_codecid, "LTP", 3)) {
			info.aac.iProfile = AAC_ADTS_PROFILE_LTP;
			c_codecid += 3;
		} 

		if (*c_codecid && !strcmp(c_codecid, "SBR")) {
			info.aac.iSBR = 1;
		} else
			info.aac.iSBR = 0;

		for (int i=0;i<16;i++) {
			if (aac_sampling_frequencies[i] == GetFrequency()) {
				info.aac.iSRI[0] = i;
			}
			if (aac_sampling_frequencies[i] == 2*GetFrequency()) {
				info.aac.iSRI[1] = i;
			}
		}


	} else 
	if (!strcmp(c_codecid, "A_AAC")) {
		if (info.m->GetCodecPrivateSize() == 2) {
			info.aac.iValid = 1;
			info.aac.iSBR = 0;
		} else
		if (info.m->GetCodecPrivateSize() == 5) {
			info.aac.iValid = 1;
			info.aac.iSBR = 1;
		} else {
			info.aac.iValid = 0;
			return AS_ERR;
		}

		BYTE* b = (BYTE*)info.m->GetCodecPrivate();
		info.aac.iProfile = (*b++>>3)-1;
//		SetProfile((*b++>>3)-1);
		info.aac.iMPEGVersion = 4;
//		SetMPEGVersion(4);

		for (int i=0;i<16;i++) {
			if (aac_sampling_frequencies[i] == (int)(.5+GetFrequency()))
				info.aac.iSRI[0] = i;
			if (aac_sampling_frequencies[i] == (int)(.25+2*GetFrequency()))
				info.aac.iSRI[1] = i;
		}

//		SetSampleRateIndex((int)FormatSpecific(MMSGFS_AAC_SAMPLERATEINDEX));
		 
//		SetChannelCount(*b++>>3 & 0x7);
//		SetDefault(!(avifile->GetStreamHeader(stream)->dwFlags & AVISF_DISABLED));
	} else	{
		info.aac.iValid = 0;
		if (!strcmp(c_codecid, "A_VORBIS")) {
			info.vorbis.iValid = 1;
			ZeroMemory(info.vorbis.dwCfg, sizeof(info.vorbis.dwCfg));
			ZeroMemory(info.vorbis.pCfg, sizeof(info.vorbis.pCfg));
			BYTE* pcp = (BYTE*)info.m->GetCodecPrivate(info.iStream);

			int i = 255; int j=0; *pcp++; int k=1;
			while (j<2) {
				info.vorbis.dwCfg[j]+=(i=(*pcp++));
				if (i!=255) j++; 
				k++;
			}

			info.vorbis.pCfg[0] = (char*)pcp;
			info.vorbis.pCfg[1] = info.vorbis.pCfg[0] + info.vorbis.dwCfg[0];
			info.vorbis.pCfg[2] = info.vorbis.pCfg[1] + info.vorbis.dwCfg[1];
			info.vorbis.dwCfg[2] = info.m->GetCodecPrivateSize()
				- info.vorbis.dwCfg[0] - info.vorbis.dwCfg[1] - k;
		}
	}

	UpdateDuration(info.m->GetMasterTrackDuration());

	AllowAVIOutput(false);
	return AS_OK;
}

int AUDIOSOURCEFROMMATROSKA::GetStrippableHeaderBytes(void* pBuffer, int max)
{
	char* c_codecid = info.m->GetCodecID(info.iStream);
	
	if (!strcmp(c_codecid, "A_AC3")) {
		if (max > 0)
			((unsigned char*)pBuffer)[0] = 0x0B;
		if (max > 1)
			((unsigned char*)pBuffer)[1] = 0x77;
		return 2;
	}
	if (!strcmp(c_codecid, "A_DTS")) {
		unsigned char b[] = {  0x7F, 0xFE, 0x80, 0x01 };
		memcpy(pBuffer, (void*)b, min(max, 4));
		return 4;
	}


	if (GetSource()->GetTrackCompression(GetSourceStream(), 0) == COMPRESSION_HDRSTRIPPING) {
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

MATROSKA* AUDIOSOURCEFROMMATROSKA::GetSource()
{
	return info.m;
}

int AUDIOSOURCEFROMMATROSKA::GetSourceStream()
{
	return info.iStream;
}

int AUDIOSOURCEFROMMATROSKA::doClose()
{
	ZeroMemory(&info,sizeof(info));

	return AS_OK;
}

int AUDIOSOURCEFROMMATROSKA::GetAvgBytesPerSec()
{
	if (info.m->IsBitrateIndicated(info.iStream)) {
		return (int)((info.m->GetTrackBitrate(info.iStream)+4)/8);
	}

	return 0;
}

int AUDIOSOURCEFROMMATROSKA::Enable(int bEnable)
{
	info.m->EnableQueue(info.iStream,bEnable);
	return 0;
}

int AUDIOSOURCEFROMMATROSKA::GetFormatTag()
{
	return 0;
}

char* AUDIOSOURCEFROMMATROSKA::GetCodecID()
{
	info.m->SetActiveTrack(info.iStream);
	return info.m->GetCodecID();
}

int AUDIOSOURCEFROMMATROSKA::GetChannelCount()
{
	info.m->SetActiveTrack(info.iStream);
	return info.m->GetChannelCount();
}

int AUDIOSOURCEFROMMATROSKA::GetGranularity()
{
	if (info.iFramesize) return info.iFramesize;

	return (int)info.m->GetAvgFramesize(info.iStream);
}

__int64 AUDIOSOURCEFROMMATROSKA::GetFrameDuration()
{
	return info.m->GetDefaultDuration(info.iStream);
}

int AUDIOSOURCEFROMMATROSKA::GetFrequency()
{
	info.m->SetActiveTrack(info.iStream);
	return (int)info.m->GetSamplingFrequency();
}

int AUDIOSOURCEFROMMATROSKA::GetOutputFrequency()
{
	info.m->SetActiveTrack(info.iStream);
	return (int)info.m->GetOutputSamplingFrequency();
}

bool AUDIOSOURCEFROMMATROSKA::IsEndOfStream()
{
	info.m->SetActiveTrack(info.iStream);

	bool bRes = (info.m->IsEndOfStream() && (iPos >= (int)curr_lace.frame_sizes.size()));
	if (bRes) {
		info.m->IsEndOfStream();
	}
	return bRes;
}

__int64 AUDIOSOURCEFROMMATROSKA::GetUnstretchedDuration()
{
	return info.m->GetMasterTrackDuration()* info.m->GetTimecodeScale() / GetTimecodeScale();
}

int AUDIOSOURCEFROMMATROSKA::GetOffset()
{
	return 0;
}

int AUDIOSOURCEFROMMATROSKA::Seek(__int64 _iPos)
{
	info.m->Seek(_iPos);
	iPos=0x7fffffff;
	return 0;
}

int AUDIOSOURCEFROMMATROSKA::GetBitDepth()
{
	info.m->SetActiveTrack(info.iStream);
	return info.m->GetBitDepth();
}

void* AUDIOSOURCEFROMMATROSKA::GetFormat()
{
	info.m->SetActiveTrack(info.iStream);
	return info.m->GetCodecPrivate();
}

int AUDIOSOURCEFROMMATROSKA::GetName(char* lpDest)
{
	info.m->SetActiveTrack(info.iStream);
	char* c=info.m->GetTrackName();
	if (c && strcmp(c,"")) {
		int l;
		memcpy(lpDest,c,l=strlen(c));
		return l;
	} else {
		*lpDest = 0;
		return 0;
	}
}

int AUDIOSOURCEFROMMATROSKA::GetLanguageCode(char* lpDest)
{
	info.m->SetActiveTrack(info.iStream);
	char* c=info.m->GetLanguage();
	if (c && strcmp(c,"")) {
		int l;
		memcpy(lpDest,c,l=strlen(c));
		return l;
	} else {
		*lpDest = 0;
		return 0;
	}
}

__int64 AUDIOSOURCEFROMMATROSKA::GuessTotalSize()
{
	return info.m->GetTrackSize(info.iStream);
}

int AUDIOSOURCEFROMMATROSKA::Read(void* lpDest, DWORD dwMicrosecDesired,DWORD* lpdwMicrosecRead,
								  __int64* lpqwNanosecRead,__int64* lpiTimecode, ADVANCEDREAD_INFO* lpAARI)
{
	int iSize;
	__int64 iNTC = 0;
	__int64 iBias = GetBias();
//	int rbres;

	READ_INFO r;
	info.m->SetActiveTrack(info.iStream);
	if (lpqwNanosecRead) {
		*lpqwNanosecRead = 0;
	}

	if (!bDelace) {
		// do not delace input data
		if (info.m->Read(&r)==READBL_OK) {
			if (lpAARI) {
				if (r.frame_sizes.size() >= 1) {
					lpAARI->iFramecount = r.frame_sizes.size();
					lpAARI->iFramesizes = new int[lpAARI->iFramecount];
					for (int j=0;j<lpAARI->iFramecount;j++)
						lpAARI->iFramesizes[j] = r.frame_sizes[j];

				} else {
					lpAARI->iFramecount = 0;
					lpAARI->iFramesizes = 0;
				}
				if (r.iFlags & RBIF_DURATION) {
					lpAARI->iDuration = r.qwDuration;
					if (lpqwNanosecRead) {
						*lpqwNanosecRead = r.qwDuration * info.m->GetTimecodeScale();
					}
				} else lpAARI->iDuration = 0;
			}
	
			if (lpiTimecode) 
				*lpiTimecode = r.qwTimecode + iBias;

			if (lpAARI && (iNTC = info.m->GetNextTimecode(info.iStream))>0)
				lpAARI->iNextTimecode = iNTC + GetBias();
			else 
				lpAARI->iNextTimecode = TIMECODE_UNKNOWN;
			
			memcpy(lpDest,r.pData->GetData(),r.pData->GetSize());
			iSize = r.pData->GetSize();
			DecBufferRefCount(&r.pData);
			info.iFramesize = iSize / max(1,r.frame_sizes.size());
			iPos = curr_lace.frame_sizes.size()+1;
		} else {

			return AS_ERR;
		}
	} else {
		// delace input data
		if (iPos >= (int)curr_lace.frame_sizes.size()) {
			// end of current lace reached -> read new one
			if (curr_lace.pData) DecBufferRefCount(&curr_lace.pData);
			ZeroMemory(&curr_lace,sizeof(curr_lace));
			if (info.m->Read(&curr_lace)!=READBL_OK) {
				return AS_ERR;
			}
			iPos = 0; iBytePosInLace = 0;
		}

		__int64 iDur = GetFrameDuration();
		if (lpqwNanosecRead) {
			*lpqwNanosecRead = iDur;
		}

		// determine size of next frame: either single frame or current frame in current lace
		iSize = (curr_lace.frame_sizes.size() > 1)?curr_lace.frame_sizes[iPos]:curr_lace.pData->GetSize();

		if (lpiTimecode) *lpiTimecode = curr_lace.qwTimecode + GetBias() + iDur*iPos/GetTimecodeScale();
		
		// next timecode
		if (lpAARI) {
			if (iPos<(int)curr_lace.frame_sizes.size() - 1) {
				lpAARI->iNextTimecode = curr_lace.qwTimecode + GetBias() + iDur*(iPos+1)/GetTimecodeScale();
			} else {
				iNTC = info.m->GetNextTimecode(info.iStream);
				if (iNTC!=TIMECODE_UNKNOWN) {
					lpAARI->iNextTimecode = iNTC + iBias;
				} else {
					printf("next timecode unknown!\n");
					if (info.m->IsEndOfSegment()) {
						lpAARI->iNextTimecode = info.m->GetMasterTrackDuration() + iBias;
						lpAARI->iFileEnds = 1;
					} else {
						lpAARI->iNextTimecode = TIMECODE_UNKNOWN;
					}
				}
			}
		}
		memcpy(lpDest,(curr_lace.pData->AsString())+iBytePosInLace,iSize);

		iBytePosInLace += iSize;
		iPos++;
	};

	return iSize;
}

bool AUDIOSOURCEFROMMATROSKA::IsCBR()
{
	return false;
}

__int64 AUDIOSOURCEFROMMATROSKA::FormatSpecific(__int64 iCode, __int64 iValue)
{
	DWORD** p = (DWORD**)&iValue;
	switch (iCode) {
		case MMSGFS_IS_AAC: return info.aac.iValid;
		case MMSGFS_IS_AC3: return info.ac3;
		case MMSGFS_IS_DTS: return info.dts;
		case MMSGFS_IS_MPEG: return info.mpeg.iValid;
		case MMSGFS_MPEG_VERSION: return (info.mpeg.iValid)?info.mpeg.iMPEGVersion:0;
		case MMSGFS_MPEG_LAYERVERSION: return (info.mpeg.iValid)?info.mpeg.iRealLayer:0;
		case MMSGFS_AAC_PROFILE: return info.aac.iProfile;
		case MMSGFS_AAC_MPEGVERSION: return info.aac.iMPEGVersion;
		case MMSGFS_AAC_ISSBR: return info.aac.iSBR;
		case MMSSFS_AAC_SETSBR: info.aac.iSBR = !!iValue; return 1; break;
		case MMSGFS_AAC_SAMPLERATEINDEX: return info.aac.iSRI[0]; break;
		case MMSGFS_AAC_DOUBLESAMPLERATEINDEX: return info.aac.iSRI[1]; break;
		case MMSGFS_IS_VORBIS: return info.vorbis.iValid; break;
		case MMSGFS_VORBIS_CONFIGPACKETS:
			p[0][0] = info.vorbis.dwCfg[0];
			p[0][1] = info.vorbis.dwCfg[1];
			p[0][2] = info.vorbis.dwCfg[2];
			p[1][0] = (DWORD)info.vorbis.pCfg[0];
			p[1][1] = (DWORD)info.vorbis.pCfg[1];
			p[1][2] = (DWORD)info.vorbis.pCfg[2];
			break;

	}

	return 0;
}
