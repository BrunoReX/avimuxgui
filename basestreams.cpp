#include "stdafx.h"
#include "basestreams.h"
#include "memory.h"
#include "math.h"


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

int STREAM::ReadAsync(READ_ASYNC_STRUCT* pRAS, DWORD dwBytes)
{
	char c[10]; c[9] = 0; for (int i=0;i<9;c[i++]=97+rand()%26);
	pRAS->hSemaphore = CreateSemaphore(NULL, 1, 1, c);
	pRAS->pOverlapped = new OVERLAPPED;
	pRAS->pOverlapped->Internal = 0;
	pRAS->pOverlapped->InternalHigh = 0;
	pRAS->pOverlapped->Offset = Read(pRAS->lpDest, dwBytes); 
	pRAS->pOverlapped->OffsetHigh = 0;
	
	return pRAS->pOverlapped->Offset;
}

// für vernünftigen Zugriff auf Dateien > 4 GB)  

struct QWORD
{
	long	lLo;
	long    lHi;
} *LPQWORD;

typedef struct
{
	HANDLE hSemaphore;
	void*  lpData;

} OVERLAPPED_ADDITIONAL;

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
	return true;
}

FILESTREAM::FILESTREAM(void)
{
	hFile=NULL;
	iCurrentSize = 0;
	cOutCache = NULL;
	iOutCachePos = 0;
	iFilesize = 0;
	bBuffered = 1;
	pAlignedInputBuffer = 0;
	pAlignedInputBufferAllocated = 0;
	iAlignedBufferSize = 0;
	iPossibleAlignedReadCount = 0;
	bDenyUnbufferedRead = 1;
	bOverlapped = 0;
}

bool _filestreamAllowBufferedRead = true;

void  filestreamAllowBufferedRead(bool bAllow)
{
	_filestreamAllowBufferedRead = bAllow;
}


const int MAXWRITEJOBS = 8;

VOID CALLBACK WriteCompletionRoutine(
  DWORD dwErrorCode,                // completion code
  DWORD dwNumberOfBytesTransfered,  // number of bytes transferred
  LPOVERLAPPED lpOverlapped         // pointer to structure with I/O 
                                    // information
									) 
{

	OVERLAPPED_ADDITIONAL* over_add = ((OVERLAPPED_ADDITIONAL*)lpOverlapped->hEvent);
	HANDLE h = (HANDLE)over_add->hSemaphore;
	
	ReleaseSemaphore(h, 1, NULL);

	delete over_add->lpData;
	delete over_add;
	delete lpOverlapped;

	return;

}
 
VOID CALLBACK ReadCompletionRoutine(
  DWORD dwErrorCode,                // completion code
  DWORD dwNumberOfBytesTransfered,  // number of bytes transferred
  LPOVERLAPPED lpOverlapped         // pointer to structure with I/O 
                                    // information
									) 
{

	HANDLE h = lpOverlapped->hEvent;
	lpOverlapped->Offset = dwNumberOfBytesTransfered;
	ReleaseSemaphore(h, 1, NULL);

	return;

}


FILESTREAM::~FILESTREAM(void)
{
	if (hFile) Close();
}

const int iOutCacheSize = 1<<21;

