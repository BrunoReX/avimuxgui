#include "stdafx.h"
#include "utf-8.h"
#include "windows.h"

#ifdef DEBUG_NEW
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif



int utf8_unicode_possible = 1;

char    cUTF8Hdr[] = { (char)0xEF, (char)0xBB, (char)0xBF, 0 };

void utf8_EnableRealUnicode(bool bEnabled)
{
	utf8_unicode_possible = bEnabled;
}

int utf8_IsUnicodeEnabled()
{
	return utf8_unicode_possible;
}
/*
int _WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, 
						 int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, 
						 LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar) {

	  int result = WideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, 
		  cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
	  return result;
}
*/
int _stdcall WStr2UTF8(const char* source, char** dest)
{
	int len = 1;

	if (source) 
		len = WStr2UTF8(source, NULL, 0);

	*dest = (char*)malloc(len);

	if (!source) {
		*dest = 0;
		return 1;
	}

	return WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)source, -1,
		*dest, len, NULL, NULL);
}

int _stdcall WStr2UTF8(const wchar_t* source, char** dest)
{
	return WStr2UTF8((char*)source, dest);
}

int _stdcall WStr2UTF8(const char* source, char* dest, int max_len)
{
	if (dest) {
		if (source!=dest) {
			return WideCharToMultiByte(CP_UTF8, 0, 
				(LPCWSTR)source, -1, dest, max_len, NULL,  NULL);
		} else {
			int dest_size = WStr2UTF8(source, NULL, 0);

			char* cTemp = NULL;
			WStr2UTF8(source, &cTemp);
			strcpy_s(dest, max_len, cTemp);
			free(cTemp); 

			return dest_size;
		}
	} else {
		return WideCharToMultiByte(CP_UTF8,0,(LPCWSTR)source,-1,NULL,0,NULL,NULL);
	}

	return 0;
}

int _stdcall WStr2UTF8(const wchar_t* source, char* dest, int max_len)
{
	return WStr2UTF8((char*)source, dest, max_len);
}


int  _stdcall WStr2Str(const char* source, char* dest, int max_len)
{
	int len = WideCharToMultiByte(CP_THREAD_ACP, 0, (LPCWSTR)source, -1,
		(LPSTR)dest, max_len, NULL, NULL);

	return len;
}

int  _stdcall WStr2Str(const char* source, char** dest)
{
	int len = 1;
	if (source)
		len = WideCharToMultiByte(CP_THREAD_ACP,0,(LPCWSTR)source,-1,NULL,0,0,0);

	*dest = (char*)malloc(len);

	return WideCharToMultiByte(CP_THREAD_ACP, 0, (LPCWSTR)source,
		-1, *dest, len, 0, 0);
}

int _stdcall UTF82WStr(const char* source, char** dest)
{
	size_t source_len = strlen(source) + 1;
	int dest_len = 2;
	
	if (source)
		dest_len = 2 * MultiByteToWideChar(CP_UTF8, 0, source, -1, 0, 0);

	if (dest) {
		*dest = (char*)malloc(dest_len);
		return sizeof(wchar_t)*MultiByteToWideChar(CP_UTF8, 0, source, -1,
			(LPWSTR)*dest, dest_len / sizeof(wchar_t));
	} else {
		return sizeof(wchar_t)*MultiByteToWideChar(CP_UTF8, 0, source, -1, 0, 0);
	}
}

int _stdcall UTF82WStr(const char* source, char* dest, int max_len)
{
	int i;

	if (!source)
		return 0;

	size_t source_len = strlen(source) + 1;

	if (dest) {
		if (source!=dest) {
			return sizeof(wchar_t) * MultiByteToWideChar(CP_UTF8, 0, source, -1,
				(LPWSTR)dest, max_len / sizeof(wchar_t));
		} else {
			char* cTemp = (char*)malloc(UTF82WStr(source, NULL, 0));
			i = sizeof(wchar_t) * MultiByteToWideChar(CP_UTF8, 0, source, 
				-1, (LPWSTR)cTemp, max_len / sizeof(wchar_t));
			memcpy(dest, cTemp, i);
			free(cTemp);
			return i;
		}
	} else {
		return 2*MultiByteToWideChar(CP_UTF8,0,source,-1,0,0);
	}
}

