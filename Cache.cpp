#include "stdafx.h"
#include "Cache.h"
#include "math.h"
#include "stdlib.h"
#include "windows.h"
#include "integers.h"

extern "C" {
	void _stdcall shr64(__int64* x, int y);
	void _stdcall zerobits64 (void* i, int j);
}


bool _cacheAllowReadAhead = false;

void  cacheAllowReadAhead(bool bAllow)
{
	_cacheAllowReadAhead = bAllow;
}

#define QUADWORD __int64

LINEARCACHE::LINEARCACHE(void)
{
	dwSize=0;
	dwPosition=0;
	lpbData=NULL;
	lpbAllocatedData = NULL;
	align = 1 << 16;
}

LINEARCACHE::~LINEARCACHE(void)
{
}

bool LINEARCACHE::SetSize(DWORD _dwSize,bool bCreate)
{
	dwSize=_dwSize;
	if (bCreate)
	{
		if (lpbAllocatedData) free (lpbAllocatedData);
		lpbAllocatedData = new BYTE[dwSize + align + 16];
		lpbData = (BYTE*)(((unsigned)lpbAllocatedData & ~(align - 1)) + align);
	}
	return (lpbData)?true:false;
}

__int64 LINEARCACHE::GetPos()
{
	return dwPosition;
}

void LINEARCACHE::SetData(void* lpSource)
{
	memcpy(lpbData,lpSource,dwSize);
}

__int64 LINEARCACHE::GetSize(void)
{
	return dwSize;
}

int LINEARCACHE::Seek(__int64 iPos)
{
	if (iPos>dwSize) return false;
	dwPosition=(int)iPos;
	return true;
}

int LINEARCACHE::Read(void* lpDest,DWORD dwBytes)
{
	DWORD	dwBytesToCopy;

	dwBytesToCopy=min(dwBytes,dwSize-dwPosition);
	if (lpDest) memcpy(lpDest,&(lpbData[dwPosition]),dwBytesToCopy);
	dwPosition+=dwBytesToCopy;
	return dwBytesToCopy;
}

int LINEARCACHE::Write(void* lpSource,DWORD dwBytes)
{
	DWORD	dwBytesToCopy;

	dwBytesToCopy=min(dwBytes,dwSize-dwPosition);
	memcpy(&(lpbData[dwPosition]),lpSource,dwBytesToCopy);
	dwPosition+=dwBytesToCopy;
	return dwBytesToCopy;
}

int LINEARCACHE::Clear(void)
{
	if (lpbAllocatedData)
	{
		free(lpbAllocatedData);
		lpbData=NULL;
		lpbAllocatedData = NULL;
	}
	return S_OK;
}

bool LINEARCACHE::IsEndOfCache(void)
{
	return (dwPosition==dwSize);
}

bool LINEARCACHE::IsValid(void)
{
	return (!!dwSize);
}

BYTE* LINEARCACHE::GetData(void)
{
	return lpbData;
}

// CACHEDSTREAM

CACHEDSTREAM::CACHEDSTREAM(DWORD dwBuffers,DWORD _dwBytesPerBuffer)
{
	lpSource=NULL;
	qwPos=0;
	stats.iHits = 0;
	stats.iMisses = 0;
	dwNbrOfBuffers=dwBuffers;
	dwBytesPerBuffer=_dwBytesPerBuffer;
	lpCacheInfo=new LINEARCACHEINFO[dwBuffers];
	bReadAhead = false;
	lplpCache=new LINEARCACHE*[dwBuffers];
	ZeroMemory(lpCacheInfo,sizeof(LINEARCACHEINFO)*dwBuffers);
	ZeroMemory(lplpCache,4*dwBuffers);
	iModVal=1;
	DWORD _dwBuffers = dwBuffers;
	while (! (_dwBuffers % 2)) {
		_dwBuffers >>= 1; iModVal = (iModVal<<1); 
	};
	
	iBufferBits = -1;
	
	while (_dwBytesPerBuffer) {
		_dwBytesPerBuffer >>= 1; iBufferBits++; }

	for (DWORD i=0;i<dwBuffers;i++)
	{
		lplpCache[i]=new LINEARCACHE;
		lplpCache[i]->SetSize(dwBytesPerBuffer);
	}
	iLastBufferInUse = 0;
	iLastBufferBegin = 0;
	iLastBufferEnd = -1;
}