int FILESTREAM::Open(char* lpFilename,DWORD _dwMode)
{
	int open_mode = 0;
	bCanRead = false;
	bCanWrite = false;
	bOverlapped = false;

	if ((_dwMode & STREAM_WRITE) == STREAM_WRITE) {
		open_mode = CREATE_ALWAYS;
		bCanWrite = true;
	}

	if ((_dwMode & STREAM_OVERLAPPED) == STREAM_OVERLAPPED) {
//		if ((_dwMode & STREAM_WRITE) == STREAM_WRITE) {
			bOverlapped = 1;
			if (bCanWrite) {
				cWriteSemaphoreName[9] = 0; for (int i=0;i<9;cWriteSemaphoreName[i++] = 97+rand()%26);
				hWriteSemaphore = CreateSemaphore(NULL, MAXWRITEJOBS, MAXWRITEJOBS, cWriteSemaphoreName);
				if (!hWriteSemaphore) {
					bOverlapped = 0;
				}
			}
//			ASSERT(hWriteSemaphore);
//		}
	}

	if ((_dwMode & STREAM_UNBUFFERED) == STREAM_UNBUFFERED) {
		bBuffered = 0;
		_dwMode = STREAM_WRITE;
	}

	if (_dwMode & STREAM_WRITE_OPEN_EXISTING) {
		_dwMode = STREAM_WRITE;
		open_mode = OPEN_EXISTING;
	}

	DWORD	dwNoBuffering = ((bBuffered)?0:FILE_FLAG_NO_BUFFERING);
	DWORD	dwOverlapped = (bOverlapped)?FILE_FLAG_OVERLAPPED:0;

	if (hFile) Close();
	hFile=NULL;
	STREAM::Open(_dwMode);
	align = 65536;
	pAlignedInputBuffer = 0;
	pAlignedInputBufferAllocated = 0;
	iAlignedBufferSize = 0;

	if (GetMode() & STREAM_READ)
	{
		 
		hFile=CreateFile(lpFilename,GENERIC_READ,FILE_SHARE_READ,
			NULL,OPEN_EXISTING,dwNoBuffering | dwOverlapped,NULL);

		bDenyUnbufferedRead = !_filestreamAllowBufferedRead;
		bCanRead = 1;
	}

	if (GetMode() & STREAM_WRITE)
	{
		hFile=CreateFile(lpFilename,GENERIC_WRITE | GENERIC_READ,0,
			NULL,open_mode,dwNoBuffering | dwOverlapped,NULL);
		if (bBuffered) {
			cOutCache = new char[iOutCacheSize];
			cOutCacheCurr = cOutCache;
		} else {
			cOutCache = NULL;
			cOutCacheCurr = NULL;
		}
	}
	iOutCachePos = 0;
	cFilename = new char[1+strlen(lpFilename)];
	strcpy(cFilename, lpFilename);
	iCurrPos = 0;
	iFilesize = 0;
	return ((hFile!=INVALID_HANDLE_VALUE)?STREAM_OK:STREAM_ERR);
}

int FILESTREAM::Close(void)
{
	Flush();
	if (bOverlapped) {
		if (bCanWrite) {
			for (int i=MAXWRITEJOBS;i;i--) {
				while (WAIT_IO_COMPLETION == WaitForSingleObjectEx(hWriteSemaphore, INFINITE, true));
			}
			CloseHandle(hWriteSemaphore);
			hWriteSemaphore = NULL;
		}
	}

	if (hFile) CloseHandle(hFile);
	if (cOutCache) delete cOutCache;
	if (pAlignedInputBufferAllocated) delete pAlignedInputBufferAllocated;
	pAlignedInputBufferAllocated = NULL;
	iAlignedBufferSize = 0;
	hFile=NULL;
	return STREAM_OK;
}

int FILESTREAM::Seek(__int64 qwPos)
{
	if (qwPos & alignment_mask) {
		iPossibleAlignedReadCount = 0;
	}
	if (qwPos < 0) {
		MessageBox(0,"Fatal seek error: qwPos < 0 !","Error",MB_OK | MB_ICONERROR);
	}
	if (bCanWrite) {
		if (qwPos > iCurrentSize) {
//			MessageBox(0,"Error: Seeking behind end of file","Error",MB_OK | MB_ICONERROR);
//			return STREAM_ERR;
//			Sleep(1);
		}
		Flush();
	}
	if (GetPos()!=qwPos) SetFilePointer64(hFile,qwPos+GetOffset());
	iCurrPos = qwPos + GetOffset();
	return STREAM_OK;
}

