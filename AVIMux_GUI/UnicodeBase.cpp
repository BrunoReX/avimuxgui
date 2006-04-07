#include "stdafx.h"
#include "UnicodeBase.h"
#include "..\utf-8.h"

CUnicodeBase::CUnicodeBase()
{
	InitUnicode(1);
}

int CUnicodeBase::IsUnicode()
{
	return iUnicodeEnabled;
}

void CUnicodeBase::InitUnicode(int enable)
{
	iUnicodeEnabled = enable;

	if (IsUnicode()) {
		_fromUTF8 = UTF82WStr;
		_toUTF8 = WStr2UTF8;
	} else {
		_fromUTF8 = UTF82Str;
		_toUTF8 = Str2UTF8;
	}
}

void CUnicodeBase::fromUTF8(char* s, char* d)
{
	(*_fromUTF8)(s, d);
}

void CUnicodeBase::toUTF8(char* s, char* d)
{
	(*_toUTF8)(s, d);
}