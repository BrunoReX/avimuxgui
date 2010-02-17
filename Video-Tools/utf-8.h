#include "../Common/UTF-8.h"

#ifndef I_UTF82
#define I_UTF82

void utf8_EnableRealUnicode(bool bEnabled);
int  utf8_IsUnicodeEnabled();

#endif

/*

int  _stdcall UTF82WStr(char* source, char* dest, int max_len = 16384);
int  _stdcall UTF82WStr(char* source, char** dest);

int  _stdcall UTF82Str(char* source, char* dest, int max_len = 16384);
int  _stdcall UTF82Str(char* source, char** dest);

int  _stdcall WStr2UTF8(char* source, char* dest, int max_len = 16384);
int	 _stdcall WStr2UTF8(wchar_t* source, char* dest, int max_len = 16384);
int  _stdcall WStr2UTF8(char* source, char** dest);
int  _stdcall WStr2UTF8(wchar_t* source, char** dest);

int  _stdcall Str2UTF8(char* source, char* dest, int max_len = 16384);
int  _stdcall Str2UTF8(char* source, char** dest);

int  _stdcall Str2WStr(char* source, char* dest, int max_len = 16384);
int  _stdcall Str2WStr(char* source, char** dest);

int  _stdcall WStr2Str(char* source, char* dest, int max_len = 16384); 
int  _stdcall WStr2Str(char* source, char** dest);

extern char    cUTF8Hdr[];

static const int CHARACTER_ENCODING_ANSI     = 0x01;
static const int CHARACTER_ENCODING_UTF8     = 0x02;
static const int CHARACTER_ENCODING_UTF16_LE = 0x03;
static const int CHARACTER_ENCODING_UTF16_BE = 0x04;

int StringConvert(const char* source, int source_format, int max_source_len,
				   char** dest, int dest_format);
int FromUTF8(char* source, wchar_t** dest);
int FromUTF8(char* source, char** dest);

int  UTF8CharLen(char in);
int IsUTF8(const char* src, size_t max_source_len);

#endif
*/