__int64 FILESTREAM::GetPos(void)
{
	return iCurrPos + iOutCachePos - GetOffset();
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


/*int FILESTREAM::Read(void* p, DWORD dwBytes, DWORD dwFlag)
{

	if ((dwFlag & READF_ASYNC) == READF_ASYNC) {
		READ_ASYNC_STRUCT* pRAS = (READ_ASYNC_STRUCT*)p;
		return read(pRAS, dwBytes);
	} else {
		return read(p, dwBytes);

	}

	
}
*/
/*int FILESTREAM::Read(READ_ASYNC_STRUCT** pRAS, DWORD dwBytes)
{
	if (*pRAS) {
		delete *pRAS;
	}
	*pRAS = new READ_ASYNC_STRUCT;
	ZeroMemory(*pRAS, sizeof(READ_ASYNC_STRUCT));

	return Read(*pRAS, dwBytes);
}
*/

int FILESTREAM::ReadAsync(READ_ASYNC_STRUCT* pRAS, DWORD dwBytes)
{
	char cSemName[10]; cSemName[9] = 0;
	for (int i=0;i<9;cSemName[i++]=97+rand()%26);

	// if file is not overlapped -> do synchronous i/o
	if (!bOverlapped || bBuffered) {
		__int64 j = GetPos(); DWORD* dwPos = (DWORD*)&j;
		dwBytes = Read(pRAS->lpDest, dwBytes);
		pRAS->hSemaphore = CreateSemaphore(NULL, 1, 1, cSemName);

		OVERLAPPED* pOver = new OVERLAPPED;
		pRAS->pOverlapped = pOver;

		pOver->Offset = dwBytes;
		pOver->OffsetHigh = dwPos[1];
		pOver->Internal = 0;
		pOver->InternalHigh = 0;
		pOver->hEvent = pRAS->hSemaphore;

		return dwBytes;
	}

	pRAS->hSemaphore = CreateSemaphore(NULL, 0, 1, cSemName);

	OVERLAPPED* pOver = new OVERLAPPED;
	pRAS->pOverlapped = pOver;

	__int64 j = GetPos(); DWORD* dwPos = (DWORD*)&j;
	pOver->Offset = dwPos[0];
	pOver->OffsetHigh = dwPos[1];
	pOver->Internal = 0;
	pOver->InternalHigh = 0;
	pOver->hEvent = pRAS->hSemaphore;

	int rfe_res = ReadFileEx(hFile, pRAS->lpDest, dwBytes, pOver, ReadCompletionRoutine);
	iCurrPos += dwBytes;

	return rfe_res;
}

int FILESTREAM::WaitForCompletion(READ_ASYNC_STRUCT* pRAS)
{
	while (WAIT_IO_COMPLETION == WaitForSingleObjectEx(pRAS->hSemaphore, INFINITE, true));
	
	return 1;
}

int FILESTREAM::Read(void* lpDest,DWORD dwBytes)
{
	DWORD	dwRead,dwTotal;
	BYTE*	lpbDest=(BYTE*)lpDest;

//	if (!(GetMode() & STREAM_READ)) return STREAM_ERR;
	dwTotal=0;
	dwRead=1;

	if (!bBuffered) {
		iPossibleAlignedReadCount = 0;
		if (!ispowof2(dwBytes)) {
			 bBuffered = 1;
			 __int64 iPos = GetPos();
			 Seek(0);
			 Open(cFilename, STREAM_READ);
			 Seek(iPos);
	//		printf("changing to buffered read\n");
			 return Read(lpDest, dwBytes);
		} else {
			if (iAlignedBufferSize-align < (int)dwBytes) {
				if (pAlignedInputBufferAllocated) {
					delete pAlignedInputBufferAllocated;
				}
				int s = dwBytes;
				iAlignedBufferSize = s+align+16;
				pAlignedInputBufferAllocated=calloc(1, iAlignedBufferSize);
				pAlignedInputBuffer = (void*)( ((unsigned)(pAlignedInputBufferAllocated) & ~(align-1)) + align);
			}

			BYTE* lpbDest2 = (BYTE*)pAlignedInputBuffer;
			__int64 k = GetPos();

			if (!bOverlapped) {
				while ((dwTotal<dwBytes)&&(dwRead))
				{	
					DWORD dwBytesToRead = dwBytes;
					ReadFile(hFile,lpbDest2,dwBytesToRead,&dwRead,NULL);
					if (dwRead) {
						memcpy(&lpbDest[dwTotal], lpbDest2, dwRead);
						dwTotal+=dwRead;
						iCurrPos += dwRead;
					}
				} 
			} else {
				__int64 j = GetPos(); DWORD* dwPos = (DWORD*)&j;
				OVERLAPPED* over = new OVERLAPPED;
				HANDLE h;
				DWORD dwBytesToRead = dwBytes;
				over->Offset = dwPos[0];
				over->OffsetHigh = dwPos[1];
				over->Internal = 0;
				over->InternalHigh = 0;
				char cReadSemaphoreName[10]; cReadSemaphoreName[9] = 0;
				for (int i=0;i<9;i++) {
					cReadSemaphoreName[i] = 65+rand()%26;
				}
				over->hEvent = h = CreateSemaphore(NULL,0, 1, cReadSemaphoreName);
				if (ReadFileEx(hFile, lpbDest2, dwBytesToRead, over, ReadCompletionRoutine)) {
					while (WaitForSingleObjectEx(h, INFINITE, true) == WAIT_IO_COMPLETION);
					CloseHandle(h);
					dwTotal = over->Offset;
					memcpy(lpbDest, lpbDest2, dwTotal);
				} else {
					dwTotal = 0;
				}

				iCurrPos += dwTotal;
				delete over;
			}
		}
	} else {
		Flush();
		if (!bOverlapped) {
			while ((dwTotal<dwBytes)&&(dwRead))
			{	
				ReadFile(hFile,&(lpbDest[dwTotal]),min(dwBytes-dwTotal,131072),&dwRead,NULL);
				dwTotal+=dwRead;
				iCurrPos += dwRead;
			}
		} else {
				__int64 j = GetPos(); DWORD* dwPos = (DWORD*)&j;
				OVERLAPPED* over = new OVERLAPPED;
				HANDLE h;
				DWORD dwBytesToRead = dwBytes;
				over->Offset = dwPos[0];
				over->OffsetHigh = dwPos[1];
				over->Internal = 0;
				over->InternalHigh = 0;
				char cReadSemaphoreName[10]; cReadSemaphoreName[9] = 0;
				for (int i=0;i<9;i++) {
					cReadSemaphoreName[i] = 65+rand()%26;
				}

				over->hEvent = h = CreateSemaphore(NULL,0, 1, cReadSemaphoreName);
				if (ReadFileEx(hFile, lpbDest, dwBytesToRead, over, ReadCompletionRoutine)) {
					while (WaitForSingleObjectEx(h, INFINITE, true) == WAIT_IO_COMPLETION);
					CloseHandle(h);
					dwTotal = over->Offset;
				} else {
					dwTotal = 0;
				}


				iCurrPos += dwTotal;
				delete over;
		}
		if (!bDenyUnbufferedRead) {
			if (!(GetPos() & alignment_mask) & !(dwBytes & alignment_mask)) {
				if (iPossibleAlignedReadCount++ > 10) {
					__int64 j = GetPos();
					Close();
					bBuffered = 0;
					Seek(0);
			//		printf("changing to unbuffered read\n");
					Open(cFilename, STREAM_READ | ((bOverlapped)?STREAM_OVERLAPPED:0));
					Seek(j);
				}
			} else {
				iPossibleAlignedReadCount = 0;
			}
		}
	}

//	iCurrPos += dwTotal;
	return dwTotal;
}

void FILESTREAM::Flush()
{
	int iWritten;

	if (bCanWrite && bBuffered) {
		if (iOutCachePos) {
			iWritten = Write2Disk(cOutCache, (int)iOutCachePos);
			iOutCachePos = 0;
			cOutCacheCurr = cOutCache;
			iCurrPos += iWritten;
		}
	}
}

int FILESTREAM::Write2Disk(void* lpSource, DWORD dwBytes)
{
	DWORD	dwWritten;

	if (!bOverlapped) {
		WriteFile(hFile,lpSource,dwBytes,&dwWritten,NULL);
		return dwWritten;
	} else {
		__int64 j = iCurrPos; DWORD* dwPos = (DWORD*)&j;
		OVERLAPPED* over = new OVERLAPPED;
		OVERLAPPED_ADDITIONAL* over_add = new OVERLAPPED_ADDITIONAL;
			
		over_add->hSemaphore = hWriteSemaphore;
		over_add->lpData = new char[dwBytes + align];
		void* pAlignedData = (void*)( ((unsigned)(over_add->lpData) & ~(align-1)) + align);

		memcpy(pAlignedData, lpSource, dwBytes);

		over->hEvent = (HANDLE)over_add;
		over->Offset = dwPos[0];
		over->OffsetHigh = dwPos[1];
		over->Internal = 0;
		over->InternalHigh = 0;
			
		while (WAIT_IO_COMPLETION == WaitForSingleObjectEx(hWriteSemaphore, INFINITE, true));
		int wfe_res = WriteFileEx(hFile, pAlignedData , dwBytes, over, WriteCompletionRoutine);

		return dwBytes;
	}
}

int FILESTREAM::Write(void* lpSource,DWORD dwBytes)
{
	DWORD	dwWritten;

	if (bBuffered) {
		if (!bCanWrite) return STREAM_ERR;
	
		if (iOutCachePos + dwBytes > iOutCacheSize) {
			Flush();
		}

		if (dwBytes >= iOutCacheSize) {
			dwWritten = Write2Disk(lpSource, dwBytes);
		} else {
			memcpy(cOutCacheCurr,lpSource,dwBytes);
			cOutCacheCurr += dwBytes;
			iOutCachePos += dwBytes;
		}
		
		iCurrentSize += dwBytes;
	} else {
		dwWritten = Write2Disk(lpSource, dwBytes);
		iCurrentSize += dwBytes;
		iCurrPos += dwBytes;
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
	if (!GetSource()) return 0;
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