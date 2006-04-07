#ifndef I_CHAPTERS
#define I_CHAPTERS

#include "AVIMux_GUI\multimedia_source.h"
#include "buffers.h"
#include "AVIMux_GUI\dynarray.h"
#include "XML.h"

const int CHAP_LAST = -0x01;
const int CHAP_INVALIDINDEX = -0x02;
const int CHAP_IMPF_DELETE = 0x01;

typedef struct
{
	CStringBuffer*		cString;
	CBuffer*			cLanguage;
	CBuffer*			cCountry;
} CHAPTER_DISPLAY_INFO;

typedef struct 
{
	int						iCount;
	CHAPTER_DISPLAY_INFO**	cDisp;
} CHAPTER_DISPLAY;

typedef struct
{
	int		iCount;
	int*	iTracks;
} CHAPTER_TRACKS;

typedef struct
{
	int					iUID;
	__int64				iTimestart;
	__int64				iTimeend;
	CHAPTER_DISPLAY		display;
	CHAPTER_TRACKS		tracks;
	void*				subchapters; // CHAPTERS* subchapters
	void*				csubchapters;
	int					bHidden;
	int					bEnabled;
} CHAPTER_INFO;

typedef struct
{
	int				iCount;
	CHAPTER_INFO**	chapters;
} CHAPTERS;

typedef struct
{
	__int64			iBegin;
	__int64			iEnd;
	int				bHidden;
	int				bEnabled;
	char			cText[1024];
	char			cLng[8];
} SINGLE_CHAPTER_DATA;

class CChapters: public MULTIMEDIASOURCE
{
	private:
		CHAPTERS*	chapters;
	protected:
		int			Map(int iIndex);
		int			GenerateUID(int iIndex);
		
	public:
		CChapters();
		CChapters(CHAPTERS* chapters, int iBias = NULL);
		~CChapters();
		int			AddChapter(__int64 iBegin, __int64 iEnd, char* cText, int bEnabled = 1, int bHidden = 0);
		int			AddChapter(SINGLE_CHAPTER_DATA* pSCD);
		int			DeleteChapterDisplay(int iIndex, int iIndex2);
		int			EnableChapter(int iIndex, bool bEnable);
		int			GetChapter(int iIndex, __int64* lpiBegin, __int64* lpiEnd, char* cText);
		int			GetChapter(CDynIntArray* aIndex, __int64* lpiBegin, __int64* lpiEnd, char* cText);
		int			GetChapter(int iIndex, SINGLE_CHAPTER_DATA* lpSCD);
		int			GetChapter(CDynIntArray* aIndex, SINGLE_CHAPTER_DATA* lpSCD);
		int			GetChapterCount();
		int			GetChapterDisplayCount(int iIndex);
		char*		GetChapterText(int iIndex, int iIndex2 = 0);
		char*		GetChapterLng(int iIndex, int iIndex2 = 0);
		int			HideChapter(int iIndex, bool bHidden);
		int			SetChapterBegin(int iIndex, __int64 iBegin);
		int			SetChapterText(int iIndex, char* cText, int iIndex2 = 0);
		int			SetChapterLng(int iIndex, char* cText, int iIndex2 = 0);
		int			SetChapterEnd(int iIndex, __int64 iEnd);
		int			SetChapterDisplayCount(int iIndex, int iNewCount);

		__int64		GetChapterBegin(int iIndex);
		__int64		GetChapterEnd(int iIndex);
		int			Delete();
		int			DeleteChapter(int iIndex);
		bool		HasSubChapters(int iIndex);
		int			Import(CChapters* c, __int64 iImportBias = 0, int iFlags = 0);
		int			ImportFromXML(XMLNODE* xmlNode);
		int			IsChapterEnabled(int iIndex);
		int			IsChapterHidden(int iIndex);
		CChapters*	GetSubChapters(int iIndex);	
		CChapters*	GetSubChapters(CDynIntArray* aIndex);	
		int			GetUID(int iIndex);
		int			SetUID(int iIndex, int iUID);
		int			CreateXMLTree_aux(XMLNODE** ppNode);
		int			CreateXMLTree(XMLNODE** ppNode);

};


#endif