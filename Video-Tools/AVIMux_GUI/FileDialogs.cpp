#include "stdafx.h"
#include "FileDialogs.h"

#include "windows.h"
#include "..\utf-8.h"

char* reserve(char* s)
{
	if (!s)
		return 0;

	return (char*)calloc(1, 4+2*strlen(s));
}

char* reserve(const char* s)
{
	return reserve((char*)s);
}

void PrepareSimpleDialog(void* lpofn, HWND m_hWnd, const char* cFilter)
{
	memset(lpofn, 0, sizeof(OPENFILENAME));

	OPENFILENAME* o = (OPENFILENAME*)lpofn;

	o->hwndOwner = m_hWnd;
	o->lpstrDefExt = "avi";
	o->lpstrFile = (char*)calloc(1,16384);
	o->lpstrFilter = cFilter;

	o->lStructSize = sizeof(*o);
	o->nMaxFile = 8192;
}

int GetOpenSaveFileNameUTF8(void* lpofn, int open)
{
	int res = 0;
	int count;

	OPENFILENAMEA*	ofda = (OPENFILENAMEA*)lpofn;
	OPENFILENAMEW*	ofdw = (OPENFILENAMEW*)lpofn;

	char* cFilter = reserve(ofda->lpstrFilter);
	char* cCustomFilter = reserve(ofda->lpstrCustomFilter);
	char* cDefExt = reserve(ofda->lpstrDefExt);
	char* cFile = (char*)calloc(2,16384);
	char* cInitialDir = reserve(ofda->lpstrInitialDir);
	char* cFileTitle = reserve(ofda->lpstrFileTitle);
	char* cTemplateName = reserve(ofda->lpTemplateName);

	OPENFILENAME* o = (OPENFILENAME*)calloc(1,sizeof(OPENFILENAME));
	memcpy(o, ofda, sizeof(OPENFILENAME));
	o->lpstrFilter = cFilter;
	o->lpstrCustomFilter = cCustomFilter;
	o->lpstrDefExt = cDefExt;
	o->lpstrFile = cFile;
	o->lpstrInitialDir = cInitialDir;
	o->lpstrFileTitle = cFileTitle;
	o->lpTemplateName = cTemplateName;

	fromUTF8((void*)ofda->lpstrFilter, cFilter);
	fromUTF8((void*)ofda->lpstrCustomFilter, cCustomFilter);
	fromUTF8((void*)ofda->lpstrDefExt, cDefExt);
	fromUTF8((void*)ofda->lpstrFile, cFile);
	fromUTF8((void*)ofda->lpstrInitialDir, cInitialDir);
	fromUTF8((void*)ofda->lpstrFileTitle, cFileTitle);
	fromUTF8((void*)ofda->lpTemplateName, cTemplateName);

	if (utf8_IsUnicodeEnabled()) {
		unsigned short* s = (unsigned short*)cFilter;
		count = 0;
		while (*s && count < 2) {
			if (*s == '|') {
				count++;
				*s = 0;
			} else count = 0;
			s++;
		}
	} else {
		unsigned char* s = (unsigned char*)cFilter;
		count = 0;
		while (*s && count < 2) {
			if (*s == '|') {
				count++;
				*s = 0;
			} else count = 0;
			s++;
		}

	}

	res = (open)?(*UGetOpenFileName())(o):(*UGetSaveFileName())(o);

	toUTF8((void*)o->lpstrFile, ofda->lpstrFile);

	free(cFilter);
	free(cCustomFilter);
	free(cDefExt);
	free(cFile);
	free(cInitialDir);
	free(cFileTitle);
	free(cTemplateName);

	free(o);
	return res;
}