/*

LINEARCACHE		one segment of a larger cache
CACHEDSTREAM	stream which uses intelligent caching to
                compensate for stupid M$ Windows

*/

#ifndef F_CACHE_I
#define F_CACHE_I

#include "windows.h"
#include "basestreams.h"

// kleiner Cache, der nur 1 Teilstück enthält
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
		~LINEARCACHE(void);
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

typedef struct 
{
	__int64		qwStart;
	DWORD		dwSysTime;
	bool		bDirty;
	bool		bLoading;
	DWORD		dwMaxUsedSize;
	READ_ASYNC_STRUCT* pRAS;
} LINEARCACHEINFO;

typedef struct 
{
	int			iHits;
	int			iMisses;
} CACHESTATS;

const int CACHE_READ  = 0x01;
const int CACHE_WRITE = 0x02;

void  cacheAllowReadAhead(bool bAllow);

class CACHEDSTREAM : public STREAM
{
	private:
		union {
			STREAM*			lpSource;
			STREAM*			lpDest;
		};
		STREAM*			GetSource();
		STREAM*			GetDest();
		__int64			qwPos;
		LINEARCACHE**	lplpCache;
		LINEARCACHEINFO* lpCacheInfo;

		DWORD			dwNbrOfBuffers;
		DWORD			dwBytesPerBuffer;
		bool			bReadAhead;

		int				iAccess;
		__int64			iLastPosWritten;
		int				iModVal;
		int				iBufferBits;
		int				iLastBufferInUse;
		__int64			iLastBufferBegin;
		__int64			iLastBufferEnd;
		CACHESTATS		stats;

		bool				IsReading();

		bool				AllBuffersUsed(DWORD* lpdwFree);
		void				DeleteOldestBuffer(DWORD* lpdwFree);
		bool				IsBufferUsed(DWORD dwNbr);
		bool				IsInBuffer(DWORD dwNbr,__int64 _qwPos,DWORD* lpdwHowMany);
		bool				FindBuffer(__int64 _qwPos,DWORD* lpdwNbr,DWORD* lpdwHowMany,DWORD* lpdwOffset);
		DWORD				GetBytesPerBuffer(void);
		int					GetNbrOfBuffers(void);
		int					LoadSegment(DWORD dwNbr,__int64 qwStart);
		int					WriteCachelineBack(DWORD dwNbr);
		int					PrepareForWriting(__int64 qwPos, DWORD* lpdwNbr,DWORD* lpdwHowMany,DWORD* lpdwOffset);
		int					LastBlockRead();
	public:
		CACHEDSTREAM(DWORD dwBuffers,DWORD dwBytesPerBuffer);
		~CACHEDSTREAM(void);
		int					Open (STREAM* lpSource, int mode = CACHE_READ);
		int			virtual	Close(void);
		void		virtual EnableReadAhead(bool bEnable = true);
		int			virtual	Seek(__int64 qwPos);
		__int64		virtual GetPos(void);
		__int64		virtual GetSize(void);
		int			virtual	Read(void* lpDest,DWORD dwBytes);
		int			virtual	Write(void* lpSrc,DWORD dwBytes);
		bool		virtual IsEndOfStream(void);
		int			virtual InvalidateCache(void);
		int			virtual GetGranularity(void) { return 1; }
		int			virtual	GetCacheStats(CACHESTATS* lpstats);
		int			virtual GetOffset();
		int			virtual SetOffset(int iNewOffset);
};



#endif