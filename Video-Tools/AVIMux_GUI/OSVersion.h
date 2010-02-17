#ifndef I_OSVERSION
#define I_OSVERSION

bool DoesOSSupportUnicode();

bool GetOSVersionString(std::string& result);
bool GetOSVersionString(std::wstring& result);
std::basic_string<TCHAR> GetOperatingSystemByAvailableAPIFunctions();

bool IsOSWin2kplus();

#endif