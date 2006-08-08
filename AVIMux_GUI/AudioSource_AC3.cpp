#include "stdafx.h"
#include "silence.h"
#include "audiosource_ac3.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

	//////////////////////
	// AC3 audio source //
	//////////////////////

#pragma pack(push,1)

#define silence ((SILENCE*)_silence)

typedef struct
{
	WORD	wSync;
	WORD	wCRC;
	BYTE	bFrszcode;
	BYTE	bsi;
	BYTE	AC3mod;

} AC3FRMHDR, *LPAC3FRMHDR;

AC3SOURCE::AC3SOURCE()
{
	_silence = NULL;
	ZeroMemory(&ac3info,sizeof(ac3info));
	lpbFirstFrame = NULL;
//	lpRCB = NULL;
}

AC3SOURCE::AC3SOURCE(STREAM* lpStream)
{
	_silence = NULL;
//	lpRCB = NULL;
	lpbFirstFrame = NULL;
	ZeroMemory(&ac3info,sizeof(ac3info));
	Open(lpStream);
}

int AC3SOURCE::GetFormatTag()
{
	return 0x2000;
}

bool AC3SOURCE::IsCBR()
{
	return true;
}

int AC3SOURCE::GetStrippableHeaderBytes(void* pBuffer, int max)
{
	if (max > 0)
		((unsigned char*)pBuffer)[0] = 0x0B;
	if (max > 1)
		((unsigned char*)pBuffer)[1] = 0x77;

	return 2;
}