CACHEDSTREAM::~CACHEDSTREAM()
{
}

void CACHEDSTREAM::EnableReadAhead(bool bEnable)
{
	bReadAhead = bEnable;
}

STREAM* CACHEDSTREAM::GetSource()
{
	return lpSource;
}

int CACHEDSTREAM::GetCacheStats(CACHESTATS* lpstats)
{
	if (lpstats) memcpy(lpstats,&stats,sizeof(stats));
	return 1;
}

int CACHEDSTREAM::Open(STREAM* _lpSource, int mode)
{
	lpSource=_lpSource;
	qwPos=0;
	iLastPosWritten = 0;
	iAccess = mode;
	LoadSegment(0,0);


	return STREAM_OK;
}

int CACHEDSTREAM::Close()
{
	for (DWORD i=0;i<dwNbrOfBuffers;i++)
	{
		if (lplpCache)
		{
			if (lplpCache[i]) 
			{
				WriteCachelineBack(i);
				lplpCache[i]->Clear();
				delete lplpCache[i];
				lplpCache[i]=NULL;
			}
		}
	}
	if (lplpCache)
	{
		free(lplpCache);
		lplpCache=NULL;
	}
	if (lpCacheInfo) free(lpCacheInfo);
	if (!IsReading()) {
		GetDest()->TruncateAt(iLastPosWritten);
		GetDest()->Close();
		delete GetDest();
	}
	lpCacheInfo=NULL;
	return STREAM_OK;
}

__int64 CACHEDSTREAM::GetSize()
{
	return GetSource()->GetSize()-GetOffset();
}

__int64 CACHEDSTREAM::GetPos()
{
	return qwPos-GetOffset();
}

int CACHEDSTREAM::GetOffset()
{
	return GetSource()->GetOffset();
}

int CACHEDSTREAM::SetOffset(int iNewOffset)
{
	return GetSource()->SetOffset(iNewOffset);
}

int CACHEDSTREAM::Seek(QUADWORD _qwPos)
{
	qwPos=_qwPos+GetOffset();
	if (IsEndOfStream() && IsReading()) qwPos=GetSize()+GetOffset();
	return STREAM_OK;
}

bool CACHEDSTREAM::IsEndOfStream()
{
	return (GetSize()<=GetPos())?true:false;
}

// MUST NOT check for IsEndOfStream. That would break writing!
int CACHEDSTREAM::Read(void* lpDest,DWORD dwBytes)
{
	DWORD	dwRead,dwCount,dwNbrOfBuffer,dwOffset,dwFree;
	BYTE*	lpbDest=(BYTE*)lpDest;
	__int64 qwLatestBlock;
	__int64	q;

	DWORD dwBytesToRead;
	
	dwBytesToRead = (IsReading()?(DWORD)(min(dwBytes,GetSize()-GetPos())):dwBytes);
	if (!dwBytesToRead) return 0;

	{
		dwCount = 1;
		if (FindBuffer(GetPos(),&dwNbrOfBuffer,(dwBytes>1)?&dwCount:NULL,&dwOffset))
		{
			stats.iHits++;
			
			if (dwCount>=dwBytes)
			{
				lplpCache[dwNbrOfBuffer]->Seek(dwOffset);
				qwPos+=(dwRead=lplpCache[dwNbrOfBuffer]->Read(lpDest,dwBytes));
				return dwRead;
			}
			else
			{
				lplpCache[dwNbrOfBuffer]->Seek(dwOffset);
				qwPos+=lplpCache[dwNbrOfBuffer]->Read(lpDest,dwCount);
				return dwCount+Read((lpDest?&(lpbDest[dwCount]):NULL),dwBytes-dwCount);
			}
		}
		else
		{
			stats.iMisses++;
			q = GetPos();
			shr64(&q,iBufferBits);

			qwLatestBlock=LastBlockRead();		

			if ( ((int)(q-qwLatestBlock)<(GetNbrOfBuffers()>>1))&&(q>qwLatestBlock) )
			{
				for (DWORD i=1;i<(q-qwLatestBlock);i++)
				{
					dwFree = (int)(q % iModVal);
					LoadSegment(dwFree,(qwLatestBlock+i)*GetBytesPerBuffer());
				}
			}

			dwFree = (int)(q % iModVal);
			__int64  qwPos2 = qwPos;
			zerobits64(&qwPos2, iBufferBits);
		
			__int64 j = GetPos();
			LoadSegment(dwFree,qwPos2);
			j = GetPos();
			dwRead=Read(lpDest,dwBytes);
			j = GetPos();

			return dwRead;


/*			__int64 iTime = rdtsc();

			stats.iMisses++;

			dwBlockToRead=(DWORD)(GetPos()/GetBytesPerBuffer());
			dwLatestBlock=LastBlockRead();		

			if ( ((int)(dwBlockToRead-dwLatestBlock)<(GetNbrOfBuffers()>>1))&&(dwBlockToRead>dwLatestBlock) )
			{
				for (DWORD i=1;i<(dwBlockToRead-dwLatestBlock);i++)
				{
					dwFree = dwBlockToRead % iModVal;
					LoadSegment(dwFree,(dwLatestBlock+i)*GetBytesPerBuffer());
				}
			}

			dwFree = dwBlockToRead % iModVal;

			qwPos2 = (qwPos/GetBytesPerBuffer()*GetBytesPerBuffer());
			__int64 iTime2 = rdtsc()-iTime;
			fprintf(stderr,"%10I64d",iTime2);
			LoadSegment(dwFree,qwPos2);

			dwRead=Read(lpDest,dwBytes);
			return dwRead;
*/		}
	}

	return dwRead;
}

