#include "stdafx.h"
#include "chapters.h"
#include "matroska_segment.h"
#include "buffers.h"
#include "formattime.h"

CChapters::CChapters()
{
	chapters = (CHAPTERS*)calloc(1,sizeof(CHAPTERS));
	SetTimecodeScale(1);
}

CChapters::CChapters(CHAPTERS* _chapters, int iBias)
{
	chapters = _chapters;
	SetBias(iBias,BIAS_UNSCALED);
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

	scd.bEnabled = bEnabled;
	scd.bHidden  = bHidden;
	scd.iBegin   = qwBegin;
	scd.iEnd     = qwEnd;
	strcpy(scd.cText, cText);
	ZeroMemory(scd.cLng, sizeof(scd.cLng));
	strcpy(scd.cLng, "und");

	return AddChapter(&scd);
}

int CChapters::AddChapter(SINGLE_CHAPTER_DATA* pSCD)
{
	__int64 qwBegin = pSCD->iBegin;
	__int64 qwEnd   = pSCD->iEnd;
	int     bEnabled= pSCD->bEnabled;
	int     bHidden = pSCD->bHidden;
	char*   cText   = (char*)pSCD->cText;
	char*   cLng    = (char*)pSCD->cLng;

	if (!chapters) 	chapters = (CHAPTERS*)calloc(1,sizeof(CHAPTERS));

	int i = chapters->iCount;
	chapters->chapters = (CHAPTER_INFO**)realloc(chapters->chapters,(i+1)*sizeof(CHAPTER_INFO*));
	chapters->chapters[i] = (CHAPTER_INFO*)calloc(1,sizeof(CHAPTER_INFO));
	chapters->chapters[i]->iTimestart = qwBegin;
	chapters->chapters[i]->iTimeend = qwEnd;
	chapters->chapters[i]->bEnabled = bEnabled;
	chapters->chapters[i]->bHidden = bHidden;
	CHAPTER_DISPLAY*	cdi = &chapters->chapters[i]->display;
	cdi->iCount=1;
	cdi->cDisp = (CHAPTER_DISPLAY_INFO**)calloc(cdi->iCount,sizeof(void**));
	cdi->cDisp[0] = (CHAPTER_DISPLAY_INFO*)calloc(1,sizeof(CHAPTER_DISPLAY_INFO));
	cdi->cDisp[0]->cString = new CStringBuffer(cText);
	if (cLng && *cLng) cdi->cDisp[0]->cLanguage = new CStringBuffer(cLng); else cdi->cDisp[0]->cLanguage = new CStringBuffer("und");

	chapters->iCount++;

	return i;
}

int CChapters::SetChapterBegin(int iIndex,__int64 iBegin)
{
	chapters->chapters[Map(iIndex)]->iTimestart = iBegin + GetBias(BIAS_UNSCALED);
	return 0;
}

int CChapters::SetChapterText(int iIndex, char* cText, int iIndex2)
{
	iIndex = Map(iIndex);
	CHAPTER_DISPLAY*	cdi = &chapters->chapters[iIndex]->display;
	
	if (iIndex2>=cdi->iCount) {
		SetChapterDisplayCount(iIndex, iIndex2+1);
	}

	if (cdi->cDisp[iIndex2]->cString) DecBufferRefCount(&cdi->cDisp[iIndex2]->cString);
	cdi->cDisp[iIndex2]->cString = new CStringBuffer(cText);
	
	return 0;
}

int CChapters::SetChapterDisplayCount(int iIndex, int iNewCount)
{
	iIndex = Map(iIndex);
	CHAPTER_DISPLAY*	cdi = &chapters->chapters[iIndex]->display;

	if (iNewCount>cdi->iCount) {
		cdi->cDisp = (CHAPTER_DISPLAY_INFO**)realloc(cdi->cDisp, sizeof(CHAPTER_DISPLAY_INFO*)*(iNewCount));
		ZeroMemory(&cdi->cDisp[cdi->iCount], (iNewCount-cdi->iCount)*sizeof(CHAPTER_DISPLAY_INFO*));

		for (int j=cdi->iCount;j<iNewCount;j++) {
			cdi->cDisp[j] = new CHAPTER_DISPLAY_INFO;
			ZeroMemory(cdi->cDisp[j], sizeof(CHAPTER_DISPLAY_INFO));
			cdi->cDisp[j]->cString = new CStringBuffer("");
		}
	}

	if (iNewCount<cdi->iCount) {
		for (int j=iNewCount;j<cdi->iCount;j++) {
			if (cdi->cDisp[j]->cLanguage) DecBufferRefCount(&cdi->cDisp[j]->cLanguage);
			if (cdi->cDisp[j]->cString) DecBufferRefCount(&cdi->cDisp[j]->cString);
		}
		if (!iNewCount) delete cdi->cDisp;
	}

	cdi->iCount = iNewCount;

	return iNewCount;
}

