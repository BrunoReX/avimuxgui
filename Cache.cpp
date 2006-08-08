/* The CACHE class implements a transparent thread safe cache system
   that features asnychronous read ahead, asynchronous writeback and
   unbuffered I/O with an underlying FILESTREAM class. However, note
   that thread-safety and asnychronous read-ahead are mutually exclusive.

   I had to do this a second time from scratch because the first attempt
   ended up "fubar". It should work correctly now.

   Todo: Check if latest modifications to respect the stream offset
   did not b0rk anything

*/


#include "stdafx.h"
#include "Cache.h"
#include "math.h"
#include "stdlib.h"
#include "windows.h"
#include "crtdbg.h"
#include "limits.h"
#include "Filestream.h"

#ifdef DEBUG_NEW
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/////////////////////
/* new cache class */
/////////////////////

#define MAX_CACHE_LINES 1024

#define QUADWORD __int64

/* this is only required for Vorbis audio source class */
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
		lpbData = (BYTE*)(((size_t)lpbAllocatedData & ~(align - 1)) + align);
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


#include <algorithm>

operator <(CACHE_LINE& one, CACHE_LINE& other)
{
	if (one.iSourceOffset < other.iSourceOffset) return true;
	return false;
}


CACHE::CACHE()
{
	Init();
}

void CACHE::Init() 
{
	iNumberOfCacheLines = 0;
	iAccessMode = -1;
	sts.iLastAccessedCacheLineIndexRead = -1;
	sts.iLastAccessedCacheLineIndexWrite = -1;
	SetSource(NULL);
	iStreamSize = -1;
	sts.iBytesLeftInCurrentReadCacheLine = -1;
	sts.iBytesLeftInCurrentWriteCacheLine = -1;
	cache_lines = NULL;
	bAsynchWrite = false;
	bReadAhead = false;
	iLastPossibleWritePos = -1;
	bCanGrow = false;
	bSingleAsynchWrite = false;
	bImmedWriteback = false;
	bThreadsafe = false;
	bLog = false;
	fLog = stdout;
	iPrereadCacheLines = 1;

	free_cache_lines.clear();
}

bool CACHE::AllocateCacheLine(CACHE_LINE** cache_line, int size, int alignment)
{
	BYTE* p = (BYTE*)malloc(size + alignment + 1);
	
	if (!p)
		return false;

	*cache_line = new CACHE_LINE;
	(*cache_line)->pAllocated = p;

	p = (BYTE*)((size_t)p &~ (alignment-1));
	p = (BYTE*)((size_t)p + alignment);
	(*cache_line)->pData = p;

	return true;	
}

int CACHE::LockCacheLine(int iCacheLineIndex)
{
	if (iCacheLineIndex < 0 || iCacheLineIndex >= iNumberOfCacheLines)
		return 0;

	cache_lines[iCacheLineIndex]->locked++;

	return 1;
}

int CACHE::LockCacheLine(CACHE_LINE* cache_line)
{
	cache_line->locked++;

	return 1;
}

int CACHE::UnlockCacheLine(CACHE_LINE* cache_line)
{
	cache_line->locked--;

	return 1;
}

bool CACHE::CacheLineGetThreadSpecific(CACHE_LINE* cache_line)
{
	EnterCritical();

	DWORD dwCurrentThread = GetCurrentThreadId();

	if (cache_line->dwLastThread == dwCurrentThread) {
		LeaveCritical();
		return true;
	}

	CACHE_LINE_THREAD_SPECIFIC_MAP::iterator iter = cache_line->thread_specific_map.find(dwCurrentThread);

//	_ASSERT(cache_line->thread_specific_map.size() <= 1);

	if (iter != cache_line->thread_specific_map.end()) {
		cache_line->thread_specific = iter->second;
	} else {
		CACHE_LINE_THREAD_SPECIFIC* new_thread_specific = new CACHE_LINE_THREAD_SPECIFIC;
		new_thread_specific->iCurrentReadPos = -1;
		new_thread_specific->iCurrentWritePos = -1;
		cache_line->thread_specific_map[dwCurrentThread] = new_thread_specific;
		cache_line->thread_specific = new_thread_specific;
	}

	cache_line->dwLastThread = dwCurrentThread;
	LeaveCritical();

	return true;
}

bool CACHE::CacheLineGetThreadSpecific_NCS(CACHE_LINE* cache_line)
{
	DWORD dwCurrentThread = GetCurrentThreadId();

	if (cache_line->dwLastThread == dwCurrentThread)
		return true;


	CACHE_LINE_THREAD_SPECIFIC_MAP::iterator iter = cache_line->thread_specific_map.find(dwCurrentThread);

//	_ASSERT(cache_line->thread_specific_map.size() <= 1);

	if (iter != cache_line->thread_specific_map.end()) {
		cache_line->thread_specific = iter->second;
	} else {
		CACHE_LINE_THREAD_SPECIFIC* new_thread_specific = new CACHE_LINE_THREAD_SPECIFIC;
		new_thread_specific->iCurrentReadPos = -1;
		new_thread_specific->iCurrentWritePos = -1;
		cache_line->thread_specific_map[dwCurrentThread] = new_thread_specific;
		cache_line->thread_specific = new_thread_specific;
	}

	cache_line->dwLastThread = dwCurrentThread;

	return true;
}

bool CACHE::CacheLineDeleteThreadSpecificMap(CACHE_LINE* cache_line)
{
	if (bThreadsafe) {
		_ASSERT(!IsCacheLineLocked(cache_line));

		CACHE_LINE_THREAD_SPECIFIC_MAP::iterator iter = cache_line->thread_specific_map.begin();
		for (; iter != cache_line->thread_specific_map.end(); iter++) 
			delete iter->second;

		cache_line->thread_specific_map.clear();
		cache_line->dwLastThread = 0;
	}
	return true;
}

int CACHE::GetCacheLineCurrentReadPos(CACHE_LINE* cache_line)
{
	if (bThreadsafe) {
		EnterCritical();
		CacheLineGetThreadSpecific_NCS(cache_line);
		int res = cache_line->thread_specific->iCurrentReadPos;
		LeaveCritical();
		return res;
	} else
		return cache_line->sts.iCurrentReadPos;
}

int CACHE::SetCacheLineCurrentReadPos(CACHE_LINE* cache_line, int position)
{
	if (bThreadsafe) {
		EnterCritical();
		CacheLineGetThreadSpecific_NCS(cache_line);
		cache_line->thread_specific->iCurrentReadPos = position;
		LeaveCritical();
	} else
		cache_line->sts.iCurrentReadPos = position;

	return 1;
}

