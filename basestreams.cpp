#include "stdafx.h"
#include "basestreams.h"
#include "memory.h"
#include "math.h"
#include "utf-8.h"
#include "unicodecalls.h"
#include "stdlib.h"
#include "stdio.h"
#include "crtdbg.h"
#include "Filenames.h"

#ifdef DEBUG_NEW
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif


int STREAM::GetOffset()
{
	 return iOffset;
}

int alignment_mask = 0x7FFF;

int STREAM::SetOffset(int iNewOffset)
{
	 iOffset = iNewOffset; 
	 return 1;
}

int STREAM::TruncateAt(__int64 iPosition)
{
	return 1;
}

bool STREAM::IsEnabled(int flag)
{
	return _Enable(flag, false, false);
}

bool STREAM::Enable(int flag)
{
	return _Enable(flag, true, true);
}

bool STREAM::Disable(int flag)
{
	return _Enable(flag, true, false);
}

bool STREAM::SetFlag(int flag, int value)
{
	return _Enable(flag, true, !!value);
}

STREAM_FILTER::STREAM_FILTER()
{
	source = NULL;
}

STREAM_FILTER::~STREAM_FILTER()
{
	if (GetSource())
		Close();
}

void STREAM_FILTER::SetSource(STREAM* s)
{
	source = s;
}

STREAM* STREAM_FILTER::GetSource()
{
	return source;
}

int STREAM_FILTER::Close()
{
	if (GetSource() && IsSourceAttached()) {
		GetSource()->Close();
		delete GetSource();
		SetSource(NULL);
	}

	return 1;
}

// für vernünftigen Zugriff auf Dateien > 4 GB)  

struct QWORD
{
	long	lLo;
	long    lHi;
} *LPQWORD;

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

bool SetFilePointer64 (HANDLE hFile, __int64 qwPos)
{
	SetFilePointer(hFile,((QWORD*)&qwPos)->lLo,&(((QWORD*)&qwPos)->lHi),FILE_BEGIN);
	return true;
}

bool GetFilePointer64 (HANDLE hFile,__int64* qwPos)
{
	*qwPos=0;
	((QWORD*)qwPos)->lLo=SetFilePointer(hFile,0,&(((QWORD*)qwPos)->lHi),FILE_CURRENT);
	return true;
}

bool GetFileSize64 (HANDLE hFile,__int64* qwSize)
{
	*qwSize=0;
	((QWORD*)qwSize)->lLo=GetFileSize(hFile,(DWORD*)&(((QWORD*)qwSize)->lHi));

    if (((QWORD*)qwSize)->lLo == INVALID_FILE_SIZE)
		*qwSize = 0;

	return true;
}

FILESTREAM::FILESTREAM(void)
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

/*bool _filestreamAllowUnbufferedRead = true;

void  filestreamAllowUnbufferedRead(bool bAllow)
{
	_filestreamAllowUnbufferedRead = bAllow;
}
*/
FILESTREAM::~FILESTREAM(void)
{
	if (hFile) 
		Close();

	free(cFilename);
}

const int iOutCacheSize = 1<<21;

