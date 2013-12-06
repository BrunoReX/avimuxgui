#ifndef I_CFileStream
#define I_CFileStream

#include "basestreams.h"
#include <deque>

const int FILESTREAM_ASYNCH_IO_INITIATED = 0x13;
const int FILESTREAM_ASYNCH_IO_FINISHED  = 0x17;
const int FILESTREAM_ASYNCH_IO_FAILED    = 0x00;

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


class CFileStream : public STREAM
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
		/* finish any pending jobs; this is not yet implemented */
		void		virtual Flush();

public:
		/* simple constructor */
		CFileStream(void);

		/* simple destructor */
		virtual ~CFileStream(void);

		/* open a file; the filename must be given in UTF-8 encoding */
		int			virtual	Open (char* lpFileName, DWORD _dwMode);

		/* open a file; the filename must be given in UTF-16 encoding */
		int			virtual Open (wchar_t* lpFileName, DWORD _dwMode);

		/* read the HANDLE of the current file */
		HANDLE				GethFile (void) { return hFile; }

		/* close the file */
		int			virtual	Close(void);

		/* seek to a position within that file */
		int			virtual	Seek(__int64 qwPos);

		/* request the current position within the file */
		__int64		virtual GetPos(void);

		/* request the size of the file; note that this class only supports
		   real files, it won't work with stuff like /dev/null anyway */
		__int64		virtual GetSize(void);

		/* read dwBytes bytes from the file to lpDest. This call returns after
		   reading is complete or has failed. If the file is in asynchronous mode,
		   it schedules the read operation and waits until it's finished. */
		int			virtual	Read(void* lpDest,DWORD dwBytes);

		/* Schedule an asynchronous read job. It returns 
             CFileStream_ASYNCH_IO_FINISHED
		       if the operation is finished before the function returns, 
		     CFileStream_ASYNCH_IO_INITIATED
		       if the operation is not finished when the function returns,
		     CFileStream_ASYNCH_IO_FAILED
		       if the operation failed 
		   Note: Asnychronous means using overlapped writing; it does NOT
		         mean using a separate write thread */
		int			virtual	ReadAsync(void* pDest, DWORD dwBytes, OVERLAPPED* overlapped);

		/* schedule an asynchronous write operation. The return values are the
		   same as for ReadAsnyc
		   Note: Asnychronous means using overlapped writing; it does NOT
		         mean using a separate write thread */
		int			virtual	WriteAsync(void* pDest, DWORD dwBytes, OVERLAPPED* overlapped);

		/* Wait until an asynchronous operation is complete. This does refer
		   to overlapped I/O only, not to threaded I/O */
		int			virtual WaitForAsyncIOCompletion(OVERLAPPED* overlapped, DWORD* pdwBytesTransferred);

		/* returns true if and only if the given overlapped operation has been
		   finished. This does not refer to threaded operations */
		int			virtual IsOverlappedIOComplete(OVERLAPPED* overlapped);

		/* When threaded writing is disabled, the given data is written to
		   the file before the call returns. When threaded writing is enabled,
		   a write call is scheduled, then the function returns. When trying to
		   read data afterwards, all pending jobs are finished before actually
		   reading anything. This way, writing can be postponed as long as a larger
		   file is being created without reading from it while writing it. */
		int			virtual Write(void* lpSource, DWORD dwBytes);

		/* returns true when the end of the file has been reached during
		   reading */
		bool		virtual IsEndOfStream(void);

		/* returns the smallest number of bytes that can be read */
		int			virtual GetGranularity(void) { return 1; }

		/* truncate the file at the given position. When using unbuffered
		   writing, only multiples of the drive's sector size can be written,
		   so after closing a file created in unbuffered write mode, the file
		   must be reopened and truncated to cut off padding at the end */
		int			virtual TruncateAt(__int64 iPosition);
};



#endif