int _stdcall Str2WStr(const char* source, char** dest)
{
	if (!source) {
		*dest = new char[2];
		memset(*dest, 0, 2);
		return 2;
	}

/*	if (*dest) 
		free(*dest);
*/	
	int dest_len = Str2WStr(source, NULL, 0);

	*dest = (char*)calloc(1, dest_len);

	return 2*MultiByteToWideChar(CP_THREAD_ACP,0,source,-1,(LPWSTR)*dest,dest_len/2);
}

int _stdcall Str2WStr(const char* source, char* dest, int max_len)
{
	if (!source) {
		memset(dest, 0, 2);
		return 2;
	}

	size_t source_len = 1 + strlen(source);

	if (source!=dest) {
		if (!dest)
			return 2 * MultiByteToWideChar(CP_THREAD_ACP, 0, source, -1, NULL, 0);

		return 2*MultiByteToWideChar(CP_THREAD_ACP,0,source,-1,(LPWSTR)dest,max_len/2);
	} else {
		char* cTemp = new char[2 * source_len];
		int i = 2*MultiByteToWideChar(CP_THREAD_ACP,0,source,-1,(LPWSTR)cTemp,max_len/2);
		memcpy(dest, cTemp, i);
		delete[] cTemp;

		return i;
	}
}

int _stdcall UTF82Str(const char* source, char** dest)
{
	if (!dest) {
		return -1;
	}

	if (!source) {
		*dest = (char*)calloc(1, 1);	
		return 1;
	}

	unsigned short* temp = NULL;
	
	if (utf8_unicode_possible) {
		UTF82WStr(source,(char**)&temp);
		int dest_len = WideCharToMultiByte(CP_THREAD_ACP,0,(LPCWSTR)temp,-1,0,0,0,0);

		if (dest) {
			*dest = (char*)calloc(1, dest_len);
			int r = WideCharToMultiByte(CP_THREAD_ACP,0,(LPCWSTR)temp,-1,*dest,dest_len,0,0);
			free(temp);
			return r;
		} else {
			int r = WideCharToMultiByte(CP_THREAD_ACP,0,(LPCWSTR)temp,-1,0,0,0,0);
			free(temp);
			return r;
		}
	} else {
/*		*dest = (char*)calloc(1, strlen(source)+1);
		strcpy(*dest, source);
		return (int)strlen(source)+1;*/
		*dest = _strdup(source);
		return (int)strlen(*dest)+1;
	}
}

int _stdcall UTF82Str(const char* source, char* dest, int max_len)
{
	int i;

	if (!source) {
		if (dest)
			*dest = 0;
		return 1;
	}

	unsigned short* temp = NULL;
	
	if (utf8_unicode_possible) {
		UTF82WStr(source, (char**)&temp);
		if (dest) {
			i = WideCharToMultiByte(CP_THREAD_ACP,0,(LPCWSTR)temp,-1,dest,max_len,0,0);
			delete[] temp;
			return i;
		} else {
			i = WideCharToMultiByte(CP_THREAD_ACP,0,(LPCWSTR)temp,-1,0,0,0,0);
			delete[] temp;
			return i;
		}
	} else {
		delete[] temp;
		if (dest) 
			strcpy_s(dest, max_len, source);
		
		return (int)strlen(source);
	}
}

