#ifndef F_CACHE_I
#define F_CACHE_I

#include "basestreams.h"

// legacy stuff needed by vorbis audio source in AVI-Mux GUI
class LINEARCACHE: public STREAM
{
	private:
		DWORD	dwSize;
		DWORD	dwPosition;
		BYTE*	lpbData;
		BYTE*	lpbAllocatedData;
		int		align;
	public:
		LINEARCACHE(void);
		virtual ~LINEARCACHE(void);
		int		Clear();
		__int64	GetSize(void);
		BYTE*	GetData(void);
		int		Read(void* lpDest,DWORD dwBytes);
		int		Write(void* lpSource,DWORD dwBytes);
		bool	IsEndOfCache(void);
		__int64 GetPos();
		bool	IsValid(void);
		bool	SetSize(DWORD dwSize,bool bCreate=true);
		void	SetData(void* lpSource);
		int		virtual Seek(__int64 qwPos);
};

#include <deque>
#include <map>
/* new, less b0rked cache class */

const int CACHE_OPEN_INVALID_MODE = -0x02;
const int CACHE_OPEN_INCOMPATIBLE_MODE = -0x03;
const int CACHE_OPEN_INVALID_STREAM = -0x04;
const int CACHE_OPEN_ALREADY_OPEN = -0x05;
const int CACHE_OPEN_NO_ERROR     = 0x01;
const int CACHE_OPEN_ATTACH       = 0x20;
const int CACHE_OPEN_READ         = 0x40;
const int CACHE_OPEN_WRITE        = 0x80;

const int CACHE_WRITEBACK_FAILURE = -0x05;
const int CACHE_LOAD_FAILURE      = -0x08;
const int CACHE_NO_FREE_CACHE_LINE= -0x0D;

const int CACHE_READ_AHEAD        = 0x17;
const int CACHE_ASYNCH_WRITE      = 0x31;
//const int CACHE_FORCE_SINGLE_ASYNCH_WRITE = 0x23; // not yet implemented
const int CACHE_CAN_GROW          = 0x43;
const int CACHE_IMMED_WRITEBACK   = 0x82;
const int CACHE_CREATE_LOG        = 0x67;
const int CACHE_THREADSAFE		  = 0x9B;

const int CACHE_FCL_NOT_FOUND     = -0x01;

/* Flags

        bValid:         SET when   a cache line has already been used 
        bBeingRead:     SET when   an asynchronous read operation into this cache line is
		                being performed
		bBeingWritten:  SET when   an asynchronous write operation from this cache line into
		                the underlying file is being performed
		bDirty:         SET when   a write operation to a cache line has been performed after
		                that cache line has been loaded from or written to the underlying file

*/

typedef struct
{
	int		iCurrentReadPos;
	int		iCurrentWritePos;
} CACHE_LINE_THREAD_SPECIFIC;

typedef std::map<DWORD, CACHE_LINE_THREAD_SPECIFIC*> CACHE_LINE_THREAD_SPECIFIC_MAP;

typedef struct
{
	unsigned char*	pData;
	unsigned char*  pAllocated;
	
	int				iSize;
	int				iUsedSize;
	
	CACHE_LINE_THREAD_SPECIFIC sts;
// thread specific
	CACHE_LINE_THREAD_SPECIFIC_MAP thread_specific_map;
	DWORD			dwLastThread;
	CACHE_LINE_THREAD_SPECIFIC*	thread_specific;

	__int64			iSourceOffset;
	bool			bValid;
	bool			bBeingRead;
	bool			bBeingWritten;
	bool			bDirty;
	bool			bWasGrown;

	OVERLAPPED		overlapped;

	int				iReadAccessCount;
	int				iLastAccessTime;
	int				locked;
} CACHE_LINE;

bool operator <(CACHE_LINE& one, CACHE_LINE& other);

typedef struct {
	int				iLastAccessedCacheLineIndexRead;
	int				iLastAccessedCacheLineIndexWrite;
	int				iBytesLeftInCurrentReadCacheLine;
	int				iBytesLeftInCurrentWriteCacheLine;
	__int64			iReadPosition;
	__int64			iWritePosition;
} CACHE_THREAD_SPECIFIC;

typedef std::map<DWORD, CACHE_THREAD_SPECIFIC*> CACHE_THREAD_SPECIFIC_MAP;

#define CCache CACHE

