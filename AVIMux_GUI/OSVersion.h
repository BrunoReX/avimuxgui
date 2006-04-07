#ifndef I_OSVERSION
#define I_OSVERSION

bool DoesOSSupportUnicode();
bool GetOSVersionString(char* buf, int buf_len);
bool IsOSWin2kplus();

#endif