int CACHE::IncCacheLineCurrentReadPos(CACHE_LINE* cache_line, int inc_val)
{
	if (bThreadsafe) {
		EnterCritical();
		CacheLineGetThreadSpecific_NCS(cache_line);
		cache_line->thread_specific->iCurrentReadPos += inc_val;
		LeaveCritical();
	} else
		cache_line->sts.iCurrentReadPos += inc_val;

	return 1;
}


int CACHE::GetCacheLineCurrentWritePos(CACHE_LINE* cache_line)
{
	if (bThreadsafe) {
		EnterCritical();
		CacheLineGetThreadSpecific_NCS(cache_line);
		int res = cache_line->thread_specific->iCurrentWritePos;
		LeaveCritical();
		return res;
	} else
		return cache_line->sts.iCurrentWritePos;
}

int CACHE::SetCacheLineCurrentWritePos(CACHE_LINE* cache_line, int position)
{
	if (bThreadsafe) {
		EnterCritical();
		CacheLineGetThreadSpecific_NCS(cache_line);
		cache_line->thread_specific->iCurrentWritePos = position;
		LeaveCritical();
	} else
		cache_line->sts.iCurrentWritePos = position;

	return 1;
}

int CACHE::IncCacheLineCurrentWritePos(CACHE_LINE* cache_line, int inc_val)
{
	if (bThreadsafe) {
		EnterCritical();
		CacheLineGetThreadSpecific_NCS(cache_line);
		cache_line->thread_specific->iCurrentWritePos += inc_val;
		LeaveCritical();
	} else
		cache_line->sts.iCurrentWritePos += inc_val;


	return 1;
}


int CACHE::UnlockCacheLine(int iCacheLineIndex)
{
	if (iCacheLineIndex < 0 || iCacheLineIndex >= iNumberOfCacheLines)
		return 0;

	cache_lines[iCacheLineIndex]->locked--;

	return 1;
}

bool CACHE::IsCacheLineLocked(int iCacheLineIndex)
{
	if (iCacheLineIndex < 0 || iCacheLineIndex >= iNumberOfCacheLines)
		return 0;

	return !!cache_lines[iCacheLineIndex]->locked;
}

bool CACHE::IsCacheLineLocked(CACHE_LINE* cache_line)
{
	return !!cache_line->locked;
}

bool CACHE::InitCache(int iBuffers, int iBytesPerBuffer, bool bPermanentGrow)
{
	if (iBuffers < 1 && iNumberOfCacheLines < 1)
		return false;

	if (iBuffers > MAX_CACHE_LINES)
		iBuffers = MAX_CACHE_LINES;

	if (iBytesPerBuffer == 0)
		return false;

	int j=iBytesPerBuffer;

	if (iInitialNumberOfCacheLines == 0 && iBytesPerBuffer <= 0)
		return false;

	if (j != -1) {
		int k=0;
		while (j>0) {
			k+=(j&1);
			j>>=1;
		}
		if (k != 1)
			return false;
	}

	int existing_cache_lines = iNumberOfCacheLines;

	if (iBytesPerBuffer == -1) {
		if (existing_cache_lines > 0)
			iBytesPerBuffer = cache_lines[0]->iSize;
		else 
			iBytesPerBuffer = 1<<19;
	} else {
		if (existing_cache_lines > 0)
			if (cache_lines[0]->iSize != iBytesPerBuffer) 
				return false;
	}

	if (existing_cache_lines <= 0) {
		cache_lines = (CACHE_LINE**)calloc(sizeof(CACHE_LINE*), MAX_CACHE_LINES/*iBuffers*/);
		iInitialNumberOfCacheLines = iBuffers;
	} else
		if (bPermanentGrow)
			iInitialNumberOfCacheLines = iBuffers;
	/* else
		if (iBuffers > iNumberOfCacheLines) 
			cache_lines = (CACHE_LINE**)realloc(cache_lines, sizeof(CACHE_LINE*)*iBuffers);
*/
	if (!cache_lines)
		return false;

	for (int i=existing_cache_lines; i< iBuffers; i++) {
		if (!AllocateCacheLine(&cache_lines[i], iBytesPerBuffer, 16384))
			return false;

		cache_lines[i]->bDirty = false;
		cache_lines[i]->bValid = false;
		cache_lines[i]->iSourceOffset = -1;
		cache_lines[i]->iSize = iBytesPerBuffer;
		cache_lines[i]->sts.iCurrentReadPos = -1;
		cache_lines[i]->sts.iCurrentWritePos = -1;
		cache_lines[i]->bBeingRead = false;
		cache_lines[i]->bBeingWritten = false;
		cache_lines[i]->iReadAccessCount = 0;
		cache_lines[i]->iUsedSize = 0;
		cache_lines[i]->overlapped.hEvent = NULL;
		cache_lines[i]->overlapped.Offset = 0;
		cache_lines[i]->overlapped.OffsetHigh = NULL;
		cache_lines[i]->iLastAccessTime = 0;
		cache_lines[i]->bWasGrown = false;
		cache_lines[i]->locked = 0;
		cache_lines[i]->dwLastThread = 0;
		free_cache_lines.push_back(i);
	}

	if (existing_cache_lines > iBuffers) {
		for (i = existing_cache_lines-1; i>=iBuffers; i--) {
			free(cache_lines[i]->pAllocated);
			cache_lines[i]->pAllocated = 0;
			cache_lines[i]->iSize = 0;
			cache_lines[i]->bValid = 0;
			cache_lines[i]->iSourceOffset = -INT_MAX;
			free(cache_lines[i]);
			cache_lines[i] = NULL;
		}
	}

/*	if (iBuffers < iNumberOfCacheLines) 
		cache_lines = (CACHE_LINE**)realloc(cache_lines, sizeof(CACHE_LINE*)*iBuffers);
*/
	iNumberOfCacheLines = iBuffers;
	iCacheLineSize = iBytesPerBuffer;

	return true;
}

CACHE::CACHE(int iBuffers, int iBytesPerBuffer)
{
	Init();
	InitCache(iBuffers, iBytesPerBuffer);
}

CACHE::~CACHE()
{
	Disable(CACHE_READ_AHEAD);

	Close();

	for (int i=0; i<iNumberOfCacheLines; i++) {
		/* complete open read requests -> success is obivously not necessary */
		if (cache_lines[i]->bBeingRead)
			WaitUntilReadyForRead(i, false);

		if (cache_lines[i]->pAllocated)
			free(cache_lines[i]->pAllocated);

		cache_lines[i]->pAllocated = NULL;
		cache_lines[i]->pData = NULL;
		cache_lines[i]->bValid = 0;
		delete cache_lines[i];
	}

	if (cache_lines)
		free(cache_lines);

	DeleteThreadSpecific();
}

