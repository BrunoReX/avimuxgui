#include "stdafx.h"
#include "textfiles.h"
#include "..\utf-8.h"
#include "..\Assert.h"
#include <vector>

CTextFile::CTextFile()
{
	iCharCoding = 0;
	iHdrSize = 0;
	source=NULL;
}

CTextFile::CTextFile(DWORD _dwMode, STREAM* s, int iOutputFormat)
{
	iCharCoding = 0;
	iHdrSize = 0;
	Open(_dwMode, s);
	SetOutputEncoding(iOutputFormat);
}

CTextFile::~CTextFile()
{
	Close();
}

bool CTextFile::IsEndOfStream()
{
	return source->IsEndOfStream();
}

int CTextFile::Open(DWORD _dwMode, STREAM* s)
{
	STREAM::Open(_dwMode);
	source = s;
	Seek(0);

	while (!iCharCoding) {
		int c = 0;
		source->Read(&c,1);
		if (c == 0xEF) {
			if (source->Read(&c,1) && c == 0xBB && 
				source->Read(&c,1) && c == 0xBF) 
			{
				iCharCoding = CHARACTER_ENCODING_UTF8;	
				iHdrSize = 3;
			}
		} else
		if (c == 0xFF) {
			if (source->Read(&c,1) && c == 0xFE) {
				iCharCoding = CHARACTER_ENCODING_UTF16_LE;
				iHdrSize = 2;
			}
		} else
		if (c == 0xFE) {
			if (source->Read(&c, 1) && c == 0xFF) {
				iCharCoding = CHARACTER_ENCODING_UTF16_BE;
				iHdrSize = 2;
			}
		}
		else {
			iCharCoding = CHARACTER_ENCODING_ANSI;
			iHdrSize = 0;
		}

	}
	Seek(0);

	return STREAM_OK;
}

int CTextFile::ReOpen()
{
	iCharCoding = 0;
	iHdrSize = 0;
	return Open(STREAM_READ,source);
}

int CTextFile::Seek(__int64 qwPos)
{
	return source->Seek(qwPos + iHdrSize + GetOffset());
}

__int64 CTextFile::GetSize()
{
	return source->GetSize() - iHdrSize;
}

bool CTextFile::IsUTF8In()
{
	return (iCharCoding == CHARACTER_ENCODING_UTF8);
}

bool CTextFile::IsUTF8Out()
{
	return (iOutputCoding == CHARACTER_ENCODING_UTF8);
}

bool CTextFile::IsAnsiIn()
{
	return (iCharCoding == CHARACTER_ENCODING_ANSI);
}

bool CTextFile::IsAnsiOut()
{
	return (iOutputCoding == CHARACTER_ENCODING_ANSI);
}

bool CTextFile::IsUTF16In()
{
	return (iCharCoding == CHARACTER_ENCODING_UTF16_LE ||
		iCharCoding == CHARACTER_ENCODING_UTF16_BE);
}

bool CTextFile::IsUTF16Out()
{
	return (iOutputCoding == CHARACTER_ENCODING_UTF16_LE);
}

int CTextFile::GetFileInputEncoding()
{
	return iCharCoding;
}

int CTextFile::GetInputEncoding()
{
	if (iCharCoding != CHARACTER_ENCODING_UTF16_BE)
		return GetFileInputEncoding();
	else
		return CHARACTER_ENCODING_UTF16_LE;
}

int CTextFile::GetOutputEncoding()
{
	return iOutputCoding;
}

int CTextFile::Read(void* d, int iBytes)
{
	return source->Read(d,iBytes);
}

int CTextFile::ReadLine(char* d)
{
	return ReadLine(d, 1024);
}

int CTextFile::ReadLine(char* d, int max_len)
{
	char* pTemp = NULL;
	
	int size_read = ReadLine(&pTemp);

	if (size_read < 0) {
		free(pTemp);
		return -1;
	}
	
	if (size_read < max_len) {
		memcpy(d, pTemp, size_read);
	} else {
		memcpy(d, pTemp, max_len);
	}

	if (max_len > 0) {
		d[min(size_read, max_len-1)] = 0;
	} else {
		return (!size_read?-1:0);
	}

	if (max_len > 3) {
		if (GetOutputEncoding() == CHARACTER_ENCODING_UTF16_LE) {
			d[min(size_read-1, max_len-2)] = 0;
			d[min(size_read-2, max_len-3)] = 0;
		}
	} else {
		memset(d, 0, max_len);
	}

	free(pTemp);

	return (min(max_len-1, size_read));
}

