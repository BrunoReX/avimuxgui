#include "StdAfx.h"
#include "FillASIs.h"

void FillAC3_ASI (AUDIO_STREAM_INFO** asi,AC3SOURCE* ac3source)
{
	WAVEFORMATEX*	lpwfe;

	if (!(*asi)) 
	{
		*asi=new AUDIO_STREAM_INFO;
		ZeroMemory(*asi,sizeof(AUDIO_STREAM_INFO));
		(*asi)->dwFlags=ASIF_ALLOCATED;
	}
	(*asi)->dwType=AUDIOTYPE_AC3;
	if (!(*asi)->lpASH)
	{
		(*asi)->lpASH=new AVIStreamHeader;
	}
	ZeroMemory((*asi)->lpASH,sizeof(AVIStreamHeader));
	(*asi)->lpASH->dwRate=ac3source->GetAvgBytesPerSec();
	(*asi)->lpASH->dwScale=1;
	(*asi)->lpASH->dwSampleSize=1;
	(*asi)->lpASH->fccType=MakeFourCC("auds");
	(*asi)->lpASH->dwQuality=0xFFFFFFFF;
	(*asi)->lpASH->dwSuggestedBufferSize=0;//4*ac3source->GetFrameSize();
	
	if (!(*asi)->lpFormat)
	{
		(*asi)->lpFormat=new WAVEFORMATEX;
	}
	ZeroMemory((*asi)->lpFormat,sizeof(WAVEFORMATEX));
	lpwfe=(WAVEFORMATEX*)((*asi)->lpFormat);
	lpwfe->wFormatTag=0x2000;
	lpwfe->nChannels=ac3source->GetChannelCount();
	lpwfe->nBlockAlign=1;
	lpwfe->cbSize=0;
	lpwfe->nSamplesPerSec=ac3source->GetFrequency();
	(*asi)->audiosource=ac3source;
}

