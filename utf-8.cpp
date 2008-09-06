#include "stdafx.h"
#include "utf-8.h"
#include "windows.h"
#include "memory.h"
#include "stdlib.h"
#include "stdio.h"
#include "assert.h"

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

int _WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, 
						 int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, 
						 LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar) {

	  int result = WideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, 
		  cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
	  return result;
}

int _stdcall WStr2UTF8(char* source, char** dest)
{
	int len = 1;

	if (source) 
		len = WStr2UTF8(source, NULL, 0);

/*	if (*dest) 
		free(*dest);*/
	
	*dest = (char*)calloc(1, len);

	if (!source) {
		*dest = 0;
		return 1;
	}

	return _WideCharToMultiByte(CP_UTF8,0,(LPCWSTR)source,-1,*dest,len,NULL,NULL);
}

int _stdcall WStr2UTF8(wchar_t* source, char** dest)
{
	return WStr2UTF8((char*)source, dest);
}

int _stdcall WStr2UTF8(char* source, char* dest, int max_len)
{
	if (dest) {
		if (source!=dest) {
			return _WideCharToMultiByte(CP_UTF8,0,(LPCWSTR)source,-1,dest,max_len,NULL,NULL);
		} else {
			int dest_size = WStr2UTF8(source, NULL, 0);
			char* cTemp = NULL;
			WStr2UTF8(source, &cTemp);
			strcpy(dest, cTemp);
			free(cTemp); 
			return dest_size;
		}
	} else {
		return _WideCharToMultiByte(CP_UTF8,0,(LPCWSTR)source,-1,NULL,0,NULL,NULL);
	}

	return 0;
}

int _stdcall WStr2UTF8(wchar_t* source, char* dest, int max_len)
{
	return WStr2UTF8((char*)source, dest, max_len);
}


int  _stdcall WStr2Str(char* source, char* dest, int max_len)
{
	int len = WideCharToMultiByte(CP_THREAD_ACP, 0, (LPCWSTR)source, -1,
		(LPSTR)dest, max_len, NULL, NULL);

	return len;
}

int  _stdcall WStr2Str(char* source, char** dest)
{
	int len = 1;
	if (source)
		len = _WideCharToMultiByte(CP_THREAD_ACP,0,(LPCWSTR)source,-1,NULL,0,0,0);

/*	if (*dest) {
		free(*dest);
	}*/
	*dest = (char*)calloc(1, len);

	return _WideCharToMultiByte(CP_THREAD_ACP,0,(LPCWSTR)source,-1,*dest,len,0,0);
}

int _stdcall UTF82WStr(char* source, char** dest)
{
	size_t source_len = strlen(source) + 1;
	int dest_len = 2;
	
	if (source)
		dest_len = 2 * MultiByteToWideChar(CP_UTF8,0,source,-1,0,0);

	if (dest) {
		/*if (*dest) free(*dest); */
		*dest = (char*)calloc(1, dest_len);
		return 2*MultiByteToWideChar(CP_UTF8,0,source,-1,(LPWSTR)*dest,dest_len / 2);
	} else {
		return 2*MultiByteToWideChar(CP_UTF8,0,source,-1,0,0);
	}
}

int _stdcall UTF82WStr(char* source, char* dest, int max_len)
{
	int i;

	if (!source)
		return 0;

	size_t source_len = strlen(source) + 1;

	if (dest) {
		if (source!=dest) {
			return 2*MultiByteToWideChar(CP_UTF8,0,source,-1,
				(LPWSTR)dest,max_len / 2);
		} else {
			char* cTemp = (char*)calloc(1, UTF82WStr(source, NULL, 0));
			i=2*MultiByteToWideChar(CP_UTF8,0,source,-1,(LPWSTR)cTemp,max_len / 2);
			memcpy(dest, cTemp, i);
			free(cTemp);
			return i;
		}
	} else {
		return 2*MultiByteToWideChar(CP_UTF8,0,source,-1,0,0);
	}
}

int _stdcall Str2WStr(char* source, char** dest)
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

int _stdcall Str2WStr(char* source, char* dest, int max_len)
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

int _stdcall UTF82Str(char* source, char** dest)
{
	if (!dest) {
		return -1;
	}

/*	if (*dest)
		free(*dest);
*/
	if (!source) {
		*dest = (char*)calloc(1, 1);	
		return 1;
	}

	unsigned short* temp = NULL;
	
	if (utf8_unicode_possible) {
		UTF82WStr(source,(char**)&temp);
		int dest_len = _WideCharToMultiByte(CP_THREAD_ACP,0,(LPCWSTR)temp,-1,0,0,0,0);

		if (dest) {
			*dest = (char*)calloc(1, dest_len);
			int r = _WideCharToMultiByte(CP_THREAD_ACP,0,(LPCWSTR)temp,-1,*dest,dest_len,0,0);
			free(temp);
			return r;
		} else {
			int r = _WideCharToMultiByte(CP_THREAD_ACP,0,(LPCWSTR)temp,-1,0,0,0,0);
			free(temp);
			return r;
		}
	} else {
		*dest = (char*)calloc(1, strlen(source)+1);
		strcpy(*dest, source);
		return (int)strlen(source)+1;
	}
}

