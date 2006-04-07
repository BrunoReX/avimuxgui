#include "stdafx.h"
#include "utf-8.h"
#include "windows.h"
#include "memory.h"
#include "stdlib.h"

int utf8_unicode_possible = 1;

void utf8_EnableRealUnicode(bool bEnabled)
{
	utf8_unicode_possible = bEnabled;
}

int utf8_IsUnicodeEnabled()
{
	return utf8_unicode_possible;
}

int _stdcall WStr2UTF8(char* source, char** dest)
{
	if (*dest) {
		delete *dest;
		*dest = (char*)calloc(1,2048);
	}

	return WideCharToMultiByte(CP_UTF8,0,(unsigned short*)source,-1,*dest,1000,NULL,NULL);
}

int _stdcall WStr2UTF8(char* source, char* dest)
{
	if (dest) {
		if (source!=dest) {
			return WideCharToMultiByte(CP_UTF8,0,(unsigned short*)source,-1,dest,1000,NULL,NULL);
		} else {
			char cTemp[1024]; cTemp[0]=0;
			WideCharToMultiByte(CP_UTF8,0,(unsigned short*)source,-1,cTemp,1000,NULL,NULL);
			strcpy(dest, cTemp);
			return strlen(dest);
		}
	}
	return 0;
}

int  _stdcall WStr2Str(char* source, char** dest)
{
	unsigned short temp[1000];
	temp[0]=0;
	return WideCharToMultiByte(CP_THREAD_ACP,0,(unsigned short*)source,-1,*dest,1000,0,0);
}

int _stdcall UTF82WStr(char* source, char** dest)
{
	int source_len = 500;

	if (dest) {
		if (*dest) delete *dest;
		*dest = (char*)calloc(1,source_len);

		return 2*MultiByteToWideChar(CP_UTF8,0,source,-1,(unsigned short*)*dest,500);
	} else {
		return 2*MultiByteToWideChar(CP_UTF8,0,source,-1,0,0);
	}
}

int _stdcall UTF82WStr(char* source, char* dest)
{
	int source_len = 500, i;
	char cTemp[1024]; cTemp[0]=0;

	if (dest) {
		if (source!=dest) {
			return 2*MultiByteToWideChar(CP_UTF8,0,source,-1,(unsigned short*)dest,500);
		} else {
			memcpy(dest, cTemp, i=2*MultiByteToWideChar(CP_UTF8,0,source,-1,(unsigned short*)cTemp,500));
			return i;
		}
	} else {
		return 2*MultiByteToWideChar(CP_UTF8,0,source,-1,0,0);
	}
}

int _stdcall Str2WStr(char* source, char** dest)
{
	int source_len = 500;

	if (*dest) delete *dest;
	*dest = (char*)calloc(1,source_len);

	return 2*MultiByteToWideChar(CP_THREAD_ACP,0,source,-1,(unsigned short*)*dest,500);
}

int _stdcall Str2WStr(char* source, char* dest)
{
	int source_len = 500;

	if (source!=dest) {
		return 2*MultiByteToWideChar(CP_THREAD_ACP,0,source,-1,(unsigned short*)dest,500);
	} else {
		char cTemp[1000];
		int i = 2*MultiByteToWideChar(CP_THREAD_ACP,0,source,-1,(unsigned short*)cTemp,500);
		memcpy(dest, cTemp, i);
		return i;
	}
}

int _stdcall UTF82Str(char* source, char** dest)
{
	unsigned short temp[1000];
	temp[0]=0;

	if (utf8_unicode_possible) {
		UTF82WStr(source,(char**)&temp);
		if (dest) {
			return WideCharToMultiByte(CP_THREAD_ACP,0,temp,-1,*dest,1000,0,0);
		} else {
			return WideCharToMultiByte(CP_THREAD_ACP,0,temp,-1,0,0,0,0);
		}
	} else {
		if (dest) {
			strcpy(*dest, source);
		}
		return strlen(source);
	}
}

int _stdcall UTF82Str(char* source, char* dest)
{
	unsigned short temp[1000];
	ZeroMemory(temp,sizeof(temp));
	
	if (utf8_unicode_possible) {
		UTF82WStr(source,(char*)temp);
		if (dest) {
			return WideCharToMultiByte(CP_THREAD_ACP,0,temp,-1,dest,1000,0,0);
		} else {
			return WideCharToMultiByte(CP_THREAD_ACP,0,temp,-1,0,0,0,0);
		}
	} else {
		if (dest) {
			strcpy(dest, source);
		}
		return strlen(source);
	}
}

int _stdcall Str2UTF8(char* source, char* dest)
{
	int source_len = 500;
	unsigned short temp[1000];

	if (utf8_unicode_possible) {
		ZeroMemory(temp,sizeof(temp));

		if (dest) {
			MultiByteToWideChar(CP_THREAD_ACP,0,source,-1,(unsigned short*)temp,500);
			return WideCharToMultiByte(CP_UTF8,0,temp,-1,dest,1000,0,0);
		} else {
			MultiByteToWideChar(CP_THREAD_ACP,0,source,-1,(unsigned short*)temp,500);
			return WideCharToMultiByte(CP_UTF8,0,temp,-1,0,0,0,0);
		}
	} else {
		if (dest) {
			strcpy(dest, source);
		}
		return strlen(source);
	}

}

int _stdcall Str2UTF8(char* source, char** dest)
{
	int source_len = 500;
	unsigned short temp[1000];
	ZeroMemory(temp,sizeof(temp));

	if (utf8_unicode_possible) {
		if (dest) {
			if (*dest) delete *dest;
			*dest = (char*)calloc(1,source_len);

	  		MultiByteToWideChar(CP_THREAD_ACP,0,source,-1,(unsigned short*)temp,500);
			int res = WideCharToMultiByte(CP_UTF8,0,temp,-1,*dest,1000,0,0);
			return res;
		} else {
		  	MultiByteToWideChar(CP_THREAD_ACP,0,source,-1,(unsigned short*)temp,500);
			return WideCharToMultiByte(CP_UTF8,0,temp,-1,0,0,0,0);
		}
	} else {
		if (dest) {
			strcpy(*dest, source);
		}
		return strlen(source);
	}
}

