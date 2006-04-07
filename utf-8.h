#ifndef I_UTF8
#define I_UTF8

void utf8_EnableRealUnicode(bool bEnabled);
int  utf8_IsUnicodeEnabled();
int  _stdcall UTF82WStr(char* source, char** dest);
int  _stdcall UTF82WStr(char* source, char* dest);
int  _stdcall UTF82Str(char* source, char** dest);
int  _stdcall UTF82Str(char* source, char* dest);
int  _stdcall WStr2UTF8(char* source, char** dest);
int  _stdcall WStr2UTF8(char* source, char* dest);
int  _stdcall Str2UTF8(char* source, char* dest);
int  _stdcall Str2UTF8(char* source, char** dest);
int  _stdcall Str2WStr(char* source, char** dest);
int  _stdcall Str2WStr(char* source, char* dest);
int  _stdcall WStr2Str(char* source, char** dest);


#endif