#include "stdafx.h"
#include "FileStream.h"
#include "Filenames.h"
#include "UTF-8.h"
#include "../Common/Path.h"
#include "UnicodeCalls.h"

#ifndef _ASSERT
#ifndef _DEBUG
#define _ASSERT(a);
#else
#define _ASSERT(a) { if (!(a)) printf("B0rked in line %d\n", __LINE__); }
#endif
#endif

/*static struct QWORD
{
	long	lLo;
	long    lHi;
} *LPQWORD;
*/
__int64 round(double x)
{
	if ((x-(__int64)x)>0.5) 
	{
		return (__int64)x+1;
	}
	else
	{
		return (__int64)x;
	}
}

static bool SetFilePointer64 (HANDLE hFile, __int64 qwPos)
{
	SetFilePointer(hFile,((QWORD*)&qwPos)->lLo,&(((QWORD*)&qwPos)->lHi),FILE_BEGIN);
	return true;
}

static bool GetFilePointer64 (HANDLE hFile,__int64* qwPos)
{
	*qwPos=0;
	((QWORD*)qwPos)->lLo=SetFilePointer(hFile,0,&(((QWORD*)qwPos)->lHi),FILE_CURRENT);
	return true;
}


static bool GetFileSize64 (HANDLE hFile,__int64* qwSize)
{
	*qwSize=0;
	((QWORD*)qwSize)->lLo=GetFileSize(hFile,(DWORD*)&(((QWORD*)qwSize)->lHi));

    if (((QWORD*)qwSize)->lLo == INVALID_FILE_SIZE)
		*qwSize = 0;

	return true;
}

bool IsJobQueueEmpty(DATA_TRANSFER_THREAD_DATA* dttd)
{
	EnterCriticalSection(&dttd->critical);
	bool bEmpty = dttd->jobs.empty();
	LeaveCriticalSection(&dttd->critical);
	return bEmpty;
}

DWORD WINAPI DataTransfer_Thread(void* pData)
{
	DATA_TRANSFER_THREAD_DATA* dttd = (DATA_TRANSFER_THREAD_DATA*)pData;

	HANDLE h[2] = {
		dttd->hTerminateRequest, dttd->hTransferJob
	};

	bool bTerminateRequest = false;

	while (!bTerminateRequest || !IsJobQueueEmpty(dttd)) {
		DWORD wait = WaitForMultipleObjects(2, h, false, INFINITE);
		
		/* terminate request or transfer job */
		if (wait >= WAIT_OBJECT_0 && wait <= WAIT_OBJECT_0 + 1) {
			/* termination request */
			if (wait == WAIT_OBJECT_0) {
				bTerminateRequest = true;
				//WaitForSingleObject(dttd->hTerminateRequest, INFINITE);
			}

			/* job */
			if (wait == WAIT_OBJECT_0 + 1) {
				EnterCriticalSection(&dttd->critical);
				DATA_TRANSFER_JOB* j = dttd->jobs[0];
				dttd->jobs.pop_front();
				LeaveCriticalSection(&dttd->critical);

				if (j->dwOperation == DTJ_OPERATION_WRITE) {
					SetFilePointer64(dttd->file, j->position);
					WriteFile(dttd->file, j->pData, j->size, &j->dwTransferred, NULL);

					EnterCriticalSection(&dttd->critical);
					if (j->dwTransferred != j->size) {
						dttd->failed_jobs.push_back(j);
						j->bSuccess = false;
					} else {
						j->bSuccess = true;
						if (j->bDataAllocated)
							free(j->pData);
						j->bDataAllocated = false;
						dttd->succeeded_jobs.push_back(j);
					}
					dttd->currently_buffered -= j->size;
					LeaveCriticalSection(&dttd->critical);
					ReleaseSemaphore(j->hDone, 1, NULL);
				} else if (j->dwOperation == DTJ_OPERATION_READ) {
					SetFilePointer64(dttd->file, j->position);
					ReadFile(dttd->file, j->pData, j->size, &j->dwTransferred, NULL);
					EnterCriticalSection(&dttd->critical);
					if (!j->dwTransferred) {
						j->bSuccess = false;
						dttd->failed_jobs.push_back(j);
					} else {
						j->bSuccess = true;
						dttd->succeeded_jobs.push_back(j);
					}
					LeaveCriticalSection(&dttd->critical);
					ReleaseSemaphore(j->hDone, 1, NULL);
				} else if (j->dwOperation == DTJ_OPERATION_TRUNCATE) {
					SetFilePointer64(dttd->file, j->position);

					EnterCriticalSection(&dttd->critical);
					if (!SetEndOfFile(dttd->file)) {
						j->bSuccess = false;
						dttd->failed_jobs.push_back(j);
					} else {
						j->bSuccess = true;
						dttd->succeeded_jobs.push_back(j);
					}
					LeaveCriticalSection(&dttd->critical);

					ReleaseSemaphore(j->hDone, 1, NULL);
				}
			}
		}
	}

	/* this should not happen, but did due to a bug */
	if (dttd->currently_buffered) {
		char c[1024];
		sprintf_s(c, "dttd->currently_buffered != 0:\n\n  dttd->currently_buffered = %d\n  jobs left: %d",
			dttd->currently_buffered, dttd->jobs.size());
		//MessageBoxA(0, c, "Internal Error", MB_OK | MB_ICONERROR);
	}

	ReleaseSemaphore(dttd->hTerminatedSignal, 1, NULL);

	return 0;
}