bool CACHE::GetThreadSpecific()
{
	EnterCritical();

	DWORD dwCurrentThread = GetCurrentThreadId();
	if (dwCurrentThread == dwLastThread) {
		LeaveCritical();
		return true;
	}

	CACHE_THREAD_SPECIFIC_MAP::iterator iter = thread_specific_map.find(dwCurrentThread);
	if (iter != thread_specific_map.end()) {
		thread_specific = iter->second;
	} else {
		CACHE_THREAD_SPECIFIC* new_thread_specific = new CACHE_THREAD_SPECIFIC;
		new_thread_specific->iBytesLeftInCurrentReadCacheLine = -1;
		new_thread_specific->iBytesLeftInCurrentWriteCacheLine = -1;
		new_thread_specific->iLastAccessedCacheLineIndexRead = -1;
		new_thread_specific->iLastAccessedCacheLineIndexWrite = -1;
		new_thread_specific->iReadPosition = -1;
		new_thread_specific->iWritePosition = -1;
		thread_specific_map[dwCurrentThread] = new_thread_specific;
		thread_specific = new_thread_specific;
	}

	dwLastThread = dwCurrentThread;
	LeaveCritical();

	return true;
}

/* can be called from inside a function that is already in a critical section */
bool CACHE::GetThreadSpecific_NCS()
{
	DWORD dwCurrentThread = GetCurrentThreadId();
	if (dwCurrentThread == dwLastThread) {
		return true;
	}

	CACHE_THREAD_SPECIFIC_MAP::iterator iter = thread_specific_map.find(dwCurrentThread);
	if (iter != thread_specific_map.end()) {
		thread_specific = iter->second;
	} else {
		CACHE_THREAD_SPECIFIC* new_thread_specific = new CACHE_THREAD_SPECIFIC;
		new_thread_specific->iBytesLeftInCurrentReadCacheLine = -1;
		new_thread_specific->iBytesLeftInCurrentWriteCacheLine = -1;
		new_thread_specific->iLastAccessedCacheLineIndexRead = -1;
		new_thread_specific->iLastAccessedCacheLineIndexWrite = -1;
		new_thread_specific->iReadPosition = -1;
		new_thread_specific->iWritePosition = -1;
		thread_specific_map[dwCurrentThread] = new_thread_specific;
		thread_specific = new_thread_specific;
	}

	dwLastThread = dwCurrentThread;

	return true;
}

bool CACHE::DeleteThreadSpecific()
{
	CACHE_THREAD_SPECIFIC_MAP::iterator iter = thread_specific_map.begin();

	for (; iter != thread_specific_map.end(); iter++) 
		delete iter->second;

	thread_specific_map.clear();
	dwLastThread = 0;

	return true;
}

int CACHE::GetLastAccessedCacheLineIndex(bool read) 
{
	if (!bThreadsafe)
		return (read?sts.iLastAccessedCacheLineIndexRead:sts.iLastAccessedCacheLineIndexWrite);
	else {
		EnterCritical();
		GetThreadSpecific_NCS();
		int result = (read?thread_specific->iLastAccessedCacheLineIndexRead:thread_specific->iLastAccessedCacheLineIndexWrite);
		LeaveCritical();
		return result;
	}
}

__int64 CACHE::GetAccessPosition(bool read)
{
	if (!bThreadsafe)
		return (read?sts.iReadPosition:sts.iWritePosition)/*-GetOffset()*/;
	else {
		EnterCritical();
		GetThreadSpecific_NCS();
		__int64 result = (read?thread_specific->iReadPosition:thread_specific->iWritePosition);
		LeaveCritical();
		return result;
	}
}

int CACHE::GetBytesLeftInCurrentCacheLine(bool read)
{
	if (!bThreadsafe)
		return (read?sts.iBytesLeftInCurrentReadCacheLine:sts.iBytesLeftInCurrentWriteCacheLine);
	else {
		EnterCritical();
		GetThreadSpecific_NCS();
		int result = (read?thread_specific->iBytesLeftInCurrentReadCacheLine:thread_specific->iBytesLeftInCurrentWriteCacheLine);
		LeaveCritical();
		return result;
	}
}

bool CACHE::SetBytesLeftInCurrentCacheLine(bool read, int bytes, bool relative)
{
	if (!bThreadsafe) {
		int* i = (read?&sts.iBytesLeftInCurrentReadCacheLine:&sts.iBytesLeftInCurrentWriteCacheLine);
		if (relative)
			(*i) += bytes;
		else
			(*i) = bytes;
	} else {
		EnterCritical();
		GetThreadSpecific_NCS();
		int* i = (read?&thread_specific->iBytesLeftInCurrentReadCacheLine:&thread_specific->iBytesLeftInCurrentWriteCacheLine);
		if (relative)
			(*i) += bytes;
		else
			(*i) = bytes;

		LeaveCritical();
	}

	return true;
}

bool CACHE::SetAccessPosition(bool read, __int64 position, bool relative)
{
	if (!bThreadsafe) {
		__int64* i = (read?&sts.iReadPosition:&sts.iWritePosition);
		if (relative)
			(*i) += position;
		else
			(*i) = position;
	} else {
		EnterCritical();
		GetThreadSpecific_NCS();
		__int64* i = (read?&thread_specific->iReadPosition:&thread_specific->iWritePosition);
		if (relative)
			(*i) += position;
		else
			(*i) = position;

		LeaveCritical();
	}

	return true;
}

bool CACHE::SetLastAccessedCacheLineIndex(bool read, int bytes, bool relative)
{
	if (!bThreadsafe) {
		int* i=(read?&sts.iLastAccessedCacheLineIndexRead:&sts.iLastAccessedCacheLineIndexWrite);
		if (relative)
			(*i) += bytes;
		else
			(*i) = bytes;
	} else {
		EnterCritical();
		GetThreadSpecific_NCS();
		int* i=(read?&thread_specific->iLastAccessedCacheLineIndexRead:&thread_specific->iLastAccessedCacheLineIndexWrite);
		if (relative)
			(*i) += bytes;
		else
			(*i) = bytes;
		LeaveCritical();
	}

	return true;
}

