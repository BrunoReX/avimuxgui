#ifndef I_UNICODE_BASE
#define I_UNICODE_BASE

typedef int(_stdcall *CONVERT_STRINGS)(char*, char*, int);
typedef int(_stdcall *CONVERT_STRINGS_ALLOC)(char*, char**);

class CUnicodeBase

{
private:
	CONVERT_STRINGS		_fromUTF8, _toUTF8;
	CONVERT_STRINGS_ALLOC _fromUTF8_alloc, _toUTF8_alloc;
	int					iUnicodeEnabled;
// Konstruktion
public:
	CUnicodeBase();
	void	InitUnicode(int enable);
	int		IsUnicode();
	int		fromUTF8(char* s, char* d, int max_len = 16384);
	int		toUTF8(char* s, char* d, int max_len = 16384);
	int		fromUTF8(char* s, char** d);
	int		toUTF8(char* s, char** d);
};

#endif