CFileStream::CFileStream(void)
{
	hFile=NULL;
	iCurrentSize = 0;
	iFilesize = 0;
	bBuffered = 1;
	iPossibleAlignedReadCount = 0;
	bDenyUnbufferedRead = 1;
	bOverlapped = 0;
	cFilename = NULL;
}

CFileStream::~CFileStream(void)
{
	if (hFile) 
		Close();

	free(cFilename);
}

const int iOutCacheSize = 1<<21;

int CFileStream::Open(const char* _lpFilename,StreamMode::StreamModes _dwMode)
{
	int open_mode = 0;
	bCanRead = false;
	bCanWrite = false;
	bOverlapped = false;
	bThreaded = false;

	const char* lpFilename = _lpFilename;

	int lpwFileNameLength = 32768;
	wchar_t* lpwFilename = (wchar_t*)calloc(2, lpwFileNameLength);

	if ((_dwMode & StreamMode::Write) == StreamMode::Write) {

		if ((_dwMode & StreamMode::Read) == StreamMode::Read) {
			open_mode = OPEN_ALWAYS;
		} else {
			open_mode = CREATE_ALWAYS;
		}
		bCanWrite = true;
	}

	if ((_dwMode & StreamMode::Overlapped) == StreamMode::Overlapped) {
		bOverlapped = 1;
	}

	if ((_dwMode & StreamMode::Unbuffered) == StreamMode::Unbuffered) {
		bBuffered = 0;
	}

	if ((_dwMode & StreamMode::Threaded) == StreamMode::Threaded) {
		bThreaded = 1;
		bOverlapped = 0;
		//_dwMode &=~ StreamMode::Overlapped;
		_dwMode = _dwMode & (~StreamMode::Overlapped);
	}

	if (_dwMode & StreamMode::WriteOpenExisting) {
		open_mode = OPEN_EXISTING;
		//_dwMode &=~ StreamMode::WriteOpenExisting;
		_dwMode = _dwMode & (~StreamMode::WriteOpenExisting);
		_dwMode |= StreamMode::Write;
		//_dwMode = _dwMode | StreamMode::Write;
	}

	DWORD	dwNoBuffering = ((bBuffered)?0:FILE_FLAG_NO_BUFFERING);
	DWORD	dwOverlapped = (bOverlapped)?FILE_FLAG_OVERLAPPED:0;

	if (hFile) Close();
	hFile=NULL;
	STREAM::Open(_dwMode);

//	char cLongFN[65536];
//	Filename2LongFilename(lpFilename, (char*)cLongFN, 32768);
	std::string longFileName = CPath::GetLongFilename<char>(lpFilename);

	//fromUTF8(cLongFN, lpwFilename);
	//fromUTF8(longFileName.c_str(), lpwFilename);
	CUTF8 utf8LongFileName(longFileName.c_str());
	wcscpy_s(lpwFilename, lpwFileNameLength, utf8LongFileName.WStr());
	
	if (GetMode() & StreamMode::Read && !(GetMode() & StreamMode::Write)) {
		hFile=CreateFileW(lpwFilename,GENERIC_READ,FILE_SHARE_READ,
			NULL, OPEN_EXISTING, dwNoBuffering | dwOverlapped,NULL);
		bCanRead = 1;
	}

	if (GetMode() & StreamMode::Write) {
		DWORD new_mode = dwNoBuffering | dwOverlapped;
		if (dwNoBuffering)
			new_mode |= FILE_FLAG_WRITE_THROUGH;
		hFile=CreateFileW(lpwFilename, GENERIC_WRITE | GENERIC_READ, 0,
			NULL, open_mode, new_mode, NULL);

		if (hFile == INVALID_HANDLE_VALUE || !hFile) {
			hFile=CreateFileW(lpwFilename, GENERIC_WRITE | GENERIC_READ, 0,
				NULL, open_mode, dwNoBuffering | dwOverlapped, NULL);
		}
	}

	if (hFile == INVALID_HANDLE_VALUE) {
		STREAM::Close();
		free(lpwFilename);
		return STREAM_ERR;
	}

	size_t bufLen = 1+strlen(lpFilename);
	cFilename = new char[bufLen];
	strcpy_s(cFilename, bufLen, lpFilename);
	iCurrPos = 0;
	iFilesize = GetSize();

	free(lpwFilename);

	if (bThreaded) {
		dttd = new DATA_TRANSFER_THREAD_DATA;
		InitializeCriticalSection(&dttd->critical);
		dttd->file = hFile;
		dttd->hTransferJob = CreateSemaphore(NULL, 0, 65536, NULL);
		dttd->hTerminateRequest = CreateSemaphore(NULL, 0, 1, NULL);
		dttd->hTerminatedSignal = CreateSemaphore(NULL, 0, 1, NULL);
		dttd->currently_buffered = 0;
		DWORD dwID;
		if (!CreateThread(NULL, 0, DataTransfer_Thread, dttd, NULL, &dwID)) {
			bThreaded = false;
		}
	}

	return ((hFile!=INVALID_HANDLE_VALUE)?STREAM_OK:STREAM_ERR);
}