int CACHE::Open(STREAM* stream, int mode)
{
	if (iAccessMode != -1)
		return CACHE_OPEN_ALREADY_OPEN;

	bAsynchWrite = false;
	bReadAhead = false;
//	hWriteQueueSemaphore = NULL;
	bImmedWriteback = false;

	_ASSERT(stream);
	if (!stream)
		return CACHE_OPEN_INVALID_STREAM;

	_ASSERT(stream->GetMode());
	if (!stream->GetMode())
		return CACHE_OPEN_INVALID_STREAM;

	if (iNumberOfCacheLines < 1)
		if (!InitCache(16, 1<<19))
			return false;

	bool bInvalidMode = ((mode & CACHE_OPEN_READ) || (mode & CACHE_OPEN_WRITE)) == false;
	_ASSERT(!bInvalidMode);
	if (bInvalidMode)
		return CACHE_OPEN_INVALID_MODE;

	bool bIncompatibleMode = (mode & CACHE_OPEN_READ) && !(stream->GetMode() & STREAM_READ) ||
							 (mode & CACHE_OPEN_WRITE) && (!(stream->GetMode() & STREAM_WRITE));// || 
//							 !(stream->GetMode() & STREAM_WRITE));
	_ASSERT(!bIncompatibleMode);
	if (bIncompatibleMode)
		return CACHE_OPEN_INCOMPATIBLE_MODE;

	if ((mode & CACHE_OPEN_ATTACH) == CACHE_OPEN_ATTACH)
		AttachSource();

	if (stream->GetMode() & STREAM_OVERLAPPED) {
		if (mode & CACHE_OPEN_WRITE) {
			Enable(CACHE_ASYNCH_WRITE); 	
//			hWriteQueueSemaphore = CreateSemaphore(NULL, 1, 1, NULL);
		}
		if (mode & CACHE_OPEN_READ)
			Enable(CACHE_READ_AHEAD);
	}

	SetSource(stream);
	iAccessMode = mode;

	iStreamSize = GetSource()->GetSize();
	sts.iReadPosition = -1;
	sts.iWritePosition = -1;
	iLastPossibleWritePos = max(0, iStreamSize);
	
	//hAccessMutex = CreateMutex(NULL, false, NULL); 
	InitializeCriticalSection(&critical_section);

//	Enable(CACHE_THREADSAFE);
	Enable(CACHE_CAN_GROW);

	dwLastThread = 0;
	GetThreadSpecific();

	Seek(0);

	return CACHE_OPEN_NO_ERROR;
}

int enter_count = 0;
int leave_count = 0;

bool CACHE::EnterCritical()
{
	enter_count++;
	if (bThreadsafe)
		EnterCriticalSection(&critical_section);

	return true;
}

bool CACHE::LeaveCritical()
{
	leave_count++;
	if (bThreadsafe) {
		_ASSERT(critical_section.LockCount >= 0);
		LeaveCriticalSection(&critical_section);
	}

	return true;
}

int CACHE::Close()
{
	if (iAccessMode == -1)
		return true;

	if (CanWrite()) {
		for (int i=0; i<iNumberOfCacheLines; i++) {
			if (!cache_lines[i]->bBeingWritten && cache_lines[i]->bDirty)
				WriteCacheLineToTargetStream(i);
			WaitUntilReadyForWrite(i);
		}

		GetSource()->TruncateAt(iStreamSize);
	}

	iAccessMode = -1;
	iStreamSize = 0;
	sts.iReadPosition = -1;

	STREAM_FILTER::Close();

//	SetSource(NULL);
	
	DeleteCriticalSection(&critical_section);

	return 0;
}

bool CACHE::IsPositionInCacheLine(__int64 iPosition, int iCacheLineIndex)
{
	if (iCacheLineIndex < 0 || iCacheLineIndex >= iNumberOfCacheLines)
		return false;

	CACHE_LINE* cl = cache_lines[iCacheLineIndex];

	if (!cl->bValid)
		return false;

	if (cl->iSourceOffset > iPosition || cl->iSourceOffset + cl->iSize <= iPosition)
		return false;

	return true;
}

int CACHE::FindCacheLine(__int64 iPosition, int iFirstLook)
{
	if (iFirstLook >= 0 && iFirstLook < iNumberOfCacheLines) {
		if (IsPositionInCacheLine(iPosition, iFirstLook))
			return iFirstLook;
	}

	if (iPosition >= iStreamSize && !CanWrite()) 
		return CACHE_FCL_NOT_FOUND;

	if (iPosition >= iLastPossibleWritePos && CanWrite())
		return CACHE_FCL_NOT_FOUND;

	for (int j=0;j<iNumberOfCacheLines;j++)
		if (IsPositionInCacheLine(iPosition, j))
			return j;

	return CACHE_FCL_NOT_FOUND;
}

bool CACHE::WriteCacheLineToTargetStream(int iCacheLineIndex)
{
	if (iCacheLineIndex < 0 || iCacheLineIndex >= iNumberOfCacheLines)
		return false;

	if (!cache_lines[iCacheLineIndex]->bDirty || cache_lines[iCacheLineIndex]->bBeingWritten)
		return true;

	if (!CanWrite())   // should not occur anyway
		return false;

	CACHE_LINE* cl = cache_lines[iCacheLineIndex];

	bool unbuffered = !!(GetSource()->GetMode() & STREAM_UNBUFFERED);
	bool overlapped = !!(GetSource()->GetMode() & STREAM_OVERLAPPED);
	int size = (unbuffered)?cache_lines[iCacheLineIndex]->iSize:cache_lines[iCacheLineIndex]->iUsedSize;
	
	if (overlapped) {
//		if (IsEnabled(CACHE_FORCE_SINGLE_ASYNCH_WRITE));
//			WaitForSingleObject(hWriteQueueSemaphore, INFINITE);

		cl->bBeingWritten = true;

		if (IsEnabled(CACHE_CREATE_LOG)) {
			char cTxt[256]; memset(cTxt, 0, sizeof(cTxt));
			sprintf(cTxt, "asynchronous write: %d bytes @ %12I64d \n", 
				size, cache_lines[iCacheLineIndex]->iSourceOffset);
			fprintf(fLog, cTxt);
		}

		GetSource()->Seek(cl->iSourceOffset);

		int write_result = GetSource()->WriteAsync(cl->pData, size,	&cl->overlapped);

		if (write_result == FILESTREAM_ASYNCH_IO_FAILED) {
			cl->bBeingWritten = false;
			return false;
		}

		if (write_result == FILESTREAM_ASYNCH_IO_FINISHED) {
			WaitUntilReadyForWrite(iCacheLineIndex);
		}

		if (!IsEnabled(CACHE_ASYNCH_WRITE)) {
			int wait = WaitUntilReadyForWrite(iCacheLineIndex);
			if (wait <= 0)
				return false;
		}

	} else {
		GetSource()->Seek(cache_lines[iCacheLineIndex]->iSourceOffset);
		if (GetSource()->Write(cache_lines[iCacheLineIndex]->pData, size) != size)
			return false;
	}

	cache_lines[iCacheLineIndex]->bDirty = false;

	return true;
}

