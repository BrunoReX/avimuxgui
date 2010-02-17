#include "stdafx.h"
#include "stdio.h"
#include "fopenutf8.h"

FILE* fopenutf8(const char* filename, char* access, int unicode)
{
	FILE* file = NULL;
	if (unicode) {
		char* c = NULL;
		UTF82WStr((char*)filename,&c);
		char mode[8]; mode[0]=0;
		UTF82WStr(access, mode, 8);
		_wfopen_s(&file, (const wchar_t*)c,(const wchar_t*)mode);
		free(c);
	} else {
		fopen_s(&file, filename, access);
	}
	return file;
}