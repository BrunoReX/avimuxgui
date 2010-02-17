// ChapterDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "AVIMux_GUIDlg.h"
#include "FileDialogs.h"
#include "ChapterDlg.h"
#include "formattext.h"
#include "trees.h"
#include "languages.h"
#include "windows.h"
#include "../strings.h"
#include "UnicodeListControl.h"
#include "version.h"
#include "UTF8Windows.h"
#include "AttachedWindows.h"
#include "ResizeableDialog.h"
#include <vector>
#include "chapterdlg.h"
#include "TABHandler.h"
#include "LanguageCodes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CChapterDlg 


CChapterDlg::CChapterDlg(CWnd* pParent /*=NULL*/)
	: CResizeableDialog(CChapterDlg::IDD, pParent)
{
	EnableAutomation();

	//{{AFX_DATA_INIT(CChapterDlg)
	//}}AFX_DATA_INIT
}


void CChapterDlg::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CResizeableDialog::OnFinalRelease();
}

void CChapterDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizeableDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChapterDlg)
	DDX_Control(pDX, IDC_CHAPTERSEGMENTUID_LABEL, m_ChapterSegmentUID_Label);
	DDX_Control(pDX, IDC_CHAPTERUID_LABEL, m_ChapterUID_Label);
	DDX_Control(pDX, IDC_CHAPTER_TITLE, m_Chapter_Title);
	DDX_Control(pDX, IDC_CHAPTERS_USAGE_LABEL, m_Chapters_Usage_Label);
	DDX_Control(pDX, IDC_CHAPTERUID, m_ChapterUID);
	DDX_Control(pDX, IDC_CHAPTERSEGMENTUID, m_ChapterSegmentUID);
	DDX_Control(pDX, IDC_CHAPTERDISPLAY_LNG, m_ChapterDisplay_Lng);
	DDX_Control(pDX, IDC_CHAPTER_DISPLAY, m_ChapterDisplay);
	DDX_Control(pDX, IDCANCEL, m_Cancel);
	DDX_Control(pDX, IDOK, m_OK);
	DDX_Control(pDX, IDC_CHAPTERTREE, m_Chapters);
	DDX_Control(pDX, IDC_SAVEAS, m_Saveas);
	DDX_Control(pDX, IDC_MAKESUBCHAPTER, m_Subchapter);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChapterDlg, CResizeableDialog)
	//{{AFX_MSG_MAP(CChapterDlg)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_CHAPTERTREE, OnEndlabeleditTree1)
	ON_BN_CLICKED(IDC_MAKESUBCHAPTER, OnMakesubchapter)
	ON_BN_CLICKED(IDC_SAVEAS, OnSaveas)
	ON_NOTIFY(TVN_BEGINLABELEDITA, IDC_CHAPTERTREE, OnBeginlabeleditTree1)
	ON_WM_KEYDOWN()
	ON_NOTIFY(NM_RCLICK, IDC_CHAPTERTREE, OnRclickChaptertree)
	ON_NOTIFY(TVN_SELCHANGEDA, IDC_CHAPTERTREE, OnSelchangedChaptertree)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_CHAPTER_DISPLAY, OnItemchangedChapterDisplay)
	ON_CBN_SELCHANGE(IDC_CHAPTERDISPLAY_LNG, OnSelchangeChapterdisplayLng)
	ON_EN_CHANGE(IDC_CHAPTERSEGMENTUID, OnChangeChaptersegmentuid)
	ON_WM_SIZE()
	ON_NOTIFY(TVN_BEGINLABELEDITW, IDC_CHAPTERTREE, OnBeginlabeleditTree1)
	ON_NOTIFY(TVN_ENDLABELEDITA, IDC_CHAPTERTREE, OnEndlabeleditTree1)
	ON_NOTIFY(TVN_ENDLABELEDITW, IDC_CHAPTERTREE, OnEndlabeleditTree1)
	ON_NOTIFY(TVN_SELCHANGEDW, IDC_CHAPTERTREE, OnSelchangedChaptertree)
	ON_CBN_EDITCHANGE(IDC_CHAPTERDISPLAY_LNG, OnSelchangeChapterdisplayLng)
	ON_NOTIFY(LVN_KEYDOWN, IDC_CHAPTER_DISPLAY, OnKeydownChapterDisplay)
	//}}AFX_MSG_MAP
	ON_STN_CLICKED(IDC_CHAPTERS_USAGE_LABEL, OnStnClickedChaptersUsageLabel)
//	ON_BN_CLICKED(IDOK, OnBnClickedOk)
//	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
ON_WM_DESTROY()
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CChapterDlg, CResizeableDialog)
	//{{AFX_DISPATCH_MAP(CChapterDlg)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_IChapterDlg zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {0ED67DC1-B00E-4DC7-B6F8-F8EA55C83790}
static const IID IID_IChapterDlg =
{ 0xed67dc1, 0xb00e, 0x4dc7, { 0xb6, 0xf8, 0xf8, 0xea, 0x55, 0xc8, 0x37, 0x90 } };