int _stdcall UTF82Str(char* source, char* dest, int max_len)
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
			i = _WideCharToMultiByte(CP_THREAD_ACP,0,(LPCWSTR)temp,-1,dest,max_len,0,0);
			delete[] temp;
			return i;
		} else {
			i = _WideCharToMultiByte(CP_THREAD_ACP,0,(LPCWSTR)temp,-1,0,0,0,0);
			delete[] temp;
			return i;
		}
	} else {
		delete[] temp;
		if (dest) 
			strcpy(dest, source);
		
		return (int)strlen(source);
	}
}

int _stdcall Str2UTF8(char* source, char* dest, int max_len)
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
			i = _WideCharToMultiByte(CP_UTF8,0,(LPCWSTR)temp,-1,dest,max_len,0,0);
			delete[] temp;
			return i;
		} else {
			MultiByteToWideChar(CP_THREAD_ACP,0,source,-1,(LPWSTR)temp,temp_size);
			i = _WideCharToMultiByte(CP_UTF8,0,(LPCWSTR)temp,-1,0,0,0,0);
			delete[] temp;
			return i;
		}
	} else {
		delete[] temp;
		if (dest) {
			if ((int)source_len < max_len) 
				strcpy(dest, source);
			else {
				strncpy(dest, source, max_len);
				dest[(int)max_len-1] = 0;
			}
		}
		return 1+(int)strlen(source);
	}

}

int _stdcall Str2UTF8(char* source, char** dest)
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
		*dest = (char*)calloc(1, 1+strlen(source));
		strcpy(*dest, source);
		return (int)(1+strlen(source));
	}
}

int StringConvert(const char* source, int source_format, int max_source_len,
						   char** dest, int dest_format)
{
	__ASSERT(source_format == CHARACTER_ENCODING_ANSI ||
		source_format == CHARACTER_ENCODING_UTF16_LE ||
		source_format == CHARACTER_ENCODING_UTF8,
		"bad source_format");

//	printf(__FILE__);

	__ASSERT(dest_format == CHARACTER_ENCODING_ANSI ||
		dest_format == CHARACTER_ENCODING_UTF16_LE ||
		dest_format == CHARACTER_ENCODING_UTF8,
		"bad dest_format");

	char* _source = (char*)malloc(2*max_source_len+2);

	if (source_format == CHARACTER_ENCODING_UTF16_LE)
		wcsncpy((wchar_t*)_source, (wchar_t*)source, max_source_len);
	else
		strncpy(_source, source, max_source_len);

	switch (source_format)
	{
		case CHARACTER_ENCODING_ANSI:
			switch (dest_format) {
				case CHARACTER_ENCODING_ANSI: *dest = _strdup(_source); break;
				case CHARACTER_ENCODING_UTF8: Str2UTF8(_source, dest); break;
				case CHARACTER_ENCODING_UTF16_LE: Str2WStr(_source, dest); break;
			}
			break;
		case CHARACTER_ENCODING_UTF8:
			switch (dest_format) {
				case CHARACTER_ENCODING_ANSI: UTF82Str(_source, dest); break;
				case CHARACTER_ENCODING_UTF8: *dest = _strdup(_source); break;
				case CHARACTER_ENCODING_UTF16_LE: UTF82WStr(_source, dest); break;
			}
			break;
		case CHARACTER_ENCODING_UTF16_LE:
//		case CHARACTER_ENCODING_UTF16_BE:
			switch (dest_format) {
				case CHARACTER_ENCODING_ANSI: 
					WStr2Str(_source, dest); 
					break;
				case CHARACTER_ENCODING_UTF8: 
					WStr2UTF8(_source, dest); 
					break;
				case CHARACTER_ENCODING_UTF16_LE: 
					*dest = (char*)_wcsdup((wchar_t*)_source); 
					break;
			}
			break;
	}
	free(_source);

	return 1;
}

int FromUTF8(char* source, wchar_t** dest)
{
	return StringConvert(source, CHARACTER_ENCODING_UTF8,
		1+strlen(source), (char**)dest, CHARACTER_ENCODING_UTF16_LE);
}

int FromUTF8(char* source, char** dest)
{
	return StringConvert(source, CHARACTER_ENCODING_UTF8,
		1+strlen(source), (char**)dest, CHARACTER_ENCODING_ANSI);
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