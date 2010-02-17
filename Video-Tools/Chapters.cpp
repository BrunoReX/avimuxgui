#include "stdafx.h"
#include "chapters.h"
#include "matroska_segment.h"
#include "buffers.h"
#include "formattime.h"
#include "stdio.h"
#include "UID.h"

#ifdef DEBUG_NEW
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif


/*

  Todo: 
	* GetTags should define an error condition
	* replace stupid dynarrays with vectors

*/

char* physicalequiv2string(int index)
{
	switch (index) {
		case CHIPE_UNDEFINED: return "not specified"; break;
		case CHIPE_INDEX: return "Index"; break;
		case CHIPE_TRACK: return "Track"; break;
		case CHIPE_SESSION: return "Session"; break;
		case CHIPE_LAYER: return "Layer"; break;
		case CHIPE_SIDE: return "Side"; break;
		case CHIPE_MEDIUM: return "Medium"; break;
		case CHIPE_SET: return "Set"; break;
		default: return "<\?\?\?>"; break;
	}
	return "";
}

void __int642hex(__int64 value, char* cDest, int min_len, int group_size, int space)
{
	unsigned __int32* i = (unsigned __int32*)&value;
	unsigned __int16* i16 = (unsigned __int16*)&value;
	unsigned __int8* i8 = (unsigned __int8*)&value;

	char* s = ((space==0)?"":" ");

	if (min_len == 8 || i[1]) {
		if (group_size == 8)
			sprintf(cDest, "%08X%08X", i[0],i[1]);
		if (group_size == 4)
			sprintf(cDest, "%08X%s%08X", i[1],s,i[0]);
		if (group_size == 2)
			sprintf(cDest, "%04X%s%04X%s%04X%s%04X", i16[3],s,i16[2],s,i16[1],s,i16[0]);
		if (group_size == 1)
			sprintf(cDest, "%02X%s%02X%s%02X%s%02X%s%02X%s%02X%s%02X%s%02X", 
			i8[7],s,i8[6],s,i8[5],s,i8[4],s,i8[3],s,i8[2],s,i8[1],s,i8[0]);
	} else {
		if (group_size >= 4)
			sprintf(cDest, "%08X", i[0]);
		if (group_size == 2)
			sprintf(cDest, "%04X%s%04X", i16[1],s,i16[0]);
		if (group_size == 1)
			sprintf(cDest, "%02X%s%02X%s%02X%s%02X", i8[3],s,i8[2],s,i8[1],s,i8[0]);
	}
}

void __int128hex(char* value, char* cDest, int group_size, int space)
{
	unsigned __int32* i32 = (unsigned __int32*)value;
	unsigned __int16* i16 = (unsigned __int16*)value;
	unsigned __int8 * i8  = (unsigned __int8*)value;

	char* s;
	if (space) s = " "; else s="";

	if (group_size == 16)
		sprintf(cDest, "%08X%08X%08X%08X", i32[3],i32[2],i32[1],i32[0]);
	else
	if (group_size == 8)
		sprintf(cDest, "%08X%08X%s%08X%08X", i32[1],i32[0],s,i32[3],i32[2]);
	else
	if (group_size == 4)
		sprintf(cDest, "%08X%s%08X%s%08X%s%08X", i32[0],s,i32[1],s,i32[2],s,i32[3]);
	else
	if (group_size == 2)
		sprintf(cDest, "%04X%s%04X%s%04X%s%04X%s%04X%s%04X%s%04X%s%04X", 
		i16[0],s,i16[1],s,i16[2],s,i16[3],s,i16[4],s,i16[5],s,i16[6],s,i16[7]);
	else
	if (group_size == 1)
		sprintf(cDest, "%02X%s%02X%s%02X%s%02X%s%02X%s%02X%s%02X%s%02X%s%02X%s%02X%s%02X%s%02X%s%02X%s%02X%s%02X%s%02X", 
		i8[0],s,i8[1],s,i8[2],s,i8[3],s,i8[4],s,i8[5],s,i8[6],s,i8[7],s,
		i8[8],s,i8[9],s,i8[10],s,i8[11],s,i8[12],s,i8[13],s,i8[14],s,i8[15]);
}

int hval(char x)
{
	if (x>='0' && x<='9') return x-48;
	if (x>='A' && x<='F') return x-55;
	if (x>='a' && x<='f') return x-87;

	return -1;
}

char* hex2int128(const char* in, char* out)
{
	int digit_count = 0;
	char* o = out;

	while (*in) {
		while (*in && *in==' ') in++;
		if (!*in)
			return (digit_count == 16)?o:NULL;
		if (*in == '0' && in[1]=='x') 
			in+=2;
		if (hval(in[0])>-1 && hval(in[1])>-1)
			*out++ = (hval(in[0])<<4) + hval(in[1]);
		else
			return 0;
		digit_count++;
		in+=2;
	}

	return (digit_count == 16)?o:NULL;
}

CHAPTER_INFO::CHAPTER_INFO()
{
	iUID = 0;
	iTimestart = 0;
	iTimeend = 0;
	iPhysicalEquiv = 0;
//	memset(&display, 0, sizeof(display));
	memset(&tracks, 0, sizeof(tracks));
	subchapters = NULL;
	csubchapters = NULL;
	bHidden = 0;
	bDefault = 0;
	bEnabled = 1;
	bOrdered = 0;
	bEdition = 0;
	iUIDLen = 0;
	memset(cSegmentUID, 0, sizeof(cSegmentUID));
	bSegmentUIDValid = 0;
}

CHAPTER_DISPLAY_INFO::CHAPTER_DISPLAY_INFO()
{
	cCountry = NULL;
	cLanguage = NULL;
	cString = NULL;
}

CHAPTER_DISPLAY_INFO::~CHAPTER_DISPLAY_INFO()
{
//	DecBufferRefCount(&cCountry);
//	DecBufferRefCount(&cLanguage);
//	DecBufferRefCount(&cString);
}

IMPORTCHAPTERSTRUCT::IMPORTCHAPTERSTRUCT()
{
	chapters = NULL;
	index_start = 0;
	index_end = CHAP_LAST;
	can_merge = 1;
	bias = 0;
	flags = 0;
}

CChapters::CChapters()
{
	chapters = (CHAPTERS*)calloc(1,sizeof(CHAPTERS));
	SetTimecodeScale(1);
	bSingleDefault = false;
	parent = NULL;
}

CChapters::CChapters(CHAPTERS* _chapters, int iBias)
{
	chapters = _chapters;
	SetBias(iBias,BIAS_UNSCALED);
	parent = NULL;
}

CChapters::~CChapters()
{
}

int CChapters::Map(int iIndex)
{
	if (iIndex == CHAP_LAST) return GetChapterCount()-1;
	return iIndex;
}

