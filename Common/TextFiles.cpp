#include "stdafx.h"
#include "Stream.h"
#include "TextFiles.h"
#include "utf-8.h"
#include <vector>

CTextFile::CTextFile()
{
	iCharCoding = CharacterEncoding::Undefined;
	iHdrSize = 0;
	source=NULL;
}

CTextFile::CTextFile(StreamMode::StreamModes _dwMode, STREAM* s, CharacterEncoding::CharacterEncodings iOutputFormat)
{
	iCharCoding = CharacterEncoding::Undefined;
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

int CTextFile::Open(StreamMode::StreamModes _dwMode, STREAM* s)
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
				iCharCoding = CharacterEncoding::UTF8;	
				iHdrSize = 3;
			}
		} else
		if (c == 0xFF) {
			if (source->Read(&c,1) && c == 0xFE) {
				iCharCoding = CharacterEncoding::UTF16LE;
				iHdrSize = 2;
			}
		} else
		if (c == 0xFE) {
			if (source->Read(&c, 1) && c == 0xFF) {
				iCharCoding = CharacterEncoding::UTF16BE;
				iHdrSize = 2;
			}
		}
		else {
			// Change default encoding to UTF-8 in order to allow for BOMless UTF-8 text files.
			// This should work because the line reader verifies each line if it really is UTF-8
			// and uses ANSI if not
			iCharCoding = CharacterEncoding::UTF8;
			iHdrSize = 0;
		}

	}
	Seek(0);

	return STREAM_OK;
}

int CTextFile::ReOpen()
{
	iCharCoding = CharacterEncoding::Undefined;
	iHdrSize = 0;
	return Open(StreamMode::Read,source);
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
	return (iCharCoding == CharacterEncoding::UTF8);
}

bool CTextFile::IsUTF8Out()
{
	return (iOutputCoding == CharacterEncoding::UTF8);
}

bool CTextFile::IsAnsiIn()
{
	return (iCharCoding == CharacterEncoding::ANSI);
}

bool CTextFile::IsAnsiOut()
{
	return (iOutputCoding == CharacterEncoding::ANSI);
}

bool CTextFile::IsUTF16In()
{
	return (iCharCoding == CharacterEncoding::UTF16LE ||
		iCharCoding == CharacterEncoding::UTF16BE);
}

bool CTextFile::IsUTF16Out()
{
	return (iOutputCoding == CharacterEncoding::UTF16LE);
}

CharacterEncoding::CharacterEncodings CTextFile::GetFileInputEncoding()
{
	return iCharCoding;
}

CharacterEncoding::CharacterEncodings CTextFile::GetInputEncoding()
{
	if (iCharCoding != CharacterEncoding::UTF16BE)
		return GetFileInputEncoding();
	else
		return CharacterEncoding::UTF16LE;
}

CharacterEncoding::CharacterEncodings CTextFile::GetOutputEncoding()
{
	return iOutputCoding;
}

int CTextFile::Read(void* d, int iBytes)
{
	return source->Read(d,iBytes);
}

#if FALSE
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
		if (GetOutputEncoding() == CharacterEncoding::UTF16LE) {
			d[min(size_read-1, max_len-2)] = 0;
			d[min(size_read-2, max_len-3)] = 0;
		}
	} else {
		memset(d, 0, max_len);
	}

	free(pTemp);

	return (min(max_len-1, size_read));
}
#endif

int CTextFile::ReadLine(std::wstring& result)
{
	if (source->IsEndOfStream())
		return -1;

	CharacterEncoding::CharacterEncodings inputFileEncoding = GetFileInputEncoding();
	if (inputFileEncoding == CharacterEncoding::UTF16LE ||
		inputFileEncoding == CharacterEncoding::UTF16BE)
	{
		// input and output character encoding is UTF-16
		std::wstring line;

		do
		{
			wchar_t nextCharacter;
			if (source->Read(&nextCharacter, sizeof(wchar_t)) != sizeof(wchar_t))
			{
				break;
			}
			else
			{
				// character read successfully
				if (inputFileEncoding == CharacterEncoding::UTF16BE)
				{
					nextCharacter = ((nextCharacter >> 8) | (nextCharacter << 8)) & 0xFFFF;
				}
			}

			if (nextCharacter == 0x0A)
				break;

			if (nextCharacter != 0x0A && nextCharacter != 0x0D)
				line.push_back(nextCharacter);
			
		} while (true);

		result = line;
		return static_cast<int>(result.size());
	}
	else
	{
		std::string line;

		// input is ANSI or UTF-8, output is UTF-16
		do
		{
			char nextCharacter;
			if (source->Read(&nextCharacter, sizeof(char)) != sizeof(char))
			{
				break;
			}

			if (nextCharacter == 0x0A)
				break;

			if (nextCharacter != 0x0A && nextCharacter != 0x0D)
				line.push_back(nextCharacter);

		} while (true);

		if (inputFileEncoding == CharacterEncoding::UTF8) 
		{
			if (IsUTF8(line.c_str(), line.size())) {
				CUTF8 temp(line.c_str(), CharacterEncoding::UTF8);
				result = temp;
			} else {
				CUTF8 temp(line.c_str(), CharacterEncoding::ANSI);
				result = temp;
			}
		} 

		if (inputFileEncoding == CharacterEncoding::ANSI)
		{
			CUTF8 temp(line.c_str(), CharacterEncoding::ANSI);
			result = temp;
		}

		return static_cast<int>(result.size());
	}

	return 0;
}

