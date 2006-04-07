#include "stdafx.h"
#include "UnicodeBase.h"
#include "..\utf-8.h"

CUnicodeBase::CUnicodeBase()
{
	InitUnicode(utf8_IsUnicodeEnabled());
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
		_fromUTF8_alloc = UTF82WStr;
		_toUTF8_alloc = WStr2UTF8;
	} else {
		_fromUTF8 = UTF82Str;
		_toUTF8 = Str2UTF8;
		_fromUTF8_alloc = UTF82Str;
		_toUTF8_alloc = Str2UTF8;
	}
}

int CUnicodeBase::fromUTF8(char* s, char* d, int max_len)
{
	return (*_fromUTF8)(s, d, max_len);
}

int CUnicodeBase::toUTF8(char* s, char* d, int max_len)
{
	return (*_toUTF8)(s, d, max_len);
}

int CUnicodeBase::fromUTF8(char* s, char** d)
{
	return (*_fromUTF8_alloc)(s, d);
}

int CUnicodeBase::toUTF8(char* s, char** d)
{
	return (*_toUTF8_alloc)(s, d);
}