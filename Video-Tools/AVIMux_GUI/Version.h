#ifndef  I_VERSION
#define  I_VERSION

//void ComposeVersionString(char* buffer);
//char* GetAMGVersionString(char* buffer, int buf_len);

#ifdef _UNICODE
#define GetAMGVersionString GetAMGVersionStringW
std::wstring GetAMGVersionStringW();
#else
#define GetAMGVersionString GetAMGVersionStringA
std::string GetAMGVersionStringA();
#endif

std::basic_string<TCHAR> GetAMGVersionDate();
std::basic_string<TCHAR> GetAMGVersionTime();
std::basic_string<TCHAR> ComposeVersionString();

#endif