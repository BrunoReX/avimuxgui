#include "stdafx.h"
#include "MessageBoxHelper.h"
#include "..\Strings.h"
#include "Languages.h"

DWORD MessageBoxHelper::ShowByID(HWND parent, DWORD titleID, DWORD messageID, UINT uType)
{
	CUTF8 title(LoadString(titleID, LOADSTRING_UTF8), CharacterEncoding::UTF8);
	CUTF8 msg(LoadString(messageID, LOADSTRING_UTF8), CharacterEncoding::UTF8);

	return MessageBox(parent, msg.TStr(), title.TStr(), uType);
}

DWORD MessageBoxHelper::CouldNotOpenFileForWrite(const CUTF8& fileName)
{
	std::string msgTemplate = LoadString(STR_ERR_COULDNOTOPENWRITE, LOADSTRING_UTF8);
	std::string message = string_replace_first<TCHAR>(msgTemplate, _T("%s"), fileName);
	std::string title = LoadString(STR_GEN_ERROR, LOADSTRING_UTF8);
		
	return ::MessageBoxW(NULL, CUTF8(message).WStr(), CUTF8(title).WStr(), MB_ICONERROR | MB_OK);
}