void FillAAC_ASI(AUDIO_STREAM_INFO** asi,AACSOURCE* aacsource)
{
	WAVEFORMATEX*			lpwfe;

	if (!(*asi)) 
	{
		*asi=new AUDIO_STREAM_INFO;
		ZeroMemory(*asi,sizeof(AUDIO_STREAM_INFO));
		(*asi)->dwFlags=ASIF_ALLOCATED;
	}
	if (!(*asi)->lpASH)
	{
		(*asi)->lpASH=new AVIStreamHeader;
		ZeroMemory((*asi)->lpASH,sizeof(AVIStreamHeader));
	}
//	(*asi)->iDelay=0;
	(*asi)->lpFormat=malloc(sizeof(WAVEFORMATEX)+5);
	lpwfe=(WAVEFORMATEX*)((*asi)->lpFormat);
	ZeroMemory((*asi)->lpFormat,sizeof(WAVEFORMATEX)+5);


	(*asi)->lpASH->fccType=MakeFourCC("auds");
	(*asi)->lpASH->dwRate=aacsource->GetFrequency();
	(*asi)->lpASH->dwSuggestedBufferSize = 8192;
	(*asi)->lpASH->dwQuality = 0xFFFFFFFF;
	(*asi)->lpASH->dwScale=1024;

	(*asi)->dwType=AUDIOTYPE_AAC;
	
	lpwfe->wFormatTag=AAC_WFORMATTAG;
	lpwfe->nChannels=aacsource->GetChannelCount();
	lpwfe->nBlockAlign=4096;
	lpwfe->cbSize=0;
	lpwfe->nSamplesPerSec=aacsource->GetFrequency();

	BYTE* b = ((BYTE*)lpwfe)+sizeof(WAVEFORMATEX);
	int SRI = (int)aacsource->FormatSpecific(MMSGFS_AAC_SAMPLERATEINDEX);
	int profile = (int)aacsource->FormatSpecific(MMSGFS_AAC_PROFILE);
	int sbr = (int)aacsource->FormatSpecific(MMSGFS_AAC_ISSBR);
	lpwfe->cbSize += 2;
	*b++ = (BYTE)((profile +1) << 3 | (SRI >> 1));
	*b++ = (BYTE)(((SRI & 0x1) << 7) | (aacsource->GetChannelCount() << 3));
	if (sbr) {
		int syncExtensionType = 0x2B7;
		int iDSRI = (int)aacsource->FormatSpecific(MMSGFS_AAC_DOUBLESAMPLERATEINDEX);
		lpwfe->cbSize += 3;
		*b++ = (syncExtensionType >> 3) & 0xFF;
		*b++ = ((syncExtensionType & 0x7) << 5) | 5;
		*b++ = ((1 & 0x1) << 7) | (iDSRI << 3);
	}
	
	(*asi)->audiosource=aacsource;
	
}
void FillDTS_ASI (AUDIO_STREAM_INFO** asi,DTSSOURCE* dtssource)
{
	WAVEFORMATEX*	lpwfe;

	if (!(*asi)) 
	{
		*asi=new AUDIO_STREAM_INFO;
		ZeroMemory(*asi,sizeof(AUDIO_STREAM_INFO));
		(*asi)->dwFlags=ASIF_ALLOCATED;
	}
	(*asi)->dwType=AUDIOTYPE_DTS;
//	(*asi)->iDelay=0;
	if (!(*asi)->lpASH)
	{
		(*asi)->lpASH=new AVIStreamHeader;
	}
	ZeroMemory((*asi)->lpASH,sizeof(AVIStreamHeader));
	(*asi)->lpASH->dwRate=dtssource->GetAvgBytesPerSec();
	(*asi)->lpASH->dwScale=1;
	(*asi)->lpASH->dwSampleSize=1;
	(*asi)->lpASH->dwQuality=0xFFFFFFFF;
	(*asi)->lpASH->dwSuggestedBufferSize=2*dtssource->GetFrameSize();
	(*asi)->lpASH->fccType=MakeFourCC("auds");
	if (!(*asi)->lpFormat)
	{
		(*asi)->lpFormat=new WAVEFORMATEX;
	}
	ZeroMemory((*asi)->lpFormat,sizeof(WAVEFORMATEX));
	lpwfe=(WAVEFORMATEX*)((*asi)->lpFormat);
	lpwfe->wFormatTag=0x2001;
	lpwfe->nChannels=dtssource->GetChannelCount();
	lpwfe->nBlockAlign=1;
	lpwfe->cbSize=0;
	lpwfe->nSamplesPerSec=dtssource->GetFrequency();
	(*asi)->audiosource=dtssource;
}


typedef struct {
  WAVEFORMATEX  Format;
  union {
    WORD  wValidBitsPerSample;
    WORD  wSamplesPerBlock;
    WORD  wReserved;
  } Samples;
  DWORD   dwChannelMask; 
  GUID    SubFormat;
} WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE;

typedef struct tagVORBISFORMAT2
{
	DWORD Channels;
	DWORD SamplesPerSec;
	DWORD BitsPerSample;	
	DWORD HeaderSize[3]; // 0: Identification, 1: Comment, 2: Setup
} VORBISFORMAT2, *PVORBISFORMAT2, FAR *LPVORBISFORMAT2;


void FillVorbis_ASI (AUDIO_STREAM_INFO** asi,VORBISFROMOGG* vorbis)
{
	WAVEFORMATEXTENSIBLE*	lpwfe;

	if (!(*asi)) 
	{
		*asi=new AUDIO_STREAM_INFO;
		ZeroMemory(*asi,sizeof(AUDIO_STREAM_INFO));
		(*asi)->dwFlags=ASIF_ALLOCATED;
	}

	(*asi)->lpFormat=malloc(sizeof(WAVEFORMATEXTENSIBLE)+5);
	lpwfe=(WAVEFORMATEXTENSIBLE*)((*asi)->lpFormat);
	ZeroMemory((*asi)->lpFormat,sizeof(WAVEFORMATEXTENSIBLE)+5);


}

