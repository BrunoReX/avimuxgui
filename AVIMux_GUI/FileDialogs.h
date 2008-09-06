#ifndef I_FILEDIALOGS
#define I_FILEDIALOGS


#include "windows.h"
#include "..\UnicodeCalls.h"

void PrepareSimpleDialog(void* lpopn, HWND m_hWnd, const char* cFilter);
int GetOpenSaveFileNameUTF8(void* lpofn, int open);


#endif