int CFileStream::GetName(char* pDest, uint32 buf_len)
{
	if (!buf_len)
		return 1 + static_cast<int>(strlen(cFilename));

	int BytesToCopy = static_cast<int>(max(buf_len-1, strlen(cFilename)));

	memcpy(pDest, cFilename, BytesToCopy);
	pDest[BytesToCopy]=0;

	return BytesToCopy;
}

int CFileStream::Open(const wchar_t* lpFileName, StreamMode::StreamModes _dwMode)
{
	char* utf8 = NULL;
	WStr2UTF8((const char*)lpFileName, &utf8);
	int res = Open(utf8, _dwMode);
	free(utf8);
	return res;
}

int CFileStream::Close(void)
{
	if (bThreaded) {
		ReleaseSemaphore(dttd->hTerminateRequest, 1, NULL);
		WaitForSingleObject(dttd->hTerminatedSignal, INFINITE);
		CloseHandle(dttd->hTerminatedSignal);
		CloseHandle(dttd->hTerminateRequest);
		CloseHandle(dttd->hTransferJob);
		DeleteCriticalSection(&dttd->critical);
		delete dttd;
	}

	if (hFile) 
		CloseHandle(hFile);
	hFile = NULL;

	return STREAM_OK;
}

int CFileStream::Seek(__int64 qwPos)
{
	_ASSERT(qwPos >= 0);

/*	if (qwPos & alignment_mask) {
		iPossibleAlignedReadCount = 0;
	}
*/	if (qwPos < 0) {
		//MessageBoxA(0,"Fatal seek error: qwPos < 0 !","Error",MB_OK | MB_ICONERROR);
	}

	if (!(GetMode() && StreamMode::Write) && 
		qwPos + GetOffset() >= GetSize() && 
		(GetMode() & StreamMode::Read))
			qwPos = GetSize() - GetOffset();

	if (GetPos()!=qwPos && !bThreaded) 
		SetFilePointer64(hFile,qwPos+GetOffset());
	
	iCurrPos = qwPos + GetOffset();
	
	return STREAM_OK;
}

__int64 CFileStream::GetPos(void)
{
	return iCurrPos - GetOffset();
}

__int64 CFileStream::GetSize(void)
{
	__int64	qwRes;
	
	if (iFilesize) return iFilesize - GetOffset();

	GetFileSize64(hFile,&qwRes);
	iFilesize = qwRes;
	qwRes-=GetOffset();
	return (qwRes);
}

int ispowof2(DWORD x)
{
	if (x < 2048) return 0;
	while (!(x & 1)) x>>=1;
	return !!(x==1);
}

