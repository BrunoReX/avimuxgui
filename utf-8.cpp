#include "stdafx.h"
#include "utf-8.h"
#include "windows.h"
#include "memory.h"
#include "stdlib.h"
#include "stdio.h"

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

	if (*dest) 
		free(*dest);
	
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

	if (*dest) {
		free(*dest);
	}
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
		if (*dest) free(*dest);
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

	if (*dest) 
		free(*dest);
	
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

	if (*dest)
		free(*dest);

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
		temp_size = 1+strlen(source);
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

	if (*dest)
		free(*dest);

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

