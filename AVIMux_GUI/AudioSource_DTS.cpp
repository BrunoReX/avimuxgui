#include "stdafx.h"
#include "audiosource_dts.h"

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
	while ((dwOffset<=GetResyncRange())&&(!dwRes))
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
	if (dwOffset>=GetResyncRange())
	{
		return 0;
	}

	return 1;
}

bool DTSSOURCE::IsCBR()
{
	return true;
}

int DTSSOURCE::ProcessFrameHeader(DTSINFO*	lpdtsinfo)
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

	DWORD		dwFrameSize;
	DWORD		dwChannels;
	DWORD		dwFrequency;
	float		fBitrate;

	qwOldPos=GetSource()->GetPos();

	bitsource->ReadBits(32);				// Sync
	bitsource->ReadBits(1);					// Frametype
	bitsource->ReadBits(5);					// deficit sample count
	bitsource->ReadBits(1);					// CRC present?
	bitsource->ReadBits(7);					// number of PCM samples per block
	dwFrameSize=(bitsource->ReadBits(14)+1);	// primary frame byte size
	dwChannels=channel_table[bitsource->ReadBits(6)]; // audio channel arrangement
	dwFrequency=sample_rate_table[bitsource->ReadBits(4)]; // core audio sample frequency
	fBitrate=bitrate_table[bitsource->ReadBits(5)]; // transmission bitrate
    if(lpdtsinfo)
	{
		lpdtsinfo->fBitrate=fBitrate;
		lpdtsinfo->dwFrameSize=dwFrameSize;
		lpdtsinfo->dwFrequency=dwFrequency;
		lpdtsinfo->dwChannels=dwChannels;
	}

	GetSource()->Seek(qwOldPos);
	return 1;
}

int DTSSOURCE::Open(STREAM *lpStream)
{
	if (!lpStream) return AS_ERR;
	if (CBRAUDIOSOURCE::Open(lpStream)==AS_ERR) return AS_ERR;
	bitsource=new BITSTREAM;
	bitsource->Open(lpStream);

	lpStream->SetOffset(0);	
	lpStream->Seek(0);

	if (!Resync()) return 0;
	ProcessFrameHeader(&dtsinfo);

	return 1;
}

int DTSSOURCE::GetFormatTag()
{
	return 0x2001;
}

int DTSSOURCE::doRead(void* lpDest,DWORD dwMicroSecDesired,DWORD* lpdwMicroSecRead,__int64* lpqwNanoSecRead)
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
		free(bitsource);
	}
	CBRAUDIOSOURCE::doClose();
	return 1;
}

#pragma pack(pop)