int CChapters::GetChapterDisplayCount(int iIndex)
{
	iIndex = Map(iIndex);
	CHAPTER_DISPLAY*	cdi = &chapters->chapters[iIndex]->display;

	return cdi->iCount;
}

int CChapters::SetChapterLng(int iIndex, char* cText, int iIndex2)
{
	iIndex = Map(iIndex);
	CHAPTER_DISPLAY*	cdi = &chapters->chapters[iIndex]->display;
	
	if (iIndex2>=cdi->iCount) {
		SetChapterDisplayCount(iIndex, iIndex2+1);
	}

	if (cdi->cDisp[iIndex2]->cLanguage) DecBufferRefCount(&cdi->cDisp[iIndex2]->cLanguage);
	cdi->cDisp[iIndex2]->cLanguage = new CStringBuffer(cText);

	return 0;
}

int CChapters::GetChapter(CDynIntArray* aIndex, __int64* lpiBegin, __int64* lpiEnd, char* cText)
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
	if (cText) strcpy(cText,cLevel->chapters->chapters[iIndex]->display.cDisp[0]->cString->Get());
	return 0;
}

int CChapters::GetChapter(int iIndex, __int64* lpiBegin, __int64* lpiEnd, char* cText)
{
	iIndex=Map(iIndex);
	if (lpiBegin) *lpiBegin = chapters->chapters[iIndex]->iTimestart + GetBias(BIAS_UNSCALED);
	if (lpiEnd) *lpiEnd = GetChapterEnd(iIndex);
	if (cText) strcpy(cText,chapters->chapters[iIndex]->display.cDisp[0]->cString->Get());
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
	if (iIndex >= cLevel->GetChapterCount()) {
		return CHAP_INVALIDINDEX;
	}

	if (!lpSCD) return 0;
	ZeroMemory(lpSCD, sizeof(*lpSCD));
	if (iIndex>=GetChapterCount()) return CHAP_INVALIDINDEX;
	lpSCD->iBegin = GetChapterBegin(iIndex);//chapters->chapters[iIndex]->iTimestart + GetBias(BIAS_UNSCALED);
	lpSCD->iEnd = GetChapterEnd(iIndex);//chapters->chapters[iIndex]->iTimeend;
	strcpy(lpSCD->cText, chapters->chapters[iIndex]->display.cDisp[0]->cString->Get());
	lpSCD->bEnabled = chapters->chapters[iIndex]->bEnabled;
	lpSCD->bHidden = chapters->chapters[iIndex]->bHidden;

	return 0;
}

int CChapters::GetChapter(int iIndex, SINGLE_CHAPTER_DATA *lpSCD)
{
	iIndex=Map(iIndex);
	if (!lpSCD) return 0;
	ZeroMemory(lpSCD, sizeof(*lpSCD));
	if (iIndex>=GetChapterCount()) return CHAP_INVALIDINDEX;
	lpSCD->iBegin = GetChapterBegin(iIndex);//chapters->chapters[iIndex]->iTimestart + GetBias(BIAS_UNSCALED);
	lpSCD->iEnd = GetChapterEnd(iIndex);
	strcpy(lpSCD->cText, chapters->chapters[iIndex]->display.cDisp[0]->cString->Get());
	CBuffer* b = chapters->chapters[iIndex]->display.cDisp[0]->cLanguage;
	char* c = NULL;
	if (b) c = (char*)chapters->chapters[iIndex]->display.cDisp[0]->cLanguage->GetData();
	if (c) 	strcpy(lpSCD->cLng, c); else lpSCD->cLng[0]=0;
	lpSCD->bEnabled = chapters->chapters[iIndex]->bEnabled;
	lpSCD->bHidden = chapters->chapters[iIndex]->bHidden;

	return 0;
}

char* CChapters::GetChapterText(int iIndex, int iIndex2)
{
	iIndex=Map(iIndex);
	if (iIndex>=GetChapterCount()) return (char*)CHAP_INVALIDINDEX;

	return (char*)chapters->chapters[iIndex]->display.cDisp[iIndex2]->cString->GetData();
}

char* CChapters::GetChapterLng(int iIndex, int iIndex2)
{
	iIndex=Map(iIndex);
	if (iIndex>=GetChapterCount()) return (char*)CHAP_INVALIDINDEX;

	return (char*)chapters->chapters[iIndex]->display.cDisp[iIndex2]->cLanguage->GetData();
}