BEGIN_INTERFACE_MAP(CChapterDlg, CResizeableDialog)
	INTERFACE_PART(CChapterDlg, IID_IChapterDlg, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CChapterDlg 

HTREEITEM CChapterDlg::GetSelectedChapter()
{
	if (m_Chapters.GetRootItem())
		return hSelectedChapter;
	else
		return NULL;
}

int CChapterDlg::SetChapters(CChapters* chapters)
{
	c = chapters;
	return 0;
}

CChapters* CChapterDlg::GetChapters()
{
	return c;
}




int DecomposeEntry(char* b, __int64* iBegin, __int64* iEnd, char** pcText)
{
	char*	cFrom = NULL;
	char*	cTill = NULL;
	int     only_text = 0;
	int     keep = 0;

	cFrom = b;

	if (!strstr(b, ">")) only_text = 1;

	while (*b && *b!='>') {
		if (*b == '-') {
			*b++ = 0;
			while (*b==' ') b++;
			cTill = b;
		} else
		b++;
	};

	if (!cTill) {
		cTill = "end";
		keep = 1;
	}

	if (!only_text) *b++=0;
	while (*b && *b==' ') *b++;
	*pcText = b;

	if (!_stricmp(cTill,"end")) {
		if (!keep)
			*iEnd = -1;
		else 
			*iEnd = -2;
	} else {
		*iEnd = Str2Millisec(cTill)*1000000;
	}

	if (!strlen(cFrom))
		*iBegin = -1;
	else
		*iBegin = Str2Millisec(cFrom)*1000000;

	return 0;
}

int AddChaptersToTree(CUnicodeTreeCtrl* tree,HTREEITEM hParent, CChapters* c,int start, int end)
{
	if (end == CHAP_LAST) end = c->GetChapterCount()-1;
	if (end >= c->GetChapterCount()) end = c->GetChapterCount()-1;
	if (start > end) return 0;
	if (start >= c->GetChapterCount()) return 0;

	for (int i=start;i<=end;i++) {
		char cText[150]; memset(cText, 0, sizeof(cText));
		char cBuffer[500]; memset(cBuffer,0,sizeof(cBuffer));

		SINGLE_CHAPTER_DATA scd;
		c->GetChapter(i,&scd);

		CHAPTER_ENTRY* ce = (CHAPTER_ENTRY*)calloc(1,sizeof(CHAPTER_ENTRY));
		ce->c = c;
		ce->iIndex = i;
		ce->cText = new char[1024];
		ce->cText[0] = 0;
		strcpy(ce->cText, scd.cText);

		HTREEITEM item = Tree_Insert(tree," ",hParent);
		tree->SetItemData(item,(DWORD)ce);
		tree->SetItemText(item, LPSTR_TEXTCALLBACK);
		if (c->HasSubChapters(i)) {
			AddChaptersToTree(tree,item,c->GetSubChapters(i));
		}

	}

	return 0;
}

void CChapterDlg::InitChapterDisplayColumns()
{
	RECT rect;

	if (!m_ChapterDisplay.m_hWnd)
		return;

	while (m_ChapterDisplay.DeleteColumn(0));
	m_ChapterDisplay.GetWindowRect(&rect);
	DWORD dwDeltaH=abs(rect.top-rect.bottom);
	m_ChapterDisplay.InsertColumn(0,LoadString(STR_CHPDLG_DISP_LANGUAGE),LVCFMT_CENTER,(rect.right-rect.left)/7);
	m_ChapterDisplay.InsertColumn(1,LoadString(STR_CHPDLG_DISP_TITLE), LVCFMT_LEFT,(rect.right-rect.left)*6/7-20);
}


BOOL CChapterDlg::OnInitDialog() 
{
	// TODO: Zusätzliche Initialisierung hier einfügen
	selected_chapter_entry = 0;
	chapter_title_changed = 0;
	iSelectedChapterLanguageEntry=-1;

	CResizeableDialog::OnInitDialog();

	m_Chapters.InitUnicode();
	m_ChapterDisplay.InitUnicode();

	hChapterTitle = m_Chapter_Title.m_hWnd;
	m_Chapter_Title.ShowWindow(SW_HIDE);

	RECT r; m_Chapter_Title.GetWindowRect(&r);
	ScreenToClient(&r);
	CreateEditUTF8(r, m_hWnd, GetInstance(), (HFONT)GetFont()->m_hObject, hChapterTitle);
	
	ReinitFont(NULL);

/* set default chapter language title to display */
	char* _default = NULL;
	if (GetAttribs()->Exists("tree_default_title_languages")) {
		GetAttribs()->GetStr("tree_default_title_languages", &_default);
		m_Chapters.SetTitleLanguagePriorityString(_default);
		free(_default);
	}

	SetWindowTextUTF8(hChapterTitle, "");
	::EnableWindow(hChapterTitle, false);
	m_ChapterDisplay_Lng.EnableWindow(0);

	LANGUAGE_CODES* lngcd = GetLanguageCodesObject();
	for (int i=0;i<lngcd->GetCount();i++) {
		char buf[65536];
		buf[0]=0;
		sprintf(buf , "%s - %s", lngcd->GetCode(i), lngcd->GetFullName(i));
		m_ChapterDisplay_Lng.SetItemData(m_ChapterDisplay_Lng.AddString(buf),(LPARAM)lngcd->GetCode(i));
	}

/*	for (int i=0;i<sizeof(languages)/sizeof(char*[2]);i++) {
		char c[1024]; c[0]=0;
		sprintf(c, "%s - %s", languages[i][1], languages[i][0]);
		m_ChapterDisplay_Lng.SetItemData(m_ChapterDisplay_Lng.AddString(c),(LPARAM)languages[i][1]);
	}
*/
	timer_set = 0;
	InitChapterDisplayColumns();
	
	SendDlgItemMessage(IDOK,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_GEN_OK));
	SendDlgItemMessage(IDCANCEL,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_GEN_CANCEL));
	m_Chapters_Usage_Label.SetWindowText(LoadString(STR_CHPDLG_USAGE_LABEL));

	m_Saveas.SetWindowText(LoadString(STR_CHPDLG_SAVEAS));
	m_Subchapter.SetWindowText(LoadString(STR_CHPDLG_SUBCHAPTER));
	SetWindowText(LoadString(STR_CHPDLG_TITLE));

	AddChaptersToTree(&m_Chapters,NULL,c);
	m_Chapters.SetFocus();
	m_Chapters.SetChapters(c);

	hSelectedChapter = m_Chapters.GetSelectedItem();

	OnDisplayChapterUids();

	m_ChapterUID.SetDisabledTextColor(RGB(0,0,0));
	m_ChapterSegmentUID.EnableWindow(0);