bool CACHE::CanRead()
{
	if (iAccessMode < 0)
		return false;

	return !!(iAccessMode & CACHE_OPEN_READ);
}

bool CACHE::CanWrite()
{
	if (iAccessMode < 0)
		return false;

	return !!(iAccessMode & CACHE_OPEN_WRITE);
}

int CACHE::LoadSegmentIntoCacheLine(__int64 iPosition, int iCacheLineIndex)
{
	if (iCacheLineIndex < 0 || iCacheLineIndex >= iNumberOfCacheLines)
		return false;

	if (!WriteCacheLineToTargetStream(iCacheLineIndex))
		return CACHE_WRITEBACK_FAILURE;

	CACHE_LINE* cl = cache_lines[iCacheLineIndex];

	if (!cl->iSize) {
		MessageBoxA(NULL, "Interal Error", "Fatal", MB_OK);
	}

	cl->iSourceOffset = (iPosition / cl->iSize) * cl->iSize;
	GetSource()->Seek(cl->iSourceOffset);
	
	if (!bReadAhead) {
		if (GetSource()->GetMode() & STREAM_OVERLAPPED) {
			GetSource()->ReadAsync(cl->pData, cl->iSize, &cl->overlapped);
			cl->bValid = true;
			cl->bDirty = false;
			cl->bBeingRead = true;
			cl->bBeingWritten = false;
			int wait = WaitUntilReadyForRead(iCacheLineIndex, false);
			if (wait == CACHE_LOAD_FAILURE)
				return wait;
		} else {
			cl->iUsedSize = GetSource()->Read(cl->pData, cl->iSize);
			_ASSERT(cl->iUsedSize);
		}

		if (cl->iUsedSize <= 0)
			return false;

		cl->iLastAccessTime = GetTickCount();
		cl->bBeingRead = false;
		cl->bBeingWritten = false;
		cl->bDirty = false;
		cl->bValid = true;
		cl->iReadAccessCount = 0;
		cl->overlapped.hEvent = NULL;

		SetCacheLineCurrentReadPos(cl, (int)(iPosition - cl->iSourceOffset));

		return true;
	} else {
		DWORD dwWritten = 0;

		/* maybe return WRITEBACK_FAILURE here? */
		if (cl->bBeingWritten) {
			if (WaitUntilReadyForWrite(iCacheLineIndex) <= 0)
				return false;
		}

		cl->bBeingRead = true;
		cl->bValid = true;
		cl->bDirty = false;
		cl->bBeingWritten = false;
		cl->iReadAccessCount = 0;
		cl->iUsedSize = cl->iSize;
		
		if (GetSource()->ReadAsync(cl->pData, cl->iSize, &cl->overlapped) == FILESTREAM_ASYNCH_IO_FAILED)
			return CACHE_LOAD_FAILURE;

		cl->iLastAccessTime = GetTickCount();

		return true;
	}
}

bool CACHE::PrepareCacheLineForOverwrite(int iCacheLineIndex)
{
	if (WriteCacheLineToTargetStream(iCacheLineIndex)) {
		//cache_lines[iCacheLineIndex].iUsedSize = 0;

		/* is this necessary / does this make sense? */
		SetCacheLineCurrentReadPos(cache_lines[iCacheLineIndex], 0);

		return true;
	}

	return false;
}

int CACHE::FindCacheLineIndexToOverwrite()
{
	EnterCritical();

	int min_time = INT_MAX;
	int min_index = -1;

	/* find a cache line that is not being read, not being written, not locked,
	   and has not been accessed for the longest time */
	for (int j=0; j<iNumberOfCacheLines; j++) {
		if (min_time > cache_lines[j]->iLastAccessTime && 
			!cache_lines[j]->bBeingRead && 
			!cache_lines[j]->bBeingWritten &&
			!IsCacheLineLocked(j)) {
			min_time = cache_lines[j]->iLastAccessTime;
			min_index = j;
		}
	}

	/* if the cache line that has been found is dirty, write it back and then find
	   a cache line index to overwrite */
	if (min_index > -1 && cache_lines[min_index]->bDirty) {
		PrepareCacheLineForOverwrite(min_index);
		int result = FindCacheLineIndexToOverwrite();
		LeaveCritical();
		return result;
	}

	/* if cache has been grown */
	if (iInitialNumberOfCacheLines < iNumberOfCacheLines) {
		if (min_index == iNumberOfCacheLines - 1) {
			if (IsEnabled(CACHE_CREATE_LOG))
				fprintf(fLog, "shrinking cache...\n");
			
			CacheLineDeleteThreadSpecificMap(cache_lines[iNumberOfCacheLines - 1]);
			InitCache(iNumberOfCacheLines - 1, -1);
			int result = FindCacheLineIndexToOverwrite();
			LeaveCritical();
			return result;
		}
	}

	if (min_index < 0) {
		/* when here, all cache lines are being read, being written or locked */
		HANDLE	*h = (HANDLE*)malloc(sizeof(HANDLE)*iNumberOfCacheLines);
		DWORD*  cli = (DWORD*)malloc(sizeof(DWORD)*iNumberOfCacheLines);
		int handle_count = 0;

		for (j=0; j<iNumberOfCacheLines; j++) if (cache_lines[j]->bBeingWritten) {
			h[handle_count] = cache_lines[j]->overlapped.hEvent;
			cli[handle_count++] = j;
		}

		/* when cache lines are being written, wait for one of the writebacks
		   to complete */
		if (handle_count > 0) {
			int idx = -1;
			while (idx < WAIT_OBJECT_0 || idx > WAIT_OBJECT_0 + handle_count - 1)
				idx = WaitForMultipleObjectsEx(handle_count, h, false, INFINITE, true);

			idx -= WAIT_OBJECT_0;
			WaitUntilReadyForWrite(cli[idx]);
			free(h);
			free(cli);
			int result = FindCacheLineIndexToOverwrite();
			LeaveCritical();
			return result;
		} else {
			free(h);
			free(cli);

			/* when having arrived here, all cache lines are locked or being read,
			   thus the cache must either grow, wait for a read operation to complete,
			   or report a fatal error */

			if (IsEnabled(CACHE_CREATE_LOG)) {
				fprintf(fLog, "Must grow cache...\n");
/*#ifdef _T
				if (IsEnabled(CACHE_CAN_GROW))
					MessageBox(0, _T("Must grow cache"), _T("Cache:"), MB_OK);
				else
					MessageBox(0, _T("Must grow cache but can't"), _T("Cache:"), MB_OK | MB_ICONERROR);
#endif			
*/			}

			/* now check if a cache line that is marked being read has actually been
			   read completely and is only marked being read because WaitUntilReadyForRead
			   has not been called because the CACHE has not yet tried to access it */
			int cli = -1;
			for (int j=0; j<iNumberOfCacheLines; j++) {
				CACHE_LINE* cl = cache_lines[j];
				if (cl->bBeingRead && !IsCacheLineLocked(cl)) {
					cli = j;
					if (GetSource()->IsOverlappedIOComplete(&cl->overlapped)) {
						LockCacheLine(j);
						WaitUntilReadyForRead(j, false);
						UnlockCacheLine(j);
						LeaveCritical();
						return j;
					}
				}
			}

			/* all cache lines are either still really being read and/or locked */
			if (IsEnabled(CACHE_CAN_GROW) && iNumberOfCacheLines < MAX_CACHE_LINES) {
				InitCache(iNumberOfCacheLines + 1, -1);

				/* InitCache push_backs the new cache_line, but EnsureFreeCacheline
				   also does, thus remove the first one */
				free_cache_lines.pop_back();
				cache_lines[iNumberOfCacheLines-1]->bWasGrown = true;
				LeaveCritical();
				return iNumberOfCacheLines - 1;
			} else {
				if (cli > -1) {
					WaitUntilReadyForRead(cli, false);
					LeaveCritical();
					return cli;
				}

//				Sleep(500);
//				return FindCacheLineIndexToOverwrite();
//				_ASSERT(0);
				LeaveCritical();
				return -1;
			}
		}
	}

	if (bThreadsafe) {
		CacheLineDeleteThreadSpecificMap(cache_lines[min_index]);
//		cache_lines[min_index]->dwLastThread = 0;
	}

	LeaveCritical();

	return min_index;		
}

