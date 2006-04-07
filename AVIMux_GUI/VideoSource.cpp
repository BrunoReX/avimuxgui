#include "stdafx.h"
#include "videosource.h"
#include "avifile.h"



// Videofilter

/*
VIDEOFILTER::VIDEOFILTER(void)
{
	lpSource=NULL;
}

VIDEOFILTER::~VIDEOFILTER(void)
{
}

DWORD VIDEOFILTER::Open(VIDEOSOURCE *_lpSource)
{
	if (!_lpSource) return VS_ERROR;
	lpSource=_lpSource;
	return VS_OK;
}

DWORD VIDEOFILTER::Close(bool bCloseSource)
{
	if (bCloseSource) if (GetSource()) GetSource()->Close(false);
	lpSource=NULL;	
	return VS_OK;
}

VIDEOSOURCE* VIDEOFILTER::GetSource(void)
{
	return lpSource;
}

int VIDEOFILTER::GetResolution(int* lpiWidth,int* lpiHeight)
{
	if (!GetSource()) return VS_INVALIDCALL;
	return GetSource()->GetResolution(lpiWidth,lpiHeight);
}

// Frameratechanger

FRAMERATECHANGER::FRAMERATECHANGER(void)
{
	lpdwFrameTable=NULL;
	qwForcedNSPF=0;
	dwNewNbrOfFrames=0;
}

FRAMERATECHANGER::~FRAMERATECHANGER(void)
{
}

DWORD FRAMERATECHANGER::Open(VIDEOSOURCE* lpSource)
{
	if (VIDEOFILTER::Open(lpSource)==VS_OK)
	{
		if (lpdwFrameTable) free (lpdwFrameTable);
		lpdwFrameTable=NULL;
		if (!lpdwFrameTable) return VS_ERROR;
		return VS_OK;
	}
	else return VS_ERROR;
}

__int64 FRAMERATECHANGER::GetNanoSecPerFrame(void)
{
	return qwForcedNSPF;
}

DWORD FRAMERATECHANGER::SetNanoSecPerFrame(__int64 qwNSPF)
{
	__int64	qwSNS,qwDNS,qwSNSPF;
	int			iSourceFrameCount;
	DWORD		i;

	if (!GetSource()) return VS_INVALIDCALL;
	qwForcedNSPF=qwNSPF;

	qwSNS=0;
	qwDNS=0;
	qwSNSPF=GetSource()->GetNanoSecPerFrame();
	iSourceFrameCount=0;

	dwNewNbrOfFrames=(DWORD)round((double)(qwSNSPF/qwNSPF)*GetSource()->GetNbrOfFrames());
	if (lpdwFrameTable) free (lpdwFrameTable);
	lpdwFrameTable=new DWORD[GetSource()->GetNbrOfFrames()];

	
	for (i=0;i<GetNbrOfFrames();i++)
	{
		if (abs((int)(qwSNS-qwDNS))<qwNSPF)
		{
			lpdwFrameTable[i]=iSourceFrameCount++;
			qwSNS+=qwSNSPF;
			qwDNS+=qwNSPF;
		}
		else if ((int)(qwSNS-qwDNS)>qwNSPF)
		{
			if (IsSourceKeyFrame(i))
			{
				lpdwFrameTable[i]=iSourceFrameCount;
				qwDNS+=qwNSPF;
			}
			else
			{
				lpdwFrameTable[i]=iSourceFrameCount++;
				qwSNS+=qwSNSPF;
				qwDNS+=qwNSPF;
			}
		}
		else if ((int)(qwDNS-qwSNS)>qwNSPF)
		{
			if (IsSourceKeyFrame(i+1))
			{
				lpdwFrameTable[i]=iSourceFrameCount+1;
				iSourceFrameCount+=2;
				qwDNS+=qwNSPF;
				qwSNS+=2*qwSNSPF;
			}
			else
			{
				lpdwFrameTable[i]=iSourceFrameCount++;
				qwSNS+=qwSNSPF;
				qwDNS+=qwNSPF;
			}
		}
	}

	return VS_OK;
}

bool FRAMERATECHANGER::IsSourceKeyFrame(DWORD dwNbr)
{
	if (!GetSource()) return false;

	return GetSource()->IsKeyFrame(dwNbr);
}

DWORD FRAMERATECHANGER::GetNbrOfFrames(void)
{
	return dwNewNbrOfFrames;
}

bool FRAMERATECHANGER::IsKeyFrame(DWORD dwNbr)
{
	if ((dwNbr)>=GetNbrOfFrames()) return false;
	return IsSourceKeyFrame(lpdwFrameTable[dwNbr]);
}

  */