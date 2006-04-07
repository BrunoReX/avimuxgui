#include "stdafx.h"
#include "textfiles.h"
#include "..\utf-8.h"

CTEXTFILE::CTEXTFILE()
{
	iCharCoding = 0;
	iHdrSize = 0;
}

CTEXTFILE::CTEXTFILE(DWORD _dwMode, STREAM* s, int iOutputFormat)
{
	iCharCoding = 0;
	iHdrSize = 0;
	Open(_dwMode, s);
	SelectOutputFormat(iOutputFormat);
}

int CTEXTFILE::Open(DWORD _dwMode, STREAM* s)
{
	STREAM::Open(_dwMode);
	source = s;
	Seek(0);

	while (!iCharCoding) {
		int c = 0;
		source->Read(&c,1);
		if (c==0xEF) {
			if (source->Read(&c,1) && c == 0xBB && source->Read(&c,1) && c == 0xBF) {
				iCharCoding = CM_UTF8;	
				iHdrSize = 3;
			}
		} else
		if (c==0xFF) {
			if (source->Read(&c,1) && c == 0xFE) {
				iCharCoding = CM_UTF16;
				iHdrSize = 2;
			}
		} else {
			iCharCoding = CM_ANSI;
			iHdrSize = 0;
		}

	}
	Seek(0);

	return STREAM_OK;
}

int CTEXTFILE::ReOpen()
{
	iCharCoding = 0;
	iHdrSize = 0;
	return Open(STREAM_READ,source);
}

int CTEXTFILE::Seek(__int64 qwPos)
{
	return source->Seek(qwPos + iHdrSize + GetOffset());
}

bool CTEXTFILE::IsUTF8In()
{
	return (iCharCoding == CM_UTF8);
}

bool CTEXTFILE::IsUTF8Out()
{
	return (iOutputCoding == CM_UTF8);
}

int CTEXTFILE::Read(void* d, int iBytes)
{
	return source->Read(d,iBytes);
}

int CTEXTFILE::ReadLine(char* d)
{
	char c;
	char s[1024];
	int  i=0;
	int  j=0;
	int  r=0;


	ZeroMemory(d,sizeof(d));
	s[0] = 0;
	while (++j && (r=source->Read(&c,1)) && c != 0x0A && j < 1023) {
		if (c!=0x0A && c!=0x0D) s[i++] = c;
	}
	if (!r && j==1) return -1;
	s[i++] = 0;

	if (IsUTF8In() && !IsUTF8Out()) {
		UTF82Str(s,d);
	} else 
	if (!IsUTF8In() && IsUTF8Out()) {
		Str2UTF8(s,d);
	} else {
		strcpy(d,s);
	}

	return (strlen(s));
}

void CTEXTFILE::SelectOutputFormat(int iFormat)
{
	iOutputCoding = iFormat;
}

void CTEXTFILE::SelectInputFormat(int iFormat)
{
	iCharCoding = iFormat;
}

__int64 CTEXTFILE::GetPos()
{
	return source->GetPos() - iHdrSize - GetOffset();
}