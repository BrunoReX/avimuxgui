#ifndef I_CHAPTERS
#define I_CHAPTERS

#include "multimedia_source.h"
#include "buffers.h"
#include "dynarray.h"
#include "XML.h"
#include "Tags.h"
#include <vector>

const int CHAP_LAST			= -0x23;
const __int64 CHAP_INVALIDINDEX = -0x37;//-0x8000000000000000;
const int CHAP_IMPF_DELETE = 0x01;

const int CHAP_SETUIDLEN_RECURSIVE = 0x01;

class CHAPTER_DISPLAY_INFO
{
public:
	CHAPTER_DISPLAY_INFO();
	virtual ~CHAPTER_DISPLAY_INFO();
	CStringBuffer*		cString;
	CBuffer*			cLanguage;
	CBuffer*			cCountry;
};

typedef std::vector<CHAPTER_DISPLAY_INFO> CHAPTER_DISPLAY;

typedef struct
{
	size_t		iCount;
	__int64*	iTracks;
} CHAPTER_TRACKS;

// values for chapterinfo_physical_equivalents
const int CHIPE_UNDEFINED  = 00;
const int CHIPE_INDEX      = 10;
const int CHIPE_TRACK      = 20;
const int CHIPE_SESSION    = 30;
const int CHIPE_LAYER      = 40;
const int CHIPE_SIDE       = 50;
const int CHIPE_MEDIUM     = 60;
const int CHIPE_SET        = 70;

const int CHI_PHYSICAL_EQUIVALENTS [] = {
	CHIPE_UNDEFINED, CHIPE_INDEX, CHIPE_TRACK, CHIPE_SESSION, CHIPE_LAYER,
	CHIPE_SIDE, CHIPE_MEDIUM, CHIPE_SET
};

const int CHIPE_COUNT = sizeof(CHI_PHYSICAL_EQUIVALENTS) / sizeof(CHI_PHYSICAL_EQUIVALENTS[0]);

char* physicalequiv2string(int index);

class CHAPTER_INFO
{
public:
	CHAPTER_INFO();

	union {
		__int64				iUID;
		unsigned __int32    iUID32[2];
	};
	__int64				iTimestart;
	__int64				iTimeend;
	int					iPhysicalEquiv;
	CHAPTER_DISPLAY		display;
	CHAPTER_TRACKS		tracks;
	void*				subchapters; // CHAPTERS* subchapters
	void*				csubchapters;
	int					bHidden;
	int					bDefault;
	int					bEnabled;
	int					bOrdered;
	int					bEdition;
	int					iUIDLen;
	char				cSegmentUID[16];
	int					bSegmentUIDValid;
	TAG_INDEX_LIST		pTags;
};

typedef struct
{
	size_t				iCount;
	CHAPTER_INFO**	chapters;
} CHAPTERS;

typedef struct
{
	__int64			iBegin;
	__int64			iEnd;
	int				bIsEdition;
	int				bHidden;
	int				bEnabled;
	int				bDefault;
	char			cText[1024];
	char			cLng[8];
	int				bSegmentUIDValid;
	char			cSegmentUID[16];
	union {
		int				iPhysicalEquiv;
		int				bOrdered;
	};
} SINGLE_CHAPTER_DATA;

const int CHAP_GCT_RETURN_NULL  = 0x00;
const int CHAP_GCT_RETURN_FIRST = 0x01;
const int CHAP_GCT_ALLOW_UND    = 0x02;


const int CHAP_IMPXML_SEVERAL_DEFAULT_EDITIONS = -0x01;
const int CHAP_IMPXML_NONUNIQUE_UID            = -0x02;
const int CHAP_IMPXML_COULD_NOT_PARSE_EDITION  = -0x03;
const int CHAP_IMPXML_UNKNOWN_ELEMENT          = -0x04;
const int CHAP_IMPXML_NO_CHAPTER_FILE          = -0x05;

const int CHAP_IMPXML_OK                       =  0x01;

class IMPORTCHAPTERSTRUCT
{
public:
	IMPORTCHAPTERSTRUCT();

	void*		chapters;
	__int64		bias;
	int			flags;
	int			index_start;
	int			index_end;
	bool		can_merge;
};

class CChapters: public MULTIMEDIASOURCE
{
	private:
		int			bSingleDefault;
		CHAPTERS*	chapters;
		CChapters*  parent;
		
	protected:
		int			Map(int iIndex);
		__int64		GenerateUID(int iIndex);
		
