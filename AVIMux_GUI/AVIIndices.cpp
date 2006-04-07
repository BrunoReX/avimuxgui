#include "stdafx.h"
#include "aviindices.h"


				///////////////////////
				//  INDEX - Methoden //
				///////////////////////

bool IsMP3SampleCount(int x) 
{
	if (x % 384 == 0) {
		return true;
	}
	if (x % 576 == 0) {
		return true;
	}

	return false;
}

INDEX::INDEX(void)
{
	qwOffset=0;
	dwFlags=0;
	dwSize=0;
	dwStream=0xfffffffe;
	lpNext=NULL;
	dwTest=0;
	dwKind=0;
	qwBegin=0;
	qwEnd=(__int64)1<<63-1;
}

INDEX::~INDEX(void)
{
}

DWORD INDEX::GetSize()
{
	INDEX*	lpCurr;
	DWORD	dwSize=0;
	
// rekursiver Aufruf führt zu Stack-Overflows!
	lpCurr=this;
	while (lpCurr)
	{
		if (lpCurr->dwStream != 0xfffffffe) dwSize+=16;
		lpCurr=lpCurr->lpNext;
	}
	return (dwSize);
}

void INDEX::SetData(DWORD _dwStream,DWORD _dwFlags,_int64 _qwOffset,DWORD _dwSize,DWORD _dwKind)
{
	dwStream=_dwStream;
	dwFlags=_dwFlags;
	if (_qwOffset) qwOffset=_qwOffset;
	dwSize=_dwSize;
	dwKind=_dwKind;
}

DWORD INDEX::SelectStream(DWORD dwStreamNbr)
{
	return 0;
}

DWORD INDEX::GetStream(void)
{
	return dwStream;
}

DWORD INDEX::Store(void* lpDest)
{
	DWORD*	lpdwDest=(DWORD*)lpDest;
	INDEX*	lpCurr;
	
// rekursiver Aufruf führt zu Stack-Overflows!
	lpCurr=this;
	while (lpCurr)
	{
		if (lpCurr->dwStream==0xffffffff)
		{
			*lpdwDest++=MakeFourCC("rec ");
			lpCurr->dwFlags |= AVIIF_LIST;
		}
		else
		{
			switch (lpCurr->dwKind)
			{
				case AVI_VIDEOSTREAM:
					*lpdwDest++=MKFOURCC(48+(lpCurr->dwStream/10),48+(lpCurr->dwStream%10),'d','c');
					break;
				case AVI_AUDIOSTREAM:
					*lpdwDest++=MKFOURCC(48+(lpCurr->dwStream/10),48+(lpCurr->dwStream%10),'w','b');
					break;
				case AVI_TEXTSTREAM:
					*lpdwDest++=MKFOURCC(48+(lpCurr->dwStream/10),48+(lpCurr->dwStream%10),'t','x');
					break;
			}
//			*lpdwDest++=(!lpCurr->dwStream)?MakeFourCC("00dc"):MKFOURCC(48+(lpCurr->dwStream/10),48+(lpCurr->dwStream%10),'w','b');
		}

		if (lpCurr->dwStream != 0xfffffffe) {
			*lpdwDest++=lpCurr->dwFlags;
			*lpdwDest++=(DWORD)lpCurr->qwOffset;
			*lpdwDest++=lpCurr->dwSize;
		}
		lpCurr=lpCurr->lpNext;
	}
	return (0);
}

void INDEX::Delete(void)
{
	INDEX*	lpCurr=lpNext;
	INDEX*  lpTemp;

	while (lpCurr)
	{
		lpTemp=lpCurr->lpNext;
		delete lpCurr;
		lpCurr=lpTemp;
	}

	return;

}

bool INDEX::Valid(void)
{
	return (qwOffset>0);
}


__int64 INDEX::GetOffset(void)
{
	return qwOffset;
}

DWORD INDEX::GetChunkSize(void)
{
	return dwSize;
}

DWORD INDEX::GetFlags(void)
{
	return dwFlags;
}

DWORD INDEX::GetKindOfStream(void)
{
	return dwKind;
}

DWORD INDEX::SetRange(__int64 _qwBegin,__int64 _qwEnd)
{
	qwBegin=_qwBegin;
	qwEnd=_qwEnd;

	return 1;
}

__int64 INDEX::GetBegin()
{
	return qwBegin;
}

__int64 INDEX::GetEnd()
{
	return qwEnd;
}

				///////////////////////////////
				//  EXTENDEDINDEX - Methoden //
				///////////////////////////////