/*
	hChapterTitle = m_Chapter_Title.m_hWnd;
	RECT r; m_Chapter_Title.GetWindowRect(&r);
	ScreenToClient(&r);
	m_Chapter_Title.ShowWindow(SW_HIDE);

	CreateEditUTF8(r, m_hWnd, GetInstance(), (HFONT)GetFont()->m_hObject, hChapterTitle);

	SetWindowTextUTF8(hChapterTitle, "");
	::EnableWindow(hChapterTitle, false);
*/
	// align subchapter button to lower left edge of dialog
	AttachWindow(m_Subchapter, ATTB_LEFT, m_hWnd, 20);
	AttachWindow(m_Subchapter, ATTB_BOTTOM, m_hWnd, -20);
	
	// align saveas button to subchapter button
	AttachWindow(m_Saveas, ATTB_LEFT, m_Subchapter, ATTB_RIGHT, 20);
	AttachWindow(m_Saveas, ATTB_BOTTOM, m_Subchapter, ATTB_BOTTOM, 0);
	
	// align ok button to subchapter button and lower right edge of dialog
	AttachWindow(m_OK, ATTB_BOTTOM, m_Subchapter, 0);
	AttachWindow(m_OK, ATTB_RIGHT , m_hWnd, -20);

	// align chapteruid label to top of subchapter button
	AttachWindow(m_ChapterUID_Label, ATTB_BOTTOM, m_Subchapter, ATTB_TOP, -20);
	
	// align chapteruid edit field to its label and right end of dialog
	AttachWindow(m_ChapterUID, ATTB_VCENTER, m_ChapterUID_Label, ATTB_VCENTER, 0);
	AttachWindow(m_ChapterUID, ATTB_RIGHT, m_hWnd, -20);
	AttachWindow(m_ChapterUID, ATTB_LEFT, m_ChapterUID_Label, ATTB_RIGHT, 50);

	// align bottom of chaptersegmentuid label to top of chapteruid label
	AttachWindow(m_ChapterSegmentUID_Label, ATTB_BOTTOM, m_ChapterUID_Label, ATTB_TOP, -8);
	
	// align chaptersegmentuid to chapteruid
	AttachWindow(m_ChapterSegmentUID, ATTB_LEFTRIGHT, m_ChapterUID);
	AttachWindow(m_ChapterSegmentUID, ATTB_VCENTER, m_ChapterSegmentUID_Label, ATTB_VCENTER, 0);

	// align chaptertext edit to chaptersegmentuid
	AttachWindow(hChapterTitle, ATTB_LEFTRIGHT, m_ChapterSegmentUID);
	AttachWindow(hChapterTitle, ATTB_BOTTOM, m_ChapterSegmentUID, ATTB_TOP, 0);

	// align langage code thing to chaptertext and chaptersegmentuidlabel
	AttachWindow(m_ChapterDisplay_Lng, ATTB_TOPBOTTOM, hChapterTitle, 0);
	AttachWindow(m_ChapterDisplay_Lng, ATTB_RIGHT, hChapterTitle, ATTB_LEFT, -4);
	AttachWindow(m_ChapterDisplay_Lng, ATTB_LEFT, m_ChapterSegmentUID_Label);

	// attach chapterdisplay to chaptersegmenduidlabel and chaptertext
	AttachWindow(m_ChapterDisplay, ATTB_LEFT, m_ChapterSegmentUID_Label);
	AttachWindow(m_ChapterDisplay, ATTB_RIGHT, hChapterTitle);
	AttachWindow(m_ChapterDisplay, ATTB_BOTTOM, hChapterTitle, ATTB_TOP, -4);
	AttachWindow(m_ChapterDisplay, m_Chapters, ATTB_HEIGHTRATIO, 1., 0.5);

	// attach chapter tree to chapterdisplay, top label, 
	// left<chapterdisplaylng>, right<chapterdisplay>
	AttachWindow(m_Chapters, ATTB_TOP, m_Chapters_Usage_Label, ATTB_BOTTOM, 4);
	AttachWindow(m_Chapters, ATTB_RIGHT, m_ChapterDisplay);
	AttachWindow(m_Chapters, ATTB_BOTTOM, m_ChapterDisplay, ATTB_TOP, -4);
	AttachWindow(m_Chapters, ATTB_LEFT, m_ChapterDisplay_Lng);

	// attach headline to chapter tree
	AttachWindow(m_Chapters_Usage_Label, ATTB_LEFTRIGHT, m_Chapters);

	COMBOBOXINFO cbi; 
	cbi.cbSize = sizeof(cbi);
	GetComboBoxInfo(m_ChapterDisplay_Lng, &cbi);

	TABHandler_install(cbi.hwndItem, hChapterTitle, true);
	TABHandler_install(hChapterTitle, m_ChapterSegmentUID, false);
	TABHandler_install(m_ChapterSegmentUID, m_Chapters, false);
	TABHandler_install(m_Chapters.m_hWnd, m_ChapterDisplay.m_hWnd, false);
	TABHandler_install(m_ChapterDisplay, cbi.hwndItem, false);

//	ReinitPosition();

	return false;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

BOOL CChapterDlg::DestroyWindow() 
{
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen
	RemoveFromTree(&m_Chapters,m_Chapters.GetRootItem());

	

	return CResizeableDialog::DestroyWindow();
}