int CACHE::EnsureFreeCacheLine()
{
	int cache_line_index;

	if (!free_cache_lines.empty()) {
		while (free_cache_lines[0] >= iNumberOfCacheLines) 
			free_cache_lines.pop_front();
	}

	if (!free_cache_lines.empty())
		return true;

	cache_line_index = FindCacheLineIndexToOverwrite();
	
	if (cache_line_index < 0)
		return false;

//	PrepareCacheLineForOverwrite(cache_line_index);

	free_cache_lines.push_back(cache_line_index);

	/* if a last-accessed cache line was freed, it cannot be reused, so
	   the iLastAccessedCacheLinexxxx elements must be deleted */
	if (IsEnabled(CACHE_THREADSAFE)) {
		CACHE_THREAD_SPECIFIC_MAP::iterator iter;
		CACHE_THREAD_SPECIFIC* thread_specific;

		iter = thread_specific_map.begin();
		for (; iter != thread_specific_map.end(); iter++) {
			thread_specific = iter->second;
			if (thread_specific->iLastAccessedCacheLineIndexRead == cache_line_index ||
				thread_specific->iLastAccessedCacheLineIndexWrite == cache_line_index) {
					thread_specific->iLastAccessedCacheLineIndexRead = -1;
					thread_specific->iLastAccessedCacheLineIndexWrite = -1;
					thread_specific->iBytesLeftInCurrentReadCacheLine = -1;
					thread_specific->iBytesLeftInCurrentWriteCacheLine = -1;
			}
		}

	}

	return true;
}

int CACHE::UseFreeCacheLine()
{
	EnsureFreeCacheLine();

	if (free_cache_lines.empty())
		return CACHE_NO_FREE_CACHE_LINE;

	int index = free_cache_lines[0];
	free_cache_lines.pop_front();
	return index;
}

int CACHE::LoadSegment(__int64 iPosition)
{
	int free_cache_line_index = UseFreeCacheLine();
	if (free_cache_line_index == CACHE_NO_FREE_CACHE_LINE)
		return CACHE_NO_FREE_CACHE_LINE;

	int result = LoadSegmentIntoCacheLine(iPosition, free_cache_line_index);

	if (result == CACHE_WRITEBACK_FAILURE || result == CACHE_LOAD_FAILURE)
		return result;

	return true;
}

int CACHE::Read(void* pDest, DWORD dwBytes)
{
	BYTE*	pbDest = (BYTE*)pDest;

	EnterCritical();

	if (IsEndOfStream()) {
		LeaveCritical();
		return 0;
	}

	if (!dwBytes && GetBytesLeftInCurrentCacheLine(true)) {
		LeaveCritical();
		return 0;
	}

	if (dwBytes + GetAccessPosition(true) >= iStreamSize)
		dwBytes = (DWORD)(iStreamSize - GetAccessPosition(true));

	if (GetBytesLeftInCurrentCacheLine(true) > 0) {
		int last_index = GetLastAccessedCacheLineIndex(true);
		CACHE_LINE* cl = cache_lines[last_index];
		
/* Apr 11, 2006: read-ahead problem reported. When a preceding read-ahead failed,
                 it might be necessary to lock this one as well since a real 
				 read ahead could occur here under those circumstances */
		LockCacheLine(cl);
		if (WaitUntilReadyForRead(last_index, true) <= 0) {
			UnlockCacheLine(cl);
			LeaveCritical();
			return -1;
		}
		UnlockCacheLine(cl);
		/* this should actually not occur */
		if (!cl->iUsedSize) {
			LeaveCritical();
			return 0;
		}

		if (GetBytesLeftInCurrentCacheLine(true) >= (int)dwBytes) {

			LockCacheLine(cl);

			memcpy(pDest, cl->pData + GetCacheLineCurrentReadPos(cl), dwBytes);
			IncCacheLineCurrentReadPos(cl, dwBytes);
			SetBytesLeftInCurrentCacheLine(true, -(int)dwBytes, true);
			SetAccessPosition(true, dwBytes, true); 

			UnlockCacheLine(cl);

			LeaveCritical();
			return dwBytes;
		} else {
			int read = 0;
			DWORD total_read = 0;
		
			do {
				read = Read(&pbDest[total_read], 
					min((int)(dwBytes - total_read), GetBytesLeftInCurrentCacheLine(true)));

				if (read == CACHE_WRITEBACK_FAILURE || read == CACHE_LOAD_FAILURE) {
					LeaveCritical();
					return read;
				}

				total_read += read;

				if (read < 0) {
					LeaveCritical();
					return -1;
				}

				Read(NULL, 0);
			
			} while (total_read < dwBytes && read > 0);
			
			LeaveCritical();
			return total_read;
		}

	} else {
		CACHE_LINE* cl;
		int last_index = GetLastAccessedCacheLineIndex(true);
		int index = FindCacheLine(GetAccessPosition(true), last_index);
		
		if (index < 0) {
			int load = LoadSegment(GetAccessPosition(true));

			if (load <= 0) {
				LeaveCritical();
				return load;
			}

			index = FindCacheLine(GetAccessPosition(true), 
						GetLastAccessedCacheLineIndex(true));

			/* index == -1 here mains a fatal bug in this class */
			_ASSERT(index > -1);
		} 
	
		cl = cache_lines[index];
		SetCacheLineCurrentReadPos(cl, (int)(GetAccessPosition(true) - cl->iSourceOffset));
		
		/* must lock cache line for the following reason: When CACHE_READ_AHEAD is enabled
		   and the cache is accessed rather randomly, it can happen that all cache lines
		   but one are marked being read because only WaitUntilReadyForRead can mark such a
		   cache line as being completely read, which may not be called for some cache lines
		   when access is too random. When, in this situation, CACHE_READ_AHEAD is enabled,
		   the section in WaitUntilReadyForRead which prereads the next cache line could
		   select the cache line it is supposed to wait for as target for the prefetch operation. 
		*/
		LockCacheLine(index);
		int wait = WaitUntilReadyForRead(index, true);
		UnlockCacheLine(index);

		if (wait == CACHE_WRITEBACK_FAILURE || wait == CACHE_LOAD_FAILURE) {
			LeaveCritical();
			return wait;
		}

		SetBytesLeftInCurrentCacheLine(true, 
			cl->iUsedSize - GetCacheLineCurrentReadPos(cl), false);
		SetLastAccessedCacheLineIndex(true, index, false);
		//iLastAccessedCacheLineIndexRead = index;

		int r = Read(pDest, dwBytes);

		LeaveCritical();
		return r;
	}

	return -1; // alibi return, this is never executed
}