int _stdcall Str2UTF8(const char* source, char* dest, int max_len)
{
	if (!source) {
		*dest = 0;
		return 1;
	}

	if (max_len < 0)
		return 0;
	
	int temp_size;
	size_t source_len = strlen(source) + 1;
	if (utf8_unicode_possible) {
		temp_size = Str2WStr(source, (char*)NULL);
	} else {
		temp_size = 1+(int)strlen(source);
	}
	int i;
	
	unsigned short* temp = new unsigned short[temp_size];

	if (utf8_unicode_possible) {
		ZeroMemory(temp,sizeof(unsigned short) * temp_size);

		if (dest) {
			MultiByteToWideChar(CP_THREAD_ACP,0,source,-1,(LPWSTR)temp,temp_size);
			i = WideCharToMultiByte(CP_UTF8,0,(LPCWSTR)temp,-1,dest,max_len,0,0);
			delete[] temp;
			return i;
		} else {
			MultiByteToWideChar(CP_THREAD_ACP,0,source,-1,(LPWSTR)temp,temp_size);
			i = WideCharToMultiByte(CP_UTF8,0,(LPCWSTR)temp,-1,0,0,0,0);
			delete[] temp;
			return i;
		}
	} else {
		delete[] temp;
		if (dest) {
			if ((int)source_len < max_len) 
				strcpy_s(dest, max_len, source);
			else {
				strncpy_s(dest, max_len, source, max_len);
				dest[(int)max_len-1] = 0;
			}
		}
		return 1+(int)strlen(source);
	}

}

int _stdcall Str2UTF8(const char* source, char** dest)
{
	if (!dest)
		return -1;

/*	if (*dest)
		free(*dest);
*/
	if (!source) {
		*dest = (char*)calloc(1, 1);
		return 1;
	}

	if (utf8_unicode_possible) {
		unsigned short* temp = NULL;
		Str2WStr(source, (char**)&temp);
		int result = WStr2UTF8((char*)temp, dest);
		free(temp);
		return result;
	} else {
/*		*dest = (char*)calloc(1, 1+strlen(source));
		strcpy(*dest, source);*/
		*dest = _strdup(source);
		return (int)(1+strlen(source));
	}
}

int StringConvert(const char* source, CharacterEncoding::CharacterEncodings source_format,/* int max_source_len, */
				  char** dest, CharacterEncoding::CharacterEncodings dest_format)
{
/*	__ASSERT(source_format == CHARACTER_ENCODING_ANSI ||
		source_format == CHARACTER_ENCODING_UTF16_LE ||
		source_format == CHARACTER_ENCODING_UTF8,
		"bad source_format");

//	printf(__FILE__);

	__ASSERT(dest_format == CHARACTER_ENCODING_ANSI ||
		dest_format == CHARACTER_ENCODING_UTF16_LE ||
		dest_format == CHARACTER_ENCODING_UTF8,
		"bad dest_format");
*/
	char* _source = (char*)source;

//	char* _source = (char*)malloc(2*max_source_len+2);

/*	if (source_format == CHARACTER_ENCODING_UTF16_LE)
		wcsncpy((wchar_t*)_source, (wchar_t*)source, max_source_len);
	else
		strncpy(_source, source, max_source_len);
*/
	switch (source_format)
	{
		case CharacterEncoding::ANSI:
			switch (dest_format) {
				case CharacterEncoding::ANSI: *dest = _strdup(_source); break;
				case CharacterEncoding::UTF8: Str2UTF8(_source, dest); break;
				case CharacterEncoding::UTF16LE: Str2WStr(_source, dest); break;
			}
			break;
		case CharacterEncoding::UTF8:
			switch (dest_format) {
				case CharacterEncoding::ANSI: UTF82Str(_source, dest); break;
				case CharacterEncoding::UTF8: *dest = _strdup(_source); break;
				case CharacterEncoding::UTF16LE: UTF82WStr(_source, dest); break;
			}
			break;
		case CharacterEncoding::UTF16LE:
//		case CHARACTER_ENCODING_UTF16_BE:
			switch (dest_format) {
				case CharacterEncoding::ANSI: 
					WStr2Str(_source, dest); 
					break;
				case CharacterEncoding::UTF8: 
					WStr2UTF8(_source, dest); 
					break;
				case CharacterEncoding::UTF16LE: 
					*dest = (char*)_wcsdup((wchar_t*)_source); 
					break;
			}
			break;
	}
//	free(_source);

	return 1;
}

int FromUTF8(const char* source, wchar_t** dest)
{
	return StringConvert(source, CharacterEncoding::UTF8,
		(char**)dest, CharacterEncoding::UTF16LE);
}