void CChapterDlg::OnEndlabeleditTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	char s[1024]; s[0]=0;
	char*	cText = NULL;
	char*	b;
	__int64	iBegin,iEnd;
	int u = SendDlgItemMessage(IDC_CHAPTERTREE, TVM_GETUNICODEFORMAT);

	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	HTREEITEM hItem = GetSelectedChapter();
	CHAPTER_ENTRY* pCE = (CHAPTER_ENTRY*)m_Chapters.GetItemData(hItem);
	CHAPTER_ENTRY* pPM = (CHAPTER_ENTRY*)m_Chapters.GetItemData(m_Chapters.GetParentMostItem(hItem));
	
	if (!u) {
		CEdit*	edit = m_Chapters.GetEditControl();
		edit->GetWindowText(s, 1024);
	} else {
		HWND edit = TreeView_GetEditControl(m_Chapters.m_hWnd);
		SendMessageW(edit, WM_GETTEXT, 1024, (unsigned int)s);
	}
	
	b = s;

	m_Chapters.toUTF8(s, s, 1024);

	DecomposeEntry(s,&iBegin,&iEnd,&cText);

	if (!pCE->c->IsEdition(pCE->iIndex)) {
		if (iBegin > -1)
			pCE->c->SetChapterBegin(pCE->iIndex,iBegin);
		
		if (iEnd == -2)
			iEnd = pCE->c->GetChapterEnd(pCE->iIndex);

		if (iEnd == -1 && pPM->c->IsEdition(pPM->iIndex) && pPM->c->IsOrdered(pPM->iIndex)) {
			MessageBox(LoadString(STR_ERR_NOENDINORDEREDEDITION), LoadString(IDS_ERROR),
				MB_OK | MB_ICONERROR);
		} else
			pCE->c->SetChapterEnd(pCE->iIndex,iEnd);
	}
	
	char cBuffer[500]; ZeroMemory(cBuffer,sizeof(cBuffer));

	strcpy(pCE->cText, cText);

	m_OK.EnableWindow(1);
	m_Cancel.EnableWindow(1);
	m_Saveas.EnableWindow(1);
	m_Subchapter.EnableWindow(1);
	InvalidateRect(NULL);

	*pResult = 0;
}

void CChapterDlg::OnMakesubchapter() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	HTREEITEM hItem = GetSelectedChapter();//m_Chapters.GetSelectedItem();
	HTREEITEM hNext = NULL;
	HTREEITEM hNew = NULL;
	CHAPTER_ENTRY* pCE;
	CHAPTER_ENTRY* pNew = (CHAPTER_ENTRY*)calloc(1,sizeof(CHAPTER_ENTRY));

	if (hItem) {
		pCE = (CHAPTER_ENTRY*)m_Chapters.GetItemData(hItem);
		pNew->c = pCE->c->GetSubChapters(pCE->iIndex);
	} else {
		pCE = (CHAPTER_ENTRY*)calloc(1,sizeof(CHAPTER_ENTRY));
		pCE->c = c;
		pNew->c = c;
	}
	
	if (!m_Chapters.GetRootItem()) {
		pNew->c->AddEmptyEdition();
		pNew->c->SetChapterText(CHAP_LAST, "text", 0);
		pNew->c->SetChapterLng(CHAP_LAST, "und", 0);
	}
	else
		pNew->c->AddChapter(0,-1,"title");

	char cBuffer[200];
	__int64 j = pCE->c->GetChapterBegin(pCE->iIndex);
	FormatChapterEntry(j,-1,"title",cBuffer);
	pNew->iIndex = pNew->c->GetChapterCount()-1;
	pNew->cText = new char[1024];
	strcpy(pNew->cText, "title");
	
	m_Chapters.SetItemData(hNew = Tree_Insert(&m_Chapters,LPSTR_TEXTCALLBACK,hItem),(DWORD)pNew);
	m_Chapters.Expand(hItem,TVE_EXPAND);
	m_Chapters.SelectItem(hNew);
	m_Chapters.EditLabel(hNew);
}

int RenderChapters2File(CFileStream* f, CChapters* c)
{
	for (int i=0;i<c->GetChapterCount();i++) {
		char cTime[30];
		char cTotal[128]; cTotal[0]=0;

		SINGLE_CHAPTER_DATA scd;
		c->GetChapter(i, &scd);

		f->WriteString("ADD ");
		Millisec2Str(scd.iBegin/1000000,cTime);
		f->WriteString(cTime);
		f->WriteString(" ");
		if (scd.iEnd!=-1) {
			Millisec2Str(scd.iEnd/1000000,cTime);
		} else {
			sprintf(cTime,"END");
		}

		sprintf(cTotal,"%s %s %d %d %s",cTime,scd.cLng,scd.bEnabled,scd.bHidden,scd.cText);
		f->WriteString(cTotal);
		f->WriteString("\n");

		if (c->HasSubChapters(i)) {
			f->WriteString("SUB BEGIN\n");
			RenderChapters2File(f,c->GetSubChapters(i));
			f->WriteString("SUB END\n");
		}
	}

	return 0;
}

int RenderChapters2File(FILE* f, CChapters* c)
{
	for (int i=0;i<c->GetChapterCount();i++) {
		char cTime[30];
		char cTotal[128]; cTotal[0]=0;

		SINGLE_CHAPTER_DATA scd;
		c->GetChapter(i, &scd);

		fprintf(f, "ADD ");
		Millisec2Str(scd.iBegin/1000000,cTime);
		fprintf(f, cTime);
		fprintf(f, " ");
		if (scd.iEnd!=-1) {
			Millisec2Str(scd.iEnd/1000000,cTime);
		} else {
			sprintf(cTime,"END");
		}

		sprintf(cTotal,"%s %s %d %d %s",cTime,scd.cLng,scd.bEnabled,scd.bHidden,scd.cText);
		fprintf(f, cTotal);
		fprintf(f, "\n");

		if (c->HasSubChapters(i)) {
			fprintf(f, "SUB BEGIN\n");
			RenderChapters2File(f,c->GetSubChapters(i));
			fprintf(f, "SUB END\n");
		}
	}

	return 0;
}

