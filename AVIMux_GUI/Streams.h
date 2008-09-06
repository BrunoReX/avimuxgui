#ifndef f_STREAM_I
#define f_STREAM_I

/*

MEMORYSTREAM: maps a file to memory
AVISTREAM: maps a stream of an AVI file to a plain file source
AVILISTSTREAM: maps a stream of an AVI-List to a plain file source

 */
/*
#include "..\BaseStreams.h"
//#include "AVIFile.h"
#include "..\Cache.h"

// cached eine ganze Datei in den RAM

typedef struct
{
	bool			bOpened;
	DWORD			dwNbrOfLCs;
	DWORD			dwLastWriteDest;
	__int64		qwPosition;
} MEMORYSTREAM_STATUS;
class MEMORYSTREAM : public STREAM
{
	private:
		MEMORYSTREAM_STATUS	status;
		__int64		qwSize;
		DWORD			dwMode;
		LINEARCACHE**	lplpCache;
		int			virtual ReadPart(void* lpDest,DWORD dwBytes);
		int			virtual WritePart(void* lpSource,DWORD dwBytes);
	public:
		MEMORYSTREAM(void);
		~MEMORYSTREAM(void);
		int					Open(CFileStream* lpSource, DWORD dwMode);
		int			virtual Close(void);
		int			virtual Seek(__int64 qwPos);
		__int64		virtual GetPos(void);
		__int64		virtual GetSize(void);
		int			virtual Read(void* lpDest,DWORD dwBytes);
		int			virtual Write(void* lpSource,DWORD dwBytes);
		bool		virtual IsEndOfStream(void);
		int			virtual GetGranularity(void) { return 1; }
};

*/
#endif