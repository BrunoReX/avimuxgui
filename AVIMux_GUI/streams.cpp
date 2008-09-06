#include "stdafx.h"
#include "streams.h"
#include "..\utf-8.h"
// MEMORYSTREAM
/*
MEMORYSTREAM::MEMORYSTREAM(void)
{
	ZeroMemory(&status,sizeof(status));
};

MEMORYSTREAM::~MEMORYSTREAM(void) 
{

};

int MEMORYSTREAM::Open(CFileStream* lpSource, DWORD _dwMode)
{
	DWORD		dwNbrOfMB;
	DWORD		i;
	void*		lpBuffer;

	status.bOpened=true;

	if (_dwMode==STREAM_READ)
	{
		qwSize=lpSource->GetSize();
		dwNbrOfMB=(DWORD)((qwSize>>20)+((qwSize%(1<<20))?1:0));
		lpBuffer=malloc(1<<20);
		lplpCache=(LINEARCACHE**)malloc(4*dwNbrOfMB);
		for (i=0;i<dwNbrOfMB;i++)
		{
			lplpCache[i]=new LINEARCACHE;
			if (i<dwNbrOfMB-1) 
			{
				lplpCache[i]->SetSize(1<<20);
			}
			else
			{
				lplpCache[i]->SetSize((DWORD)(qwSize%(1<<20)));
			}
			lpSource->Read(lpBuffer,1<<20);
			lplpCache[i]->SetData(lpBuffer);
		}
		free(lpBuffer);
		dwMode = _dwMode;
	}
	else
	if (_dwMode=STREAM_WRITE)
	{
		qwSize=0;
		status.qwPosition=0;
		status.dwNbrOfLCs=0xFFFFFFFF;
		status.dwLastWriteDest=0xFFFFFFFF;
		status.bOpened=true;
	}

	status.qwPosition=0;
	lpSource->Close();
	return STREAM_OK;
}

int MEMORYSTREAM::Close()
{
	DWORD		dwNbrOfMB;
	DWORD		i;

	if (status.bOpened)
	{
		status.bOpened=false;
		dwNbrOfMB=(qwSize>>20)+(qwSize%(1<<20))?1:0;
		for (i=0;i<dwNbrOfMB;i++)
		{
			delete lplpCache[i];
		}
		free(lplpCache);
		return STREAM_OK;
	}
	else
	{
		return STREAM_ERR;
	}
}

int MEMORYSTREAM::Seek(__int64 qwPos)
{
	if (!status.bOpened) return STREAM_ERR;

	if (qwPos>qwSize) return STREAM_ERR;

	status.qwPosition=qwPos;
	return STREAM_OK;
}

__int64 MEMORYSTREAM::GetPos()
{
	return status.qwPosition;
}

__int64 MEMORYSTREAM::GetSize()
{
	return qwSize;
}

int MEMORYSTREAM::ReadPart(void* lpDest,DWORD dwBytes)
{
	DWORD	dwCurrMB,dwOffset,dwRead;

	dwCurrMB=(DWORD)((status.qwPosition>>20)+(((status.qwPosition+1)%(1<<20))?0:-1));
	dwOffset=(DWORD)(status.qwPosition%(1<<20));
	lplpCache[dwCurrMB]->Seek(dwOffset);
	dwRead=lplpCache[dwCurrMB]->Read(lpDest,dwBytes);
	status.qwPosition+=dwRead;
	return dwRead;
}

int	MEMORYSTREAM::WritePart(void* lpSource,DWORD dwBytes)
{
	DWORD	dwCurrMB,dwOffset;

	dwCurrMB=(DWORD)((status.qwPosition>>20)+(((status.qwPosition+1)%(1<<20))?0:-1));
	dwOffset=(DWORD)(status.qwPosition%(1<<20));

	return 0;
}

int MEMORYSTREAM::Read(void* lpDest,DWORD dwBytes)
{
	DWORD	dwRead=0;
	BYTE*	lpbDest=(BYTE*)lpDest;

	while ( (!IsEndOfStream())&&(dwRead<dwBytes) )
	{
		dwRead+=ReadPart(&(lpbDest[dwRead]),dwBytes-dwRead);
	}
	
	return dwRead;
}

int MEMORYSTREAM::Write(void* lpSource,DWORD dwBytes)
{
	return 0;
}


bool MEMORYSTREAM::IsEndOfStream()
{
	return (status.qwPosition>=qwSize);
}

*/