int FILESTREAM::Open(char* _lpFilename,DWORD _dwMode)
{
	int open_mode = 0;
	bCanRead = false;
	bCanWrite = false;
	bOverlapped = false;
	char* lpFilename = _lpFilename;
	int dealloc_lpfilename = 0;

	unsigned short* lpwFilename = (unsigned short*)calloc(1, 32768);

	if ((_dwMode & STREAM_WRITE) == STREAM_WRITE) {

		if ((_dwMode & STREAM_READ) == STREAM_READ) {
			open_mode = OPEN_ALWAYS;
		} else {
			open_mode = CREATE_ALWAYS;
		}
		bCanWrite = true;
	}

	if ((_dwMode & STREAM_OVERLAPPED) == STREAM_OVERLAPPED) {
		bOverlapped = 1;
	}

	if ((_dwMode & STREAM_UNBUFFERED) == STREAM_UNBUFFERED) {
		bBuffered = 0;
	}

	if (_dwMode & STREAM_WRITE_OPEN_EXISTING) {
		open_mode = OPEN_EXISTING;
		_dwMode &=~ STREAM_WRITE_OPEN_EXISTING;
		_dwMode |= STREAM_WRITE;
	}

	DWORD	dwNoBuffering = ((bBuffered)?0:FILE_FLAG_NO_BUFFERING);
	DWORD	dwOverlapped = (bOverlapped)?FILE_FLAG_OVERLAPPED:0;

	if (hFile) Close();
	hFile=NULL;
	STREAM::Open(_dwMode);

/*	if (utf8_IsUnicodeEnabled()) {
		fromUTF8("\\\\?\\", lpwFilename);
		fromUTF8(lpFilename, lpwFilename + 4);
	} else*/

	char cLongFN[65536];
	Filename2LongFilename(lpFilename, (char*)cLongFN, 32768);

	fromUTF8(cLongFN, lpwFilename);
	
	if (GetMode() & STREAM_READ && !(GetMode() & STREAM_WRITE)) {
		hFile=(*UCreateFile())(lpwFilename,GENERIC_READ,FILE_SHARE_READ,
			NULL,OPEN_EXISTING,dwNoBuffering | dwOverlapped,NULL);
//		bDenyUnbufferedRead = !_filestreamAllowUnbufferedRead;
		bCanRead = 1;
	}

	if (GetMode() & STREAM_WRITE) {
		DWORD new_mode = dwNoBuffering | dwOverlapped;
		if (dwNoBuffering)
			new_mode |= FILE_FLAG_WRITE_THROUGH;
		hFile=(*UCreateFile())(lpwFilename,GENERIC_WRITE | GENERIC_READ,0,
			NULL,open_mode, new_mode,NULL);

		if (hFile == INVALID_HANDLE_VALUE || !hFile) {
			hFile=(*UCreateFile())(lpwFilename,GENERIC_WRITE | GENERIC_READ,0,
				NULL,open_mode,dwNoBuffering | dwOverlapped,NULL);
		}
	}

	if (hFile == INVALID_HANDLE_VALUE) {
		STREAM::Close();
		free(lpwFilename);
		return STREAM_ERR;
	}

	cFilename = new char[1+strlen(lpFilename)];
	strcpy(cFilename, lpFilename);
	iCurrPos = 0;
	iFilesize = GetSize();

	if (dealloc_lpfilename)
		free(lpFilename);

	free(lpwFilename);

	return ((hFile!=INVALID_HANDLE_VALUE)?STREAM_OK:STREAM_ERR);
}

int FILESTREAM::Close(void)
{
	if (hFile) 
		CloseHandle(hFile);
	hFile = NULL;

	return STREAM_OK;
}

int FILESTREAM::Seek(__int64 qwPos)
{
	_ASSERT(qwPos >= 0);

/*	if (qwPos & alignment_mask) {
		iPossibleAlignedReadCount = 0;
	}
*/	if (qwPos < 0) {
		MessageBoxA(0,"Fatal seek error: qwPos < 0 !","Error",MB_OK | MB_ICONERROR);
	}

	if (!(GetMode() && STREAM_WRITE) && qwPos + GetOffset() >= GetSize() && (GetMode() & STREAM_READ))
		qwPos = GetSize() - GetOffset();

	if (GetPos()!=qwPos) {
		SetFilePointer64(hFile,qwPos+GetOffset());
	}
	
	iCurrPos = qwPos + GetOffset();
	
	return STREAM_OK;
}

__int64 FILESTREAM::GetPos(void)
{
	return iCurrPos - GetOffset();
}

__int64 FILESTREAM::GetSize(void)
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

int FILESTREAM::ReadAsync(void* pDest, DWORD dwBytes, OVERLAPPED* overlapped)
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



int FILESTREAM::WriteAsync(void* pDest, DWORD dwBytes, OVERLAPPED* overlapped)
{
	__int64 j = GetPos(); DWORD* dwPos = (DWORD*)&j;

	overlapped->Offset = dwPos[0];
	overlapped->OffsetHigh = dwPos[1];
	overlapped->Internal = 0;
	overlapped->InternalHigh = 0;
	overlapped->hEvent = CreateEventA(NULL, true, false, "");

	ResetEvent(overlapped->hEvent);
	int res = WriteFileEx(hFile, pDest, dwBytes, overlapped, FileIOCompletionRoutine);
	
//	int res = WriteFile(hFile, pDest, dwBytes, NULL, overlapped);
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
		return FILESTREAM_ASYNCH_IO_INITIATED;
//		return FILESTREAM_ASYNCH_IO_FINISHED;
	}

	return FILESTREAM_ASYNCH_IO_FAILED;
}