int CChapters::DeleteChapterDisplay(int iIndex, int iIndex2)
{
	if (iIndex >= GetChapterCount()) return CHAP_INVALIDINDEX;
	if (iIndex2 >= GetChapterDisplayCount(iIndex)) return CHAP_INVALIDINDEX;

	for (int i=iIndex2;i<GetChapterDisplayCount(iIndex)-1;i++) {
		SetChapterText(iIndex, GetChapterText(iIndex, i+1), i);
		SetChapterLng(iIndex, GetChapterLng(iIndex, i+1), i);
	}

	SetChapterDisplayCount(iIndex, GetChapterDisplayCount(iIndex)-1);

	return 0;
}

__int64 CChapters::GetChapterEnd(int iIndex)
{
	__int64 i = chapters->chapters[Map(iIndex)]->iTimeend;

	if (i==-1) return i;
	i = i + GetBias(BIAS_UNSCALED);
	if (i==-1) return -2;
	return i;
}

int CChapters::IsChapterEnabled(int iIndex)
{
	iIndex = Map(iIndex);

	if (iIndex==-1) return 0;
	return chapters->chapters[iIndex]->bEnabled;
}

int CChapters::IsChapterHidden(int iIndex)
{
	iIndex = Map(iIndex);

	if (iIndex==-1) return 0;
	return chapters->chapters[iIndex]->bHidden;
}

int CChapters::EnableChapter(int iIndex,bool bEnable)
{
	iIndex = Map(iIndex);

	if (iIndex==-1) return 0;
	return (chapters->chapters[iIndex]->bEnabled = bEnable);
}

int CChapters::HideChapter(int iIndex,bool bHidden)
{
	iIndex = Map(iIndex);

	if (iIndex==-1) return 0;
	return (chapters->chapters[iIndex]->bHidden = bHidden);
}

__int64 CChapters::GetChapterBegin(int iIndex)
{
	return chapters->chapters[Map(iIndex)]->iTimestart + GetBias(BIAS_UNSCALED);
}

int CChapters::SetChapterEnd(int iIndex, __int64 iEnd)
{
	__int64 i = iEnd; __int64* j = &chapters->chapters[Map(iIndex)]->iTimeend;
	if (i==-1) {
		*j=-1;
		return 0;
	}
	i-=GetBias(BIAS_UNSCALED);
	if (i==-1) i=-2;
	*j = i;

	return 0;
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
	CHAPTER_INFO*	c = chapters->chapters[iIndex];

	SetChapterDisplayCount(iIndex, 0);

/*	if (c->display.cDisp) {
		DecBufferRefCount(&c->display.cDisp[0]->cString);
		delete c->display.cDisp[0];
		delete c->display.cDisp;
	}
*/	
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
	return (!!chapters->chapters[Map(iIndex)]->subchapters);
}
		
int CChapters::GenerateUID(int iIndex)
{
	int*	iUID = &chapters->chapters[iIndex]->iUID;
	iIndex = Map(iIndex);

	*iUID = 0;
	for (int j=0;j<4;j++) *iUID=256**iUID+(rand()&0xFF);

	return *iUID;
}			

int CChapters::GetUID(int iIndex) 
{
	iIndex=Map(iIndex);
	int*	iUID = &chapters->chapters[iIndex]->iUID;

	if (*iUID) return *iUID; else return GenerateUID(iIndex);
}

int CChapters::SetUID(int iIndex, int iUID)
{
	chapters->chapters[Map(iIndex)]->iUID = iUID;

	return 0;
}

int CChapters::Import(CChapters* c, __int64 iImportBias,int iFlags)
{
	for (int i=0;i<c->GetChapterCount();i++) {
		SINGLE_CHAPTER_DATA scd;
		c->GetChapter(i,&scd);
		scd.iBegin += iImportBias;
		if (scd.iEnd!=-1) scd.iEnd += iImportBias;

		if (!GetChapterCount() || c->GetUID(i)!=GetUID(CHAP_LAST)) {
			AddChapter(&scd);
			SetUID(CHAP_LAST,c->GetUID(i));
		} else {
			SetChapterEnd(CHAP_LAST,scd.iEnd);
		}

		for (int j=1;j<c->GetChapterDisplayCount(i);j++) {
			char* t, *l;
			t = c->GetChapterText(i, j);
			l = c->GetChapterLng(i, j);
			SetChapterText(i, t, j);
			SetChapterLng(i, l, j);
		}

		if (c->HasSubChapters(i)) {
			GetSubChapters(CHAP_LAST)->Import(c->GetSubChapters(i),iImportBias);
		}
	}

	int iRes = c->GetChapterCount();
	
	if (iFlags & CHAP_IMPF_DELETE) delete c;

	return iRes;
}

