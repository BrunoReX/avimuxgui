#include "stdafx.h"
#include "audiosource_dts.h"

#ifdef DEBUG_NEW
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

	//////////////////////
	// DTS audio source //
	//////////////////////

#pragma pack(push,1)

int DTSSOURCE::Resync()
{
	WORD DTS_sync_1[2]={ 0xFE7F,0x0180 };
	WORD DTS_sync_2[3]={ 0x1fff,0xe800, 0x007f };

	__int64	qwOldPos;
	WORD		wSyncWord;
	DWORD		dwOffset=0;

	DWORD		dwRes=0;


	qwOldPos=GetSource()->GetPos();
	while (((int)dwOffset<=GetResyncRange())&&(!dwRes))
	{
		if (GetSource())
		{
			GetSource()->Read(&wSyncWord,2);
			if (wSyncWord==DTS_sync_1[0])
			{
				GetSource()->Read(&wSyncWord,2);
				if (wSyncWord==DTS_sync_1[1])
				{
					dwRes=1;
				}
			}
			else
			if (wSyncWord==DTS_sync_2[0])
			{
				GetSource()->Read(&wSyncWord,2);
				if (wSyncWord==DTS_sync_2[1])
				{
					GetSource()->Read(&wSyncWord,2);
					if (wSyncWord==DTS_sync_2[2])
					{
						dwRes=1;
					}
				}
			}
			else
			{
				dwOffset++;
				GetSource()->Seek(qwOldPos+dwOffset);
			}
		}
	}
	if (!qwOldPos)
	{
		GetSource()->SetOffset(GetSource()->GetOffset()+dwOffset);
	}
	GetSource()->Seek(qwOldPos);
	if ((int)dwOffset>=GetResyncRange())
	{
		return 0;
	}

	return 1;
}

bool DTSSOURCE::IsCBR()
{
	return true;
}

__int64 DTSSOURCE::GetFrameDuration()
{
	return dtsinfo.nano_seconds_per_frame;
}

int DTSSOURCE::ParseFrameHeader(DTSINFO* lpdtsinfo)
{
	DWORD	channel_table[16] = { 1,2,2,2,2,3,3,4,4,5,6,6,6,7,8,8 };
	DWORD	sample_rate_table[16] =	{
		0, 8000, 16000, 32000, 0, 0, 11025, 22050,
		44100, 0, 0, 12000, 24000, 48000, 0, 0 };
	float	bitrate_table[32] = { 
		  32,  56,  64,  96, 112, 128, 192, 224,
		 256, 320, 384, 448, 512, 576, 640, 754.50f,
		 960,1024,1152,1280,1344,1408,1411.2f,1472,
		1509.75f,1920,2048,3072,3840,   0,   0,   0 };

	__int64	qwOldPos;
	int bitpos;
	DWORD		dwFrameSize;
	DWORD		dwChannels;
	DWORD		dwFrequency;
	float		fBitrate;

	qwOldPos=GetSource()->GetPos();
	bitpos = bitsource->GetBitPos();	

	uint32 uiSync = bitsource->ReadBits(32);// Sync
	if (uiSync != 0x7ffe8001)
		return 0;

	bitsource->ReadBits(1);					// Frametype
	bitsource->ReadBits(5);					// deficit sample count
	bitsource->ReadBits(1);					// CRC present?
	DWORD dwPCMSampleCount = bitsource->ReadBits(7);					// number of PCM samples per block
	dwFrameSize=(bitsource->ReadBits(14)+1);	// primary frame byte size
	dwChannels=channel_table[bitsource->ReadBits(6)]; // audio channel arrangement
	dwFrequency=sample_rate_table[bitsource->ReadBits(4)]; // core audio sample frequency
	fBitrate=bitrate_table[bitsource->ReadBits(5)]; // transmission bitrate
	DWORD dwIgnore = bitsource->ReadBits(10);
	DWORD dwLFEFlag = bitsource->ReadBits(2);

//	double seconds = (double)dwFrameSize / fBitrate / 125; 
    if(lpdtsinfo)
	{
		lpdtsinfo->fBitrate=fBitrate;
		lpdtsinfo->dwFrameSize=dwFrameSize;
		lpdtsinfo->dwFrequency=dwFrequency;
		lpdtsinfo->nano_seconds_per_frame = (32. * (dwPCMSampleCount + 1.)) / dwFrequency * 1000000000;
		if (dwLFEFlag == 1 || dwLFEFlag == 2)
		{
			lpdtsinfo->dwLFE = 1;			
		}
		else
		{
			lpdtsinfo->dwLFE = 0;			
		}
		lpdtsinfo->dwChannels=dwChannels + lpdtsinfo->dwLFE;
	}

	GetSource()->Seek(qwOldPos);
	bitsource->SetBitPos(bitpos);
	return 1;
}

char* DTSSOURCE::GetChannelString()
{
	char cTemp[8]; cTemp[0]=0;
	
	_snprintf(cTemp, 8, "%d.%d", GetChannelCount()-dtsinfo.dwLFE, dtsinfo.dwLFE);
	return _strdup(cTemp);
}

int DTSSOURCE::Open(STREAM *lpStream)
{
	if (!lpStream || lpStream->GetSize() <= 0) 
		return AS_ERR;

	if (CBinaryAudioSource::Open(lpStream)==AS_ERR)
		return AS_ERR;

	bitsource=new BITSTREAM;
	bitsource->Open(lpStream);

	lpStream->SetOffset(0);	
	lpStream->Seek(0);

	if (!Resync()) 
		return AS_ERR;

	ParseFrameHeader(&dtsinfo);

	SetCurrentTimecode(0, TIMECODE_UNSCALED);

	return 1;
}