int CChapters::AddChapter(__int64 qwBegin, __int64 qwEnd, char* cText, int bEnabled, int bHidden)
{
	SINGLE_CHAPTER_DATA		scd;
	memset(&scd, 0, sizeof(scd));

	scd.bEnabled  = bEnabled;
	scd.bHidden   = bHidden;
	scd.iBegin    = qwBegin;
	scd.iEnd      = qwEnd;
	strcpy(scd.cText, cText);
	ZeroMemory(scd.cLng, sizeof(scd.cLng));
	strcpy(scd.cLng, "und");

	return AddChapter(&scd);
}

int CChapters::Swap(int iIndex1, int iIndex2)
{
	iIndex1 = Map(iIndex1);
	iIndex2 = Map(iIndex2);
	if (iIndex1 == iIndex2) 
		return 0;

	if (iIndex1 >= GetChapterCount()) 
		return 0;

	if (iIndex2 >= GetChapterCount())
		return 0;

	if (iIndex1 < 0 || iIndex2 < 0)
		return 0;

	CHAPTER_INFO* ch;
	ch = chapters->chapters[iIndex2];
	chapters->chapters[iIndex2] = chapters->chapters[iIndex1];
	chapters->chapters[iIndex1] = ch;

	return 1;
}

int CChapters::AddChapter(SINGLE_CHAPTER_DATA* pSCD)
{
	__int64 qwBegin = pSCD->iBegin;
	__int64 qwEnd   = pSCD->iEnd;
	int     bEnabled= pSCD->bEnabled;
	int     bHidden = pSCD->bHidden;
	char*   cText   = (char*)pSCD->cText;
	char*   cLng    = (char*)pSCD->cLng;
	CHAPTER_INFO*   c;

	if (!chapters) 	chapters = (CHAPTERS*)calloc(1,sizeof(CHAPTERS));

	int i = chapters->iCount;
	chapters->chapters = (CHAPTER_INFO**)realloc(chapters->chapters,(i+1)*sizeof(CHAPTER_INFO*));
	chapters->chapters[i] = new CHAPTER_INFO;

	c = chapters->chapters[i];
	c->iTimestart = qwBegin;
	c->iTimeend = qwEnd;
	c->bEnabled = bEnabled;
	c->bHidden = bHidden;
	c->bEdition = pSCD->bIsEdition;
	c->iUID = 0;
	c->iUIDLen = -1;
	c->bSegmentUIDValid = pSCD->bSegmentUIDValid;
	if (pSCD->bSegmentUIDValid)
		memcpy(c->cSegmentUID, pSCD->cSegmentUID, 16);

	if (pSCD->bIsEdition) {
		c->bOrdered = pSCD->bOrdered;
		c->bDefault = pSCD->bDefault;
	}
	else
		c->iPhysicalEquiv = pSCD->iPhysicalEquiv;

	CHAPTER_DISPLAY*	cdi = &c->display;

	chapters->iCount++;

	if (cText && *cText || cLng && *cLng) {
		CHAPTER_DISPLAY_INFO cdi;
		
		if (cText && *cText)
			cdi.cString = new CStringBuffer(cText);
		else
			cdi.cString = new CStringBuffer("");
		
		if (cLng && *cLng)
			cdi.cLanguage = new CStringBuffer(cLng);
		else
			cdi.cLanguage = new CStringBuffer("und");

		c->display.push_back(cdi);
	} 

	bSingleDefault = 0;

	return i;
}

int CChapters::AddEmptyEdition()
{
	SINGLE_CHAPTER_DATA scd; memset(&scd, 0, sizeof(scd));

	scd.bIsEdition = 1;
	scd.iEnd = -1;
	
	return AddChapter(&scd);
}

int CChapters::SetChapterBegin(int iIndex,__int64 iBegin)
{
	iIndex = Map(iIndex);

	if (!IsSegmentUIDValid(iIndex))
		chapters->chapters[iIndex]->iTimestart = iBegin + GetBias(BIAS_UNSCALED);
	else
		chapters->chapters[iIndex]->iTimestart = iBegin;

	return 0;
}

int CChapters::SetChapterText(int iIndex, const char* cText, int iIndex2)
{
	iIndex = Map(iIndex);
	CHAPTER_DISPLAY&	cdi = chapters->chapters[iIndex]->display;
	
	if (iIndex2 >= (int)cdi.size())
		SetChapterDisplayCount(iIndex, iIndex2+1);

	if (cdi[iIndex2].cString) 
		DecBufferRefCount(&cdi[iIndex2].cString);
	
	cdi[iIndex2].cString = new CStringBuffer(cText, CBN_REF1 | CSB_UTF8);
	
	return 0;
}

int CChapters::SetChapterDisplayCount(int iIndex, int iNewCount)
{
	iIndex = Map(iIndex);
	CHAPTER_DISPLAY& cd = chapters->chapters[iIndex]->display;

	if (iNewCount>(int)cd.size()) {
		for (int j=cd.size();j<iNewCount;j++) {
			CHAPTER_DISPLAY_INFO info;
			cd.push_back(info);
		}
	}
	
	if (iNewCount<(int)cd.size()) {
		for (int j=(int)cd.size()-1;j>=iNewCount;j--) {
			if (cd[j].cLanguage) DecBufferRefCount(&cd[j].cLanguage);
			if (cd[j].cString) DecBufferRefCount(&cd[j].cString);
			cd.pop_back();
		}
	}

	return iNewCount;
}

int CChapters::GetChapterDisplayCount(int iIndex)
{
	iIndex = Map(iIndex);
	if (iIndex < 0 || iIndex >= GetChapterCount())
		return -1;

	return (int)chapters->chapters[iIndex]->display.size();
}

int CChapters::SetChapterLng(int iIndex, const char* cText, int iIndex2)
{
	iIndex = Map(iIndex);
	CHAPTER_DISPLAY&	cd = chapters->chapters[iIndex]->display;
	
	if (iIndex2>=(int)cd.size()) 
		SetChapterDisplayCount(iIndex, iIndex2+1);

	if (cd[iIndex2].cLanguage) 
		DecBufferRefCount(&cd[iIndex2].cLanguage);

	cd[iIndex2].cLanguage = new CStringBuffer(cText, CBN_REF1 | CSB_UTF8);

	return 0;
}


int CChapters::GetChapter(CDynIntArray* aIndex, __int64* lpiBegin,
						  __int64* lpiEnd, char* cText)
{
	CChapters* cLevel = this;

	for (int i=0;i<aIndex->GetCount()-1;i++) {
		if (cLevel->HasSubChapters(aIndex->At(i))) {
			cLevel = GetSubChapters(aIndex->At(i));
		} else {
			return CHAP_INVALIDINDEX;
		}
	}
	
	int iIndex = aIndex->At(aIndex->GetCount()-1);
	if (iIndex >= cLevel->GetChapterCount()) {
		return CHAP_INVALIDINDEX;
	}

	if (lpiBegin) *lpiBegin = cLevel->GetChapterBegin(iIndex);
	if (lpiEnd) *lpiEnd = cLevel->GetChapterEnd(iIndex);

	if (cText) 
		strcpy(cText,cLevel->chapters->chapters[iIndex]->display[0].cString->Get());

	return 0;
}

