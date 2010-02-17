#include "stdafx.h"
#include "TitleSet.h"

CTitleSet::CTitleSet()
{
}

CTitleSet::~CTitleSet()
{
	DeleteAllTitles();
}

void CTitleSet::DeleteAllTitles()
{
	std::vector<Title_t>::iterator iter = titles.begin();

	for (; iter != titles.end(); iter++) {
		delete iter->titleLanguage;
		delete iter->titleString;
	}

	titles.clear();
}

bool CTitleSet::AddTitle_nocheck(const char* string, const char* language)
{
	Title_t newTitle;
	newTitle.titleLanguage = _strdup(language);
	newTitle.titleString = _strdup(string);
	titles.push_back(newTitle);

	return true;
}

/* returns false if the title could not be added because a title with
   the given language already exists and is different from the title
   to be set
*/
bool CTitleSet::AddTitle(const char* string, const char* language)
{
	char* s;
	if (GetTitleStringFromLanguage(language, &s)) {
		if (strcmp(s, string))
			return false;
		else
			return true;
	}

	return AddTitle_nocheck(string, language);
}

bool CTitleSet::DeleteTitle(size_t index)
{
	if (index >= GetTitleCount())
		return false;

	delete titles[index].titleLanguage;
	delete titles[index].titleString;

	for (size_t j=index+1; j<titles.size(); j++)
		titles[j-1] = titles[j];
	
	titles.pop_back();

	return true;
}

bool CTitleSet::GetLanguageIndex(const char* language, size_t* index)
{
	if (!language)
		return false;

	size_t count = titles.size();
	for (size_t j = 0; j<count; j++) {
		if (!strcmp(titles[j].titleLanguage, language)) {
			if (index)
				*index = j;
			return true;
		}
	}

	return false;
}

bool CTitleSet::DeleteTitle(const char* language)
{
	size_t index = 0;
	if (GetLanguageIndex(language, &index)) 
		return DeleteTitle(index);

	return false;
}

size_t CTitleSet::GetTitleCount()
{
	return titles.size();
}

bool CTitleSet::GetTitleLanguage(size_t index, char** pLanguage)
{
	if (index >= GetTitleCount())
		return false;

	if (pLanguage)
		*pLanguage = titles[index].titleLanguage;

	return true;
}

bool CTitleSet::GetTitleString(size_t index, char** pString)
{
	if (index >= GetTitleCount())
		return false;

	if (pString)
		*pString = titles[index].titleString;

	return true;
}

bool CTitleSet::GetTitleStringFromLanguage(const char* language, char** pTitle)
{
	if (!language)
		return false;

	std::vector<Title_t>::iterator iter = titles.begin();

	for (; iter != titles.end(); iter++) {
		if (!strcmp(language, iter->titleLanguage)) {
			if (pTitle)
				*pTitle = iter->titleString;
			return true;
		}
	}

	if (pTitle)
		*pTitle = NULL;

	return false;
}

bool CTitleSet::SetTitleForLanguage(const char* language, const char* title)
{
	if (!language)
		return false;

	std::vector<Title_t>::iterator iter = titles.begin();

	for (; iter != titles.end(); iter++) {
		if (!strcmp(language, iter->titleLanguage)) {
			delete iter->titleString;
			iter->titleString = _strdup(title);
			return true;
		}
	}

	return AddTitle_nocheck(title, language);
}

bool CTitleSet::SetTitle(const char* title)
{
	return SetTitleForLanguage("und", title);
}

bool CTitleSet::GetTitle(char** pTitle)
{
	return GetTitleStringFromLanguage("und", pTitle);
}

CHasTitles::CHasTitles()
{
	titleSet = new CTitleSet();
}

CHasTitles::~CHasTitles()
{
	delete titleSet;
	titleSet = NULL;
}

ITitleSet* CHasTitles::GetTitleSet()
{
	return titleSet;
}

ITitleSet* CTitleSet::Duplicate()
{
	std::vector<Title_t>::iterator iter = titles.begin();
	CTitleSet* newSet = new CTitleSet();

	for (; iter != titles.end(); iter++)
		newSet->AddTitle_nocheck(iter->titleString, iter->titleLanguage);
	
	return newSet;
}

void CTitleSet::Import(ITitleSet* importFrom)
{
	for (size_t j=0; j<importFrom->GetTitleCount(); j++)
	{
		char* pLng = NULL;
		char* pTxt = NULL;

		importFrom->GetTitleLanguage(j, &pLng);
		importFrom->GetTitleString(j, &pTxt);

		AddTitle_nocheck(pTxt, pLng);
	}
}