void CChapterDlg::OnSaveas() 
{
	XMLNODE* xml = NULL;
	XMLNODE* xmlChapters = NULL;
	XMLNODE* xmlTags = NULL;

//	char* f;
	const char* e;
	char* t;

	c->CreateXMLTree(&xml, &xmlChapters, &xmlTags);
	
	const char* txt_all = NULL; 
	const char* txt_chp = NULL; 
	const char* txt_tag = NULL; 

	char buf[1024]; 

	buf[0]=0;
	sprintf(buf,"%c%c%c%s%c%c%s%c%c",0xEF,0xBB,0xBF,"<?xml version=\"1.0\" encoding=\"utf-8\"?>",13,10,
		"<!DOCTYPE Segment SYSTEM \"matroskasegment.dtd\">", 13, 10);
	std::string st_all = buf;

	buf[0]=0;
	sprintf(buf,"%c%c%c%s%c%c%s%c%c",0xEF,0xBB,0xBF,"<?xml version=\"1.0\" encoding=\"utf-8\"?>",13,10,
		"<!DOCTYPE Chapters SYSTEM \"matroskachapters.dtd\">", 13, 10);
	std::string st_chp = buf;

	buf[0]=0;
	sprintf(buf,"%c%c%c%s%c%c%s%c%c",0xEF,0xBB,0xBF,"<?xml version=\"1.0\" encoding=\"utf-8\"?>",13,10,
		"<!DOCTYPE Tags SYSTEM \"matroskatags.dtd\">", 13, 10);
	std::string st_tag = buf;

	xmlTreeToString((XMLNODE*)xml, st_all);
	txt_all = st_all.c_str();

	xmlTreeToString((XMLNODE*)xmlChapters, st_chp);
	txt_chp = st_chp.c_str();

	xmlTreeToString((XMLNODE*)xmlTags, st_tag);
	txt_tag = st_tag.c_str();

	if (m_Chapters.GetCount()) {

		char* def_ext; GetAttribs()->GetStr("default_save_extension", &def_ext);

		if (_stricmp(def_ext, "xml") && _stricmp(def_ext, "mkc")) {
			MessageBox("Weird default extension defined!", "Internal Error",
				MB_OK | MB_ICONERROR);
		}

		char file_types[2048]; file_types[0]=0;
		char* possible_file_types[] = {
			"XML chapter file (*.xml)|*.xml",
			"Matroska Chapter file (*.mkc)|*.mkc",
			"All file types (*.xml, *.mkc)|*.xml;*.mkc"
		};

		if (!_stricmp(def_ext, "xml")) {
			strcat(file_types, possible_file_types[0]);
			strcat(file_types, "|");
			strcat(file_types, possible_file_types[1]);
			strcat(file_types, "|");
			strcat(file_types, possible_file_types[2]);
		} else {
			strcat(file_types, possible_file_types[1]);
			strcat(file_types, "|");
			strcat(file_types, possible_file_types[0]);
			strcat(file_types, "|");
			strcat(file_types, possible_file_types[2]);
		}
		strcat(file_types, "||");


		OPENFILENAME o; 
		PrepareSimpleDialog(&o, m_hWnd, file_types);
		o.Flags |= OFN_OVERWRITEPROMPT;

		o.lpstrDefExt = def_ext;
		int open = GetOpenSaveFileNameUTF8(&o, 0);
		free(def_ext);

		if (open) {

			t = o.lpstrFile;
			//splitpathname(t, (char**)&f, (char**)&e, NULL);
			std::string fileName;
			std::string fileExtension;
			std::string filePath;
			splitpathname<char>(t, fileName, fileExtension, filePath);
			e = fileExtension.c_str();			

			CFileStream* f = new CFileStream;
			if (f->Open(t, StreamMode::Write) != STREAM_OK) {
				MessageBox(LoadString(IDS_COULDNOTOPENOUTPUTFILE), LoadString(IDS_ERROR), MB_OK);
			} else {
				if (!_stricmp(e, "amg")) {
					f->WriteString("SET OUTPUT OPTIONS\n");
					f->WriteString("WITH SET OPTION CHAPTERS\n");
					RenderChapters2File(f,c);
					f->WriteString("END WITH\n");
					f->Close();
				} else 
				if (!_stricmp(e, "xml") || !_stricmp(e, "txt")) {
					f->WriteString((void*)txt_all);
					f->Close();

					char c[4096]; c[0]=0;
					strcpy(c, t);
					c[strlen(c)-4] = 0;
					strcat(c, ".mkvmerge.chapters.xml");
					if (f->Open(c, StreamMode::Write) != STREAM_OK) {
						MessageBox(LoadString(IDS_COULDNOTOPENOUTPUTFILE), LoadString(IDS_ERROR), MB_OK);
					} else {
						f->WriteString((void*)txt_chp);
						f->Close();
					}

					c[strlen(c)-strlen(".mkvmerge.chapters.xml")] = 0;
					strcat(c, ".mkvmerge.tags.xml");
					if (f->Open(c, StreamMode::Write) != STREAM_OK) {
						MessageBox(LoadString(IDS_COULDNOTOPENOUTPUTFILE), LoadString(IDS_ERROR), MB_OK);
					} else {
						f->WriteString((void*)txt_tag);
						f->Close();
					}
					delete f;
				} else
				if (!_strnicmp(e, "mk", 2)) {
					char u[4096]; u[0]=0;
					MATROSKA* m = new MATROSKA;
					m->Open(f, MMODE_WRITE);
					m->SetInitialHeaderSize(2048 + c->GetSize(0));
					m->BeginWrite();
					//char v[200]; v[0]=0;
					std::basic_string<TCHAR> version = ComposeVersionString();
					m->SetAppName(version.c_str());
					m->SetChapters(c, -2);
					m->SetSegmentDuration(0.0f);
					m->Close();
					delete m;
					f->Close();
					delete f;
				}
			}
		}
	}

	xmlDeleteNode(&xml);
	xmlDeleteNode(&xmlChapters);
	xmlDeleteNode(&xmlTags);
}