int DTSSOURCE::GetFormatTag()
{
	return 0x2001;
}

int DTSSOURCE::doRead(void* lpDest, 
					  DWORD dwMicroSecDesired,
					  DWORD* lpdwMicroSecRead,
					  __int64* lpqwNanoSecRead)
{
	BYTE*		lpbDest=(BYTE*)lpDest;
	int 		dwMSR1 = 0,dwMSR2 = 0;
	__int64		qwNSR1 = 0,qwNSR2 = 0;
	DWORD		dwReadFirst = 0;
	DWORD		dwReadSecond = 0;
	int			imsd = dwMicroSecDesired;

	if (dwReadFirst=ReadFrame(lpDest,(DWORD*)&dwMSR1,&qwNSR1,false))
	imsd-=dwMSR1;
	{
		if (GetFrameMode()!=FRAMEMODE_SINGLEFRAMES) {
			for (int j=1;j<GetFrameMode() && (imsd>0 || j==1);j++) {
				dwReadSecond=ReadFrame(&(lpbDest[dwReadFirst]),(DWORD*)&dwMSR2,&qwNSR2,false);
				dwReadFirst+=dwReadSecond;
				dwMSR1+=dwMSR2;
				qwNSR1+=qwNSR2;
				imsd-=dwMSR2;
			}
		}

		if (lpdwMicroSecRead) (*lpdwMicroSecRead)=dwMSR1;
		if (lpqwNanoSecRead) (*lpqwNanoSecRead)=qwNSR1;

		return dwReadFirst;
	}

	return 0;
}

int DTSSOURCE::ReadFrame(void* lpDest,DWORD* lpdwMicroSecRead,__int64 *lpqwNanoSecRead,bool bResync)
{
	DWORD	dwRead;

	dwRead=GetSource()->Read(lpDest,dtsinfo.dwFrameSize);
	double z=((double)dwRead/dtsinfo.fBitrate);
	if (lpqwNanoSecRead) *lpqwNanoSecRead=round(8000000*z);
	if (lpdwMicroSecRead) *lpdwMicroSecRead=(DWORD)round(8000*z);

	return dwRead;
}

int DTSSOURCE::ReadFrame(MULTIMEDIA_DATA_PACKET** dataPacket)
{
	DTSINFO dtsinfo;

	/* check if frame header can be read correctly */
	if (!ParseFrameHeader(&dtsinfo))
	{
		return -1;
	}

	char* data = (char*)malloc(dtsinfo.dwFrameSize);
	DWORD dwRead = GetSource()->Read(data, dtsinfo.dwFrameSize);

	if (dwRead != dtsinfo.dwFrameSize)
		return -1;

	createMultimediaDataPacket(dataPacket);
	(*dataPacket)->cData = data;
	(*dataPacket)->totalDataSize = dtsinfo.dwFrameSize;
	(*dataPacket)->duration = dtsinfo.nano_seconds_per_frame;
	(*dataPacket)->frameSizes.push_back(dtsinfo.dwFrameSize);
	(*dataPacket)->timecode = GetCurrentTimecode() * GetTimecodeScale();
	
	if (IsEndOfStream())
		(*dataPacket)->nextTimecode = TIMECODE_UNKNOWN;
	else
		(*dataPacket)->nextTimecode = (*dataPacket)->timecode + dtsinfo.nano_seconds_per_frame;

	(*dataPacket)->flags = 0;
	(*dataPacket)->compressionInfo.clear();

	IncCurrentTimecode(dtsinfo.nano_seconds_per_frame);

	return (*dataPacket)->totalDataSize;
}

int DTSSOURCE::GetFrameSize()
{
	return dtsinfo.dwFrameSize;
}

float DTSSOURCE::GetBitrate()
{
	return dtsinfo.fBitrate;
}

int DTSSOURCE::GetGranularity()
{
	return dtsinfo.dwFrameSize;
}

int DTSSOURCE::GetChannelCount()
{
	return dtsinfo.dwChannels;
}

int DTSSOURCE::GetFrequency()
{
	return dtsinfo.dwFrequency;
}

int DTSSOURCE::GetAvgBytesPerSec()
{
	return (int)(dtsinfo.fBitrate*125);
}

DTSSOURCE::DTSSOURCE()
{
	ZeroMemory(&dtsinfo,sizeof(dtsinfo));
	bitsource=NULL;
}

DTSSOURCE::DTSSOURCE(STREAM* lpStream)
{
	ZeroMemory(&dtsinfo,sizeof(dtsinfo));
	bitsource=NULL;
	Open(lpStream);
}

DTSSOURCE::~DTSSOURCE()
{
}

int DTSSOURCE::doClose()
{
	if (bitsource)
	{
		bitsource->Close();
		delete bitsource;
	}
//	CBRAUDIOSOURCE::doClose();
	return 1;
}

int DTSSOURCE::GetStrippableHeaderBytes(void* pBuffer, int max)
{
	unsigned char b[] = {  0x7F, 0xFE, 0x80, 0x01 };
	memcpy(pBuffer, (void*)b, min(max, 4));
	return 4;
}

#pragma pack(pop)