int CTextFile::ReadLine(char** d)
{
	int j = 0;
	int read = 0;

	if (GetInputEncoding() == CHARACTER_ENCODING_UTF16_LE) {
		wchar_t c;
		wchar_t pInputBuffer[4096];

		memset(pInputBuffer, 0, sizeof(pInputBuffer));

		do {
            read = source->Read(&c, 2);

			if (GetFileInputEncoding() == CHARACTER_ENCODING_UTF16_BE)
				c = (c >> 8) & 0xFF + 256 * (c & 0xFF); 

			if (read > 0 && c != 0x0A && c != 0x0D)
				pInputBuffer[j++] = c;

		} while (c != 0x0A && j < 4095 && read > 0);

		StringConvert((char*)pInputBuffer, GetInputEncoding(), j+1, 
			d, GetOutputEncoding());
	}

	if (GetFileInputEncoding() == CHARACTER_ENCODING_ANSI ||
		GetFileInputEncoding() == CHARACTER_ENCODING_UTF8)
	{
		char c;
		char pInputBuffer[4096];

		memset(pInputBuffer, 0, sizeof(pInputBuffer));

		do {
			read = source->Read(&c, 1);
			if (read > 0 && c != 0x0A && c != 0x0D)
				pInputBuffer[j++] = c;

		} while (c != 0x0A && j < 4095 && read > 0);

		StringConvert((char*)pInputBuffer, GetInputEncoding(), j+1, 
			d, GetOutputEncoding());
	}

	/* if no character was read from this line */
	if (!j || !*d) {
		*d = new char[2];
		memset(*d, 0, 2);

		/* return -1 if the end was encountered or 0 otherwise */
		if (source->IsEndOfStream())
			return -1;
		else
			return 0;
	}

	if (GetOutputEncoding() == CHARACTER_ENCODING_UTF16_LE)
		return 2*(int)wcslen((wchar_t*)*d);
	else
		return (int)strlen(*d);
}



void CTextFile::SetOutputEncoding(int iFormat)
{
	__ASSERT(iFormat == CHARACTER_ENCODING_ANSI ||
		iFormat == CHARACTER_ENCODING_UTF16_LE ||
		iFormat == CHARACTER_ENCODING_UTF8,
		"bad format");

	iOutputCoding = iFormat;
}

void CTextFile::SetFileInputEncoding(int iFormat)
{
	__ASSERT(iFormat == CHARACTER_ENCODING_ANSI ||
		iFormat == CHARACTER_ENCODING_UTF16_LE ||
		iFormat == CHARACTER_ENCODING_UTF16_BE ||
		iFormat == CHARACTER_ENCODING_UTF8,
		"bad format");

	iCharCoding = iFormat;
}

__int64 CTextFile::GetPos()
{
	return source->GetPos() - iHdrSize - GetOffset();
}

int CTextFile::ReadInputChar(unsigned char* pDest, size_t max_len)
{
	if (GetInputEncoding() == CHARACTER_ENCODING_ANSI) {
		if (max_len < 1)
			return 0;

		return Read(pDest, 1);
	}

	if (GetInputEncoding() == CHARACTER_ENCODING_UTF16_LE) {
		if (max_len < 2)
			return 0;

		return Read(pDest, 2);
	}

	if (GetInputEncoding() == CHARACTER_ENCODING_UTF8) {
		char _1st;
		if (Read(&_1st, 1) != 1)
			return 0;
		
		int size = UTF8CharLen(_1st);
		if (size > static_cast<int>(max_len))
			return 0;

		*pDest++ = _1st;
		
		int read = Read(pDest, size-1);
		if (read != size-1)
			return 0;

		return 1+read;
	}

	throw std::logic_error("Bad input encoding for CTextFile::ReadInputChar");
}

template<>
char CTextFile::ReadChar<char>(int flags)
{
	unsigned char buf[8]; memset(buf, 0, sizeof(buf));
	int charsRead = ReadInputChar(buf, sizeof(buf)-1);

	char* out = NULL;
	StringConvert((const char*)buf, GetInputEncoding(), charsRead+1, &out,
		CHARACTER_ENCODING_ANSI);

	char result = out[0];
	free(out);

	return result;
}

template<>
wchar_t CTextFile::ReadChar<wchar_t>(int flags)
{
	unsigned char buf[8]; memset(buf, 0, sizeof(buf));
	int wcharsRead = ReadInputChar(buf, sizeof(buf)-1) / 2;

	char* out = NULL;
	StringConvert((const char*)buf, GetInputEncoding(), wcharsRead+1, &out, 
		CHARACTER_ENCODING_UTF16_LE);

	wchar_t result = out[0];
	free(out);

	return result;
}
/*
	char c = 0;
	__int64 pos = GetPos();

	if (flags == TFRC_ALL) {
		Read(&c, 1);
	} else
	if (flags & TFRC_NOLINEBREAKS) {
		while ((c == 0x00 || c == 0x0A || c == 0x0D) && !IsEndOfStream() )
			Read(&c, 1);

		if (IsEndOfStream())
			c = 0x00;
	}

	if (flags & TFRC_DONTUPDATEPOS)
		Seek(pos);

	return c;
}
*/