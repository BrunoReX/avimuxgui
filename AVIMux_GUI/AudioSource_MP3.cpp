#include "stdafx.h"
#include "audiosource_mp3.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

	//////////////////////
	// MP3 audio source //
	//////////////////////

DWORD BSWAP (DWORD x)
{ 
	DWORD dwRes;
	_asm	
	{ 
		mov eax, x
		bswap eax
		mov dwRes, eax
	};
	return dwRes;
};


MP3SOURCE::MP3SOURCE(void)
{
	fh=NULL;
	bCBR=false;
}


MP3SOURCE::MP3SOURCE(STREAM* lpStream)
{
	fh=NULL;
	bCBR=false;
	Open(lpStream);
}


MP3SOURCE::~MP3SOURCE(void)
{
}

int	MP3SOURCE::doClose()
{
	delete fh;
	AUDIOSOURCE::doClose();
	return AS_OK;
}

DWORD MP3SOURCE::ReadFrameHeader(void)
{
	DWORD	dwRes;

	if (GetSource()->Read(&dwRes,4)!=4) {
		dwRes=0;
	}
	return dwRes;
}

int MP3SOURCE::Open(STREAM* lpStream)
{
	if (!lpStream || lpStream->GetSize() <= 0)
		return AS_ERR;

	DWORD		dwHeader=0;
	int			iPadd,i;
	byte		data[2000];
	DWORD		dwStart=0;
	__int64	qwPos=0;

	dwRingBufferPos=0;
	fh=new MP3FRAMEHEADER;
	AUDIOSOURCEFROMBINARY::Open(lpStream);
	GetSource()->Read(&dwHeader,4);
	fh->SetFrameHeader(dwHeader);
	if (!fh->SyncOK()||(!fh->GetFrequency())||(!fh->GetBitrate()))
	{
		// versuche zu resynchronisieren
		do
		{
			do
			{	GetSource()->Seek(GetSource()->GetPos()-3);
				GetSource()->Read(&dwHeader,4);
				fh->SetFrameHeader(dwHeader);
				qwPos=GetSource()->GetPos()+GetSource()->GetOffset();


			} while ((!fh->SyncOK())&&(qwPos<GetResyncRange())&&(!IsEndOfStream()));

			dwStart=(DWORD)(GetSource()->GetPos()-4);

			GetSource()->Seek(dwStart);
			ReadFrame(data,0,0);
			ReadFrame(data,0,0);
			GetSource()->Read(&dwHeader,4);
			fh->SetFrameHeader(dwHeader);
			dwFrequency=fh->GetFrequency();

		}
		while ( (!fh->SyncOK()||(!dwFrequency)||(!fh->GetBitrate())) && (qwPos<GetResyncRange()) & 
			(!IsEndOfStream()));		
		GetSource()->SetOffset(dwStart);
		if (GetSource()->GetOffset()>=GetResyncRange()-4) 
		{
			GetSource()->SetOffset(0);
			return AS_ERR;
		}
	}
	dwFrequency=fh->GetFrequency();
	if (!fh->GetBitrate())
	{
		GetSource()->SetOffset(0);
		GetSource()->Seek(0);
		return AS_ERR;
	}
	if (!(dwFrequency|fh->GetBitrate()|fh->GetFrameSize(&iPadd))) return AS_ERR;

	dwNanoSecPerFrame=(DWORD)((__int64)8000000*(__int64)fh->GetFrameSize(&iPadd)/fh->GetBitrate());
	dwBitrate=fh->GetBitrate();
	dwFrameSize=fh->GetFrameSize(&iPadd);
	dwChannels=fh->GetChannelsNum();
	dwLayer=fh->GetLayerVersion();
	dwMode=fh->GetMode();
	dwHasCRC=fh->HasCRC();
	dwModeEx=fh->GetModeEx();
	dwMPEGVersion=fh->GetMPEGVersion();
	dwOriginal=fh->IsOriginal();
	dwCopyright=fh->IsCopyright();
	dwPrivate=fh->IsPrivate();
	dwEmphasis=fh->GetEmphasis();
	for (i=0;i<1024;dwRingBuffer[i++]=dwBitrate);
	dwRingBufferPos=0;
	GetSource()->Seek(0);
	SetFrameMode(FRAMEMODE_ON);

	if (dwBitrate>0) {
		float fSize; int iPadd;
		fh->GetFrameSize(&iPadd,&fSize);
		iFrameDuration=round((double)8000000*(double)fSize/(double)dwBitrate);
	} else {
		iFrameDuration=FRAMEDURATION_UNKNOWN;
	}

	return (/*dwFrameSize && dwBitrate &&*/ qwPos<GetResyncRange() && qwPos < GetSize());
}

