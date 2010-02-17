#include "stdafx.h"
#include "languages.h"
#include "..\utf-8.h"
#include "stdio.h"
#include "../../Common/textfiles.h"
#include "global.h"
#include "../FileStream.h"

LANGUAGE_DESCRIPTOR*	lpCurrLang;

void SetCurrentLanguage(LANGUAGE_DESCRIPTOR* lpNewLanguage)
{
	lpCurrLang=lpNewLanguage;
}

LANGUAGE_DESCRIPTOR* GetCurrentLanguage(void)
{
	return lpCurrLang;
}

char*	LoadString(DWORD dwID, int charset)
{
	std::map<DWORD, CUTF8>::iterator itemIter = lpCurrLang->items.find(dwID);
	if (itemIter == lpCurrLang->items.end()) {
		std::ostringstream sstrNewText;
		sstrNewText << "<" << dwID << "> not found";
		std::string newText = sstrNewText.str();
		lpCurrLang->items[dwID] = newText;
		return LoadString(dwID, charset);
	}

	switch (charset)
	{
	case LOADSTRING_ANSI:
		return const_cast<char*>(itemIter->second.Str());
		break;
	case LOADSTRING_UTF8:
		return const_cast<char*>(itemIter->second.UTF8());
		break;
	case LOADSTRING_UTF16:
		return reinterpret_cast<char*>(const_cast<wchar_t*>(itemIter->second.WStr()));
		break;
	}

	return NULL;
}

LANGUAGE_DESCRIPTOR* LoadLanguageFile(const char* lpcName)
{
	DWORD  i,dwLen;
	bool	bBackslash;

	LANGUAGE_DESCRIPTOR*		lpLD;
	
	CFileStream* s = new CFileStream;
	s->Open(lpcName, StreamMode::Read);
	CTextFile* f = new CTextFile;
	f->Open(StreamMode::Read, s);
	f->SetOutputEncoding(CharacterEncoding::UTF8);
	
	std::string textFileLine;
	f->ReadLine(textFileLine);

	if (textFileLine != "[AVI-Mux GUI Language File]") {
		f->Close();
		delete f;
		s->Close();
		delete s;
		return NULL;
	}

	lpLD = new LANGUAGE_DESCRIPTOR;

	f->ReadLine(textFileLine);
	f->ReadLine(textFileLine);
	if (textFileLine != "NAME") {
		f->Close();
		delete f;
		s->Close();
		delete s;
		return NULL;
	}

	f->ReadLine(textFileLine);
	lpLD->name = textFileLine;
	f->ReadLine(textFileLine);

	while (f->ReadLine(textFileLine)>-1)
	{
		const char* cBuffer = textFileLine.c_str();
		
		DWORD currentIndex = atoi(cBuffer);
		f->ReadLine(textFileLine);
		cBuffer = textFileLine.c_str();

		std::string currentItem;

		bBackslash=false;
		if (lstrlen(cBuffer)>=2)
		{
			dwLen = textFileLine.size();
			
			for (i=0;i<dwLen;i++)
			{
				if (!bBackslash)
				{
					if (cBuffer[i]==92) {
						bBackslash=true;
					} else {
						currentItem.push_back(cBuffer[i]);
						bBackslash=false;
					}
				} else {
					if (cBuffer[i]=='n') {
						currentItem.push_back(13);
						currentItem.push_back(10);
					}
					bBackslash=false;
				}
			}

			f->ReadLine(textFileLine);
			cBuffer = textFileLine.c_str();
		}

		lpLD->items[currentIndex] = currentItem;
	}

	f->Close();
	delete f;
	s->Close();
	delete s;

	return lpLD;
}

void UnloadLanguageFile(LANGUAGE_DESCRIPTOR* lpLD)
{
	delete lpLD;
}