int CChapters::GetChapter(std::vector<int> index, __int64* pBegin,
						  __int64* pEnd, char* cText)
{
	

	return 0;
}


CChapters* CChapters::GetChapter(CDynIntArray* aIndex, int* iIndex)
{
	if (!aIndex)
		return 0;

	CChapters* cLevel = this;

	for (int i=0;i<aIndex->GetCount()-1;i++) {
		if (cLevel->HasSubChapters(aIndex->At(i))) {
			cLevel = GetSubChapters(aIndex->At(i));
		} else {
			return NULL;
		}
	}

	if (iIndex)
		*iIndex = aIndex->At(aIndex->GetCount()-1);

	return cLevel;
}

CChapters* CChapters::GetChapter(const std::vector<int>& index, int* lastIndex)
{
	std::vector<int>::const_iterator iter = index.begin();

	CChapters* cLevel = this;
	for (; iter+1 != index.end(); iter++)
	{
		if (cLevel->HasSubChapters(*iter)) {
			cLevel = GetSubChapters(*iter);
		} else {
			return NULL;
		}
	}

	if (lastIndex)
		*lastIndex = index[index.size()-1];

	return cLevel;
}

int CChapters::GetChapter(int iIndex, __int64* lpiBegin, __int64* lpiEnd, char* cText)
{
	iIndex=Map(iIndex);
	if (lpiBegin) *lpiBegin = chapters->chapters[iIndex]->iTimestart + GetBias(BIAS_UNSCALED);
	if (lpiEnd) *lpiEnd = GetChapterEnd(iIndex);

	if (GetChapterDisplayCount(iIndex)>=1 && cText)
		strcpy(cText,chapters->chapters[iIndex]->display[0].cString->Get());

	return 0;
}


int CChapters::GetChapter(CDynIntArray* aIndex, SINGLE_CHAPTER_DATA *lpSCD)
{
	CChapters* cLevel = this;

	for (int i=0;i<aIndex->GetCount()-1;i++) {
		if (cLevel->HasSubChapters(aIndex->At(i))) {
			cLevel = GetSubChapters(aIndex->At(i));
		} else {
			return CHAP_INVALIDINDEX;
		}
	}
	
	int iIndex = aIndex->At(aIndex->GetCount()-1);
	if (iIndex >= cLevel->GetChapterCount()) 
		return CHAP_INVALIDINDEX;
	
	if (!lpSCD) return 0;
	ZeroMemory(lpSCD, sizeof(*lpSCD));
	if (iIndex>=GetChapterCount()) return CHAP_INVALIDINDEX;
	
	lpSCD->iBegin = GetChapterBegin(iIndex);
	lpSCD->iEnd = GetChapterEnd(iIndex);

	strcpy(lpSCD->cText, chapters->chapters[iIndex]->display[0].cString->Get());
	lpSCD->bEnabled = chapters->chapters[iIndex]->bEnabled;
	lpSCD->bHidden = chapters->chapters[iIndex]->bHidden;
	
	if (lpSCD->bIsEdition = chapters->chapters[iIndex]->bEdition) {
		lpSCD->bOrdered = chapters->chapters[iIndex]->bOrdered;
		lpSCD->bDefault = chapters->chapters[iIndex]->bDefault;
	} else {
		lpSCD->iPhysicalEquiv = chapters->chapters[iIndex]->iPhysicalEquiv;
		lpSCD->bSegmentUIDValid = chapters->chapters[iIndex]->bSegmentUIDValid;
		if (lpSCD->bSegmentUIDValid)
			memcpy(lpSCD->cSegmentUID, chapters->chapters[iIndex]->cSegmentUID, 16);
	}

	return 0;
}

int CChapters::GetChapter(int iIndex, SINGLE_CHAPTER_DATA *lpSCD)
{
	CBuffer* b = NULL;

	iIndex=Map(iIndex);
	if (!lpSCD) 
		return 0;
	
	ZeroMemory(lpSCD, sizeof(*lpSCD));
	if (iIndex>=GetChapterCount() || iIndex < 0) 
		return CHAP_INVALIDINDEX;
	
	lpSCD->iBegin = GetChapterBegin(iIndex);
	lpSCD->iEnd = GetChapterEnd(iIndex);

	if (GetChapterDisplayCount(iIndex)>0) {
		CHAPTER_DISPLAY_INFO& cdi = chapters->chapters[iIndex]->display[0]; 
		if (cdi.cString) {
			strcpy(lpSCD->cText, cdi.cString->Get());
			b =                  cdi.cLanguage;
		} else
			lpSCD->cText[0] = 0;
	}
	else
		lpSCD->cText[0] = 0;

	char* c = NULL;
	if (b) c = (char*)chapters->chapters[iIndex]->display[0].cLanguage->GetData();
	if (c) 	strcpy(lpSCD->cLng, c); else lpSCD->cLng[0]=0;
	
	lpSCD->bEnabled   = chapters->chapters[iIndex]->bEnabled;
	lpSCD->bHidden    = chapters->chapters[iIndex]->bHidden;
	lpSCD->bIsEdition = chapters->chapters[iIndex]->bEdition;
	if (lpSCD->bIsEdition) {
		lpSCD->bDefault = IsDefault(iIndex);//chapters->chapters[iIndex]->bDefault;
		lpSCD->bOrdered = chapters->chapters[iIndex]->bOrdered;
	} else {
		lpSCD->iPhysicalEquiv = chapters->chapters[iIndex]->iPhysicalEquiv;
		lpSCD->bSegmentUIDValid = chapters->chapters[iIndex]->bSegmentUIDValid;
		if (lpSCD->bSegmentUIDValid)
			memcpy(lpSCD->cSegmentUID, chapters->chapters[iIndex]->cSegmentUID, 16);
	}

	return 0;
}

char* CChapters::GetChapterText(int iIndex, int iIndex2)
{
	iIndex=Map(iIndex);
	if (iIndex>=GetChapterCount() || iIndex < 0) 
		return (char*)CHAP_INVALIDINDEX;

	if (iIndex2 >= GetChapterDisplayCount(iIndex) || iIndex2 < 0)
		return NULL;

	if (!chapters->chapters[iIndex]->display[iIndex2].cString)
		return NULL;

	return (char*)chapters->chapters[iIndex]->display[iIndex2].cString->GetData();
}

