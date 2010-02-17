/*

STREAM:		abstract  
CFileStream  file
BITSTREAM   access to a stream bit by bit  
	
*/

#ifndef I_BASESTREAMS
#define I_BASESTREAMS

#include <vector>

/** \def STREAM_OK
 * \brief No error
 */
#define STREAM_OK		0x01
/** \def STREAM_ERR
 * \brief An unknown or undefined error.
 */
#define STREAM_ERR		-0x010000

class StreamMode
{
public:
	/** \brief Enumerates modes supported for streams
	 *
	 * This enumeration enumerates possible modes for streams. For maximum performance with
	 * very large input files that are read almost sequentially or completely sequentially,
	 * use \a Overlapped and \a Unbuffered in connection with the \a CCache class.
	 */
	enum StreamModes {
		/**
		 * No mode is given
		 */
		None = 0,

		/**
		 * \brief Stream is readable
		 */
		Read = 0x01,

		/**
		 * \brief Stream is writeable
		 */
		Write = 0x02,

		/** 
		 * \brief No file system buffering will be used
		 */
		Unbuffered = 0x04,

		/** 
		 * \brief Open a file for unbuffered reading.
		 */
		UnbufferedRead = Unbuffered | Read,

		/** 
		 * \brief Open a file for unbuffered writing.
		 */
		UnbufferedWrite = Unbuffered | Write,

		/** 
		 * \brief Open an existing file for writing
		 */
		WriteOpenExisting = 0x08,

		/** 
		 * \brief Use overlapped output
		 *
		 * Overlapped I/O means asynchronous I/O without using a worker thread in the
		 * application itself; the asynchronousness is provided by the OS.
		 */
		Overlapped = 0x10,

		/** 
		 * \brief Use a worker thread; this flag cannot be used with \a Overlapped
		 */
		Threaded = 0x20
	};
};

StreamMode::StreamModes operator| (StreamMode::StreamModes first, StreamMode::StreamModes second);
StreamMode::StreamModes operator~ (StreamMode::StreamModes first);
StreamMode::StreamModes operator& (StreamMode::StreamModes first, StreamMode::StreamModes second);
StreamMode::StreamModes& operator|= (StreamMode::StreamModes& first, StreamMode::StreamModes second);
StreamMode::StreamModes& operator&= (StreamMode::StreamModes& first, StreamMode::StreamModes second);


/** \def STREAM_MODE_UNBUFFERED
 * \brief Stream is used in unbuffered mode.
 */
//#define STREAM_MODE_UNBUFFERED			0x04

/** \def STREAM_MODE_UNBUFFERED_READ
 * \brief Stream is used in unbuffered read mode.
 */
//#define STREAM_MODE_UNBUFFERED_READ      (STREAM_READ | STREAM_UNBUFFERED)

/** \def STREAM_MODE_UNBUFFERED_WRITE
 * \brief Stream is used in unbuffered mode and is writeable.
 */
//#define STREAM_MODE_UNBUFFERED_WRITE     (STREAM_WRITE | STREAM_UNBUFFERED)

/** \def STREAM_MODE_WRITE_OPEN_EXISTING
 * \brief Stream was opened for writing but not creating.
 */
#define STREAM_MODE_WRITE_OPEN_EXISTING  0x08

/** \def STREAM_MODE_OVERLAPPED
 * \brief Stream is run in asychronous, but non-threaded mode.
 */
#define STREAM_MODE_OVERLAPPED			0x10

/** \def STREAM_MODE_THREADED
 * \brief A seperate thread is used for I/O operations.
 */
#define STREAM_MODE_THREADED				0x20

#include "windows.h"

const int READF_ASYNC = 0x01;

typedef struct
{
	long	lLo;
	long    lHi;
} QWORD, *LPQWORD;

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

		int			virtual Open(StreamMode::StreamModes _dwMode) { dwMode=_dwMode; return (STREAM_ERR); }
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


//__int64 round(double x);

class CSimpleFileStream : public STREAM
{
private:
	HANDLE m_fileHandle;
	int m_offset;
private:
	CSimpleFileStream(HANDLE hFile);
public:
	static CSimpleFileStream* FromHandle(HANDLE hFile);
	virtual int	Read(void* lpDest, DWORD dwBytes);
	virtual __int64	GetSize(void);
	virtual int	Seek(__int64 qwPos);
	virtual bool IsEndOfStream();
	virtual __int64 GetPos();

	int	virtual	GetOffset() {
		return m_offset;
	}

	int	virtual	SetOffset(int newOffset) {
		m_offset = newOffset;
		return 1;
	}

};

class CMemoryStream : public STREAM
{
private:
	std::vector<char> m_data;
	__int64 m_currentPosition;

private:
	CMemoryStream();

public:
	static CMemoryStream* Create();
	static CMemoryStream* Create(const std::vector<char>& data);
	static CMemoryStream* Create(const char* pData, unsigned int size);

	virtual int Open(StreamMode::StreamModes _dwMode);
	virtual int	Read(void* lpDest, DWORD dwBytes);
	virtual __int64	GetSize();
	virtual int	Seek(__int64 qwPos);
	virtual __int64 GetPos();
	virtual bool IsEndOfStream();
};


#endif