void CChapterDlg::OnBeginlabeleditTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	int u = SendDlgItemMessage(IDC_CHAPTERTREE, TVM_GETUNICODEFORMAT);
	char cBuffer[1024]; cBuffer[0]=0;

	CHAPTER_ENTRY* pCE = m_Chapters.GetSelectedChapterEntry();

	if (pCE && pCE->c->IsEdition(pCE->iIndex)) {
		m_Chapters.PostMessage(TVM_ENDEDITLABELNOW);
		m_Chapters.PostMessage(TVM_ENDEDITLABELNOW);
	}
	
	if (!u) {
		CEdit*	edit = m_Chapters.GetEditControl();
		char c[1024]; 
		if (!pCE->c->IsEdition(pCE->iIndex)) 
			FormatChapterEntry(pCE->c->GetChapterBegin(pCE->iIndex), 
				pCE->c->GetChapterEnd(pCE->iIndex), "", cBuffer);
		else
			strcpy(cBuffer, pCE->cText);

		UTF82Str(cBuffer, c);
		edit->SetWindowText(c);
	} else {
		HWND edit = TreeView_GetEditControl(m_Chapters.m_hWnd);
		char c[1024]; c[0]=0; c[1]=0;
		
		if (!pCE->c->IsEdition(pCE->iIndex)) 
			FormatChapterEntry(pCE->c->GetChapterBegin(pCE->iIndex), 
				pCE->c->GetChapterEnd(pCE->iIndex), "", cBuffer);
		else
			strcpy(cBuffer, pCE->cText);

		UTF82WStr(cBuffer, c);
		SendMessageW(edit, WM_SETTEXT, 0, (long)c);
	}

	m_OK.EnableWindow(0);
	m_Cancel.EnableWindow(0);
	m_Saveas.EnableWindow(0);
	m_Subchapter.EnableWindow(0);

	*pResult = 0;
}


void CChapterDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	
	CResizeableDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CChapterDlg::OnRclickChaptertree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	CPoint p;
	p.x = m_Chapters.GetMouseX();
	p.y = m_Chapters.GetMouseY();

	m_Chapters.OpenContextMenu(p);
	*pResult = 0;
}

void CChapterDlg::ApplyNewChapterTitle()
{
	if (selected_chapter_entry && iSelectedChapterLanguageEntry>-1 && chapter_title_changed) {
		char* txt = NULL;
		GetWindowTextUTF8(hChapterTitle, &txt);

	/*	int buf_len = 16384;

		char txt[16384]; 
		
		do {
			memset(txt, 0, sizeof(txt));
			GetWindowTextUTF8(hChapterTitle, (char*)txt, buf_len);
			buf_len -= 1024;
		}
		while (!txt[0]);
*/
		selected_chapter_entry->c->SetChapterText(selected_chapter_entry->iIndex,
			txt, iSelectedChapterLanguageEntry);

		RECT r;
		m_ChapterDisplay.GetItemRect(iSelectedChapterLanguageEntry, &r, LVIR_BOUNDS);
		m_ChapterDisplay.SetItemText(iSelectedChapterLanguageEntry, 1, txt);
		m_ChapterDisplay.InvalidateRect(&r);

		m_Chapters.GetItemRect(m_Chapters.GetSelectedItem(), &r, false);
		m_Chapters.InvalidateRect(&r);

		m_Chapters.UpdateWindow();
		m_ChapterDisplay.UpdateWindow();

		chapter_title_changed = 0;

		free(txt);
	}
}


void CChapterDlg::OnSelchangedChaptertree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
	HTREEITEM  hNewItem = pNMTreeView->itemNew.hItem;
	hSelectedChapter = hNewItem;

	CHAPTER_ENTRY* ce = (CHAPTER_ENTRY*)m_Chapters.GetItemData(hNewItem);

	if (ce) {

		ApplyNewChapterTitle();

		iSelectedChapterLanguageEntry=-1;
		m_ChapterDisplay_Lng.EnableWindow(0);
		
		int j = m_ChapterDisplay.GetItemCount();
		int disp_count = ce->c->GetChapterDisplayCount(ce->iIndex);
		j = m_ChapterDisplay.GetItemCount();

		selected_chapter_entry = ce;

		for (int i=0;i<disp_count+1;i++) {
			char* lng, *txt;

			if (i<disp_count) {
				lng = ce->c->GetChapterLng(ce->iIndex, i);
				txt = ce->c->GetChapterText(ce->iIndex, i);
			} else {
				lng = "<new>"; txt = "";
			}

			LVITEM  lvitem;
			ZeroMemory(&lvitem,sizeof(lvitem));
			lvitem.mask=LVIF_TEXT | LVIF_NORECOMPUTE;
			lvitem.iItem=i;
			lvitem.iSubItem=0;
			lvitem.pszText=lng;
			
			if (i<j) {
				m_ChapterDisplay.SetItemText(i, 0, lng); 
			} else 
				m_ChapterDisplay.InsertItem(&lvitem);
			m_ChapterDisplay.SetItemText(i, 1, txt);

			m_ChapterDisplay.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
			iSelectedChapterLanguageEntry = 0;
			
			int x = (int)m_ChapterDisplay.GetFirstSelectedItemPosition();
			m_ChapterDisplay_Lng.EnableWindow(1);
			
			if (disp_count > 0)
				::EnableWindow(hChapterTitle, true);
			else
				::EnableWindow(hChapterTitle, false);

			
			UpdateChapterDisplayLngEdit(0);
		}

		for (int i=j-1; i>=disp_count+1; i--) {
			m_ChapterDisplay.DeleteItem(i);
		}

		UpdateChapterSegmentUID(ce);
		UpdateChapterUIDEdit(ce->c, ce->iIndex);

		m_ChapterDisplay.InvalidateRect(NULL);
		m_ChapterDisplay.UpdateWindow();

		m_ChapterSegmentUID.EnableWindow(!ce->c->IsEdition(ce->iIndex));

	}



	*pResult = 0;
}

void CChapterDlg::UpdateChapterSegmentUID(CHAPTER_ENTRY* ce)
{
	ASSERT(ce);
	SINGLE_CHAPTER_DATA	scd; memset(&scd, 0, sizeof(scd));
	ce->c->GetChapter(ce->iIndex, &scd);
	if (scd.bSegmentUIDValid) {
		char c[64]; memset(c, 0, sizeof(c));
		__int128hex(scd.cSegmentUID, c, 1);
		m_ChapterSegmentUID.SetWindowText(c);
	} else 
		m_ChapterSegmentUID.SetWindowText("N/A");
}

void CChapterDlg::UpdateChapterUIDEdit(CChapters* c, int i)
{
	__int64 uid = c->GetUID(i); {
		char c[64]; memset(c, 0, sizeof(c));
		__int642hex(uid, c, 1, 1, 1);
		m_ChapterUID.SetWindowText(c);
	}

}