char* CChapters::GetChapterText(int iIndex, char* cLng, int behaviour)
{
	iIndex = Map(iIndex);

	if (iIndex >= GetChapterCount() || iIndex < 0)
		return NULL;

	if (!GetChapterDisplayCount(iIndex))
		return NULL;

	bool allow_und = !!(behaviour & CHAP_GCT_ALLOW_UND);

	int j = FindChapterDisplayLanguageIndex(iIndex, cLng, allow_und);

	if (j == -1) {
		if (behaviour == CHAP_GCT_RETURN_NULL)
			return NULL;
		else
			return GetChapterText(iIndex, 0);
	} else 
		return GetChapterText(iIndex, j);
}

char* CChapters::GetChapterLng(int iIndex, int iIndex2)
{
	iIndex=Map(iIndex);
	if (iIndex >= GetChapterCount() || iIndex < 0) return (char*)CHAP_INVALIDINDEX;
	if (iIndex2 >= GetChapterDisplayCount(iIndex) || iIndex2 < 0) return "";

	CHAPTER_DISPLAY_INFO& c = chapters->chapters[iIndex]->display[iIndex2];
	
	if (c.cLanguage)
		return (char*)c.cLanguage->GetData();
	else
		return "";
}

int CChapters::DeleteChapterDisplay(int iIndex, int iIndex2)
{
	if (iIndex >= GetChapterCount() || iIndex < 0) return CHAP_INVALIDINDEX;
	if (iIndex2 >= GetChapterDisplayCount(iIndex) || iIndex2 < 0) return CHAP_INVALIDINDEX;

	for (int i=iIndex2;i<GetChapterDisplayCount(iIndex)-1;i++) {
		SetChapterText(iIndex, GetChapterText(iIndex, i+1), i);
		SetChapterLng(iIndex, GetChapterLng(iIndex, i+1), i);
	}

	SetChapterDisplayCount(iIndex, GetChapterDisplayCount(iIndex)-1);

	return 0;
}

__int64 CChapters::GetChapterEnd(int iIndex)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;

	__int64 i = chapters->chapters[Map(iIndex)]->iTimeend;

	if (i==-1) return i;
	if (!IsSegmentUIDValid(iIndex))
		i = i + GetBias(BIAS_UNSCALED);

	if (i==-1) return -2;
	return i;
}

int CChapters::IsEnabled(int iIndex)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;

	if (iIndex==-1) return 0;
	return chapters->chapters[iIndex]->bEnabled;
}

int CChapters::IsEdition(int iIndex)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;

	return !!(chapters->chapters[iIndex]->bEdition);
}

int CChapters::IsDefault(int iIndex)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;

	if (!bSingleDefault)
		AssureSingleDefault();

	return !!(chapters->chapters[iIndex]->bDefault);
}

int CChapters::AssureSingleDefault()
{
	int j=GetChapterCount()-1;

	while ((!IsEdition(j) || !chapters->chapters[j]->bDefault) && j--);

	if (j>=0)
		SetIsDefault(j, 1);
	else
		SetIsDefault(0, 1);

	bSingleDefault = 1;
	
	return 1;
}

int CChapters::IsOrdered(int iIndex)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;

	return !!(chapters->chapters[iIndex]->bOrdered);
}

int CChapters::IsHidden(int iIndex)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;


	return chapters->chapters[iIndex]->bHidden;
}

int CChapters::EnableChapter(int iIndex, bool bEnable)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;

	return (chapters->chapters[iIndex]->bEnabled = bEnable);
}

int CChapters::SetIsEdition(int iIndex, bool bEnable)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;

	return (chapters->chapters[iIndex]->bEdition = !!bEnable);
}

int CChapters::SetIsDefault(int iIndex, bool bEnable)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;

	for (int j=0;j<GetChapterCount();j++)
		chapters->chapters[j]->bDefault = 0;

	return (chapters->chapters[iIndex]->bDefault = !!bEnable);
}

int CChapters::SetIsOrdered(int iIndex, bool bEnable)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;

	return (chapters->chapters[iIndex]->bOrdered = !!bEnable);
}

int CChapters::HideChapter(int iIndex,bool bHidden)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;

	return (chapters->chapters[iIndex]->bHidden = bHidden);
}

__int64 CChapters::GetChapterBegin(int iIndex)
{
	iIndex = Map(iIndex);

	if (iIndex < 0)
		return CHAP_INVALIDINDEX;

	if (iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;

	if (IsSegmentUIDValid(iIndex))
		return chapters->chapters[Map(iIndex)]->iTimestart;

	return chapters->chapters[Map(iIndex)]->iTimestart + GetBias(BIAS_UNSCALED);
}

int CChapters::SetSegmentUID(int iIndex, bool bValid, char* cUID)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;

	chapters->chapters[iIndex]->bSegmentUIDValid = bValid;

	if (bValid) {
		memcpy(chapters->chapters[iIndex]->cSegmentUID, cUID, 16);
		GetParent(1)->SetIsOrdered(iIndex, 1);
	}

	return 1;
}

bool CChapters::IsSegmentUIDValid(int iIndex)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;

	return !!chapters->chapters[iIndex]->bSegmentUIDValid;
}

int CChapters::SetChapterEnd(int iIndex, __int64 iEnd)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;

	__int64 i = iEnd; __int64* j = &chapters->chapters[iIndex]->iTimeend;
	if (i==-1) {
		*j=-1;
		return 0;
	}

	if (!IsSegmentUIDValid(iIndex))
		i-=GetBias(BIAS_UNSCALED);

	if (i==-1) i=-2;
	*j = i;

	return 0;
}

int CChapters::SetChapterPhysicalEquiv(int iIndex, int iPhysicalEquiv)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;

	chapters->chapters[Map(iIndex)]->iPhysicalEquiv = iPhysicalEquiv;
	
	return 0;
}

int CChapters::GetChapterPhysicalEquiv(int iIndex)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;

	return chapters->chapters[Map(iIndex)]->iPhysicalEquiv;
}

int CChapters::GetChapterCount()
{
	return (chapters?chapters->iCount:0);
}

int CChapters::Delete()
{
	for (int i=GetChapterCount()-1;i>=0;DeleteChapter(i--));
	if (chapters) delete chapters;
	chapters = NULL;

	return 0;
}

int CChapters::DeleteChapter(int iIndex)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;

	CHAPTER_INFO*	c = chapters->chapters[iIndex];

	SetChapterDisplayCount(iIndex, 0);

	if (HasSubChapters(iIndex)) {
		CChapters* sc = GetSubChapters(iIndex);
		sc->Delete();
		delete sc;
	}
	delete c;

	for (int i=iIndex;i<GetChapterCount()-1;i++) {
		chapters->chapters[i] = chapters->chapters[i+1];
	}

	chapters->iCount--;
	if (!chapters->iCount) {
		delete chapters->chapters;
		chapters->chapters = NULL;
	}

	return 0;
}