int AC3SOURCE::ReadFrame(void* lpDest, DWORD* lpdwMicroSecRead,
						 __int64* lpqwNanoSecRead, bool bStoreAC3Info,bool bResync)
{
	DWORD		dwFrequencies[4] = { 48000, 44100, 32000, 0 };
	DWORD		dwBitrates[38]=
		{ 32, 32, 40, 40, 48, 48, 56, 56, 64, 64,
		  80, 80, 96, 96,112,112,128,128,160,160,
		 192,192,224,224,256,256,320,320,384,384,
		 448,448,512,512,576,576,640,640 };
	DWORD		dwFrameSizes[3][38]=
		{
			{  64,  64,  80,  80,  96,  96, 112, 112, 128, 128,
			  160, 160, 192, 192, 224, 224, 256, 256, 320, 320,
			  384, 384, 448, 448, 512, 512, 640, 640, 768, 768,
			  896, 896,1024,1024,1152,1152,1280,1280 },
			{  69,  70,  87,  88, 104, 105, 121, 122, 139, 140,
			  174, 175, 208, 209, 243, 244, 278, 279, 348, 349,
			  417, 418, 487, 488, 557, 558, 696, 697, 835, 836,
			  975, 976,1114,1115,1253,1254,1393,1394 },
			{  96,  96, 120, 120, 144, 144, 168, 168, 192, 192,
			  240, 240, 288, 288, 336, 336, 384, 384, 480, 480,
			  576, 576, 672, 672, 768, 768, 960, 960,1152,1152,
			 1344,1344,1536,1536,1728,1728,1920,1920 }
		};
	DWORD		dwChannelIDs[8] = { 2, 1, 2, 3, 3, 4, 4, 5 };
	if (!lpDest) return 0;
	AC3FRMHDR*  si=(AC3FRMHDR*)lpDest;
	__int64	qwOldPos;
	DWORD		dwFreqIndex;
	DWORD		dwBitrateIndex;
	DWORD		dwBitrate;
	DWORD		dwFrameSize;
	DWORD		dwChannelIndex;
	DWORD		dwMaxGap = GetResyncRange();
	DWORD		dwChannels;
	DWORD		dwRead;
	int			iOffset;
	BYTE		temp[5000];
	__int64		qwTemp;
	DWORD		dwCrapData;
//	DWORD		dwCBRes;

	if (lpqwNanoSecRead) *lpqwNanoSecRead=0;
	if (lpdwMicroSecRead) *lpdwMicroSecRead=0;
	if (IsEndOfStream()) return 0;
	qwOldPos=GetSource()->GetPos();
	qwTemp=qwOldPos;
	iOffset=GetSource()->GetOffset();

	if (GetAccumulatedDelay() < GetFrameDuration() / 2 || GetFrameDuration() == 0) {
		if (GetSource()->Read(si,sizeof(AC3FRMHDR))!=sizeof(AC3FRMHDR)) {
			GetSource()->Seek(qwOldPos);
			if (GetSource()->Read(si,sizeof(AC3FRMHDR))!=sizeof(AC3FRMHDR)) {
				return 0; 
			}
		}
		if (si->wSync!=0x770B) {
			dwCrapData=0;
			if (!bResync) {
				GetSource()->Seek(qwOldPos); return 0;
			}	
			dwRead=0;
			while ((si->wSync!=0x770B)&&(!dwRead)&&(!IsEndOfStream())&&(dwCrapData<dwMaxGap)) {
				qwOldPos++;
				dwCrapData++;
				GetSource()->Seek(qwOldPos);
				dwRead=0;
				if (ReadFrame(temp,NULL,NULL,false,false)) {
					dwRead=ReadFrame(temp,NULL,NULL,false,false);
				}
			}
			if (dwCrapData==dwMaxGap) {
				return 0;
			}
		/*	if (lpRCB) {
				dwCBRes=(*lpRCB)(qwTemp,(DWORD)(qwOldPos-qwTemp),dwRCBUserData);
				if ((dwCBRes&0xFFFF)==AC3RCB_INSERTSILENCE)
				{
					dwReturnSilence+=dwCBRes>>16;
				}
			} */

			ConvertCrapDataToSilence(dwCrapData, ac3info.dwFrameSize);
		}

		GetSource()->Seek(qwOldPos);
//		if (!dwReturnSilence) {
//		}
		
	}

	if (GetAccumulatedDelay() >= GetFrameDuration()/2 && GetFrameDuration()>0)
	{
		if (!bUseExternalSilence) {
			memcpy(lpDest,lpbFirstFrame,ac3info.dwFrameSize);
			memcpy(si,lpbFirstFrame,7);
			dwRead=ac3info.dwFrameSize;
			AddToAccumulatedDelay(-GetFrameDuration());
			if (lpdwMicroSecRead)
				*lpdwMicroSecRead = ac3info.iFrameDuration / 1000000;
			if (lpqwNanoSecRead)
				*lpqwNanoSecRead = ac3info.iFrameDuration;
			return dwRead;
		} else {
			dwRead=silence->Read(lpDest, 0, lpdwMicroSecRead, lpqwNanoSecRead);
			AddToAccumulatedDelay(-*lpqwNanoSecRead);
			return dwRead;
		}
	}

	if (GetSource()->Read(si,sizeof(AC3FRMHDR))!=sizeof(AC3FRMHDR)) {
		GetSource()->Seek(qwOldPos);
		return 0; 
	}

	dwFreqIndex=si->bFrszcode >> 6;
	dwBitrateIndex=si->bFrszcode & 0x3F;
	dwChannelIndex=si->AC3mod >> 5; 
	if (dwFreqIndex==3) { GetSource()->Seek(qwOldPos); return 0; }
	if (dwBitrateIndex>=38) { GetSource()->Seek(qwOldPos); return 0; }

	dwBitrate=dwBitrates[dwBitrateIndex];
	dwFrameSize=2*dwFrameSizes[dwFreqIndex][dwBitrateIndex];
	dwChannels=dwChannelIDs[dwChannelIndex];

	if (bStoreAC3Info) {
		ac3info.dwChannels=dwChannels;
		ac3info.dwBitrate=dwBitrate;
		ac3info.dwFrameSize=dwFrameSize;
		ac3info.dwFrequency=dwFrequencies[dwFreqIndex];
	}

//	if (!dwReturnSilence) {
		dwRead=GetSource()->Read(&(((BYTE*)lpDest)[7]),dwFrameSize-7)+7;
//	}

	if (dwRead!=dwFrameSize) {

		ConvertCrapDataToSilence(dwRead, dwFrameSize);
/*		double fraction = (double)dwRead / (double)dwFrameSize;
		__int64 delay = fraction * GetFrameDuration();
		AddToAccumulatedDelay(delay);
*/
		dwRead=0;
	}

	double z=(dwRead/dwBitrate);

	if (lpqwNanoSecRead) 
		*lpqwNanoSecRead=round(8000000*z);
	
	if (lpdwMicroSecRead) 
		*lpdwMicroSecRead=(DWORD)round(8000*z);
	
//	if (dwReturnSilence) dwReturnSilence--;

	return dwRead;
}

int AC3SOURCE::ConvertCrapDataToSilence(int size, int frameSize)
{
	double fraction = (double)size / (double)frameSize;
	__int64 delay = fraction * GetFrameDuration();
	AddToAccumulatedDelay(delay);

	return 1;
}

