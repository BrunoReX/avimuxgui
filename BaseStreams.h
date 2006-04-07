/*

STREAM:		abstract  
FILESTREAM  file
BITSTREAM   access to a stream bit by bit  
	
*/

#ifndef I_BASESTREAMS
#define I_BASESTREAMS

#define STREAM_OK		0x01
#define STREAM_ERR		-0x01

#define STREAM_READ					0x01
#define STREAM_WRITE				0x02
#define STREAM_UNBUFFERED			0x04
#define STREAM_UNBUFFERED_WRITE     (STREAM_WRITE | STREAM_UNBUFFERED)
#define STREAM_WRITE_OPEN_EXISTING  0x08
#define STREAM_OVERLAPPED			0x10

#include "windows.h"
#include "stdlib.h"

const int READF_ASYNC = 0x01;


typedef struct
{
	void*			lpDest;
	HANDLE			hSemaphore;
	__int64			iStart;
	OVERLAPPED*		pOverlapped;
} READ_ASYNC_STRUCT;

// sowas wie abstrakter Basistyp für alle Streams
class STREAM
{
	private:
		DWORD		dwMode;
		int			iOffset;
	public:
		STREAM() { iOffset=0; };
		~STREAM() {};
		int					Open(DWORD _dwMode) { dwMode=_dwMode; return (STREAM_ERR); }
		int			virtual	Read(void* lpDest,DWORD dwBytes) { return (STREAM_ERR); }
		int			virtual	ReadAsync(READ_ASYNC_STRUCT* pRAS,DWORD dwBytes);
		int			virtual WaitForCompletion(READ_ASYNC_STRUCT* pRAS) { return (STREAM_ERR); }
		int			virtual Write(void* lpSource,DWORD dwBytes) { return (STREAM_ERR); }
		__int64		virtual	GetPos(void) { return (STREAM_ERR); }
		__int64		virtual	GetSize(void) { return (STREAM_ERR); }
		int			virtual	Close(void) { return (STREAM_ERR); }
		int			virtual	Seek(__int64 qwPos) { return (STREAM_ERR); }
		bool		virtual IsEndOfStream(void) { return (true); }
		int			virtual	GetName(char* lpcName) { if (lpcName) *lpcName=0; return false; }
		int			virtual	SetName(char*) { return false; }
		int			virtual GetAvgBytesPerSec(void) { return 0; }
		int			virtual GetGranularity(void) { return 0; }
		int			virtual GetFrequency(void) { return 0; }
		int			virtual GetChannels(void) { return 0; }
		int			virtual InvalidateCache(void) { return 0; }
		int			virtual GetMode(void) { return dwMode; }
		int			virtual	GetOffset();
		int			virtual	SetOffset(int iNewOffset);
		int			virtual TruncateAt(__int64 iPosition);
};

// generally disable buffered read if necessary (e.g. in case of bugs)
void  filestreamAllowBufferedRead(bool bAllow = true);

class FILESTREAM : public STREAM
{
	private:
		HANDLE		hFile;
		__int64		iCurrentSize;
		char*		cOutCache;
		char*		cOutCacheCurr;
		char		cWriteSemaphoreName[10];
		__int64		iOutCachePos;
		__int64		iFilesize;
		__int64		iCurrPos;
		HANDLE		hWriteSemaphore;
		bool		bBuffered;
		bool		bOverlapped;
		void*		pAlignedInputBufferAllocated;
		void*		pAlignedInputBuffer;
		int			iAlignedBufferSize;
		char*		cFilename;
		int			align;
		int			iPossibleAlignedReadCount;
		bool		bDenyUnbufferedRead;
		bool		bCanRead;
		bool		bCanWrite;

	protected:
		void		virtual Flush();
		int			virtual Write2Disk(void* lpSource,DWORD dwBytes);
//		int			virtual	read(void* lpDest,DWORD dwBytes);
//		int			virtual read(READ_ASYNC_STRUCT* pRAS, DWORD dwBytes);
	public:
		FILESTREAM(void);
		~FILESTREAM(void);
		int			virtual	Open (char* lpFileName,DWORD _dwMode);
		int					SethFile (HANDLE _hFile);
		HANDLE				GethFile (void) { return hFile; }
		int			virtual	Close(void);
		int			virtual	Seek(__int64 qwPos);
		__int64		virtual GetPos(void);
		__int64		virtual GetSize(void);
		int			virtual	Read(void* lpDest,DWORD dwBytes);
		int			virtual	ReadAsync(READ_ASYNC_STRUCT* pRAS,DWORD dwBytes);
		int			virtual WaitForCompletion(READ_ASYNC_STRUCT* pRAS);
		int			virtual Write(void* lpSource,DWORD dwBytes);
		bool		virtual IsEndOfStream(void);
		int			virtual GetGranularity(void) { return 1; }
		int			virtual TruncateAt(__int64 iPosition);
};

class BITSTREAM 
{
	private:
		STREAM*		source;
		DWORD		dwCurrBitPos;
		void		LoadWord(void);
		WORD		wData;
		int			ReadBit(int iFlag = 0);

	public:
		BITSTREAM(void)	{ source=NULL; dwCurrBitPos=0; }
		~BITSTREAM(void) {};
		STREAM*			GetSource() { return source; }
		int		virtual	Open(STREAM* lpStream);
		int		virtual Close(void) { source=NULL; return STREAM_OK; }
		bool	virtual	IsEndOfStream(void) { return (source->IsEndOfStream()&&(dwCurrBitPos==15)); }
		int				Seek(__int64	qwPos);
		int				ReadBits(int n, int iFlag = 0);
		__int64			ReadBits64(int n, int iFlag = 0);
		__int64			GetPos();



};

__int64 round(double x);

bool SetFilePointer64 (HANDLE hFile,_int64 qwPos);
bool GetFileSize64 (HANDLE hFile,_int64* qwSize);

#endif