CChapters* CChapters::GetSubChapters(int iIndex)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return (CChapters*)CHAP_INVALIDINDEX;

	HasSubChapters(iIndex);

	CChapters* cSub;
	iIndex = Map(iIndex);
	if (cSub = (CChapters*)chapters->chapters[iIndex]->csubchapters) {
		cSub->SetBias(GetBias(BIAS_UNSCALED),BIAS_UNSCALED);
		return cSub;
	}

	void** pChapters = &chapters->chapters[iIndex]->subchapters;
	if (!*pChapters) *pChapters = calloc(1,sizeof(CHAPTERS));

	cSub = new CChapters((CHAPTERS*)*pChapters);
	chapters->chapters[iIndex]->csubchapters=(void*)cSub;
	cSub->SetBias(GetBias(BIAS_UNSCALED),BIAS_UNSCALED);

	HasSubChapters(iIndex);

	return cSub;
}

CChapters* CChapters::GetSubChapters(CDynIntArray* aIndex) 
{
	CChapters* a = this;

	for (int i=0;i<aIndex->GetCount();a=a->GetSubChapters(aIndex->At(i++)));
	return a;
}


bool CChapters::HasSubChapters(int iIndex)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return false;

	CChapters* c = (CChapters*)chapters->chapters[Map(iIndex)]->csubchapters;

	if (c) 
		c->SetParent(this);

	return (!!chapters->chapters[Map(iIndex)]->subchapters);
}

void CChapters::SetParent(CChapters* P)
{
	parent = P;

	if (P)
		SetBias(P->GetBias(BIAS_UNSCALED), BIAS_UNSCALED);

}
	
/* top_level:
		0: return parent element. Returns NULL if level is already top level
		1: return top_level parent. Returns <this> when level is already top_level
*/
CChapters* CChapters::GetParent(int top_level)
{
	if (!top_level)
		return parent;

	CChapters* P = this;
	while (P->GetParent(0) && (P = P->GetParent(0)));

	return P;
}
	
__int64 CChapters::GenerateUID(int iIndex)
{
	__int64*	iUID = &chapters->chapters[iIndex]->iUID;
	iIndex = Map(iIndex);

	*iUID = 0;
	int uid_len = GetUIDLen(iIndex);

	generate_uid((char*)iUID, uid_len);

	return *iUID;
}			

int CChapters::SetUIDLen(int iIndex, int len, int flags)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;
	
	if (len < 1 || len > 8)
		return 0;

	chapters->chapters[iIndex]->iUIDLen = len;
	chapters->chapters[iIndex]->iUID    = 0;

	if ((flags & CHAP_SETUIDLEN_RECURSIVE) && HasSubChapters(iIndex)) 
		for (int j=GetSubChapters(iIndex)->GetChapterCount()-1;j>=0;j--) 
			GetSubChapters(iIndex)->SetUIDLen(j, len, flags);

	return 1;
}

int CChapters::GetUIDLen(int iIndex)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;

	if (chapters->chapters[iIndex]->iUIDLen > 0)
		return chapters->chapters[iIndex]->iUIDLen;

	if (!chapters->chapters[iIndex]->iUID)
		return (chapters->chapters[iIndex]->iUIDLen=8);

	if (!chapters->chapters[iIndex]->iUID32[1]) 
		return (chapters->chapters[iIndex]->iUIDLen=4);

	return (chapters->chapters[iIndex]->iUIDLen=8);
}

__int64 CChapters::GetUID(int iIndex) 
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;

	__int64*	iUID = &chapters->chapters[iIndex]->iUID;

	if (*iUID) return *iUID; else return GenerateUID(iIndex);
}

int CChapters::SetUID(int iIndex, __int64 iUID)
{
	iIndex = Map(iIndex);

	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;

	chapters->chapters[Map(iIndex)]->iUID = iUID;
	chapters->chapters[Map(iIndex)]->iUIDLen = 0;

	return 0;
}

CChapters* CChapters::FindUID(__int64 UID, int bEdition, int* pIndex)
{
	int c = GetChapterCount();

	for (int i=0;i<c;i++) {
		if (UID == GetUID(i) && (!!bEdition == !!IsEdition(i))) {
			if (pIndex) *pIndex = i;
			return this;
		}

		if (HasSubChapters(i)) {
			CChapters* pChapter = GetSubChapters(i)->FindUID(UID, bEdition, pIndex);
			if (pChapter) return pChapter;
		}
	}

	return 0;
}

int CChapters::FindChapterDisplayLanguageIndex(int iIndex, const char* cLng, int allow_und)
{
	if (!cLng)
		return -1;

	/* all undefined chapter languages are different languages */
	if (!strcmp(cLng, "und") && !allow_und)
		return -1;

	int c = GetChapterDisplayCount(iIndex);

	for (int i=0;i<c;i++) {
		if (!strcmp(GetChapterLng(iIndex, i), cLng))
			return i;
	}

	return -1;
}

void CChapters::InvalidateUID(int iIndex)
{
	iIndex = Map(iIndex);

	if (iIndex >= GetChapterCount()) 
		return;

	GetUIDLen(iIndex);
	SetUID(iIndex, 0);
	if (HasSubChapters(iIndex))
		for (int j=0;j<GetSubChapters(iIndex)->GetChapterCount();j++)
			GetSubChapters(iIndex)->InvalidateUID(j);

	return;
}

TAG_INDEX_LIST& CChapters::GetTags(int iIndex) 
{
	return chapters->chapters[iIndex]->pTags;
}

int CChapters::GetChapterSize(int iIndex, int flags)
{
	iIndex = Map(iIndex);
	
	if (iIndex < 0 || iIndex >= GetChapterCount())
		return CHAP_INVALIDINDEX;

	size_t size = 0;
	SINGLE_CHAPTER_DATA scd;
	GetChapter(iIndex, &scd); 

	// CRC
	size += 6;

	// chapter_uid
	size += 3 + GetUIDLen(iIndex);

	if (!scd.bIsEdition) {
		// timecode start
		size += 2 + UIntLen(scd.iBegin);
		// timecode end
		size += 2 + (scd.iEnd == -1)?8 : UIntLen(scd.iEnd);
	}
	
	// hidden
	if (scd.bHidden)
		size += (scd.bIsEdition?4:3);

	// enabled
	if (!scd.bIsEdition && !scd.bEnabled)
		size += 4;

	// default
	if (scd.bIsEdition && scd.bDefault)
		size += 4;

	// ordered
	if (scd.bOrdered && scd.bOrdered)
		size += 4;

	// segment uid
	if (scd.bSegmentUIDValid && !scd.bIsEdition) {
		size += 19;	// chaptersegmentuid element
		size += 63 + GetUIDLen(iIndex); // chaptersegmentuid tag
	}

	// physical equiv
	if (scd.iPhysicalEquiv && !scd.bIsEdition)
		size += 3 + UIntLen(scd.iPhysicalEquiv);

	// chapter display
	int j = GetChapterDisplayCount(iIndex);

	if (scd.bIsEdition) {
		// editions get their title through tags -> needs tag, crc, target
		size += 6; // CRC for Tag element
		size += 6 + GetUIDLen(iIndex); // Target EditionUID
		size += 4; // Tag ID etc

		// need about 25 bytes + title len per language
		for (int i = 0;i<j;i++) {
			size_t str_len = strlen(GetChapterText(iIndex, i));
			size_t lng_len = strlen(GetChapterLng(iIndex, i));
			size += 25 + str_len + lng_len;			
		}
	} else {

		for (int i = 0;i<j;i++) {
			size_t cdsize = 0;

			size_t str_len = strlen(GetChapterText(iIndex, i));
			cdsize += 1 + str_len + EBMLUIntLen(str_len);
	
			str_len = strlen(GetChapterLng(iIndex, i));
			cdsize += 2 + str_len + EBMLUIntLen(str_len);

			cdsize += 1 + EBMLUIntLen(cdsize);
			size += cdsize;
		}
	}


	if (HasSubChapters(iIndex)) {
		size += GetSubChapters(iIndex)->GetSize(flags);
	}

	// chapter atom
	size += (scd.bIsEdition?2:1) + EBMLUIntLen(size);

	return (int)size;
}