EXTENDEDINDEX::EXTENDEDINDEX(void)
{
	dwSelectedStream=0xffffffff;
}

EXTENDEDINDEX::~EXTENDEDINDEX(void)
{
}

DWORD EXTENDEDINDEX::GetSelectedStream(void)
{
	return dwSelectedStream;
}

DWORD EXTENDEDINDEX::SelectStream(DWORD dwStreamNbr)
{
	dwSelectedStream=dwStreamNbr;
	return AFE_OK;
}

DWORD EXTENDEDINDEX::GetSize(void)
{
	EXTENDEDINDEX*	lpCurr;
	DWORD			dwSize=32;

	lpCurr=this;
	while (lpCurr)
	{
		if (GetSelectedStream()==lpCurr->GetStream())
		{
			if ((lpCurr->GetOffset()>=GetBegin())&&(lpCurr->GetOffset()<=GetEnd()))
			{
				dwSize+=4*GetDWORDsperEntry();
			}
		}
		lpCurr=(EXTENDEDINDEX*)lpCurr->lpNext;
	}

	return dwSize;
}

DWORD EXTENDEDINDEX::GetDWORDsperEntry(void)
{
	return 0;
}

				////////////////////////////
				//  SUPERINDEX - Methoden //
				////////////////////////////

SUPERINDEX::SUPERINDEX(void)
{
	qwCurrStreamSize=0;
}

SUPERINDEX::~SUPERINDEX(void)
{
}

DWORD SUPERINDEX::GetSize()
{
	return EXTENDEDINDEX::GetSize()-8;
}

DWORD SUPERINDEX::GetDWORDsperEntry(void)
{
	return 4;
}

DWORD SUPERINDEX::SetStreamFormat(void* lpFormat)
{
	lpstrf=lpFormat;
	return AFE_OK;
}

DWORD SUPERINDEX::SetCurrentStreamSize(__int64 qwCSS)
{
	qwCurrStreamSize=qwCSS;
	return AFE_OK;
}

__int64 SUPERINDEX::GetCurrentStreamSize(void)
{
	return qwCurrStreamSize;
}

DWORD SUPERINDEX::SetStreamHeader(AVIStreamHeader* _lpHeader)
{
	lpHeader=_lpHeader;
	return AFE_OK;
}

DWORD SUPERINDEX::Store(void* lpDest)
{
	SUPERINDEX*		lpCurr;
	DWORD			dwEntries=0;
	union
	{
		BYTE*		lpbDest;
		WORD*		lpwDest;
		DWORD*		lpdwDest;
		__int64*	lpqwDest;
	};
	DWORD			dwSize;
	WAVEFORMATEX*	lpwfe=(WAVEFORMATEX*)lpstrf;
	__int64		qwLastStreamSize=0;
	DWORD*			lpdwStreamID;

	lpdwDest=(DWORD*)lpDest;
	
	*lpwDest++=(WORD)GetDWORDsperEntry();
	*lpbDest++=0;
	*lpbDest++=AVI_INDEX_OF_INDEXES;
	*lpdwDest++=0; // number of entries not yet known

	lpdwStreamID=lpdwDest;
	*lpdwDest++=0; 

	*lpqwDest++=0; // dwReserved[0]-[2]
	*lpdwDest++=0;

	lpCurr=this;
	while (lpCurr)
	{
		if (lpCurr->GetStream()==GetSelectedStream())
		{
			if (!*lpdwStreamID)
			{
				switch (lpCurr->GetKindOfStream())
				{
					case AVI_VIDEOSTREAM:
						*lpdwStreamID=MKFOURCC(48+(lpCurr->GetStream()/10),48+(lpCurr->GetStream()%10),'d','c');
						break;
					case AVI_AUDIOSTREAM:
						*lpdwStreamID=MKFOURCC(48+(lpCurr->GetStream()/10),48+(lpCurr->GetStream()%10),'w','b');
						break;
					case AVI_TEXTSTREAM:
						*lpdwStreamID=MKFOURCC(48+(lpCurr->GetStream()/10),48+(lpCurr->GetStream()%10),'t','x');
						break;
				}
			}
			dwEntries++;
			*lpqwDest++=lpCurr->GetOffset();
			*lpdwDest++=lpCurr->GetChunkSize();
			if (lpCurr->GetKindOfStream()==AVI_VIDEOSTREAM)
			{
				// Video: dwStreamTicks = Anzahl Frames = Anzahl Einträge
				*lpdwDest++=(DWORD)round((lpCurr->GetChunkSize()-32)/8);
			}
			else
			if (lpCurr->GetKindOfStream()==AVI_AUDIOSTREAM)
			{
				// Audio
				if (((lpwfe->wFormatTag!=0x55)||(!IsMP3SampleCount(lpwfe->nBlockAlign)))&&lpwfe->wFormatTag!=0xFF)
				
				{
					// CBR
					*lpdwDest++=(DWORD)round(DWORD(lpCurr->GetCurrentStreamSize()-qwLastStreamSize)/lpHeader->dwScale);
				}
				else
				{
					// VBR
					*lpdwDest++=(DWORD)round((lpCurr->GetChunkSize()-32)/8);
				}
			}
			else
			{
				// Text:
				*lpdwDest++=1;
			}
			qwLastStreamSize=lpCurr->GetCurrentStreamSize();
		}
		lpCurr=(SUPERINDEX*)lpCurr->lpNext;
	}
	
	lpdwDest=(DWORD*)lpDest;
	dwSize=24+16*dwEntries;
	lpdwDest++;
	*lpdwDest++=dwEntries;
	
	return (dwSize);
}
				///////////////////////////////
				//  STANDARDINDEX - Methoden //
				///////////////////////////////