int MP3SOURCE::GetStrippableHeaderBytes(void* pBuffer, int max)
{
	if (max > 0)
		((unsigned char*)pBuffer)[0] = 0xFF;

	return 1;
}

__int64 MP3SOURCE::GetFrameDuration()
{
	return iFrameDuration;
}

/*__int64 MP3SOURCE::GetUnstretchedDuration()
{
	return GetMaxLength();
}
*/
int	MP3SOURCE::GetAvgBytesPerSec()
{
	DWORD	dwSum=0;
	int		i;

	for(i=0;i<1000;dwSum+=dwRingBuffer[i++]);

	return (dwSum>>3);
}

bool MP3FRAMEHEADER::SyncOK(void)
{
	return ((dwFrameHeader>>21)==2047);
}

int MP3FRAMEHEADER::GetMPEGVersionIndex(void)
{
	return ((dwFrameHeader>>19)&3);
}

int MP3FRAMEHEADER::GetLayerIndex(void)
{
	return ((dwFrameHeader>>17)&3);
}

int MP3FRAMEHEADER::GetLayerVersion(void)
{
	DWORD dwLI=GetLayerIndex();
	return (dwLI)?(4-dwLI):0;
}

// idiot note: that flag is a notprotection flag in the mpeg frame headers
int MP3FRAMEHEADER::HasCRC(void)
{
	return !((dwFrameHeader>>16)&1);
}

int MP3FRAMEHEADER::GetBitrate(void)
{
	DWORD dwBitrateIndex=((dwFrameHeader>>12)&15);
	DWORD BitRates[2][3][16]={
		{
			// MPEG 2 und 2.5
			{0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,0}, // Layer III
			{0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,0}, // Layer II
	        {0, 32, 48, 56, 64, 80, 96,112,128,144,160,176,192,224,256,0}  // Layer I
		},
		{
			{0, 32, 40, 48, 56, 64, 80, 96,112,128,160,192,224,256,320,0},
			{0, 32, 48, 56, 64, 80, 96,112,128,160,192,224,256,320,384,0},
			{0, 32, 64, 96,128,160,192,224,256,288,320,352,384,416,448,0}				
		}
	};
	if (GetLayerIndex())
	{
		return BitRates[GetMPEGVersion()&1][GetLayerIndex()-1][dwBitrateIndex];
	}
	else
	{
		return 0;
	}
}

int MP3FRAMEHEADER::GetFrequencyIndex(void)
{
	return ((dwFrameHeader>>10)&3);
}

int MP3FRAMEHEADER::GetFrequency(void)
{
	
    int Frequencies[4][3] = {

        {32000, 16000,  8000}, //MPEG 2.5
        {    0,     0,     0}, //reserved
        {22050, 24000, 16000}, //MPEG 2
        {44100, 48000, 32000}  //MPEG 1
	};
	if ((GetMPEGVersionIndex()>0)&&(GetFrequencyIndex()<3))
	{
		return (Frequencies[GetMPEGVersionIndex()][GetFrequencyIndex()]);
	}
	else
		return 0;
}

int MP3FRAMEHEADER::GetPadding(void)
{
	DWORD dwTemp=(dwFrameHeader>>9)&1;
	return (dwTemp*((GetLayerVersion()==1)?4:1));
}


int MP3FRAMEHEADER::GetChannelsNum(void)
{
	if (((dwFrameHeader >> 6) & 0x03) == 3 ) return 1;
	else return 2;
}

int MP3FRAMEHEADER::GetMPEGVersion(void)
{
	switch ((dwFrameHeader>>19)&3)
	{
		case 0: return 4; break;
		case 1: return -1; break;
		case 2: return 2; break;
		case 3: return 1; break;
	}
	return -1;
}



int MP3FRAMEHEADER::GetModeEx(void)
{
	return ((dwFrameHeader>>4)&3);
}

int MP3FRAMEHEADER::GetMode(void)
{
	return ((dwFrameHeader>>6)&3);
}

int MP3FRAMEHEADER::IsCopyright(void)
{
	return ((dwFrameHeader>>3)&1);
}

int MP3FRAMEHEADER::IsOriginal(void)
{
	return ((dwFrameHeader>>2)&1);
}

int MP3FRAMEHEADER::IsPrivate(void)
{
	return ((dwFrameHeader>>8)&1);
}

int MP3FRAMEHEADER::GetEmphasis(void)
{
	return ((dwFrameHeader)&3);
}

