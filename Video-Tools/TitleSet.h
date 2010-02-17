#ifndef I_TITLE_SET
#define I_TITLE_SET

#include <vector>
#include "ITitleSet.h"


/* A TitleSet is just a collection of titles, for example for one Track.
   It is assumed that only one title is given per language
*/
class CTitleSet : public ITitleSet
{
private:
	std::vector<Title_t> titles;
protected:
	/* add a title without checking if a title for that
       language is already present */
	bool AddTitle_nocheck(const char* string, const char* language);
public:
	/* constructor */
	CTitleSet();

	/* destructor */
	virtual ~CTitleSet();

	/* add a title for a given language that does not yet exist */
	bool AddTitle(const char* string, const char* language);

	/* remove the i-th title */
	bool DeleteTitle(size_t index);

	/* remove the title in a given language */
	bool DeleteTitle(const char* language);

	/* remove all titles */
	void DeleteAllTitles();

	/* determine number of given titles */
	size_t GetTitleCount(void);

	/* determine index unter which the title in a given languge is stored */
	bool GetLanguageIndex(const char* language, size_t* index);

	/* get language of a title at a certain index */
	bool GetTitleLanguage(size_t index, char** pLanguage);

	/* get title at a certain index */
	bool GetTitleString(size_t index, char** pTitle);

	/* get title in a certain language */
	bool GetTitleStringFromLanguage(const char* language, char** pTitle);

	/* set the title for a certain language */
	bool SetTitleForLanguage(const char* language, const char* title);

	/* set a default title */
	bool SetTitle(const char* title);

	/* get a default title */
	bool GetTitle(char** pTitle);

	/* duplicate a titleset, like a copy constructor */
	ITitleSet* Duplicate();

	/* import titles from another titleset into this one */
	void Import(ITitleSet* importFrom);
};

/* A class that represents an item that can have titles should
   be derived from this class
*/
class CHasTitles : virtual public IHasTitles
{
private:
	/* the titleset managed by the class is private because derived
	   classes are supposed to access it via GetTitleSet() */
	CTitleSet* titleSet;

public:
	CHasTitles();
	virtual ~CHasTitles();
	virtual ITitleSet* GetTitleSet();
};

#endif