int CChapters::GetSize(int flags)
{
	int size = 0;
	for (int i = GetChapterCount()-1;i>=0;i--)
		size += GetChapterSize(i, flags);

	return size;
}


int CChapters::Import(CChapters* c, __int64 iImportBias, int iFlags, int index_start, int index_end, bool can_merge)
{
	int k;

	if (index_end == CHAP_LAST) 
		index_end = c->GetChapterCount()-1;
	if (index_start > index_end)
		return 0;

	int old_count = GetChapterCount();

	bSingleDefault = 0;

	for (int i=index_start;i<=index_end;i++) {
		SINGLE_CHAPTER_DATA scd;
		c->GetChapter(i,&scd);
		if (!scd.bSegmentUIDValid && !scd.bIsEdition) {
			scd.iBegin += iImportBias;
			if (scd.iEnd!=-1) scd.iEnd += iImportBias;
		}

		bool edition = !!c->IsEdition(i);
		bool merged_edition = false;
		bool merged_chapter = false;

		/* if EditioUIDs are equal, merge the editions */
		if (can_merge) if (GetChapterCount() && c->IsEdition(i) && IsEdition(CHAP_LAST)) {
			for (int j=0;j<GetChapterCount();j++) {
				if (c->GetUID(i) == GetUID(j)) {
					merged_edition = true;
					GetSubChapters(j)->Import(c->GetSubChapters(i), iImportBias);
				}
			}
		}

		int index = 0;
		CChapters* p = GetParent(1)->FindUID(c->GetUID(i),c->IsEdition(i), &index); 

		/* if beginning of chapter to import matches end of existing chapter with
		   the same ID, merge them */
		if (can_merge) if (GetChapterCount() && !IsEdition(CHAP_LAST)) {
			if (p && p->GetChapterEnd(index) <= c->GetChapterBegin(i) + iImportBias) {
				p->SetChapterEnd(index, c->GetChapterEnd(i) + iImportBias);
				merged_chapter = true;
			} 
		}

		if (!merged_edition && !merged_chapter) {
			k = AddChapter(&scd);

			if (!p)
				SetUID(CHAP_LAST, c->GetUID(i));
			else
				SetUID(CHAP_LAST, 0);
		  
		} else {
	//		SetChapterEnd(CHAP_LAST,scd.iEnd);
		}

		k = GetChapterCount()-1;

		for (int j=1;j<c->GetChapterDisplayCount(i);j++) {
			char* t, *l;
			t = c->GetChapterText(i, j);
			l = c->GetChapterLng(i, j);
			SetChapterText(k, t, j);
			SetChapterLng(k, l, j);
		}

		if (!merged_edition && c->HasSubChapters(i)) {
			GetSubChapters(CHAP_LAST)->Import(c->GetSubChapters(i),iImportBias);
		}
	}

	int iRes = c->GetChapterCount();
	
	if (iFlags & CHAP_IMPF_DELETE) delete c;

	return old_count;
}

int CChapters::Import(IMPORTCHAPTERSTRUCT& import)
{
	return Import((CChapters*)import.chapters, import.bias, import.flags,
		import.index_start, import.index_end, import.can_merge);
}

void CChapters::CopyChapterDisplay(CChapters* cSource, int iIndexSource, int iIndexDest)
{
	iIndexDest = Map(iIndexDest);
	iIndexSource = cSource->Map(iIndexSource);

	if (iIndexSource < 0 || iIndexDest < 0)
		return;

	SetChapterDisplayCount(iIndexDest, cSource->GetChapterDisplayCount(iIndexSource));

	for (int j=cSource->GetChapterDisplayCount(iIndexSource)-1;j>=0;j--) {
		SetChapterLng(iIndexDest, cSource->GetChapterLng(iIndexSource,j), j);
		SetChapterText(iIndexDest, cSource->GetChapterText(iIndexSource,j), j);
	}
}

#define IS(b,a) (!_stricmp(b->cNodeName.c_str(), a))

unsigned __int64 stroui64(const char* src)
{
	unsigned __int64 result = 0;
	while (*src)
	{
		result = result * 10 + (-48 + *src++);
	}

	return result;
}