int FromUTF8(const char* source, char** dest)
{
	return StringConvert(source, CharacterEncoding::UTF8,
		(char**)dest, CharacterEncoding::ANSI);
}

int IsUTF8(const char* src, size_t max_source_len)
{
	if (max_source_len < 0)
		return 0;

	if (max_source_len == 0)
		return 1;

	while (*src && max_source_len--)
	{
		/* get number of bytes for next character */
		int bytes = UTF8CharLen(*src++);

		/* if the 1st character is not a valid 1st character for UTF-8 */
		if (bytes < 0)
			return 0;
	
		/* if there are fewer bytes left than this character requires in
		   UTF-8, it cannot be UTF-8 */
		if (static_cast<int>(max_source_len) < --bytes)
			return 0;

		/* the next $bytes bytes must be 10xx xxxx */
		while (bytes--) {
			if ((*src++ & 0xC0) != 0x80)
				return 0;
		}
	}

	return 1;
}

int UTF8CharLen(char in)
{
	unsigned char uin = (unsigned char)in;

	if (uin < 128)
		return 1;

	if (uin < 192)
		return -1;

	if (uin < 0xE0)
		return 2;

	if (uin < 0xF0)
		return 3;

	if (uin < 0xF8)
		return 4;

	if (uin < 0xFC)
		return 5;

	if (uin < 0xFE)
		return 6;

	if (uin < 0xFF)
		return 7;

	return 8;
}

int ToUTF8(const char* source, char** dest)
{
	return StringConvert(source, CharacterEncoding::ANSI,
		(char**)dest, CharacterEncoding::UTF8);
}

int ToUTF8(const wchar_t* source, char** dest)
{
	return StringConvert((char*)source, CharacterEncoding::UTF16LE,
		(char**)dest, CharacterEncoding::UTF8);
}


CUTF8::~CUTF8()
{
}

CUTF8::CUTF8(const char *p, int Encoding)
{
	if (p)
	{
		if (Encoding == CharacterEncoding::UTF8/* || IsUTF8(p, strlen(p))*/)
		{
			m_utf8 = p;
		}
		else
		{
			m_ascii = p;
		}

		Complete();
	}
}

CUTF8::CUTF8(const char* pSrc)
{
	if (pSrc)
	{
		if (IsUTF8(pSrc, strlen(pSrc)))
		{
			m_utf8 = pSrc;
		}
		else
		{
			m_ascii = pSrc;
		}

		Complete();
	}
}

CUTF8::CUTF8(const std::string& src)
{
	if (IsUTF8(src.c_str(), src.size()))
	{
		m_utf8 = src;
	}
	else
	{
		m_ascii = src;
	}

	Complete();
}

CUTF8::CUTF8(const CUTF8& other) {
	m_utf8 = other.m_utf8;
	Complete();
}

CUTF8::CUTF8(const wchar_t* pSrc)
{
	if (pSrc)
	{
		m_unicode = pSrc;
		Complete();
	}
}

CUTF8::CUTF8(std::wstring src)
{
	m_unicode = src;
	Complete();
}

void CUTF8::Complete()
{
	char* p = NULL;

	if (!m_ascii.empty())
	{
		Str2UTF8(m_ascii.c_str(), &p);
		m_utf8 = p;
		free(p);

		Str2WStr(m_ascii.c_str(), &p);
		m_unicode = (wchar_t*)p;
		free(p);
	}
	else
	{
		if (!m_utf8.empty())
		{
			UTF82Str((char*)m_utf8.c_str(), &p);
			m_ascii = p;
			free(p);

			UTF82WStr((char*)m_utf8.c_str(), &p);
			m_unicode = (wchar_t*)p;
			free(p);
		}
		else
		{
			if (!m_unicode.empty())
			{
				WStr2Str((char*)m_unicode.c_str(), &p);
				m_ascii = p;
				free(p);

				WStr2UTF8((char*)m_unicode.c_str(), &p);
				m_utf8 = p;
				free(p);
			}
		}
	}
}