void CChapterDlg::UpdateChapterDisplayLngEdit(int iItem)
{
	HTREEITEM h = NULL;

	if (!m_Chapters.GetRootItem()) return;

	int count = m_ChapterDisplay_Lng.GetCount();

	char* c = NULL;

//	c = m_ChapterDisplay.GetItemText(iItem, 1);
	
	char* l;
	l = m_ChapterDisplay.GetItemText(iItem, 0);

	for (int i=0;i<count && !c;i++) {
		char* lng = (char*)m_ChapterDisplay_Lng.GetItemData(i);
		if (!strcmp(lng, l)) {
			m_ChapterDisplay_Lng.SetCurSel(i);
			if (selected_chapter_entry) {
				c = selected_chapter_entry->c->GetChapterText(selected_chapter_entry->iIndex, 
					lng, CHAP_GCT_RETURN_NULL | CHAP_GCT_ALLOW_UND);
			}
		}
	}

	SetWindowTextUTF8(hChapterTitle, c?c:"");
	chapter_title_changed = 0;
}

void CChapterDlg::OnItemchangedChapterDisplay(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	if (!m_Chapters.GetRootItem()) return;
	
	int iItem = pNMListView->iItem;
	int iSubItem = pNMListView->iSubItem;
	int old_state = pNMListView->uOldState;
	int new_state = pNMListView->uNewState;

	if (((old_state & LVIS_SELECTED) != LVIS_SELECTED) && 
		((new_state & LVIS_SELECTED) == LVIS_SELECTED)) {
		
		ApplyNewChapterTitle();

		::EnableWindow(hChapterTitle, true);

		m_ChapterDisplay_Lng.EnableWindow(1);

		iSelectedChapterLanguageEntry = (int)m_ChapterDisplay.GetFirstSelectedItemPosition()-1;

		UpdateChapterDisplayLngEdit(iItem);
	}


	if (iItem == m_ChapterDisplay.GetItemCount() - 1) {
		::EnableWindow(hChapterTitle, false);
	}

	*pResult = 0;
}

void CChapterDlg::OnSelchangeChapterdisplayLng() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	char* lng_code = NULL; CString s;
			
	if (!m_Chapters.GetRootItem()) return;

	int lng_code_index = m_ChapterDisplay_Lng.GetCurSel();

	if (lng_code_index != CB_ERR) {
		lng_code     = (char*)m_ChapterDisplay_Lng.GetItemData(lng_code_index);
	} else {
		m_ChapterDisplay_Lng.GetWindowText(s);
	}

	int chp_lng_index = iSelectedChapterLanguageEntry;

	if (!GetSelectedChapter()) {
		hSelectedChapter = m_Chapters.GetSelectedItem();
	}

	CHAPTER_ENTRY* ce = m_Chapters.GetSelectedChapterEntry();

	ce->c->SetChapterLng(ce->iIndex, (lng_code?lng_code:s.GetBuffer(256)), chp_lng_index);
	if (!ce->c->GetChapterText(ce->iIndex, chp_lng_index))
		ce->c->SetChapterText(ce->iIndex, "", chp_lng_index);

	if (chp_lng_index == ce->c->GetChapterDisplayCount(ce->iIndex)-1
		&& m_ChapterDisplay.GetItemCount()-1 == chp_lng_index) {
		LVITEM  lvitem;
		ZeroMemory(&lvitem,sizeof(lvitem));
		lvitem.mask=LVIF_TEXT | LVIF_NORECOMPUTE;
		lvitem.iItem=chp_lng_index+1;
		lvitem.iSubItem=0;
		lvitem.pszText="<new>";

		m_ChapterDisplay.InsertItem(&lvitem);

		SetWindowTextUTF8(hChapterTitle, "<new>");
		
		if (m_ChapterDisplay.GetItemCount() > 1)
			::EnableWindow(hChapterTitle, true);
	}
	
	m_ChapterDisplay_Lng.PostMessage(CB_SETEDITSEL, 0, MAKELPARAM(-1, -1));

	m_ChapterDisplay.SetItemText(chp_lng_index, 0, ce->c->GetChapterLng(ce->iIndex, chp_lng_index));
	RECT r;
	m_ChapterDisplay.GetItemRect(chp_lng_index, &r, LVIR_LABEL);
	m_ChapterDisplay.InvalidateRect(&r);
	m_ChapterDisplay.UpdateWindow();
}

void CChapterDlg::OnKeydownChapterDisplay(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (!m_Chapters.GetRootItem()) return;

	CHAPTER_ENTRY* ce		= m_Chapters.GetSelectedChapterEntry();
	LV_KEYDOWN* pLVKeyDown	= (LV_KEYDOWN*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	int key = pLVKeyDown->wVKey;

	if (ce && key == VK_DELETE) {
		if (iSelectedChapterLanguageEntry > -1) {
			int j = ce->c->GetChapterDisplayCount(ce->iIndex);
			if (j>0 && iSelectedChapterLanguageEntry<j) {
				for (int i=iSelectedChapterLanguageEntry;i<j-1;i++) {
					m_ChapterDisplay.SetItemText(i, 0, m_ChapterDisplay.GetItemText(i+1, 0));
					m_ChapterDisplay.SetItemText(i, 1, m_ChapterDisplay.GetItemText(i+1, 1));
				}
				ce->c->DeleteChapterDisplay(ce->iIndex, iSelectedChapterLanguageEntry);
				m_ChapterDisplay.SetItemText(j-1, 0, "<new>");
				m_ChapterDisplay.SetItemText(j-1, 1, "");
				m_ChapterDisplay.DeleteItem(j);
				m_ChapterDisplay.InvalidateRect(NULL);
				m_ChapterDisplay.UpdateWindow();
			}

			if (!ce->c->GetChapterDisplayCount(ce->iIndex)) {
				m_ChapterDisplay.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
				::EnableWindow(hChapterTitle, false);
			}
		}
	}

	*pResult = 0;
}

void CChapterDlg::OnDisplayChapterUids() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	m_Chapters.SetDisplayUIDs(0);//!!m_DisplayChapterUIDs.GetCheck());
	m_Chapters.InvalidateRect(NULL);
	m_Chapters.UpdateWindow();
}