int FILESTREAM::WaitForAsyncIOCompletion(OVERLAPPED* overlapped, DWORD* pdwBytesTransferred)
{
	DWORD r;

	BOOL b = GetOverlappedResult(hFile, overlapped, &r, true);

	if (pdwBytesTransferred)
		*pdwBytesTransferred = r;

	return b;
}

int FILESTREAM::IsOverlappedIOComplete(OVERLAPPED* overlapped)
{
	DWORD r;
	BOOL b = (S_OK == GetOverlappedResult(hFile, overlapped, &r, false));
	return b;
}

int FILESTREAM::Read(void* lpDest,DWORD dwBytes)
{
	DWORD	dwRead;
	BYTE*	lpbDest=(BYTE*)lpDest;

	_ASSERT(GetMode() & STREAM_READ);
	_ASSERT(hFile);
	
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

void FILESTREAM::Flush()
{

}

int FILESTREAM::Write(void* lpSource,DWORD dwBytes)
{
	DWORD	dwWritten;

	dwWritten = 0;
	_ASSERT(hFile);
	_ASSERT(GetMode() & STREAM_WRITE);

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

	_ASSERT(dwBytes == dwWritten);

	iCurrPos += dwBytes;
	if (iCurrPos > iCurrentSize) {
		iCurrentSize = iCurrPos;
		iFilesize = iCurrentSize;
	}

	return dwBytes;
}

bool FILESTREAM::IsEndOfStream(void)
{
	return (GetSize()<=GetPos());
}

int FILESTREAM::SethFile(HANDLE _hFile)
{
	hFile=_hFile;
	return STREAM_OK;
}

int FILESTREAM::TruncateAt(__int64 iPosition)
{
	if (bBuffered) {
		Seek(iPosition-1);
		Seek(iPosition);
		SetEndOfFile(hFile);
	} else {
		Close();
		bBuffered = 1;
		bOverlapped = 0;
		Open(cFilename, STREAM_WRITE_OPEN_EXISTING);
		TruncateAt(iPosition);
	}

	return 1;
}










int BITSTREAM::Open(STREAM* lpStream)
{
	 source=lpStream;
	 dwCurrBitPos=16;
	 if (GetSource()) GetSource()->Seek(0);
	 return (lpStream)?STREAM_OK:STREAM_ERR; 
}

int BITSTREAM::ReadBit(int iFlag)
{
	if (dwCurrBitPos>7)
	{
		dwCurrBitPos=7;
		LoadWord();
	}

	int iRes=!!(wData&(1<<(iFlag?(7-dwCurrBitPos--):dwCurrBitPos--)));
	
	return iRes;
}

int BITSTREAM::Seek(__int64 qwPos)
{
	_ASSERT(qwPos >= 0);
	_ASSERT(GetSource());

	if (!GetSource()) 
		return 0;

	if (GetSource()->Seek(qwPos>>3)==STREAM_OK)
	{
		GetSource()->Read(&wData,1);
		dwCurrBitPos=(DWORD)(qwPos&0x7);
		return 1;
	}
	else
		return 0;
}

void BITSTREAM::LoadWord(void)
{
	if (GetSource())
	{
		GetSource()->Read(&wData,1);
	}
}

int BITSTREAM::ReadBits(int n, int iFlag)
{
	return (int)ReadBits64(n, iFlag);
}

__int64 BITSTREAM::GetPos()
{
	return GetSource()->GetPos();
}

__int64 BITSTREAM::ReadBits64(int n, int iFlag)
{
	__int64	iRes=0;
	__int64 iMul=1;
	if (!GetSource()) return 0;

	for (int i=0;i<n;i++)
	{
		if (!iFlag) {
			iRes*=2;
			iRes|=ReadBit(iFlag);
		} else {
			iRes += iMul*ReadBit(iFlag);
			iMul *= 2;
		}
	}

	return iRes;
}