int CChapters::ImportFromXML(XMLNODE* xmlNode)
{
	SINGLE_CHAPTER_DATA scd; int i; int j;
	ZeroMemory(&scd,sizeof(scd));
	bool b = true; bool failed=true;

	if (!xmlNode) return 0;

	if (!strcmp(xmlNode->cNodeName, "Chapters")) {
		ImportFromXML((XMLNODE*)xmlNode->pChild);
		return 1;
	}

	if (!strcmp(xmlNode->cNodeName, "EditionEntry")) {
		ImportFromXML((XMLNODE*)xmlNode->pChild);
		return 1;
	}

    if (!strcmp(xmlNode->cNodeName, "ChapterAtom")) {
		// import a chapter
		scd.iEnd = -1;
		scd.bHidden = 0;
		scd.bEnabled = 1;
		
		strcpy(scd.cLng, "end");
		AddChapter(&scd);
		i = GetChapterCount()-1;
		XMLNODE* xmlCurr = (XMLNODE*)xmlNode->pChild;
		j = 0;
		while (xmlCurr && xmlCurr->cNodeName) {
			if (!strcmp(xmlCurr->cNodeName, "ChapterTimeStart")) {
				SetChapterBegin(i, Str2Millisec(xmlCurr->cValue) * 1000000);
				failed=false;
			}

			if (!strcmp(xmlCurr->cNodeName, "ChapterTimeEnd")) {
				SetChapterEnd(i, Str2Millisec(xmlCurr->cValue) * 1000000);
			}

			if (!strcmp(xmlCurr->cNodeName, "ChapterFlagHidden")) HideChapter(i, !!atoi(xmlCurr->cValue));
			if (!strcmp(xmlCurr->cNodeName, "ChapterFlagEnabled")) EnableChapter(i, !!atoi(xmlCurr->cValue));

			if (b && !strcmp(xmlCurr->cNodeName, "ChapterAtom")) {
				if (!GetSubChapters(i)->ImportFromXML(xmlCurr)) {
					return false;
				}
				b=false;
			}

			if (!strcmp(xmlCurr->cNodeName, "ChapterDisplay")) {
				XMLNODE* disp = (XMLNODE*)xmlCurr->pChild;

				while (disp) {
					if (!strcmp(disp->cNodeName, "ChapterString")) SetChapterText(i, disp->cValue, j);
					if (!strcmp(disp->cNodeName, "ChapterLanguage")) SetChapterLng(i, disp->cValue, j);
					disp = (XMLNODE*)disp->pNext;
				}

				j++;
			}

			xmlCurr = (XMLNODE*)xmlCurr->pNext;
		}

	  }

	ImportFromXML((XMLNODE*)xmlNode->pNext);

	return !failed;
}

int CChapters::CreateXMLTree_aux(XMLNODE** ppNode)
{
	char cBegin[50]; cBegin[0]=0;
	char cEnd[50]; cEnd[0]=0;
	*ppNode = NULL;

	for (int i=0;i<GetChapterCount();i++) {
		SINGLE_CHAPTER_DATA scd;
		GetChapter(i, &scd);
		XMLNODE* pNode = xmlAddSibling(ppNode, "ChapterAtom", "");
		XMLNODE* pDisp;

		Millisec2Str(scd.iBegin/1000000,cBegin);
		Millisec2Str(scd.iEnd/1000000,cEnd);
        xmlAddChild(pNode, "ChapterTimeStart", cBegin);
		if (scd.iEnd != -1) {
			xmlAddChild(pNode, "ChapterTimeEnd", cEnd);
		}
		if (scd.bHidden) xmlAddChild(pNode, "ChapterFlagHidden", (scd.bHidden)?"1":"0");
		if (!scd.bEnabled) xmlAddChild(pNode, "ChapterFlagEnabled", (scd.bEnabled)?"1":"0");

		int c = GetChapterDisplayCount(i);
		for (int j=0;j<c;j++) {
			pDisp = xmlAddChild(pNode, "ChapterDisplay", "");
			xmlAddChild(pDisp, "ChapterString", GetChapterText(i, j));
			xmlAddChild(pDisp, "ChapterLanguage", GetChapterLng(i, j));
		}
		pNode = pDisp;
		
		if (HasSubChapters(i)) {
			GetSubChapters(i)->CreateXMLTree_aux((XMLNODE**)&pNode->pNext);
		}
	}

	return 0;
}

int CChapters::CreateXMLTree(XMLNODE** ppNode)
{
	XMLNODE* pNode = NULL;
	*ppNode = NULL;

	pNode = xmlAddChild(
		xmlAddSibling(ppNode,"Chapters",""), "EditionEntry", "");
	CreateXMLTree_aux((XMLNODE**)&pNode->pChild);

	return 0;
}