STANDARDINDEX::STANDARDINDEX(void)
{

}

STANDARDINDEX::~STANDARDINDEX(void)
{
}

DWORD STANDARDINDEX::GetDWORDsperEntry(void)
{
	return 2;
}

DWORD STANDARDINDEX::Store(void* lpDest)
{
	STANDARDINDEX*	lpCurr;
	DWORD			dwEntries=0;
	union
	{
		BYTE*		lpbDest;
		WORD*		lpwDest;
		DWORD*		lpdwDest;
		__int64*	lpqwDest;
	};
	__int64		qwOffset;
	DWORD			dwSize;
	DWORD*			lpdwStreamID;

	lpdwDest=(DWORD*)lpDest;
	
	*lpdwDest++=MKFOURCC('i','x',48+(GetSelectedStream()/10),48+(GetSelectedStream()%10));
	*lpdwDest++=0; // size not yet known
	*lpwDest++=(WORD)GetDWORDsperEntry();
	*lpbDest++=0;
	*lpbDest++=AVI_INDEX_OF_CHUNKS;
	*lpdwDest++=0; // Anzahl Entries wissen wir auch noch nicht
/*	*lpdwDest++=(!GetSelectedStream())?MKFOURCC('0','0','d','c'):
		MKFOURCC( 48+(GetSelectedStream()/10),48+(GetSelectedStream()%10),'w','b');
*/
	lpdwStreamID=lpdwDest;
	*lpdwDest++=0;


	*lpqwDest++=(qwOffset=GetOffset()); 
	*lpdwDest++=0;// dwReserved

	lpCurr=this;
	while (lpCurr)
	{
		if (lpCurr->GetStream()==GetSelectedStream())
		{
			if ((lpCurr->GetOffset()>=GetBegin())&&(lpCurr->GetOffset()<=GetEnd()))
			{
				dwEntries++;
				*lpdwDest++=(DWORD)(lpCurr->GetOffset()-qwOffset)+8;
				*lpdwDest++=lpCurr->GetChunkSize()|( ((GetSelectedStream())||(lpCurr->GetFlags()&AVIIF_KEYFRAME))?0:(1<<31) );
				if (!*lpdwStreamID)
				{
					switch (lpCurr->GetKindOfStream())
					{
						case AVI_VIDEOSTREAM:
							*lpdwStreamID=MKFOURCC(48+(lpCurr->GetStream()/10),48+(lpCurr->GetStream()%10),'d','c');
							break;
						case AVI_AUDIOSTREAM:
							*lpdwStreamID=MKFOURCC(48+(lpCurr->GetStream()/10),48+(lpCurr->GetStream()%10),'w','b');
							break;
						case AVI_TEXTSTREAM:
							*lpdwStreamID=MKFOURCC(48+(lpCurr->GetStream()/10),48+(lpCurr->GetStream()%10),'t','x');
							break;
					}
				}
			}
		}
		lpCurr=(STANDARDINDEX*)lpCurr->lpNext;
	}
	
	lpdwDest=(DWORD*)lpDest;
	lpdwDest++;
	*lpdwDest++=(dwSize=24+8*dwEntries);
	lpdwDest++;
	*lpdwDest++=dwEntries;

	return (dwSize+8);

}