int CTextFile::ReadLine(std::string& result)
{
	if (source->IsEndOfStream())
		return -1;

	CharacterEncoding::CharacterEncodings inputFileEncoding = GetFileInputEncoding();
	if (inputFileEncoding == CharacterEncoding::UTF16LE ||
		inputFileEncoding == CharacterEncoding::UTF16BE)
	{
		std::wstring line;
		int readLineResult = ReadLine(line);
		if (readLineResult < 0)
			return readLineResult;

		CUTF8 utf8Line(line.c_str());
		if (GetOutputEncoding() == CharacterEncoding::ANSI)
			result = utf8Line.Str();
		if (GetOutputEncoding() == CharacterEncoding::UTF8)
			result = utf8Line.UTF8();

		return static_cast<int>(result.size());
	}
	else
	{
		std::string line;

		// input is ANSI or UTF-8, output is UTF-16
		do
		{
			char nextCharacter;
			if (source->Read(&nextCharacter, sizeof(char)) != sizeof(char))
			{
				break;
			}

			if (nextCharacter == 0x0A)
				break;

			if (nextCharacter != 0x0A && nextCharacter != 0x0D)
				line.push_back(nextCharacter);

		} while (true);

		if (inputFileEncoding == CharacterEncoding::UTF8) 
		{
			if (IsUTF8(line.c_str(), line.size())) {
				CUTF8 temp(line.c_str(), CharacterEncoding::UTF8);
				if (IsUTF8Out())
					result = temp.UTF8();
				else
					result = temp.Str();

			} else {
				CUTF8 temp(line.c_str(), CharacterEncoding::ANSI);
				if (IsUTF8Out())
					result = temp.UTF8();
				else
					result = temp.Str();
			}

			return static_cast<int>(result.size());
		} 

		if (inputFileEncoding == CharacterEncoding::ANSI)
		{
			CUTF8 temp(line.c_str(), CharacterEncoding::ANSI);
			if (IsUTF8Out())
				result = temp.UTF8();
			else
				result = temp.Str();

			return static_cast<int>(result.size());
		}
	}

	return 0;
}

#if FALSE
int CTextFile::ReadLine(char** d)
{
	int j = 0;
	int read = 0;

	if (GetInputEncoding() == CharacterEncoding::UTF16LE) {
		wchar_t c;
		wchar_t pInputBuffer[4096];

		memset(pInputBuffer, 0, sizeof(pInputBuffer));

		do {
            read = source->Read(&c, 2);

			if (GetFileInputEncoding() == CharacterEncoding::UTF16BE)
				c = (c >> 8) & 0xFF + 256 * (c & 0xFF); 

			if (read > 0 && c != 0x0A && c != 0x0D)
				pInputBuffer[j++] = c;

		} while (c != 0x0A && j < 4095 && read > 0);

		StringConvert((char*)pInputBuffer, GetInputEncoding(),  
			d, GetOutputEncoding());
	}

	if (GetFileInputEncoding() == CharacterEncoding::ANSI ||
		GetFileInputEncoding() == CharacterEncoding::UTF8)
	{
		char c;
		char pInputBuffer[4096];

		memset(pInputBuffer, 0, sizeof(pInputBuffer));

		do {
			read = source->Read(&c, 1);
			if (read > 0 && c != 0x0A && c != 0x0D)
				pInputBuffer[j++] = c;

		} while (c != 0x0A && j < 4095 && read > 0);

		StringConvert((char*)pInputBuffer, GetInputEncoding(),  
			d, GetOutputEncoding());
	}

	/* if no character was read from this line */
	if (!j || !*d) {
		if (!d) {
			*d = new char[2];
			memset(*d, 0, 2);
		}

		/* return -1 if the end was encountered or 0 otherwise */
		if (source->IsEndOfStream())
			return -1;
		else
			return 0;
	}

	if (GetOutputEncoding() == CharacterEncoding::UTF16LE)
		return 2*(int)wcslen((wchar_t*)*d);
	else
		return (int)strlen(*d);
}
#endif


void CTextFile::SetOutputEncoding(CharacterEncoding::CharacterEncodings iFormat)
{
	iOutputCoding = iFormat;
}

void CTextFile::SetFileInputEncoding(CharacterEncoding::CharacterEncodings iFormat)
{
	iCharCoding = iFormat;
}

__int64 CTextFile::GetPos()
{
	return source->GetPos() - iHdrSize - GetOffset();
}

int CTextFile::ReadInputChar(unsigned char* pDest, size_t max_len)
{
	if (GetInputEncoding() == CharacterEncoding::ANSI) {
		if (max_len < 1)
			return 0;

		return Read(pDest, 1);
	}

	if (GetInputEncoding() == CharacterEncoding::UTF16LE) {
		if (max_len < 2)
			return 0;

		return Read(pDest, 2);
	}

	if (GetInputEncoding() == CharacterEncoding::UTF8) {
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

	// unknown input encoding!?
	return -1;
}

template<>
char CTextFile::ReadChar<char>(int flags)
{
	unsigned char buf[8]; memset(buf, 0, sizeof(buf));
	int charsRead = ReadInputChar(buf, sizeof(buf)-1);

	char* out = NULL;
	StringConvert((const char*)buf, GetInputEncoding(), &out,
		CharacterEncoding::ANSI);

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
	StringConvert((const char*)buf, GetInputEncoding(), &out, 
		CharacterEncoding::UTF16LE);

	wchar_t result = out[0];
	free(out);

	return result;
}