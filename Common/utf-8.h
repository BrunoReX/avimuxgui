#ifndef I_UTF8
#define I_UTF8


#include <string>

int  _stdcall UTF82WStr(const char* source, char* dest, int max_len = 16384);
int  _stdcall UTF82WStr(const char* source, char** dest);

int  _stdcall UTF82Str(const char* source, char* dest, int max_len = 16384);
int  _stdcall UTF82Str(const char* source, char** dest);

int  _stdcall WStr2UTF8(const char* source, char* dest, int max_len = 16384);
int	 _stdcall WStr2UTF8(const wchar_t* source, char* dest, int max_len = 16384);
int  _stdcall WStr2UTF8(const char* source, char** dest);
int  _stdcall WStr2UTF8(const wchar_t* source, char** dest);

int  _stdcall Str2UTF8(const char* source, char* dest, int max_len = 16384);
int  _stdcall Str2UTF8(const char* source, char** dest);

int  _stdcall Str2WStr(const char* source, char* dest, int max_len = 16384);
int  _stdcall Str2WStr(const char* source, char** dest);

int  _stdcall WStr2Str(const char* source, char* dest, int max_len = 16384); 
int  _stdcall WStr2Str(const char* source, char** dest);

int  UTF8CharLen(char in);
int  IsUTF8(const char* src, size_t max_source_len);

extern char    cUTF8Hdr[];

/*
static const int CHARACTER_ENCODING_ANSI     = 0x01;
static const int CHARACTER_ENCODING_UTF8     = 0x02;
static const int CHARACTER_ENCODING_UTF16_LE = 0x03;
static const int CHARACTER_ENCODING_UTF16_BE = 0x04;
*/

class CharacterEncoding
{
public:
	enum CharacterEncodings {
		Undefined,
		ANSI,
		UTF8,
		UTF16LE,
		UTF16BE
	};
};

int StringConvert(const char* source, CharacterEncoding::CharacterEncodings source_format,/* int max_source_len,*/
				  char** dest, CharacterEncoding::CharacterEncodings dest_format);

int FromUTF8(const char* source, wchar_t** dest);
int FromUTF8(const char* source, char** dest);

int ToUTF8(const char* source, char** dest);
int ToUTF8(const wchar_t* source, char** dest);

class CUTF8
{
private:
	std::string m_ascii;
	std::string m_utf8;
	std::wstring m_unicode;
private:
	void Complete();
public:
	CUTF8() { };
	CUTF8(const CUTF8& other);
	CUTF8(const char* pSrc);
	CUTF8(const wchar_t* pSrc);
	CUTF8(const char* pSrc, int Encoding);

//	CUTF8(const std::string src);
	CUTF8(const std::string& src);

	CUTF8(std::wstring src);

	operator std::string() const {
		return m_ascii;
	}

	operator std::wstring() const {
		return m_unicode;
	}

	const char* Str() const {
		return m_ascii.c_str();
	}

	const char* UTF8() const {
		return m_utf8.c_str();
	}

	const wchar_t* WStr() const {
		return m_unicode.c_str();
	}

	// returns the size of the utf8 encoded string
	const size_t Size() const {
		return m_utf8.size();
	}

#ifdef _TCHAR_DEFINED
	const TCHAR* TStr() const {
#ifdef _UNICODE
		return WStr();
#else
		return Str();
#endif
	}
#endif
	virtual ~CUTF8();
};

#endif