	public:
		CChapters();
		CChapters(CHAPTERS* chapters, int iBias = NULL);
		virtual ~CChapters();
		void		SetParent(CChapters* P);
		CChapters*	GetParent(int top_level);
		int			AddChapter(__int64 iBegin, __int64 iEnd, char* cText, int bEnabled = 1, int bHidden = 0);
		int			AddChapter(SINGLE_CHAPTER_DATA* pSCD);
		int			AddEmptyEdition();
		int			AssureSingleDefault();
		void		CopyChapterDisplay(CChapters* cSource, int iIndexSource, int iIndexDest);
		int			DeleteChapterDisplay(int iIndex, int iIndex2);
		int			EnableChapter(int iIndex, bool bEnable);
		int			GetChapter(int iIndex, __int64* lpiBegin, __int64* lpiEnd, char* cText);
		int			GetChapter(CDynIntArray* aIndex, __int64* lpiBegin, __int64* lpiEnd, char* cText);
		int			GetChapter(int iIndex, SINGLE_CHAPTER_DATA* lpSCD);
		int			GetChapter(CDynIntArray* aIndex, SINGLE_CHAPTER_DATA* lpSCD);
		CChapters*	GetChapter(CDynIntArray* aIndex, int* iIndex);
		int			GetChapterCount();
		int			GetChapterDisplayCount(int iIndex);
		char*		GetChapterText(int iIndex, int iIndex2 = 0);
		char*		GetChapterText(int iIndex, char* cLng, int default_behaviour);
		char*		GetChapterLng(int iIndex, int iIndex2 = 0);
		int			GetChapterPhysicalEquiv(int iIndex);
		TAG_INDEX_LIST& GetTags(int iIndex);
		
		/* returns a good estimation on the size a chapter/all chapters
		   will require in a matroska file */
		int			GetChapterSize(int iIndex, int flags);
		int			GetSize(int flags);

		int			HideChapter(int iIndex, bool bHidden);
		CChapters*	FindUID(__int64 UID, int bEdition, int* pIndex);
		int			FindChapterDisplayLanguageIndex(int iIndex, char* cLng, int allow_und = 1);
		int			SetChapterBegin(int iIndex, __int64 iBegin);
		int			SetChapterText(int iIndex, char* cText, int iIndex2 = 0);
		int			SetChapterLng(int iIndex, char* cText, int iIndex2 = 0);
		int			SetChapterEnd(int iIndex, __int64 iEnd);
		int			SetChapterDisplayCount(int iIndex, int iNewCount);
		int			SetChapterPhysicalEquiv(int iIndex, int iPhysicalEquiv);
		int			SetIsEdition(int iIndex, bool bEnable);
		int			SetIsDefault(int iIndex, bool bEnable);
		int			GetUIDLen(int iIndex);
		int			SetIsOrdered(int iIndex, bool bEnable);
		int			SetUIDLen(int iIndex, int len, int flags);
		int			SetSegmentUID(int iIndex, bool bValid, char* cUID);
		bool		IsSegmentUIDValid(int iIndex);
		int			Swap(int iIndex1, int iIndex2);

		__int64		GetChapterBegin(int iIndex);
		__int64		GetChapterEnd(int iIndex);
		int			Delete();
		int			DeleteChapter(int iIndex);
		bool		HasSubChapters(int iIndex);

		// iFlags: CHAP_IMPF_DELETE -> delete c after importing
		int			Import(CChapters* c, __int64 iImportBias = 0, int iFlags = 0,
			               int index_start = 0, int index_end = CHAP_LAST, bool can_merge = true);
		int			Import(IMPORTCHAPTERSTRUCT& import);

		int			ImportFromXML(XMLNODE* xmlNode, int depth = 0);
		void		InvalidateUID(int iIndex);
		int			IsEnabled(int iIndex);
		int			IsHidden(int iIndex);
		int			IsEdition(int iIndex);
		int			IsDefault(int iIndex);
		int			IsOrdered(int iIndex);
		CChapters*	GetSubChapters(int iIndex);	
		CChapters*	GetSubChapters(CDynIntArray* aIndex);	
		__int64		GetUID(int iIndex);
		int			SetUID(int iIndex, __int64 iUID);
		int			CreateXMLTree_aux(XMLNODE** ppNode);
		int			CreateXMLTree(XMLNODE** ppNode, XMLNODE** ppChapters, XMLNODE** ppTags);
		int			CreateXMLTree_tags(XMLNODE** ppNode);

};


void __int642hex(__int64 value, char* cDest, int min_len = 4, int group_size = 8, int space = 1);
void __int128hex(char* value, char* cDest, int group_size, int space = 1);
char* hex2int128(char* in, char* out);

#endif