int MP3FRAMEHEADER::GetFrameSize(int* lpiPadd, float* fSize)
{
	DWORD		dwFreq;
	if (!(dwFreq=GetFrequency())) return 0;
	DWORD		dwPadd=GetPadding();
	DWORD		dwBitrate=GetBitrate();
	
	if ((!dwFreq)||(!dwBitrate))
	{
		return 0;
	}
	if (lpiPadd) *lpiPadd=dwPadd;

//	dwPadd=0;

    switch (GetMPEGVersion())
	{
		case 1:	switch (GetLayerVersion())
			{
				case 0: return 0; break;
				case 1:	if (fSize) *fSize = 48000.0f*(float)dwBitrate/(float)dwFreq;
						return (48000*dwBitrate/dwFreq+dwPadd); break;
				case 2: 
				case 3: if (fSize) *fSize = 144000.0f*(float)dwBitrate/(float)dwFreq;
						return (144000*dwBitrate/dwFreq+dwPadd); break;
				case 4: return 0; break;
			} break;
		case 2:
		case 4: switch (GetLayerVersion())
			{
				case 0: return 0; break;
				case 1: return (24000*dwBitrate/dwFreq+dwPadd); break;
				case 2: 
				case 3: if (fSize) *fSize = 72000.0f*(float)dwBitrate/(float)dwFreq;
						return (72000*dwBitrate/dwFreq+dwPadd); break;
				case 4: return 0; break;
			} break;
	}

	return 0;
}

int MP3SOURCE::ReadFrame(void* lpDest,DWORD* lpdwMicroSecRead,__int64* lpqwNanoSecRead)
{
	DWORD	dwRes=0;
	DWORD*	lpdwDest=(DWORD*)lpDest;
	int		iPadd=0;
	__int64	qwOldPos;
	float	fSize;

	qwOldPos=GetSource()->GetPos();
	fh->SetFrameHeader(*lpdwDest++=ReadFrameHeader());
	DWORD	dwFrameSize=fh->GetFrameSize(&iPadd,&fSize);
	
	if (lpdwMicroSecRead) *lpdwMicroSecRead=0;
	if (lpqwNanoSecRead) *lpqwNanoSecRead=0;
	if (fh->SyncOK()&&(dwFrameSize)&&(fh->GetBitrate()))
	{
		dwRingBuffer[dwRingBufferPos++]=fh->GetBitrate();
		dwRingBufferPos%=1024;
		if (lpdwMicroSecRead) *lpdwMicroSecRead=(DWORD)round((double)(fSize)/(double)fh->GetBitrate()*8000.0);
		if (lpqwNanoSecRead) *lpqwNanoSecRead=round((double)(fSize)/(double)fh->GetBitrate()*8000000.0);
		return (GetSource()->Read(lpdwDest,dwFrameSize-4)+4);
	}
	else
	{
		GetSource()->Seek(qwOldPos);
		return 0;
	}
}

int MP3SOURCE::GetFormatTag()
{
	return 0x0055;
}

int MP3SOURCE::HasCRC()
{
	return dwHasCRC;
}

int MP3SOURCE::doRead(void* lpDest,DWORD dwMicroSecDesired,DWORD* lpdwMicroSecRead,__int64* lpqwNanoSecRead)
{
	DWORD	dwBytes,dwRes,dwRead;
	char*	lpcDest=(char*)lpDest;
	DWORD	dwMicroSec;
	__int64 qwNanoSec;
// mp3 vbr does not care about dwMicroSecDesired!
// a fixed #frames is read
	if (!bCBR) 
	{
		int j = GetFrameMode();

		if (j==FRAMEMODE_ON || j == FRAMEMODE_SINGLEFRAMES) {
			return ReadFrame(lpDest,lpdwMicroSecRead,lpqwNanoSecRead);
		} else {
			dwRead = 0;
			if (*lpqwNanoSecRead) *lpqwNanoSecRead = 0;
			for (int i=0;i<j;i++) {
				dwRead+=ReadFrame(lpcDest+dwRead, lpdwMicroSecRead, &qwNanoSec);
				(*lpqwNanoSecRead) += qwNanoSec;
				if (lpdwMicroSecRead) (*lpdwMicroSecRead) = (DWORD)((*lpqwNanoSecRead)/1000);
			}
			return dwRead;
		}
	}
	else
	{
		switch (GetFrameMode())
		{
			case FRAMEMODE_OFF:
			// byte mode
				dwBytes=(DWORD)((__int64)dwMicroSecDesired*(__int64)dwBitrate/8000);
				dwRes=GetSource()->Read(lpDest,dwBytes);
				if (lpdwMicroSecRead) *lpdwMicroSecRead=(DWORD)round(8000.0*(double)dwRes/(double)dwBitrate);
				if (lpqwNanoSecRead) *lpqwNanoSecRead=round((double)8000000*(double)dwRes/(double)dwBitrate);
				return dwRes;
				break;
			case FRAMEMODE_ON:
			// frame mode
				dwBytes=(DWORD)((__int64)dwMicroSecDesired*(__int64)dwBitrate/8000);
				dwRead=1;
				dwRes=0;
				if (lpdwMicroSecRead) (*lpdwMicroSecRead)=0;
				if (lpqwNanoSecRead) (*lpqwNanoSecRead)=0;
				while (dwRead&&(dwRes<dwBytes))
				{
					dwRead=ReadFrame(lpcDest+dwRes,&dwMicroSec,&qwNanoSec);
					if (lpdwMicroSecRead) (*lpdwMicroSecRead)+=dwMicroSec;
					if (lpqwNanoSecRead) (*lpqwNanoSecRead)+=qwNanoSec;
					dwRes+=dwRead;
				}
				return dwRes;
				break;
			// single frame mode
			case FRAMEMODE_SINGLEFRAMES:
				return ReadFrame(lpDest,lpdwMicroSecRead,lpqwNanoSecRead);
				break;
			default:
				return 0;
		}

	}
}