int CACHE::Write(void* pSrc, DWORD dwBytes)
{
	BYTE* pbSrc = (BYTE*)pSrc;

	_ASSERT(CanWrite());
	if (!CanWrite())
		return 0;

	if (!dwBytes && GetBytesLeftInCurrentCacheLine(false))
		return 0;

	if (GetBytesLeftInCurrentCacheLine(false) > 0) {

		int last_index = GetLastAccessedCacheLineIndex(false);
		CACHE_LINE* cl = cache_lines[last_index];
		WaitUntilReadyForWrite(last_index);

		if (GetBytesLeftInCurrentCacheLine(false) >= (int)dwBytes) {

			/* if this assertion fails, the CACHE class is broken somewhere */
			_ASSERT(GetCacheLineCurrentWritePos(cl) + dwBytes <= iCacheLineSize);

			memcpy(cl->pData + GetCacheLineCurrentWritePos(cl), pSrc, dwBytes);
			SetAccessPosition(false, dwBytes, true);
			IncCacheLineCurrentWritePos(cl, dwBytes);
			SetBytesLeftInCurrentCacheLine(false, -(int)dwBytes, true);
	
			if (GetAccessPosition(false) > iStreamSize)
				iStreamSize = cl->iSourceOffset + GetCacheLineCurrentWritePos(cl);

			if (GetCacheLineCurrentWritePos(cl) > cl->iUsedSize)
				cl->iUsedSize = GetCacheLineCurrentWritePos(cl);

			cl->bDirty = true;

			if (!GetBytesLeftInCurrentCacheLine(false) && IsEnabled(CACHE_IMMED_WRITEBACK))
				if (!WriteCacheLineToTargetStream(GetLastAccessedCacheLineIndex(false)))
					return CACHE_WRITEBACK_FAILURE;

			return dwBytes;
		} else {
			int written = 0;
			DWORD total_written = 0;

			do {
				written = Write(pbSrc + total_written, 
					min(GetBytesLeftInCurrentCacheLine(false), (int)(dwBytes - total_written)));

				if (written == CACHE_WRITEBACK_FAILURE || written == CACHE_LOAD_FAILURE)
					return written;

				total_written += written;

				Write(NULL, 0);
			} while (total_written < dwBytes && written);

			return total_written;
		}
	} else {
		CACHE_LINE* cl = NULL;
		int index = -1;

		int last_index = GetLastAccessedCacheLineIndex(false);
		if (GetAccessPosition(false) < iLastPossibleWritePos) {
			index = FindCacheLine(GetAccessPosition(false), last_index);
			if (index < 0) {
				int load = LoadSegment(GetAccessPosition(false));

/*				if (load == CACHE_WRITEBACK_FAILURE || load == CACHE_LOAD_FAILURE)
					return load;
*/
				if (load <= 0)
					return load;

				index = FindCacheLine(GetAccessPosition(false),
					GetLastAccessedCacheLineIndex(false));
			}

			cl = cache_lines[index];
			SetCacheLineCurrentWritePos(cl, (int)(GetAccessPosition(false) - cl->iSourceOffset));

			/* must lock cache line for same reason as in Read() */
			LockCacheLine(cl);
			WaitUntilReadyForWrite(index);
			UnlockCacheLine(cl);
			SetBytesLeftInCurrentCacheLine(false, cl->iSize - GetCacheLineCurrentWritePos(cl), false);
			SetLastAccessedCacheLineIndex(false, index, false);
			//iLastAccessedCacheLineIndexWrite = index;

			return Write(pSrc, dwBytes);
		} else {
			index = UseFreeCacheLine();

			cl = cache_lines[index];
			cl->iSourceOffset = (GetAccessPosition(false) / cl->iSize) * cl->iSize;
			SetCacheLineCurrentWritePos(cl, (int)(GetAccessPosition(false) - cl->iSourceOffset));
			iLastPossibleWritePos = cl->iSourceOffset + cl->iSize - 1;
			cl->bBeingWritten = false;
			cl->bBeingRead = false;
			cl->bDirty = false;
			cl->bValid = true;
			cl->iUsedSize = 0;

			return Write(pSrc, dwBytes);
		}
	}

	return 0;
}

__int64 CACHE::GetPos()
{
	if (CanWrite())
		return GetAccessPosition(false) - GetOffset();

	return GetAccessPosition(true) - GetOffset();
}

__int64 CACHE::GetSize()
{
	return iStreamSize - GetOffset();
}

