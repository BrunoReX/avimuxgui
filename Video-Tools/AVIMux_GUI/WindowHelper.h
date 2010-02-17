#pragma once

#include "windows.h"
#include <vector>

class WindowHelper
{
public:
	static BOOL GetWindowText(CWnd& wnd, std::string& result)
	{
		int size = ::GetWindowTextLengthA(wnd) + 1;
		if (size == 1)
			return FALSE;
		std::vector<char> buffer(size);
	
		int resultSize = ::GetWindowTextA(wnd, &buffer[0], size);
		result = &buffer[0];
		return true;
	}

	static BOOL GetWindowText(CWnd& wnd, std::wstring& result)
	{
		int size = ::GetWindowTextLengthW(wnd) + 1;
		if (size == 1)
			return FALSE;
		std::vector<wchar_t> buffer(size);

		int resultSize = ::GetWindowTextW(wnd, &buffer[0], size);
		result = &buffer[0];
		return TRUE;
	}
};