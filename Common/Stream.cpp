#include "stdafx.h"
#include "Stream.h"

#ifdef DEBUG_NEW
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif




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

CSimpleFileStream::CSimpleFileStream(HANDLE hFile)
{
	m_fileHandle = hFile;
	m_offset = NULL;
}

CSimpleFileStream* CSimpleFileStream::FromHandle(HANDLE hFile)
{
	CSimpleFileStream* result = new CSimpleFileStream(hFile);

	return result;
}

int CSimpleFileStream::Read(void *lpDest, DWORD dwBytes)
{
	DWORD dwRead = 0;
	ReadFile(m_fileHandle, lpDest, dwBytes, &dwRead, NULL);

	return dwRead;
}

bool CSimpleFileStream::IsEndOfStream()
{
	__int64 currentPos;
	GetFilePointer64(m_fileHandle, &currentPos);

	__int64 size;
	GetFileSize64(m_fileHandle, &size);

	return (currentPos == size);
}

__int64 CSimpleFileStream::GetSize()
{
	__int64 result;
	GetFileSize64(m_fileHandle, &result);

	return result;
}

int CSimpleFileStream::Seek(__int64 pos)
{
	SetFilePointer64(m_fileHandle, pos + m_offset);

	return 1;
}

__int64 CSimpleFileStream::GetPos()
{
	__int64 currentPos;
	GetFilePointer64(m_fileHandle, &currentPos);

	return currentPos - m_offset; 
}

CMemoryStream::CMemoryStream()
{
	m_currentPosition = 0;
}

CMemoryStream* CMemoryStream::Create()
{
	CMemoryStream* result = new CMemoryStream();
	return result;
}

CMemoryStream* CMemoryStream::Create(const std::vector<char>& data)
{
	CMemoryStream* result = new CMemoryStream();
	result->m_data = data;

	return result;
}

CMemoryStream* CMemoryStream::Create(const char* pData, unsigned int size)
{
	CMemoryStream* result = new CMemoryStream();
	result->m_data.assign(pData, pData+size);

	return result;
}

int CMemoryStream::Open(StreamMode::StreamModes _dwMode)
{
	STREAM::Open(_dwMode);

	return STREAM_OK;
}

__int64 CMemoryStream::GetPos()
{
	return m_currentPosition;
}

int CMemoryStream::Read(void *lpDest, DWORD dwBytes)
{
	if (IsEndOfStream())
		return 0;

	DWORD dwMaxBytes = static_cast<DWORD>(GetSize() - GetPos());
	DWORD dwBytesToCopy = min(dwBytes, dwMaxBytes);
	memcpy(lpDest, &m_data[static_cast<int>(m_currentPosition)], dwBytesToCopy);
	m_currentPosition += dwBytesToCopy;
	return dwBytesToCopy;
}

__int64 CMemoryStream::GetSize()
{
	return m_data.size();
}

int CMemoryStream::Seek(__int64 pos)
{
	if (pos >= GetSize())
		pos = GetSize();

	if (pos < 0)
		pos = 0;

	m_currentPosition = pos;
	return STREAM_OK;
}

bool CMemoryStream::IsEndOfStream()
{
	return (m_currentPosition >= GetSize());
}

StreamMode::StreamModes operator| (StreamMode::StreamModes first, StreamMode::StreamModes second)
{
	return static_cast<StreamMode::StreamModes>(static_cast<unsigned int>(first) | static_cast<unsigned int>(second));
}

StreamMode::StreamModes operator~ (StreamMode::StreamModes first)
{
	return static_cast<StreamMode::StreamModes>(~ static_cast<unsigned int>(first));
}

StreamMode::StreamModes operator& (StreamMode::StreamModes first, StreamMode::StreamModes second)
{
	return static_cast<StreamMode::StreamModes>(static_cast<unsigned int>(first) & static_cast<unsigned int>(second));
}

StreamMode::StreamModes& operator|= (StreamMode::StreamModes& first, StreamMode::StreamModes second)
{
	first = static_cast<StreamMode::StreamModes>(static_cast<unsigned int>(first) | static_cast<unsigned int>(second));
	return first;
}

StreamMode::StreamModes& operator&= (StreamMode::StreamModes& first, StreamMode::StreamModes second)
{
	first = static_cast<StreamMode::StreamModes>(static_cast<unsigned int>(first) & static_cast<unsigned int>(second));
	return first;
}