int CACHEDSTREAM::Write(void* lpSrc, DWORD dwBytes)
{
	BYTE*	lpbSrc = (BYTE*)(lpSrc);
	DWORD	dwNbr, dwOffset, dwBytesInCacheline, dwWritten, dwTotal, dwLeft = dwBytes;

	dwTotal = 0;
	while (dwLeft) {
		PrepareForWriting(GetPos(), &dwNbr, &dwBytesInCacheline, &dwOffset);
		lpCacheInfo[dwNbr].bDirty = 1;
		lplpCache[dwNbr]->Seek(dwOffset);
		dwWritten = lplpCache[dwNbr]->Write(lpbSrc+dwTotal, dwLeft);
		lpCacheInfo[dwNbr].dwMaxUsedSize=max(dwOffset+dwWritten, lpCacheInfo[dwNbr].dwMaxUsedSize);
		if (dwLeft -= dwWritten) {
//			Sleep(1);
		}
		qwPos += dwWritten;
		dwTotal += dwWritten;
		if (qwPos > iLastPosWritten) {
			iLastPosWritten = qwPos;
		}
	}

	return dwTotal;
}

bool CACHEDSTREAM::IsBufferUsed(DWORD dwNbr)
{
	if (dwNbr>=(DWORD)GetNbrOfBuffers()) return false;
	return (!!lpCacheInfo[dwNbr].dwSysTime);
}

DWORD CACHEDSTREAM::GetBytesPerBuffer()
{
	return dwBytesPerBuffer;
}

int CACHEDSTREAM::GetNbrOfBuffers()
{
	return dwNbrOfBuffers;
}