class CACHE : public STREAM_FILTER
{
private:
	int				iNumberOfCacheLines;
	int				iPrereadCacheLines;
	int				iInitialNumberOfCacheLines;
	int				iCacheLineSize;
	int				iAccessMode;

	CACHE_THREAD_SPECIFIC	sts;
	DWORD			dwLastThread;
	CACHE_THREAD_SPECIFIC_MAP	thread_specific_map;
	CACHE_THREAD_SPECIFIC*	thread_specific;

	__int64			iLastPossibleWritePos;
	__int64			iStreamSize;
	CRITICAL_SECTION critical_section;

	bool			bReadAhead;
	bool			bAsynchWrite;
	bool			bSingleAsynchWrite;
	bool			bCanGrow;
	bool			bImmedWriteback;
	bool			bLog;
	bool			bThreadsafe;
	FILE*			fLog;

	std::deque<int>	free_cache_lines;
	CACHE_LINE**	cache_lines;

	bool			AllocateCacheLine(CACHE_LINE** cache_line, int size, int alignment);

	bool			IsPositionInCacheLine(__int64 position, int cache_line_index);
	int				FindCacheLine(__int64 position, int first_look);
	int				FindCacheLineIndexToOverwrite();
	int				LoadSegmentIntoCacheLine(__int64 position, int cache_line_index);
	int				EnsureFreeCacheLine();
	int				UseFreeCacheLine();
	bool			WriteCacheLineToTargetStream(int cache_line_index);
	bool			PrepareCacheLineForOverwrite(int cache_line_index);
	int				WaitUntilReadyForRead(int cache_line_index, bool bSeriousAccess);
	int				WaitUntilReadyForWrite(int cache_line_index);
	int				LoadSegment(__int64 position);
	
	int				LockCacheLine(int cache_line_index);
	int				LockCacheLine(CACHE_LINE* cache_line);
	int				UnlockCacheLine(int cache_line_index);
	int				UnlockCacheLine(CACHE_LINE* cache_line);
	bool			IsCacheLineLocked(int cache_line_index);
	bool			IsCacheLineLocked(CACHE_LINE* cache_line);

	bool			CacheLineGetThreadSpecific(CACHE_LINE* cache_line);
	bool			CacheLineGetThreadSpecific_NCS(CACHE_LINE* cache_line);

	bool			CacheLineDeleteThreadSpecificMap(CACHE_LINE* cache_line);
	int				GetCacheLineCurrentReadPos(CACHE_LINE* cache_line);
	int				SetCacheLineCurrentReadPos(CACHE_LINE* cache_line, int position);
	int				IncCacheLineCurrentReadPos(CACHE_LINE* cache_line, int inc_val);
	int				GetCacheLineCurrentWritePos(CACHE_LINE* cache_line);
	int				SetCacheLineCurrentWritePos(CACHE_LINE* cache_line, int position);
	int				IncCacheLineCurrentWritePos(CACHE_LINE* cache_line, int inc_val);

	bool			GetThreadSpecific();
	bool			GetThreadSpecific_NCS();
	bool			DeleteThreadSpecific();
	int				GetLastAccessedCacheLineIndex(bool read);
	__int64			GetAccessPosition(bool read);
	int				GetBytesLeftInCurrentCacheLine(bool read);
	bool			SetBytesLeftInCurrentCacheLine(bool read, int bytes, bool relative);
	bool			SetAccessPosition(bool read, __int64 position, bool relative);
	bool			SetLastAccessedCacheLineIndex(bool read, int index, bool relative);

	bool			CanRead();
	bool			CanWrite();
	void			Init();
protected:
	bool virtual	_Enable(int flag, bool set, bool enable);
	bool			EnterCritical();
	bool			LeaveCritical();
public:
	CACHE();
	CACHE(int iBuffers, int iBytesPerBuffer);
	bool			InitCache(int iBuffers, int iBytesPerBuffer, bool bPermanentGrow = false);
	bool			SetPrereadRange(int range);
	virtual ~CACHE();

	int				Open(STREAM* stream, int mode);
	int				Close(void);
	int				Read(void* pDest, DWORD dwBytes);
	int				Write(void* pSrc, DWORD dwBytes);
	int				Seek(__int64 position);
	bool			IsEndOfStream();
	__int64			GetPos();
	__int64			GetSize();
	int				GetGranularity();
};

#endif