int CFileStream::ReadAsync(void* pDest, DWORD dwBytes, OVERLAPPED* overlapped)
{
	__int64 j = GetPos(); DWORD* dwPos = (DWORD*)&j;
	overlapped->Offset = dwPos[0];
	overlapped->OffsetHigh = dwPos[1];
	overlapped->Internal = 0;
	overlapped->InternalHigh = 0;
	overlapped->hEvent = CreateEventA(NULL, true, false, "");
	
	int res = ReadFile(hFile, pDest, dwBytes, NULL, overlapped);

	iCurrPos += dwBytes;

	if (!res) {
		DWORD last_error = GetLastError();
		if (last_error == ERROR_IO_PENDING)
			return FILESTREAM_ASYNCH_IO_INITIATED;
	}

    if (res)
		return FILESTREAM_ASYNCH_IO_FINISHED;

	return FILESTREAM_ASYNCH_IO_FAILED;
}

VOID CALLBACK FileIOCompletionRoutine(
  DWORD dwErrorCode,
  DWORD dwNumberOfBytesTransfered,
  LPOVERLAPPED lpOverlapped
  )
{
	SetEvent(lpOverlapped->hEvent);
}



int CFileStream::WriteAsync(void* pDest, DWORD dwBytes, OVERLAPPED* overlapped)
{
	__int64 j = GetPos(); DWORD* dwPos = (DWORD*)&j;

	overlapped->Offset = dwPos[0];
	overlapped->OffsetHigh = dwPos[1];
	overlapped->Internal = 0;
	overlapped->InternalHigh = 0;
	overlapped->hEvent = CreateEventA(NULL, true, false, "");

	ResetEvent(overlapped->hEvent);
//	int res = WriteFileEx(hFile, pDest, dwBytes, overlapped, FileIOCompletionRoutine);
	
	int res = WriteFile(hFile, pDest, dwBytes, NULL, overlapped);
	iCurrPos += dwBytes;

	DWORD last_error = 0;
	if (!res)
		last_error = GetLastError(); 
//	if (!res) {
	
	if (last_error == ERROR_IO_PENDING)
//	if (res) 
		return FILESTREAM_ASYNCH_IO_INITIATED;
//	}

	if (res) {
//		return CFileStream_ASYNCH_IO_INITIATED;
		return FILESTREAM_ASYNCH_IO_FINISHED;
	}

	return FILESTREAM_ASYNCH_IO_FAILED;
}

int CFileStream::WaitForAsyncIOCompletion(OVERLAPPED* overlapped, DWORD* pdwBytesTransferred)
{
	DWORD r;

	BOOL b = GetOverlappedResult(hFile, overlapped, &r, true);

	if (pdwBytesTransferred)
		*pdwBytesTransferred = r;

	return b;
}

int CFileStream::IsOverlappedIOComplete(OVERLAPPED* overlapped)
{
	DWORD r;
	BOOL b = (S_OK == GetOverlappedResult(hFile, overlapped, &r, false));
	return b;
}

void CleanSuccessfulJobQueue(DATA_TRANSFER_THREAD_DATA* dttd)
{
		EnterCriticalSection(&dttd->critical);
		DATA_TRANSFER_JOB_QUEUE::iterator iter = dttd->succeeded_jobs.begin();
		for (; iter != dttd->succeeded_jobs.end(); iter++) {
			CloseHandle((*iter)->hDone);
			delete (*iter);
		}
		dttd->succeeded_jobs.clear();
		LeaveCriticalSection(&dttd->critical);
}

int CFileStream::Read(void* lpDest,DWORD dwBytes)
{
	DWORD	dwRead;
	BYTE*	lpbDest=(BYTE*)lpDest;

//	_ASSERT(GetMode() & STREAM_READ);
	_ASSERT(hFile);
	
	if (bThreaded) {
		DATA_TRANSFER_JOB* j = new DATA_TRANSFER_JOB;
		j->hDone = CreateSemaphore(NULL, 0, 1, NULL);
		j->position = iCurrPos;
		j->size = dwBytes;
		j->pData = lpDest;
		j->bDataAllocated = false;
		j->dwOperation = DTJ_OPERATION_READ;
		EnterCriticalSection(&dttd->critical);
		dttd->jobs.push_back(j);
		ReleaseSemaphore(dttd->hTransferJob, 1, NULL);
		CleanSuccessfulJobQueue(dttd);
		LeaveCriticalSection(&dttd->critical);

		WaitForSingleObject(j->hDone, INFINITE);
		if (!j->bSuccess)
			return 0;

		iCurrPos += j->dwTransferred;
		return j->dwTransferred;
	}

	dwRead = 0;
	if (!ReadFile(hFile, lpDest, dwBytes, &dwRead, NULL)) {
		DWORD dwError = GetLastError();
		void* msg = NULL;
		FormatMessage( 
		    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		    FORMAT_MESSAGE_FROM_SYSTEM | 
		    FORMAT_MESSAGE_IGNORE_INSERTS,
		    NULL,
			dwError,
		    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		    (LPTSTR) &msg,
		    0,
		    NULL);

		printf("%s\n", msg);
		LocalFree(msg);

		return 0;
	}

	iCurrPos += dwRead;

	return dwRead;
}

