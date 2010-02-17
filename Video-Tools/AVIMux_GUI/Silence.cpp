#include "stdafx.h"
#include "silence.h"
#include "..\..\Common\Path.h"
#include "..\FileStream.h"
//#include "stdio.h"

#define SCS_INVALID		-1

SILENCE::SILENCE()
{
	iNbrOfDescs=0;
	iCompatibleSource=SCS_INVALID;
}

SILENCE::~SILENCE()
{
}

int SILENCE::Init(const char* lpcName)
{
	int i = 0;

/*	int	i;
	bool	bAlloc=false;
	char	buffer[200];
	char*	dir;
	
	if (lpcName)
	{
		dir=(char*)malloc(400);
		bAlloc=true;
		lstrcpy(dir,lpcName);
		for (i=lstrlen(dir);i>0;i--)
		{
			if (dir[i]=='\\')
			{
				dir[i]=0;
				i=-1;
			}	
		}
	}
	else
		dir=".";

	i=0;
*/
	iNbrOfDescs=5;
	lpSD=(SILENCE_DESCRIPTOR*)malloc(iNbrOfDescs*sizeof(SILENCE_DESCRIPTOR));
	//lstrcat(dir,"\\silence files");
	

	std::string dir = (lpcName ? lpcName : ".");

//	wsprintf(buffer,"%s\\FILL-6ch-384kbps.ac3",dir);
	std::string sample_ac3_6ch_384kbps_path = CPath::Combine(dir, "FILL-6ch-384kbps.ac3");
	if (SetDescriptor(sample_ac3_6ch_384kbps_path, 1536, AUDIOTYPE_AC3, 5, 48000, 384,&(lpSD[i]))==SSD_SUCCEEDED) i++;

//	wsprintf(buffer,"%s\\FILL-6ch-448kbps.ac3",dir);
	std::string sample_ac3_6ch_448kbps_path = CPath::Combine(dir, "FILL-6ch-448kbps.ac3");
	if (SetDescriptor(sample_ac3_6ch_448kbps_path, 1792, AUDIOTYPE_AC3, 6, 48000, 448, &(lpSD[i]))==SSD_SUCCEEDED) i++;

//  wsprintf(buffer,"%s\\FILL-2ch-192kbps.ac3",dir);
	std::string sample_ac3_2ch_192kbps_path = CPath::Combine(dir, "FILL-2ch-192kbps.ac3");
	if (SetDescriptor(sample_ac3_2ch_192kbps_path, 768, AUDIOTYPE_AC3, 2, 48000, 192, &(lpSD[i]))==SSD_SUCCEEDED) i++;

//	wsprintf(buffer,"%s\\FILL-6ch-768kbps.dts",dir);
	std::string sample_dts_6ch_768kbps_path = CPath::Combine(dir, "FILL-6ch-768kbps.dts");
	if (SetDescriptor(sample_dts_6ch_768kbps_path, 1006, AUDIOTYPE_DTS, 5, 48000, 754.50, &(lpSD[i]))==SSD_SUCCEEDED) i++;

//	wsprintf(buffer,"%s\\FILL-6ch-1509kbps.dts",dir);
	std::string sample_dts_6ch_1536kbps_path = CPath::Combine(dir, "FILL-6ch-1509kbps.dtss");
	if (SetDescriptor(sample_dts_6ch_1536kbps_path, 2013, AUDIOTYPE_DTS, 5, 48000, 1509.75, &(lpSD[i]))==SSD_SUCCEEDED) i++;
//	wsprintf(buffer,"%d: %s\\FILL-6ch-1509kbps.dts",i,dir);
//	MessageBox(0,buffer,NULL,MB_OK);
	iNbrOfDescs=i;
	//if (bAlloc) delete dir;
	return 0;
}

int	SILENCE::Close()
{
	int	i;
	for (i=0;i<iNbrOfDescs;i++)
	{
		delete lpSD[i].lpData;
	}
	free(lpSD);
	return 1;
}

