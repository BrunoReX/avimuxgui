/*

STREAM:		abstract  
CFileStream  file
BITSTREAM   access to a stream bit by bit  
	
*/

#ifndef I_BASESTREAMS
#define I_BASESTREAMS

#define STREAM_OK		0x01
#define STREAM_ERR		-0x010000

#define STREAM_READ					0x01
#define STREAM_WRITE				0x02
#define STREAM_UNBUFFERED			0x04
#define STREAM_UNBUFFERED_WRITE     (STREAM_WRITE | STREAM_UNBUFFERED)
#define STREAM_WRITE_OPEN_EXISTING  0x08
#define STREAM_OVERLAPPED			0x10
#define STREAM_THREADED				0x20

#include "windows.h"

const int READF_ASYNC = 0x01;

// sowas wie abstrakter Basistyp für alle Streams
class STREAM
{
	private:
		DWORD		dwMode;
		int			iOffset;
		bool		bAttached;
	protected:
		/* flag modification */
		bool		virtual _Enable(int flag, bool set, bool enable) { return false; };
	public:
		STREAM() { iOffset=0; dwMode = 0; bAttached = false; };
		virtual ~STREAM() {};

		/* flag modification */
		bool		Enable(int flag);
		bool		Disable(int flag);
		bool		IsEnabled(int flag);
		bool		SetFlag(int flag, int value);

		int					Open(DWORD _dwMode) { dwMode=_dwMode; return (STREAM_ERR); }
		int			virtual	Read(void* lpDest,DWORD dwBytes) { return (STREAM_ERR); }
		int			virtual WaitForAsyncIOCompletion(OVERLAPPED* overlapped, DWORD* pdwBytesTransferred) { return STREAM_ERR; };
		int			virtual IsOverlappedIOComplete(OVERLAPPED* overlapped) { return false; };
		void		virtual AttachSource() { bAttached = true; }; 
		bool		virtual IsSourceAttached() { return bAttached; };
		int			virtual	ReadAsync(void* pDest, DWORD dwBytes, OVERLAPPED* overlapped) { return (STREAM_ERR); }
		int			virtual	WriteAsync(void* pDest, DWORD dwBytes, OVERLAPPED* overlapped) { return (STREAM_ERR); }

		int			virtual Write(void* lpSource,DWORD dwBytes) { return (STREAM_ERR); }
		int			virtual WriteString(void* lpSource) { return Write(lpSource, (DWORD)strlen((char*)lpSource)); }
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

class STREAM_FILTER: public STREAM
{
	private:
		STREAM*		source;
	protected:
		STREAM*		GetSource();
	public:
		STREAM_FILTER();
		virtual ~STREAM_FILTER();
		void		SetSource(STREAM* s);
		
		int Close();
};




__int64 round(double x);

bool SetFilePointer64 (HANDLE hFile,_int64 qwPos);
bool GetFileSize64 (HANDLE hFile,_int64* qwSize);

#endif