int CACHE::Seek(__int64 iPosition)
{
	EnterCritical();

	iPosition += GetOffset();

	bool r = false;
	bool w = false;

	if (GetAccessPosition(true) == iPosition && !CanWrite()) {
		LeaveCritical();
		return true;
	}

	if (GetAccessPosition(false) == iPosition && !CanRead()) {
		LeaveCritical();
		return true;
	}

	if (IsPositionInCacheLine(iPosition, GetLastAccessedCacheLineIndex(true)) && CanRead()) {
		CACHE_LINE* cl = cache_lines[GetLastAccessedCacheLineIndex(true)];
		
		SetAccessPosition(true, iPosition, false);
		SetCacheLineCurrentReadPos(cl, (int)(GetAccessPosition(true) - cl->iSourceOffset));
		SetBytesLeftInCurrentCacheLine(true, cl->iUsedSize - GetCacheLineCurrentReadPos(cl), false);

		r = true;
	}

	if (IsPositionInCacheLine(iPosition, GetLastAccessedCacheLineIndex(false)) && CanWrite()) {
		CACHE_LINE* cl = cache_lines[GetLastAccessedCacheLineIndex(false)];
		
		SetAccessPosition(false, iPosition, false);
		SetCacheLineCurrentWritePos(cl, (int)(GetAccessPosition(false) - cl->iSourceOffset));
		SetBytesLeftInCurrentCacheLine(false, cl->iSize - GetCacheLineCurrentWritePos(cl), false);

		w = true;
	}

	if (CanRead() && !r) {
		SetBytesLeftInCurrentCacheLine(true, -1, false);
		SetAccessPosition(true, iPosition, false);
	}

	if (CanWrite() && !w) {
		SetBytesLeftInCurrentCacheLine(false, -1, false);
		SetAccessPosition(false, iPosition, false);
	}

	LeaveCritical();
	
	return true;
}

bool CACHE::IsEndOfStream()
{
/*	if (GetAccessPosition(true) + 5 >= iStreamSize) {
		printf("Pos: %I64d / %I64d\n", GetAccessPosition(true), iStreamSize);
//		MessageBox(0, "Test", "Test", MB_OK);
	}
	*/
	if (GetAccessPosition(true) >= iStreamSize)
		return true;

	return false;
}

/* todo:
      Check WaitUntilReadyForRead for correct behaviour when LoadSegment
	  returns CACHE_WRITEBACK_FAILURE
*/
int CACHE::WaitUntilReadyForRead(int iCacheLineIndex, bool bSeriousAccess)
{
	if (iCacheLineIndex < 0 || iCacheLineIndex >= iNumberOfCacheLines)
		return false;

	if (!CanRead())
		return false;
		
	CACHE_LINE* cl = cache_lines[iCacheLineIndex];

	if (!cl->bValid)
		return false;

	cl->iReadAccessCount++;
	cl->iLastAccessTime = GetTickCount();

	/* must not wait for operation to be completed from two different threads,
	   thus, the check for bBeingRead must be mutexed*/
	EnterCritical();

	if (!cl->bBeingRead) {
		LeaveCritical();
		return true;
	}

	GetSource()->WaitForAsyncIOCompletion(&cl->overlapped, (DWORD*)&cl->iUsedSize);
	CloseHandle(cl->overlapped.hEvent);
	cl->bBeingRead = false;
	
	LeaveCritical();

	if (!cl->iUsedSize)
		return CACHE_LOAD_FAILURE;

	/* read-ahead is mutexed */
	EnterCritical();

	if ((GetSource()->GetMode() & STREAM_OVERLAPPED) && !IsEnabled(CACHE_THREADSAFE)) {

		int preread_count = iPrereadCacheLines;
			
		if (preread_count > iNumberOfCacheLines / 4)
			preread_count = iNumberOfCacheLines / 4;
		
		if (!bSeriousAccess)
			preread_count = 0;
		
		//
/*		if (cache_lines[0]->iSize * preread_count > 1<<22) {
			preread_count = (1<<22) / cache_lines[0]->iSize; 
		}
*/
		for (int j = 1; j <= preread_count; j++) {

			if (bReadAhead && cl->iSourceOffset + j*cl->iSize < GetSize() && iNumberOfCacheLines > 3) {
				int index = FindCacheLine(cl->iSourceOffset + j*cl->iSize, -1);
				if (index < 0) {
				//	bool bGrow = IsEnabled(CACHE_CAN_GROW);
				//	Disable(CACHE_CAN_GROW);
					int res = LoadSegment(cl->iSourceOffset + j*cl->iSize);
				//	SetFlag(CACHE_CAN_GROW, bGrow);
					if (res == CACHE_NO_FREE_CACHE_LINE)
						j = preread_count;
				}
			}
		}
	}

	LeaveCritical();

	return true;
}

int CACHE::WaitUntilReadyForWrite(int iCacheLineIndex)
{
	// finish pending read operation, however, success is not necessary
	WaitUntilReadyForRead(iCacheLineIndex, false);

	CACHE_LINE* cl = cache_lines[iCacheLineIndex];

	if (!CanWrite())
		return false;

	if (!cl->bBeingWritten)
		return true;

	DWORD dwWritten;
	GetSource()->WaitForAsyncIOCompletion(&cl->overlapped, &dwWritten);
	CloseHandle(cl->overlapped.hEvent);

	bool bSuccess = (GetSource()->GetMode() & STREAM_UNBUFFERED) && dwWritten == cl->iSize || dwWritten == cl->iUsedSize; 
	_ASSERT(bSuccess);

	cl->bBeingWritten = false;

	if (!bSuccess) {
		cl->bDirty = true;
		return CACHE_WRITEBACK_FAILURE;
	}

	return true;
}

int CACHE::GetGranularity()
{
	return 1;
}

bool CACHE::_Enable(int flag, bool set, bool enable)
{
	bool* f;

	switch (flag) {
		case CACHE_READ_AHEAD: f = &bReadAhead; break;
		case CACHE_ASYNCH_WRITE: f = &bAsynchWrite; break;
		case CACHE_CAN_GROW: f = &bCanGrow; break;
		case CACHE_IMMED_WRITEBACK: f = &bImmedWriteback; break;
		case CACHE_CREATE_LOG: f = &bLog; break;
		case CACHE_THREADSAFE: f = &bThreadsafe; break;
		default: f=NULL;
	}

	_ASSERT(f);

	if (!f)
		return false;

	bool b = *f;
	
	bool copy_status = false;
	if (flag == CACHE_THREADSAFE && !b && enable && set) {
		copy_status = true;
	}

	if (set)
		*f = enable;

	EnterCritical();
	if (copy_status) {
		GetThreadSpecific_NCS();
		memcpy(thread_specific, &sts, sizeof(sts));
		for (int j=0;j<iNumberOfCacheLines;j++) {
			CACHE_LINE* cl = cache_lines[j];
			CacheLineGetThreadSpecific_NCS(cl);
			memcpy(cl->thread_specific, &cl->sts, sizeof(cl->sts));
		}
	}
	LeaveCritical();

	return b;
}

bool CACHE::SetPrereadRange(int range)
{
	iPrereadCacheLines = range;
	
	return true;
}

