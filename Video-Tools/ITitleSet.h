#ifndef I_ITITLE_SET
#define I_ITITLE_SET

typedef struct 
{
	char* titleString;
	char* titleLanguage;
} Title_t;


/** \brief This interface provides access to a title set, i.e. a collectioon
 * of titles in different languages for one item. 
 *
 * It is assumed that only one title is given per language.
 */
class ITitleSet
{
public:
	/** \brief Constructor.
	 */
	ITitleSet() {
	}

	/** \brief Destructor.
	 */
	virtual ~ITitleSet() {
	}

	/* add a title for a given language that does not yet exist */
	virtual bool AddTitle(const char* string, const char* language) = NULL;

	/* remove the i-th title */
	virtual bool DeleteTitle(size_t index) = NULL;

	/* remove the title in a given language */
	virtual bool DeleteTitle(char* language) = NULL;

	/* remove all titles */
	virtual void DeleteAllTitles() = NULL;

	/* determine number of given titles */
	virtual size_t GetTitleCount(void) = NULL;

	/* determine index unter which the title in a given languge is stored */
	virtual bool GetLanguageIndex(char* language, size_t* index) = NULL;

	/* get language of a title at a certain index */
	virtual bool GetTitleLanguage(size_t index, char** pLanguage) = NULL;

	/* get title at a certain index */
	virtual bool GetTitleString(size_t index, char** pTitle) = NULL;

	/* get title in a certain language */
	virtual bool GetTitleStringFromLanguage(const char* language, char** pTitle) = NULL;

	/* set the title for a certain language */
	virtual bool SetTitleForLanguage(const char* language, const char* title) = NULL;

	/* set a default title */
	virtual bool SetTitle(const char* title) = NULL;

	/* get a default title */
	virtual bool GetTitle(char** pTitle) = NULL;

	/* duplicate a titleset, like a copy constructor */
	virtual ITitleSet* Duplicate() = NULL;

	/* import titles from another titleset into this one */
	virtual void Import(ITitleSet* importFrom) = NULL;
};

class IHasTitles
{
public:
	IHasTitles() { };
	virtual ~IHasTitles() { };
	virtual ITitleSet* GetTitleSet() = NULL;
};

#endif