int CChapters::ImportFromXML(XMLNODE* xmlNode, int depth)
{
	SINGLE_CHAPTER_DATA scd; int i; int j;
	ZeroMemory(&scd,sizeof(scd));
	bool b = true; bool failed=true;
	int res = 0;
	char cBuffer[32]; memset(cBuffer, 0, sizeof(cBuffer));

	if (!xmlNode) 
		return CHAP_IMPXML_OK;
	else

	if IS(xmlNode,"Segment") {

		TOPO_SORT_ENTRY	tsle[1] = {
			{ 1, "Chapters", "Tags" }
		};
		TOPO_SORT_LIST  tsl;
		tsl.iCount = 1;
		tsl.entries = (TOPO_SORT_ENTRY**)calloc(1,sizeof(TOPO_SORT_ENTRY*));
		tsl.entries[0] = &tsle[0];

		xmlTopoSort((XMLNODE*)xmlNode->pChild, (XMLNODE**)&xmlNode->pChild, &tsl);

		return ImportFromXML((XMLNODE*)xmlNode->pChild, depth + 1);
	} else

	if (!_stricmp(xmlNode->cNodeName.c_str(), "Chapters")) {
		if ((res = ImportFromXML((XMLNODE*)xmlNode->pChild, depth + 1)) && res != CHAP_IMPXML_OK)
			return res;
	} else
	if (!_stricmp(xmlNode->cNodeName.c_str(), "EditionEntry")) {
		bSingleDefault = 0;
		scd.bIsEdition = 1;
		XMLNODE* xmlCurr = (XMLNODE*)xmlNode->pChild;
		j = 0;
		unsigned __int64 uid = 0;
		while (xmlCurr && xmlCurr->cNodeName.size()) {
			if (!_stricmp(xmlCurr->cNodeName.c_str(), "EditionFlagHidden")) 
				scd.bHidden = !!atoi(xmlCurr->cValue.c_str()); else
			if (!_stricmp(xmlCurr->cNodeName.c_str(), "EditionFlagOrdered"))
				scd.bOrdered = !!atoi(xmlCurr->cValue.c_str()); else
			if (!_stricmp(xmlCurr->cNodeName.c_str(), "EditionManaged"))
				scd.bOrdered = !!atoi(xmlCurr->cValue.c_str()); else
			if (!_stricmp(xmlCurr->cNodeName.c_str(), "EditionFlagDefault"))
				scd.bDefault = !!atoi(xmlCurr->cValue.c_str()); else
			if (!_stricmp(xmlCurr->cNodeName.c_str(), "EditionUID"))
				sscanf(xmlCurr->cValue.c_str(), "%I64u", &uid);
//				uid = _atoi64(xmlCurr->cValue);
			
			xmlCurr = (XMLNODE*)xmlCurr->pNext;
		}

		CChapters* e = NULL; int iIndex = 0;
		e = FindUID(uid, 1, &iIndex);
		if (e) {
			return CHAP_IMPXML_NONUNIQUE_UID;
		}

		AddChapter(&scd);
		
		SetUID(CHAP_LAST, uid);
		
		res = GetSubChapters(CHAP_LAST)->ImportFromXML((XMLNODE*)xmlNode->pChild, depth + 1);

		if (res == CHAP_IMPXML_UNKNOWN_ELEMENT) {
			char msg[1024]; msg[0]=0;
			sprintf(msg, "Error parsing edition #%d: Unknown child in <EditionEntry>", GetChapterCount(),
				xmlNode->cNodeName);
			MessageBoxA(0, msg, "XML Chapter File Parser Error", MB_OK | MB_ICONERROR);
			res = CHAP_IMPXML_OK;				 
		}

		if (res != CHAP_IMPXML_OK)
			return res;
	} else
	if (!_strnicmp(xmlNode->cNodeName.c_str(), "Edition", 7)) {

	} else
    if (!_stricmp(xmlNode->cNodeName.c_str(), "ChapterAtom")) {
		// import a chapter
		scd.iEnd = -1;
		scd.bHidden = 0;
		scd.bEnabled = 1;
		
		strcpy(scd.cLng, "und");
		AddChapter(&scd);
		i = GetChapterCount()-1;
		XMLNODE* xmlCurr = (XMLNODE*)xmlNode->pChild;
		j = 0;
		while (xmlCurr && xmlCurr->cNodeName.c_str()) {
			unsigned __int64 uid = NULL;

			if IS(xmlCurr, "ChapterTimeStart") {
				SetChapterBegin(i, Str2Millisec((char*)xmlCurr->cValue.c_str()) * 1000000);
				failed=false;
			}

			if IS(xmlCurr, "ChapterTimeEnd") 
				SetChapterEnd(i, Str2Millisec((char*)xmlCurr->cValue.c_str()) * 1000000);

			if IS(xmlCurr, "ChapterFlagHidden")
				HideChapter(i, !!atoi(xmlCurr->cValue.c_str()));
			if IS(xmlCurr, "ChapterFlagEnabled") 
				EnableChapter(i, !!atoi(xmlCurr->cValue.c_str()));
			if IS(xmlCurr, "ChapterPhysicalEquiv")
				SetChapterPhysicalEquiv(i, atoi(xmlCurr->cValue.c_str()));
			if IS(xmlCurr, "ChapterSegmentUID")
				SetSegmentUID(i, !!hex2int128((char*)xmlCurr->cValue.c_str(), cBuffer), cBuffer);

			if IS(xmlCurr, "ChapterUID") {

				sscanf(xmlCurr->cValue.c_str(), "%I64u", &uid);
				//uid = _atoi64(xmlCurr->cValue);
				
			
				CChapters* c = NULL; int iIndex = NULL;
				c = FindUID(uid, 0,&iIndex);
				if (c) {
					return CHAP_IMPXML_NONUNIQUE_UID;
				}
				SetUID(i, uid);
			}

			if (b && IS(xmlCurr, "ChapterAtom")) {
				if ((res = GetSubChapters(i)->ImportFromXML(xmlCurr, depth + 1)) != CHAP_IMPXML_OK
					&& res != CHAP_IMPXML_UNKNOWN_ELEMENT) 
					return res;
				
				b=false;
			}

			if (!_stricmp(xmlCurr->cNodeName.c_str(), "ChapterDisplay")) {
				XMLNODE* disp = (XMLNODE*)xmlCurr->pChild;

				while (disp) {
					if (!_stricmp(disp->cNodeName.c_str(), "ChapterString")) 
						SetChapterText(i, disp->cValue.c_str(), j);
					if (!_stricmp(disp->cNodeName.c_str(), "ChapterLanguage"))
						SetChapterLng(i, disp->cValue.c_str(), j);
					disp = (XMLNODE*)disp->pNext;
				}

				j++;
			}
			xmlCurr = (XMLNODE*)xmlCurr->pNext;
		}
	} else

    if IS(xmlNode,"Tags") {
		int res = ImportFromXML((XMLNODE*)xmlNode->pChild, depth + 1);
		if (res != CHAP_IMPXML_OK)
			return res;
	} else

    if IS(xmlNode,"Tag") {
		XMLNODE* pFirst = (XMLNODE*)xmlNode->pChild;
		XMLNODE* pCurr  = pFirst;

		while (pCurr && _stricmp(pCurr->cNodeName.c_str(), "Targets"))
			pCurr = (XMLNODE*)pCurr->pNext;

		XMLNODE* pTargets = pCurr;
		XMLNODE* pFirstTarget = (XMLNODE*)pTargets->pChild;
		XMLNODE* pCurrTarget  = pFirstTarget;

		while (pCurrTarget) {
			CChapters* pChapter = NULL;
			int        iIndex   = 0;
			unsigned __int64    UID      = 0;
			pCurr = pFirst;

			if (!_stricmp(pCurrTarget->cNodeName.c_str(), "EditionUID")) {
				UID = stroui64(pCurrTarget->cValue.c_str());
				pChapter = FindUID(UID, 1, &iIndex);
				if (!pChapter) {
					pCurrTarget = (XMLNODE*)pCurrTarget->pNext;
					continue;
				}
			}

			while (pCurr) {
				if (!_stricmp(pCurr->cNodeName.c_str(), "Simple")) {
					XMLNODE* pSC_First = (XMLNODE*)pCurr->pChild;
					XMLNODE* pSC_Curr  = pSC_First;
					std::string cString;
					std::string cLanguage;
					std::string cName;

					while (pSC_Curr) {
						if (!_stricmp(pSC_Curr->cNodeName.c_str(), "Name"))
							cName      = pSC_Curr->cValue;
						if (!_stricmp(pSC_Curr->cNodeName.c_str(), "String"))
							cString    = pSC_Curr->cValue;
						if (!_stricmp(pSC_Curr->cNodeName.c_str(), "Language") || !_stricmp(pSC_Curr->cNodeName.c_str(), "TagLanguage"))
							cLanguage  = pSC_Curr->cValue;

						pSC_Curr = (XMLNODE*)pSC_Curr->pNext;
					}

					if (!_stricmp(cName.c_str(), "TITLE")) {
						int count = pChapter->GetChapterDisplayCount(iIndex);
						int index2 = count - 1;

						if (!cLanguage.empty()) {
							int found = pChapter->FindChapterDisplayLanguageIndex(iIndex, cLanguage.c_str());
							if (found > -1)
								index2 = found;
							
							pChapter->SetChapterLng(iIndex, cLanguage.c_str(), count);
						}

						if (!cString.empty()) 
							pChapter->SetChapterText(iIndex, cString.c_str(), count);
						
					}
				}
				pCurr = (XMLNODE*)pCurr->pNext;
			}
			pCurrTarget = (XMLNODE*)pCurrTarget->pNext;
		}
	} else
	if (depth == 0)
		return CHAP_IMPXML_NO_CHAPTER_FILE;
	else {
		return CHAP_IMPXML_UNKNOWN_ELEMENT;
	}

	return ImportFromXML((XMLNODE*)xmlNode->pNext, depth);
}

