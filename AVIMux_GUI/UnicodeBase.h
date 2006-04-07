#ifndef I_UNICODE_BASE
#define I_UNICODE_BASE

typedef int(_stdcall *CONVERT_STRINGS)(char*, char*);

class CUnicodeBase

{
private:
	CONVERT_STRINGS		_fromUTF8, _toUTF8;
	int					iUnicodeEnabled;
// Konstruktion
public:
	CUnicodeBase();
	void	InitUnicode(int enable);
	int		IsUnicode();
	void	fromUTF8(char* s, char* d);
	void	toUTF8(char* s, char* d);
};

#endif