void FillMP3_ASI (AUDIO_STREAM_INFO** asi,MP3SOURCE* mp3source)
{
	MPEGLAYER3WAVEFORMAT*	lpmp3;
	MPEG1WAVEFORMAT*		lpmpg;
	WAVEFORMATEX*			lpwfe;

	if (!(*asi)) 
	{
		*asi=new AUDIO_STREAM_INFO;
		ZeroMemory(*asi,sizeof(AUDIO_STREAM_INFO));
		(*asi)->dwFlags=ASIF_ALLOCATED;
	}
	if (!(*asi)->lpASH)
	{
		(*asi)->lpASH=new AVIStreamHeader;
		ZeroMemory((*asi)->lpASH, sizeof(AVIStreamHeader));
		(*asi)->lpASH->dwScale = (mp3source->GetLayerVersion()==1)?384:(((mp3source->GetMPEGVersion()==1)?1152:576));
	}
//	(*asi)->iDelay=0;

	if (mp3source->GetLayerVersion() == 3)  {
		(*asi)->lpFormat=malloc(sizeof(MPEGLAYER3WAVEFORMAT));
		lpmp3=(MPEGLAYER3WAVEFORMAT*)((*asi)->lpFormat);
		ZeroMemory((*asi)->lpFormat,sizeof(MPEGLAYER3WAVEFORMAT));
		lpwfe=&(lpmp3->wfx);
	} else {
		(*asi)->lpFormat=malloc(sizeof(MPEG1WAVEFORMAT));
		lpmpg=(MPEG1WAVEFORMAT*)((*asi)->lpFormat);
		ZeroMemory((*asi)->lpFormat,sizeof(MPEG1WAVEFORMAT));
		lpwfe=&(lpmpg->wfx);
	}



	(*asi)->lpASH->fccType=MakeFourCC("auds");
	(*asi)->lpASH->dwRate=mp3source->GetFrequency();
	(*asi)->dwType=(mp3source->IsCBR())?AUDIOTYPE_MP3CBR:AUDIOTYPE_MP3VBR;

	if (mp3source->GetLayerVersion() == 3)  {
		lpwfe->wFormatTag=0x55;
		lpwfe->cbSize=12;
		lpmp3->wID=1;
		lpmp3->nBlockSize=mp3source->GetFrameSize();
		lpwfe->nBlockAlign=(mp3source->IsCBR())?1:
			(
				(
					(mp3source->GetLayerVersion()==1)?384:(((mp3source->GetMPEGVersion()==1)?1152:576))
				)
			);
		(*asi)->lpASH->dwScale = lpwfe->nBlockAlign;
	} else {
		// layer
		switch (mp3source->GetLayerVersion()) {
			case 1: lpmpg->fwHeadLayer = ACM_MPEG_LAYER1; break;
			case 2: lpmpg->fwHeadLayer = ACM_MPEG_LAYER2; break;
		}

		switch (mp3source->GetMode()) {
			case 0: lpmpg->fwHeadMode |= ACM_MPEG_STEREO; break;
			case 1: lpmpg->fwHeadMode |= ACM_MPEG_JOINTSTEREO; break;
			case 2: lpmpg->fwHeadMode |= ACM_MPEG_DUALCHANNEL; break;
			case 3: lpmpg->fwHeadMode |= ACM_MPEG_SINGLECHANNEL; break;
		}

		lpmpg->fwHeadModeExt= 0x0F;
		
		if (mp3source->HasCRC())
			lpmpg->fwHeadFlags  |= ACM_MPEG_PROTECTIONBIT;
		if (mp3source->GetMPEGVersion() == 1)
			lpmpg->fwHeadFlags	|= ACM_MPEG_ID_MPEG1;
		if (mp3source->IsOriginal()) 
			lpmpg->fwHeadFlags	|= ACM_MPEG_ORIGINALHOME;
		if (mp3source->IsCopyrighted())
			lpmpg->fwHeadFlags	|= ACM_MPEG_COPYRIGHT;


		lpmpg->dwHeadBitrate	= 0;
		lpmpg->wHeadEmphasis	= 1 + mp3source->GetEmphasis();
		lpwfe->cbSize = sizeof(MPEG1WAVEFORMAT)-sizeof(WAVEFORMATEX);
		lpwfe->wFormatTag = 0x0050;
		lpwfe->nBlockAlign= 1;
	}

	lpwfe->nSamplesPerSec=mp3source->GetFrequency();
	lpwfe->nChannels=mp3source->GetChannelCount();

	(*asi)->audiosource=mp3source;
}