#undef IS

int CChapters::CreateXMLTree_aux(XMLNODE** ppNode)
{
	char cBegin[50]; cBegin[0]=0; 
	char cEnd[50]; cEnd[0]=0;
	char cTxt[50]; cTxt[0]=0;
	*ppNode = NULL;
	XMLNODE* pNode;

	for (int i=0;i<GetChapterCount();i++) {
		SINGLE_CHAPTER_DATA scd;
		GetChapter(i, &scd);

		if (scd.bIsEdition) {
			pNode = xmlAddSibling(ppNode, "EditionEntry", "");
			if (scd.bDefault)
				xmlAddChild(pNode, "EditionFlagDefault", "1");
			if (scd.bHidden)
				xmlAddChild(pNode, "EditionFlagHidden", "1");
			if (scd.bOrdered)
				xmlAddChild(pNode, "EditionFlagOrdered", "1");
			
			sprintf(cTxt, "%I64u", GetUID(i));
			pNode = xmlAddChild(pNode, "EditionUID", cTxt);
		} else {
			pNode = xmlAddSibling(ppNode, "ChapterAtom", "");
			XMLNODE* pDisp;

			Millisec2Str(scd.iBegin/1000000,cBegin);
			Millisec2Str(scd.iEnd/1000000,cEnd);
			xmlAddChild(pNode, "ChapterTimeStart", cBegin);
			if (scd.iEnd != -1) {
				xmlAddChild(pNode, "ChapterTimeEnd", cEnd);
			}
			if (scd.bHidden) 
				xmlAddChild(pNode, "ChapterFlagHidden", (scd.bHidden)?"1":"0");
			if (!scd.bEnabled) 
				xmlAddChild(pNode, "ChapterFlagEnabled", (scd.bEnabled)?"1":"0");
			if (scd.iPhysicalEquiv != CHIPE_UNDEFINED) {
				sprintf(cTxt,"%d", scd.iPhysicalEquiv);
				xmlAddChild(pNode, "ChapterPhysicalEquiv", cTxt);
			}
			sprintf(cTxt, "%I64u", GetUID(i));
			xmlAddChild(pNode, "ChapterUID", cTxt);
			if (scd.bSegmentUIDValid) {
				__int128hex(scd.cSegmentUID, cTxt, 1);
				xmlAddAttribute(xmlAddChild(pNode, "ChapterSegmentUID", cTxt), "format", "hex");
			}

			int c = GetChapterDisplayCount(i);
			for (int j=0;j<c;j++) {
				pDisp = xmlAddChild(pNode, "ChapterDisplay", "");
				xmlAddChild(pDisp, "ChapterString", GetChapterText(i, j));
				xmlAddChild(pDisp, "ChapterLanguage", GetChapterLng(i, j));
			}
			pNode = pDisp;
		}

		if (HasSubChapters(i)) 
			GetSubChapters(i)->CreateXMLTree_aux((XMLNODE**)&pNode->pNext);
		
		
	}

	return 0;
}

int CChapters::CreateXMLTree_tags(XMLNODE** ppNode)
{
	int j;
	XMLNODE* pTag;
	XMLNODE* pTarget;
	XMLNODE* pSimpleTag;
	char cTxt[50]; memset(cTxt,0,sizeof(cTxt));

	for (j=0;j<GetChapterCount();j++) 
		if (IsEdition(j)) {
			pTag = xmlAddSibling(ppNode, "Tag", "");
			sprintf(cTxt,"%I64u",GetUID(j));
			pTarget = xmlAddChild(pTag,"Targets","");
			xmlAddChild(pTarget,"EditionUID",cTxt);

			for (int k=0;k<GetChapterDisplayCount(j);k++) {
				pSimpleTag = xmlAddChild(pTag,"Simple","");
				xmlAddChild(pSimpleTag,"Name","TITLE");
				xmlAddChild(pSimpleTag,"String",GetChapterText(j,k));
				xmlAddChild(pSimpleTag,"TagLanguage",GetChapterLng(j,k));
			}
	}

	return 1;
}

int CChapters::CreateXMLTree(XMLNODE** ppNode, XMLNODE** ppChapters, XMLNODE** ppTags)
{
	XMLNODE* pNode = NULL;
	*ppNode = NULL;
	XMLNODE* pSegment = NULL;

	pSegment = xmlAddSibling(ppNode, "Segment", "");

	pNode = xmlAddChild(pSegment,"Chapters","");
	CreateXMLTree_aux((XMLNODE**)&pNode->pChild);

	pNode = xmlAddChild(pSegment,"Tags","");
	CreateXMLTree_tags((XMLNODE**)&pNode->pChild);

	if (ppChapters) {
		pNode = xmlAddSibling(ppChapters,"Chapters","");
		CreateXMLTree_aux((XMLNODE**)&pNode->pChild);
	}
	if (ppTags) {
		pNode = xmlAddSibling(ppTags,"Tags","");
		CreateXMLTree_tags((XMLNODE**)&pNode->pChild);
	}

	return 0;
}