int CACHEDSTREAM::LoadSegment(DWORD dwNbr,QUADWORD qwStart)
{
	if (!bReadAhead || !_cacheAllowReadAhead) {
		__int64 j = GetPos();
		WriteCachelineBack(dwNbr);
		j = GetPos();

		if (dwNbr>=(DWORD)GetNbrOfBuffers()) return 0;
	
		if (IsReading() || qwStart < iLastPosWritten) {
			if (GetSource()->GetPos()!=qwStart) GetSource()->Seek(qwStart);
		}

		if (IsReading()) {
			lplpCache[dwNbr]->SetSize(
				lpCacheInfo[dwNbr].dwMaxUsedSize=GetSource()->Read(lplpCache[dwNbr]->GetData(),GetBytesPerBuffer()),false);
		} else {
			if (qwStart < iLastPosWritten) {
				lplpCache[dwNbr]->SetSize(GetBytesPerBuffer(), false);
				lpCacheInfo[dwNbr].dwMaxUsedSize=GetSource()->Read(lplpCache[dwNbr]->GetData(),GetBytesPerBuffer());
			} else {
				lplpCache[dwNbr]->SetSize(GetBytesPerBuffer());
				ZeroMemory(lplpCache[dwNbr]->GetData(), GetBytesPerBuffer());
				lpCacheInfo[dwNbr].dwMaxUsedSize=0;
			}
		}

		lpCacheInfo[dwNbr].dwSysTime=GetTickCount();
		lpCacheInfo[dwNbr].qwStart=qwStart;
		if (!lpCacheInfo[dwNbr].dwMaxUsedSize) {
			lpCacheInfo[dwNbr].dwMaxUsedSize = (DWORD)lplpCache[dwNbr]->GetSize();
		}
		iLastBufferInUse = dwNbr;
		iLastBufferBegin = qwStart;
		iLastBufferEnd = qwStart + GetBytesPerBuffer() - 1;
		lpCacheInfo[dwNbr].bDirty = 0;
		return 0;
	} else {
		LINEARCACHEINFO* lci = &lpCacheInfo[dwNbr];
		if (lci->bLoading && qwStart == lci->pRAS->iStart) {
			lci->qwStart = qwStart;
			iLastBufferInUse = dwNbr;
			iLastBufferBegin = qwStart;
			iLastBufferEnd = qwStart + GetBytesPerBuffer() - 1;

			GetSource()->WaitForCompletion(lci->pRAS);
			lci->bLoading = 0;
			CloseHandle(lci->pRAS->hSemaphore);
			lplpCache[dwNbr]->SetSize(lci->pRAS->pOverlapped->Offset, false);
			lpCacheInfo[dwNbr].dwMaxUsedSize = lci->pRAS->pOverlapped->Offset;
			delete lci->pRAS;
			lci->dwSysTime = GetTickCount();
			lci->pRAS = NULL;
		} else {
			if (lci->bLoading) {
				GetSource()->WaitForCompletion(lci->pRAS);
				lci->bLoading = 0;
				CloseHandle(lci->pRAS->hSemaphore);
				delete lci->pRAS;
				lci->dwSysTime = GetTickCount();
				lci->pRAS = NULL;
			}
			EnableReadAhead(false);
			LoadSegment(dwNbr, qwStart);
			EnableReadAhead();
		}

		dwNbr++;
		dwNbr%=iModVal;
		WriteCachelineBack(dwNbr);
		lci = &lpCacheInfo[dwNbr];
	
//		lci->qwStart = 
		lci->qwStart = 0;
		lci->pRAS = new READ_ASYNC_STRUCT;
		lci->pRAS->lpDest = lplpCache[dwNbr]->GetData();
		lci->pRAS->iStart = qwStart + GetBytesPerBuffer();
		GetSource()->Seek(lci->pRAS->iStart);
		if (!(lci->bLoading = GetSource()->ReadAsync(lci->pRAS, GetBytesPerBuffer()))) {
			CloseHandle(lci->pRAS->hSemaphore);
			delete lci->pRAS->pOverlapped;
			delete lci->pRAS;
			lci->pRAS = NULL;
		}

		return 0;
				
	}
}

bool CACHEDSTREAM::IsInBuffer(DWORD dwNbr,QUADWORD _qwPos,DWORD* lpdwHowMany)
{
	if (!IsBufferUsed(dwNbr)) return false;
	if (lpCacheInfo[dwNbr].bLoading) return false;
	if (lpdwHowMany) *lpdwHowMany=0;
	if (_qwPos<lpCacheInfo[dwNbr].qwStart)
	{
		return false;
	}
	else
	{
		if (lpCacheInfo[dwNbr].qwStart+GetBytesPerBuffer()<=_qwPos)
		{
			return false;
		}
		else
		{
			if (lpdwHowMany) *lpdwHowMany=(DWORD)(lpCacheInfo[dwNbr].qwStart+GetBytesPerBuffer()-_qwPos);
			return true;
		}
	}
}


// find oldest cache line
// output: *lpdwFree: index of deleted cache line
void CACHEDSTREAM::DeleteOldestBuffer(DWORD* lpdwFree)
{
	LINEARCACHEINFO*	lpOldest;
	int					i,iOldest;

	lpOldest=&(lpCacheInfo[0]);
	iOldest=0;

	for (i=1;i<GetNbrOfBuffers();i++)
	{
		if (IsBufferUsed(i))
		{
			if (lpCacheInfo[i].dwSysTime<lpOldest->dwSysTime)
			{
				lpOldest=&(lpCacheInfo[i]);
				iOldest=i;
			}
		}
	}

	*lpdwFree=iOldest;
	WriteCachelineBack(iOldest);
	lpCacheInfo[iOldest].dwSysTime=0;
	lpCacheInfo[iOldest].qwStart=0;
}

