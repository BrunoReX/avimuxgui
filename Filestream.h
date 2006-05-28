#ifndef I_FILESTREAM
#define I_FILESTREAM

#include "basestreams.h"
#include <deque>
const FILESTREAM_ASYNCH_IO_INITIATED = 0x13;
const FILESTREAM_ASYNCH_IO_FINISHED  = 0x17;
const FILESTREAM_ASYNCH_IO_FAILED    = 0x00;

#define DTJ_OPERATION_READ 0x01
#define DTJ_OPERATION_WRITE 0x02
#define DTJ_OPERATION_TRUNCATE 0x03

typedef struct
{
	__int64	position;
	DWORD	size;
	void*	pData;
	bool	bDataAllocated;
	DWORD	dwOperation;
	HANDLE	hDone;
	bool	bSuccess;
	DWORD	dwTransferred;
} DATA_TRANSFER_JOB;
#define DATA_TRANSFER_JOB_QUEUE std::deque<DATA_TRANSFER_JOB*>

typedef struct
{
	HANDLE	file;
	CRITICAL_SECTION	critical;
	DATA_TRANSFER_JOB_QUEUE jobs;
	DATA_TRANSFER_JOB_QUEUE failed_jobs;
	DATA_TRANSFER_JOB_QUEUE succeeded_jobs;
	HANDLE	hTerminateRequest;
	HANDLE	hTransferJob;
	HANDLE	hTerminatedSignal;
	int	currently_buffered;
} DATA_TRANSFER_THREAD_DATA;


class FILESTREAM : public STREAM
{
	private:
		HANDLE		hFile;
		__int64		iCurrentSize;
		char		cWriteSemaphoreName[10];
		__int64		iFilesize;
		__int64		iCurrPos;
		bool		bBuffered;
		bool		bOverlapped;
		char*		cFilename;
		int			iPossibleAlignedReadCount;
		bool		bDenyUnbufferedRead;
		bool		bCanRead;
		bool		bCanWrite;
		bool		bThreaded;

	/* separate output thread */
		DATA_TRANSFER_THREAD_DATA* dttd;


	protected:
		void		virtual Flush();
	public:
		FILESTREAM(void);
		virtual ~FILESTREAM(void);
		int			virtual	Open (char* lpFileName,DWORD _dwMode); // utf-8
		int			virtual Open (wchar_t* lpFileName, DWORD _dwMode);
		int					SethFile (HANDLE _hFile);
		HANDLE				GethFile (void) { return hFile; }
		int			virtual	Close(void);
		int			virtual	Seek(__int64 qwPos);
		__int64		virtual GetPos(void);
		__int64		virtual GetSize(void);
		int			virtual	Read(void* lpDest,DWORD dwBytes);
		int			virtual	ReadAsync(void* pDest, DWORD dwBytes, OVERLAPPED* overlapped);
		int			virtual	WriteAsync(void* pDest, DWORD dwBytes, OVERLAPPED* overlapped);
		int			virtual WaitForAsyncIOCompletion(OVERLAPPED* overlapped, DWORD* pdwBytesTransferred);
		int			virtual IsOverlappedIOComplete(OVERLAPPED* overlapped);
		int			virtual Write(void* lpSource,DWORD dwBytes);
		bool		virtual IsEndOfStream(void);
		int			virtual GetGranularity(void) { return 1; }
		int			virtual TruncateAt(__int64 iPosition);
};



#endif