int MP3SOURCE::GetFrequency(void)
{
	return dwFrequency;
}

int MP3SOURCE::GetMicroSecPerFrame(void)
{
	return (DWORD)round(dwNanoSecPerFrame/1000);
}

__int64 MP3SOURCE::GetNanoSecPerFrame(void)
{
	return dwNanoSecPerFrame;
}

int MP3SOURCE::GetChannelCount(void)
{
	return dwChannels;
}

int MP3SOURCE::GetMode(void)
{
	return dwMode;
}

int MP3SOURCE::GetModeEx(void)
{
	return dwModeEx;
}

int MP3SOURCE::IsPrivate(void)
{
	return dwPrivate;
}

int MP3SOURCE::IsCopyrighted()
{
	return dwCopyright;
}

int MP3SOURCE::IsOriginal()
{
	return dwOriginal;
}

int MP3SOURCE::GetEmphasis()
{
	return dwEmphasis;
}

__int64 MP3SOURCE::FormatSpecific(__int64 iCode, __int64 iValue)
{
	if (iCode == MMSGFS_MPEG_LAYERVERSION) return GetLayerVersion();
	if (iCode == MMSGFS_MPEG_VERSION) return GetMPEGVersion();
	if (iCode == MMSGFS_IS_MPEG) return true;

	return 0;
}

bool MP3SOURCE::ScanForCBR(DWORD dwNbrOfFrames)
{
	int		iFirstFrameSize,iFrameSize;
	__int64 qwPos;
	__int64 q;
	DWORD	i=0;
	bool	bVBRFound=false;
	bool	bConstantFrameSize=true;
	bool	bEndOfRange=false;
	int		iPadd;

	qwPos=GetSource()->GetPos();
	iFirstFrameSize=fh->GetFrameSize(&iPadd);
	iFrameSize=iFirstFrameSize;
	while ( bConstantFrameSize=(abs((iFrameSize=fh->GetFrameSize(&iPadd))-iFirstFrameSize)<=1),
//		 bEndOfRange=GetSource()->IsEndOfStream(),
		 bEndOfRange|=(dwNbrOfFrames==SCANFORCBR_ALL)?0:(i>=dwNbrOfFrames),
		 bConstantFrameSize&&(!bEndOfRange)&&(dwFrameSize)
		)
	{
		fh->SetFrameHeader(ReadFrameHeader());
		dwFrameSize=fh->GetFrameSize(&iPadd);
		GetSource()->Seek(GetSource()->GetPos()+dwFrameSize-4);
		q=GetSource()->GetPos();
		bEndOfRange=GetSource()->IsEndOfStream();
		i++;
	}
	bVBRFound=(abs(iFrameSize-iFirstFrameSize)>1) && !GetSource()->IsEndOfStream();
	GetSource()->Seek(qwPos);

	if ((!bVBRFound)&&(dwNbrOfFrames==SCANFORCBR_ALL)) bCBR=true;

	return (!bVBRFound);
}

void MP3SOURCE::AssumeCBR()
{
	bCBR=true;
}

void MP3SOURCE::AssumeVBR()
{
	bCBR=false;
}

bool MP3SOURCE::IsAVIOutputPossible()
{
	return (GetLayerVersion() == 3 || IsCBR());
}

int MP3SOURCE::GetMPEGVersion()
{
	return (int)dwMPEGVersion;
}

int MP3SOURCE::IsCompatible(AUDIOSOURCE* a)
{
	if (!a->FormatSpecific(MMSGFS_IS_MPEG))
		return MMSIC_COMPRESSION;

	if (GetLayerVersion() != a->FormatSpecific(MMSGFS_MPEG_LAYERVERSION))
		return MMSIC_MPEG_LAYERVERSION;

	if (GetMPEGVersion() != a->FormatSpecific(MMSGFS_MPEG_VERSION))
		return MMSIC_MPEG_VERSION;

	return MMS_COMPATIBLE;
}