int AC3SOURCE::doRead(void* lpDest,DWORD dwMicroSecDesired,
					  DWORD* lpdwMicroSecRead,__int64* lpqwNanoSecRead)
{
	if (IsEndOfStream())
		return 0;

	BYTE*		lpbDest=(BYTE*)lpDest;
	DWORD		dwMSR=0,dwMSR_Total=0;
	__int64	qwNSR=0,qwNSR_Total=0;
	DWORD		dwRead,i;
	DWORD		dwReadTotal=0;
	DWORD		dwFramesToRead;
	DWORD		dwFramesDemanded = ((dwMicroSecDesired/1000) + 16) / 32;
	
	if (GetFrameMode() == FRAMEMODE_SINGLEFRAMES || GetFrameMode() == FRAMEMODE_OFF) {
		dwFramesToRead = 1;
	} else {
		if (GetFrameMode() == FRAMEMODE_ON) {
			dwFramesToRead = 2 + (dwFramesDemanded % 2);
		} else {
			dwFramesToRead = max(2,min(GetFrameMode(), (int)dwFramesDemanded));
		}
	}

	for (i=0;i<dwFramesToRead;i++) {
		dwRead=ReadFrame(&lpbDest[dwReadTotal],&dwMSR,&qwNSR,false,true);
		dwReadTotal+=dwRead;
		dwMSR_Total+=dwMSR;
		qwNSR_Total+=qwNSR;
	}
	
	if (lpdwMicroSecRead) 
		*lpdwMicroSecRead = dwMSR_Total;
	
	if (lpqwNanoSecRead) 
		*lpqwNanoSecRead = qwNSR_Total;

	return dwReadTotal;
}

int AC3SOURCE::Open(STREAM* lpStream)
{
	char	Buffer[10240];
	int		iOffset=0;

	if (!lpStream || lpStream->GetSize() <= 0)
		return AS_ERR;

//	dwRCBUserData=0;
//	lpRCB=NULL;
//	dwReturnSilence=0;
	lpbFirstFrame=NULL;
	ZeroMemory(Buffer,sizeof(Buffer));
	int	iRes=AUDIOSOURCEFROMBINARY::Open(lpStream);
	GetSource()->SetOffset(0);
	GetSource()->Seek(0);
	if (iRes==AS_OK)
	{
//		SetResyncRange(32768);
		if (ReadFrame(Buffer,NULL,NULL,true,true)) {
			GetSource()->SetOffset((DWORD)(GetSource()->GetPos()-(__int64)ac3info.dwFrameSize));
		} else {
			return AS_ERR;
		}
	}
	GetSource()->Seek(0);
	
	lpbFirstFrame=new byte[ac3info.dwFrameSize];
	
	
	int dwOldRange = GetResyncRange();
	SetResyncRange(0);
	ReadFrame(lpbFirstFrame,NULL,&ac3info.iFrameDuration,false,false);
	GetSource()->Seek(0);

	_silence=new SILENCE;

	char dir[512];
	GetModuleFileName(NULL, dir, 512);	
	silence->Init(dir);
	bUseExternalSilence=(silence->SetFormat(AUDIOTYPE_AC3,ac3info.dwChannels,ac3info.dwFrequency,
		(float)ac3info.dwBitrate)==SSF_SUCCEEDED);
	SetResyncRange(dwOldRange);
	return iRes;
}

__int64 AC3SOURCE::GetFrameDuration()
{
	return ac3info.iFrameDuration;
}

int AC3SOURCE::GetChannelCount()
{
	return ac3info.dwChannels;
}

int AC3SOURCE::GetBitrate()
{
	return ac3info.dwBitrate;
}

int AC3SOURCE::GetGranularity()
{
	return ac3info.dwFrameSize;
}

int AC3SOURCE::GetAvgBytesPerSec()
{
	return ac3info.dwBitrate*125;
}

int AC3SOURCE::GetFrequency()
{
	return ac3info.dwFrequency;
}

int AC3SOURCE::doClose()
{
//	return CBRAUDIOSOURCE::doClose();

	return 1;
}

AC3SOURCE::~AC3SOURCE()
{
	if (silence) {
		silence->Close();
		delete silence;
		_silence = NULL;
	}
	if (lpbFirstFrame) free (lpbFirstFrame);
}

#pragma pack(pop)