int CACHEDSTREAM::InvalidateCache()
{
	for (DWORD i=0;i<(DWORD)GetNbrOfBuffers();i++)
	{
		if (IsBufferUsed(i))
		{
			WriteCachelineBack(i);
			lpCacheInfo[i].dwSysTime=0;
			lpCacheInfo[i].qwStart=0;
		}
	}
	return STREAM_OK;
}

int CACHEDSTREAM::LastBlockRead(void)
{
	QUADWORD  qwPos=0;

	for (int i=0;i<GetNbrOfBuffers();i++)
	{
		if (IsBufferUsed(i))
		{
			if (lpCacheInfo[i].qwStart>qwPos) qwPos=lpCacheInfo[i].qwStart;
		}
	}
	return (DWORD)(qwPos/GetBytesPerBuffer());
}

// output: return   : no buffer free for new cache line <=> return true
//         *lpdwFree: index of free cache line 
bool CACHEDSTREAM::AllBuffersUsed(DWORD* lpdwFree)
{
	bool	bRes=true;

	for (int i=0;(i<GetNbrOfBuffers())&&(bRes);i++)
	{
		if (!IsBufferUsed(i)) if (lpdwFree) *lpdwFree=i;
		bRes&=IsBufferUsed(i);
	}
	return bRes;
}

// input:  _qwPos       : absolute position within stream
// output: *lpdwNbr     : index of cache line
//         *lpdwHowMany : bytes from desired position to end of cache line
//         *lpdwPOffset : offset of _qwPos within cache line 
bool CACHEDSTREAM::FindBuffer(QUADWORD _qwPos,DWORD* lpdwNbr,DWORD* lpdwHowMany,DWORD* lpdwOffset)
{
	__int64 i;
	int		j;
	bool	bRes=false;
	i = _qwPos + GetOffset(); 
	int iBuffer;

	if (i>=iLastBufferBegin && i<=iLastBufferEnd) {
		if (lpdwNbr) *lpdwNbr=iLastBufferInUse;
		if (lpdwOffset) *lpdwOffset=(DWORD)(i-lpCacheInfo[iLastBufferInUse].qwStart);
		if (lpdwHowMany) *lpdwHowMany=(DWORD)(iLastBufferEnd-i+1);
		bRes=true;
		return bRes;
	}

	j = iBufferBits; 
	shr64(&i,j);
	iBuffer = (int)i % iModVal;

	if (IsInBuffer(j=iBuffer,_qwPos,lpdwHowMany)) {
		iLastBufferInUse = j;
		iLastBufferBegin = lpCacheInfo[j].qwStart;
		iLastBufferEnd = lpCacheInfo[j].qwStart + GetBytesPerBuffer()-1;

		if (lpdwNbr) *lpdwNbr=j;
		if (lpdwOffset) *lpdwOffset=(DWORD)(_qwPos-lpCacheInfo[j].qwStart);
		bRes=true;
	}
	return bRes;
}

int CACHEDSTREAM::WriteCachelineBack(DWORD dwNbr)
{
	if (GetDest() && lpCacheInfo[dwNbr].bDirty) {
		GetDest()->Seek(lpCacheInfo[dwNbr].qwStart);
		GetDest()->Write(lplpCache[dwNbr]->GetData(), lpCacheInfo[dwNbr].dwMaxUsedSize);
		int last_byte = (int)(lpCacheInfo[dwNbr].qwStart + GetBytesPerBuffer() - 1);
		lpCacheInfo[dwNbr].bDirty = 0;
	}

	return 0;
}

STREAM* CACHEDSTREAM::GetDest()
{
	return (iAccess == CACHE_WRITE?lpDest:NULL);
}

int CACHEDSTREAM::PrepareForWriting(__int64 qwPos, DWORD* lpdwNbr, DWORD* lpdwMaxBytes, DWORD *lpdwOffset)
{
	if (!FindBuffer(qwPos, lpdwNbr, lpdwMaxBytes, lpdwOffset)) {
		if (qwPos < GetBytesPerBuffer()) {
			Sleep(1);
		}
		Seek(qwPos);
		Read(NULL, 1);
		Seek(qwPos);
		FindBuffer(qwPos, lpdwNbr, lpdwMaxBytes, lpdwOffset);
	} else {
		Seek(qwPos);
	}

	return 1;
}

bool CACHEDSTREAM::IsReading()
{
	return iAccess == CACHE_READ;
}