int SILENCE::SetDescriptor(const std::string& lpcName, DWORD dwSize,
						   DWORD dwFormat, DWORD dwChannels, DWORD dwFreq,
						   float fBitrate, SILENCE_DESCRIPTOR* lpSD)
{
	std::string fileName = CPath::GetAbsolutePath(lpcName);
	CFileStream* fs = new CFileStream();
	if (STREAM_OK == fs->Open(fileName.c_str(), StreamMode::Read))
	{
		lpSD->lpData=malloc(dwSize);
		lpSD->dwSize=dwSize;
		
		if (dwSize != fs->Read(lpSD->lpData, dwSize))
			return SSD_FAILED;

		lpSD->dwFormat=dwFormat;
		lpSD->dwChannels=dwChannels;
		lpSD->fBitrate=fBitrate;
		lpSD->dwFreq=dwFreq;
		fs->Close();
		delete fs;
		return SSD_SUCCEEDED;
	}
	else
	{
		return SSD_FAILED;
	}
}

int SILENCE::SetFormat(DWORD dwFormat,DWORD dwChannels,DWORD dwFreq,float fBitrate)
{
	int		i=-1;
	iCompatibleSource=-1;
	
	for (i=0;i<iNbrOfDescs;i++)
	{
		if ((DWORD)lpSD[i].fBitrate==(DWORD)fBitrate)
		{
			if (lpSD[i].dwChannels==dwChannels)
			{
				if (lpSD[i].dwFormat==dwFormat)
				{
					if (lpSD[i].dwFreq==dwFreq)
					{
						iCompatibleSource=i;
					}
				}
			}
		}

	}
	return ((iCompatibleSource==-1)?SSF_FAILED:SSF_SUCCEEDED);
}

int SILENCE::Read(void* lpDest,DWORD dwMilliSec,DWORD* lpdwMicroSecRead,__int64* lpqwNanoSecRead)
{
	int		i=iCompatibleSource;
	int		j;
	char*	lpcDest=(char*)lpDest;
	DWORD	dwMSPB;
	DWORD	dwBlocksToReturn;

	if (iCompatibleSource!=SCS_INVALID)
	{
		if (lpDest)
		{
			dwMSPB=(DWORD)((float)lpSD[i].dwSize*8000/lpSD[i].fBitrate);
			if (!dwMilliSec)
			{
				dwBlocksToReturn=1;
			}
			else
			{
				switch (lpSD[i].dwFormat)
				{
					case AUDIOTYPE_MP3VBR:
						dwBlocksToReturn=1;
						break;
					case AUDIOTYPE_AC3:
					case AUDIOTYPE_DTS:
						if ((1000*dwMilliSec/dwMSPB)==1)
						{
							dwBlocksToReturn=1;
						}
						else
						if ((1000*dwMilliSec/dwMSPB)%2)
						{
							dwBlocksToReturn=3;
						}
						else
						{
							dwBlocksToReturn=2;
						}
						break;
					default:
						dwBlocksToReturn=1;
						break;
				}
			}
			for (j=0;j<(int)dwBlocksToReturn;j++)
			{
				memcpy(&(lpcDest[j*lpSD[i].dwSize]),lpSD[i].lpData,lpSD[i].dwSize);
			}
			
			if (lpdwMicroSecRead)
			{
				*lpdwMicroSecRead=(DWORD)((float)lpSD[i].dwSize*dwBlocksToReturn*8000/lpSD[i].fBitrate);
			}
			if (lpqwNanoSecRead)
			{
				*lpqwNanoSecRead=(__int64)((double)lpSD[i].dwSize*dwBlocksToReturn*8000000/lpSD[i].fBitrate);
			}
			return lpSD[i].dwSize*dwBlocksToReturn;
		}
		else
		{
			if (lpdwMicroSecRead) *lpdwMicroSecRead=0;
			if (lpqwNanoSecRead) *lpqwNanoSecRead=0;
			return 0;
		}
	}
	else
	{
		if (lpdwMicroSecRead) *lpdwMicroSecRead=0;
		if (lpqwNanoSecRead) *lpqwNanoSecRead=0;
		return 0;
	}

	return 0;
}