void CFileStream::Flush()
{

}

int CFileStream::Write(void* lpSource, DWORD dwBytes)
{
	DWORD	dwWritten;

	dwWritten = 0;
	_ASSERT(hFile);
//	_ASSERT(GetMode() & STREAM_WRITE);

	if (bThreaded) {

		while (dttd->currently_buffered > 32000000) {
			EnterCriticalSection(&dttd->critical);			
			HANDLE hWait = dttd->jobs[0]->hDone;
			LeaveCriticalSection(&dttd->critical);
			WaitForSingleObject(hWait, INFINITE);
		}

		DATA_TRANSFER_JOB* dtj = new DATA_TRANSFER_JOB;
		dtj->bDataAllocated = true;
		dtj->dwOperation = DTJ_OPERATION_WRITE;
		dtj->pData = malloc(dwBytes);
		memcpy(dtj->pData, lpSource, dwBytes);
		dtj->position = iCurrPos;
		dtj->size = dwBytes;
		dtj->hDone = CreateSemaphore(NULL, 0, 1, NULL);
		
		EnterCriticalSection(&dttd->critical);
		dttd->currently_buffered += dwBytes;
		dttd->jobs.push_back(dtj);
		ReleaseSemaphore(dttd->hTransferJob, 1, NULL);
	
		bool bDeferredError = !dttd->failed_jobs.empty();

		CleanSuccessfulJobQueue(dttd);
		LeaveCriticalSection(&dttd->critical);

		if (bDeferredError)
			return 0;

		iCurrPos += dwBytes;
		return dwBytes;
	}

	Seek(iCurrPos);

	if (!WriteFile(hFile, lpSource, dwBytes, &dwWritten, 0)) {
		DWORD dwError = GetLastError();
		void* msg = NULL;
		FormatMessage( 
		    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		    FORMAT_MESSAGE_FROM_SYSTEM | 
		    FORMAT_MESSAGE_IGNORE_INSERTS,
		    NULL,
			dwError,
		    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		    (LPTSTR) &msg,
		    0,
		    NULL);

		printf("%s\n", msg);
		LocalFree(msg);

		return 0;
	}

//	_ASSERT(dwBytes == dwWritten);

	iCurrPos += dwBytes;
	if (iCurrPos > iCurrentSize) {
		iCurrentSize = iCurrPos;
		iFilesize = iCurrentSize;
	}

	return dwBytes;
}

bool CFileStream::IsEndOfStream(void)
{
	return (GetSize()<=GetPos());
}
/*
int CFileStream::SethFile(HANDLE _hFile)
{
	hFile=_hFile;
	return STREAM_OK;
}
*/
/* only to be called by CACHE class before closing file definitely */
int CFileStream::TruncateAt(__int64 iPosition)
{
	if (bBuffered) {
		if (!bThreaded) {
			Seek(iPosition-1);
			Seek(iPosition);
			SetEndOfFile(hFile);
		} else {
			DATA_TRANSFER_JOB* j = new DATA_TRANSFER_JOB;
			j->bDataAllocated = false;
			j->dwOperation = DTJ_OPERATION_TRUNCATE;
			j->position = iPosition;
			j->hDone = CreateSemaphore(NULL, 0, 1, NULL);
			j->size = 0;
			EnterCriticalSection(&dttd->critical);
			dttd->jobs.push_back(j);
			LeaveCriticalSection(&dttd->critical);
			ReleaseSemaphore(dttd->hTransferJob, 1, NULL);
		}
	} else {
		Close();
		bBuffered = 1;
		bOverlapped = 0;
		Open(cFilename, StreamMode::WriteOpenExisting);
		TruncateAt(iPosition);
	}

	return 1;
}