void CChapterDlg::OnChangeChaptersegmentuid() 
{
	// TODO: Wenn dies ein RICHEDIT-Steuerelement ist, sendet das Steuerelement diese

	CHAPTER_ENTRY* ce = m_Chapters.GetSelectedChapterEntry();

	if (!ce)
		return;

	if (m_ChapterSegmentUID.IsValid()) {
		char c[16]; 
		m_ChapterSegmentUID.GetUID(c);

		ce->c->SetSegmentUID(ce->iIndex, 1, c);
		double d = m_ChapterSegmentUID.GetSegmentDuration();
		if (d > -0.01 && ce->c->GetChapterEnd(ce->iIndex) == -1) {
			ce->c->SetChapterEnd(ce->iIndex, (__int64)d);
			RECT r; m_Chapters.GetItemRect(GetSelectedChapter(), &r, false);
			m_Chapters.InvalidateRect(&r);
		}

		HTREEITEM hEdition = GetSelectedChapter();
		while (m_Chapters.GetParentItem(hEdition))
			hEdition = m_Chapters.GetParentItem(hEdition);

		ce = (CHAPTER_ENTRY*)m_Chapters.GetItemData(hEdition);
		if (!ce->c->IsEdition(ce->iIndex)) 
			MessageBox("Fatal Internal Error", "Error", MB_OK | MB_ICONERROR);

		ce->c->SetIsOrdered(ce->iIndex, 1);
		RECT r; m_Chapters.GetItemRect(hEdition, &r, false);
		m_Chapters.InvalidateRect(&r);
		m_Chapters.UpdateWindow();
	} else {
		char c[128]; memset(c,0,sizeof(c));
		m_ChapterSegmentUID.GetWindowText(c, 128);
		if (!strcmp(c, ""))
			ce->c->SetSegmentUID(ce->iIndex, 0, NULL);
	}

	m_ChapterSegmentUID.InvalidateRect(NULL);
	m_ChapterSegmentUID.UpdateWindow();
}

BOOL CChapterDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen

	if (HIWORD(wParam) == EN_KILLFOCUS && lParam == (LPARAM)hChapterTitle) {
		int chp_lng_index = iSelectedChapterLanguageEntry;
		CHAPTER_ENTRY* ce = m_Chapters.GetSelectedChapterEntry();

//		if (chapter_title_changed) {
			ApplyNewChapterTitle();
//			m_ChapterDisplay.SetItemText(chp_lng_index, 1, ce->c->GetChapterText(ce->iIndex, chp_lng_index));
//		}
/*
		RECT r;
		m_ChapterDisplay.GetItemRect(chp_lng_index, &r, LVIR_BOUNDS);
		m_ChapterDisplay.InvalidateRect(&r);
		m_ChapterDisplay.UpdateWindow();

		m_Chapters.GetItemRect(m_Chapters.GetSelectedItem(), &r, false);
		m_Chapters.InvalidateRect(&r);
		m_Chapters.UpdateWindow();
*/
		return 1;
	} else
	if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == hChapterTitle) {
		chapter_title_changed = 1;
		return 1;
	} else

	if (LOWORD(wParam) == IDCANCEL && GetFocushWnd(this) == hChapterTitle) {
	// ignore new chapter title
		int chp_lng_index = iSelectedChapterLanguageEntry;
		CHAPTER_ENTRY* ce = m_Chapters.GetSelectedChapterEntry();
		SetWindowTextUTF8(hChapterTitle, ce->c->GetChapterText(ce->iIndex, chp_lng_index));
		return 1;

	} else
	if (LOWORD(wParam) == IDCANCEL && GetFocushWnd(this) == m_ChapterSegmentUID.m_hWnd) {
	// ignore new chapter title
		CHAPTER_ENTRY* ce = m_Chapters.GetSelectedChapterEntry();
		UpdateChapterSegmentUID(ce);
		return 1;
	} else	

	return CResizeableDialog::OnCommand(wParam, lParam);
}

void CChapterDlg::OnCancel() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	Sleep(1);
}

void CChapterDlg::EnableChapterTitleEdit(bool enabled)
{
	::EnableWindow(hChapterTitle, enabled);
}

void CChapterDlg::DeleteChapterLanguageDisplay()
{
	m_ChapterDisplay.DeleteAllItems();
}

void CChapterDlg::AfterLastDeleted()
{
	DeleteChapterLanguageDisplay();
	EnableChapterTitleEdit(0);
	SetWindowTextUTF8(hChapterTitle, "");
	m_ChapterDisplay_Lng.EnableWindow(0);
	m_ChapterSegmentUID.SetWindowText("N/A");
	m_ChapterUID.SetWindowText("");
	selected_chapter_entry = NULL;
	iSelectedChapterLanguageEntry = -1;
}

int id;

void CChapterDlg::OnSize(UINT nType, int cx, int cy) 
{
	if (m_Chapters_Usage_Label.m_hWnd) 
		m_Chapters_Usage_Label.InvalidateRect(NULL);

	CResizeableDialog::OnSize(nType, cx, cy);

	InitChapterDisplayColumns();
}



void CChapterDlg::OnStnClickedChaptersUsageLabel()
{
	// TODO: Fügen Sie hier Ihren Kontrollbehandlungscode für die Benachrichtigung ein.
}

void CChapterDlg::OnBnClickedOk()
{
	// TODO: Fügen Sie hier Ihren Kontrollbehandlungscode für die Benachrichtigung ein.
}

void CChapterDlg::OnBnClickedCancel()
{
	// TODO: Fügen Sie hier Ihren Kontrollbehandlungscode für die Benachrichtigung ein.
}

void CChapterDlg::OnDestroy()
{
	CResizeableDialog::OnDestroy();


	// TODO: Fügen Sie hier Ihren Meldungsbehandlungscode ein.
}
