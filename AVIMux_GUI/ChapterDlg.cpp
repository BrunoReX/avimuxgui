// ChapterDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "ChapterDlg.h"
#include "formattext.h"
#include "trees.h"
#include "languages.h"
#include "windows.h"
#include "strings.h"
#include "UnicodeListControl.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CChapterDlg 


CChapterDlg::CChapterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChapterDlg::IDD, pParent)
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

	CDialog::OnFinalRelease();
}

void CChapterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChapterDlg)
	DDX_Control(pDX, IDC_CHAPTERDISPLAY_LNG, m_ChapterDisplay_Lng);
	DDX_Control(pDX, IDC_CHAPTERDISPLAY_EDIT, m_ChapterDisplay_Edit);
	DDX_Control(pDX, IDC_CHAPTER_DISPLAY, m_ChapterDisplay);
	DDX_Control(pDX, IDCANCEL, m_Cancel);
	DDX_Control(pDX, IDOK, m_OK);
	DDX_Control(pDX, IDC_CHAPTERTREE, m_Chapters);
	DDX_Control(pDX, IDC_SAVEAS, m_Saveas);
	DDX_Control(pDX, IDC_MAKESUBCHAPTER, m_Subchapter);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChapterDlg, CDialog)
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
	ON_NOTIFY(TVN_ENDLABELEDITW, IDC_CHAPTERDISPLAY_EDIT, OnEndlabeleditChapterdisplayEdit)
	ON_NOTIFY(LVN_KEYDOWN, IDC_CHAPTER_DISPLAY, OnKeydownChapterDisplay)
	ON_NOTIFY(TVN_BEGINLABELEDITW, IDC_CHAPTERDISPLAY_EDIT, OnBeginlabeleditChapterdisplayEdit)
	ON_NOTIFY(TVN_BEGINLABELEDITA, IDC_CHAPTERDISPLAY_EDIT, OnBeginlabeleditChapterdisplayEdit)
	ON_NOTIFY(TVN_ENDLABELEDITA, IDC_CHAPTERDISPLAY_EDIT, OnEndlabeleditChapterdisplayEdit)
	ON_NOTIFY(TVN_BEGINLABELEDITW, IDC_CHAPTERTREE, OnBeginlabeleditTree1)
	ON_NOTIFY(TVN_ENDLABELEDITA, IDC_CHAPTERTREE, OnEndlabeleditTree1)
	ON_NOTIFY(TVN_ENDLABELEDITW, IDC_CHAPTERTREE, OnEndlabeleditTree1)
	ON_NOTIFY(TVN_SELCHANGEDW, IDC_CHAPTERTREE, OnSelchangedChaptertree)
	ON_CBN_EDITCHANGE(IDC_CHAPTERDISPLAY_LNG, OnSelchangeChapterdisplayLng)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CChapterDlg, CDialog)
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

BEGIN_INTERFACE_MAP(CChapterDlg, CDialog)
	INTERFACE_PART(CChapterDlg, IID_IChapterDlg, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CChapterDlg 

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

	cFrom = b;

	while (*b && *b!='>') {
		if (*b == '-') {
			*b++ = 0;
			while (*b==' ') b++;
			cTill = b;
		} else
		b++;
	}

	if (!cTill) {
		cTill = "end";
	}

	*b++=0;
	while (*b && *b==' ') *b++;
	*pcText = b;

	if (!stricmp(cTill,"end")) {
		*iEnd = -1;
	} else {
		*iEnd = Str2Millisec(cTill)*1000000;
	}

	*iBegin = Str2Millisec(cFrom)*1000000;

	return 0;
}

int AddChaptersToTree(CUnicodeTreeCtrl* tree,HTREEITEM hParent, CChapters* c)
{
	for (int i=0;i<c->GetChapterCount();i++) {
		__int64 iBegin; __int64 iEnd; char cText[150];
		char cBuffer[500]; 

		c->GetChapter(i,&iBegin,&iEnd,cText);
		FormatChapterEntry(iBegin,iEnd,cText,cBuffer);

		HTREEITEM item = Tree_Insert(tree,LPSTR_TEXTCALLBACK,hParent);
		CHAPTER_ENTRY* ce = (CHAPTER_ENTRY*)calloc(1,sizeof(CHAPTER_ENTRY));
		ce->c = c;
		ce->iIndex = i;
		ce->cText = new char[1024];
		ce->cText[0] = 0;
		strcpy(ce->cText, cBuffer);
		tree->SetItemData(item,(DWORD)ce);
		if (c->HasSubChapters(i)) {
			AddChaptersToTree(tree,item,c->GetSubChapters(i));
		}
	}
	return 0;
}



void CChapterDlg::UpdateChapters()
{
	RemoveFromTree(&m_Chapters,m_Chapters.GetRootItem());
	AddChaptersToTree(&m_Chapters,NULL,c);
}

BOOL CChapterDlg::OnInitDialog() 
{
	// TODO: Zusätzliche Initialisierung hier einfügen
	CDialog::OnInitDialog();
	RECT rect;

	m_Chapters.InitUnicode();
	m_ChapterDisplay.InitUnicode();
	m_ChapterDisplay.GetWindowRect(&rect);
	m_ChapterDisplay_Edit.InitUnicode();

	for (int i=0;i<sizeof(languages)/sizeof(char*[2]);i++) {
		char c[1024]; c[0]=0;
		sprintf(c, "%s - %s", languages[i][1], languages[i][0]);
		m_ChapterDisplay_Lng.SetItemData(m_ChapterDisplay_Lng.AddString(c),(LPARAM)languages[i][1]);
	}
	
	DWORD dwDeltaH=abs(rect.top-rect.bottom);
	m_ChapterDisplay.InsertColumn(0,"Language",LVCFMT_CENTER,(rect.right-rect.left)/7);
	m_ChapterDisplay.InsertColumn(1,"String",LVCFMT_LEFT,(rect.right-rect.left)*6/7-20);


	SendDlgItemMessage(IDOK,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_GEN_OK));
	SendDlgItemMessage(IDCANCEL,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_GEN_CANCEL));
	m_Saveas.SetWindowText(LoadString(STR_CHPDLG_SAVEAS));
	m_Subchapter.SetWindowText(LoadString(STR_CHPDLG_SUBCHAPTER));
	SetWindowText(LoadString(STR_CHPDLG_TITLE));

	AddChaptersToTree(&m_Chapters,NULL,c);
	m_Chapters.SetFocus();

	m_Chapters.SetChapters(c);
	iSelectedChapterLanguageEntry=-1;
	m_ChapterDisplay_Lng.EnableWindow(0);
	m_ChapterDisplay_Edit.EnableWindow(0);
	hSelectedChapter = m_Chapters.GetSelectedItem();

	return false;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

BOOL CChapterDlg::DestroyWindow() 
{
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen
	RemoveFromTree(&m_Chapters,m_Chapters.GetRootItem());

	

	return CDialog::DestroyWindow();
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

	HTREEITEM hItem = hSelectedChapter;
	CHAPTER_ENTRY* pCE = (CHAPTER_ENTRY*)m_Chapters.GetItemData(hItem);
	
	if (!u) {
		CEdit*	edit = m_Chapters.GetEditControl();
		edit->GetWindowText(s, 1024);
	} else {
		HWND edit = TreeView_GetEditControl(m_Chapters.m_hWnd);
		SendMessageW(edit, WM_GETTEXT, 1024, (unsigned int)s);
	}
	
	b = s;

	m_Chapters.toUTF8(s, s);

	DecomposeEntry(s,&iBegin,&iEnd,&cText);
	pCE->c->SetChapterBegin(pCE->iIndex,iBegin);
	pCE->c->SetChapterEnd(pCE->iIndex,iEnd);
	
	pCE->c->SetChapterText(pCE->iIndex,cText);

	char cBuffer[500]; ZeroMemory(cBuffer,sizeof(cBuffer));
	FormatChapterEntry(iBegin,iEnd,cText,cBuffer);

	strcpy(pCE->cText, cBuffer);

	m_OK.EnableWindow(1);
	m_Cancel.EnableWindow(1);
	m_Saveas.EnableWindow(1);
	m_Subchapter.EnableWindow(1);
	InvalidateRect(NULL);

	*pResult = 0;
}
/*
void CChapterDlg::OnKeydownTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
	HTREEITEM hItem = m_Chapters.GetSelectedItem();
	HTREEITEM hNext = NULL;
	HTREEITEM hNew = NULL;
	HTREEITEM hParent = m_Chapters.GetParentItem(hItem);
	TVINSERTSTRUCT	tvi;


	CHAPTER_ENTRY* pCE = NULL;
	CHAPTER_ENTRY* pNew = NULL;
	int u = !!SendDlgItemMessage(IDC_CHAPTERTREE, TVM_GETUNICODEFORMAT);

	if (hItem) {
		pCE = (CHAPTER_ENTRY*)m_Chapters.GetItemData(hItem);
	} else {
		pCE = (CHAPTER_ENTRY*)calloc(1,sizeof(CHAPTER_ENTRY));
		pCE->c = c;
	}

	switch (pTVKeyDown->wVKey) {
		case VK_DELETE: 
			hNext = m_Chapters.GetNextSiblingItem(hItem);
			
			if (hItem) {
	
				RemoveFromTree(&m_Chapters,m_Chapters.GetChildItem(hItem));
				pCE->c->DeleteChapter(pCE->iIndex);
				delete pCE;

				while (hNext) {
					pCE = ((CHAPTER_ENTRY*)m_Chapters.GetItemData(hNext));
					pCE->iIndex--;
					hNext = m_Chapters.GetNextSiblingItem(hNext);
				}
			
				m_Chapters.DeleteItem(hItem);
			}
			break;
		case VK_INSERT:
			pCE->c->AddChapter(0,-1,"title");
			char cBuffer[200];
			FormatChapterEntry(0,-1,"title",cBuffer);
			pNew = (CHAPTER_ENTRY*)calloc(1,sizeof(CHAPTER_ENTRY));
			pNew->c = pCE->c;
			pNew->iIndex = pCE->c->GetChapterCount()-1;

			if (u) {
				Str2UTF8(cBuffer, cBuffer);
			}

			tvi.hParent=hParent;
			tvi.hInsertAfter=TVI_LAST;
			tvi.item.mask=TVIF_TEXT;
			tvi.item.pszText= LPSTR_TEXTCALLBACK;
			pNew->cText = new char[1024];
			strcpy(pNew->cText, cBuffer);
			hNew = m_Chapters.InsertItem(&tvi);
	
			m_Chapters.SetItemData(hNew,(DWORD)pNew);
			m_Chapters.Expand(hItem,TVE_EXPAND);
			m_Chapters.SelectItem(hNew);
			m_Chapters.EditLabel(hNew);
			break;
	}

	*pResult = 0;
}
*/
void CChapterDlg::OnMakesubchapter() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	HTREEITEM hItem = hSelectedChapter;//m_Chapters.GetSelectedItem();
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
	
	pNew->c->AddChapter(0,-1,"title");
	char cBuffer[200];
	__int64 j = pCE->c->GetChapterBegin(pCE->iIndex);
	FormatChapterEntry(j,-1,"title",cBuffer);
	pNew->iIndex = pNew->c->GetChapterCount()-1;
	pNew->cText = new char[1024];
	strcpy(pNew->cText, cBuffer);
	
	m_Chapters.SetItemData(hNew = Tree_Insert(&m_Chapters,LPSTR_TEXTCALLBACK,hItem),(DWORD)pNew);
	m_Chapters.Expand(hItem,TVE_EXPAND);
	m_Chapters.SelectItem(hNew);
	m_Chapters.EditLabel(hNew);
}

int RenderChapters2File(FILE* f, CChapters* c)
{
	for (int i=0;i<c->GetChapterCount();i++) {
//		__int64 iBegin; __int64 iEnd; char cText[200];
		char cTime[30];

		SINGLE_CHAPTER_DATA scd;
		c->GetChapter(i, &scd);

		fprintf(f,"ADD ");
		Millisec2Str(scd.iBegin/1000000,cTime);
		fprintf(f,cTime);
		fprintf(f," ");
		if (scd.iEnd!=-1) {
			Millisec2Str(scd.iEnd/1000000,cTime);
		} else {
			sprintf(cTime,"END");
		}

		fprintf(f,"%s %s %d %d %s",cTime,scd.cLng,scd.bEnabled,scd.bHidden,scd.cText);
		fprintf(f,"\n");
		if (c->HasSubChapters(i)) {
			fprintf(f,"SUB BEGIN\n");
			RenderChapters2File(f,c->GetSubChapters(i));
			fprintf(f,"SUB END\n");
		}
	}

	return 0;
}

void CChapterDlg::OnSaveas() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	XMLNODE* xml;
	char* f, *e;
	char* t;

	c->CreateXMLTree(&xml);
	char* b = (char*)malloc(1<<20);
//	char b[1048576];
	ZeroMemory(b, 1<<20);
	sprintf(b,"%c%c%c%s\n%s\n",0xEF,0xBB,0xBF,"<?xml version=\"1.0\" encoding=\"utf-8\"?>",
		"<!DOCTYPE Chapters SYSTEM \"matroskachapters.dtd\">");
	xmlTreeToString((XMLNODE*)xml, b+strlen(b), 1048576);

	if (m_Chapters.GetCount()) {
		CFileDialog* cfd = new CFileDialog(false,"xml","",0,
			"All file types (*.amg, *.xml, *.txt)|*.xml;*.amg;*.txt|script files  (*.amg)|*.amg|XML chapter file (*.txt;*.xml)|*.txt;*.xml||",NULL);
		if (cfd->DoModal()==IDOK) {
			CString s = cfd->GetPathName();
			t = s.GetBuffer(1024);
			splitpathname(t, (char**)&f, (char**)&e, NULL);
			FILE* f = fopen(s,"w");

			if (!stricmp(e, "amg")) {
				fprintf(f,"SET OUTPUT OPTIONS\n");
				fprintf(f,"WITH SET OPTION CHAPTERS\n");
				RenderChapters2File(f,c);

				fprintf(f,"END WITH\n");
			} else {
				fprintf(f, b);
			}
			fclose(f);
		}
		delete cfd;
	}

	xmlDeleteNode(&xml);
	free(b);
	
}

void CChapterDlg::OnBeginlabeleditTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	int u = SendDlgItemMessage(IDC_CHAPTERTREE, TVM_GETUNICODEFORMAT);

	HTREEITEM hItem = hSelectedChapter;
	CHAPTER_ENTRY* pCE = (CHAPTER_ENTRY*)m_Chapters.GetItemData(hItem);
	
	if (!u) {
		CEdit*	edit = m_Chapters.GetEditControl();
		char c[1024];
		UTF82Str(pCE->cText, c);
		edit->SetWindowText(c);
	} else {
		HWND edit = TreeView_GetEditControl(m_Chapters.m_hWnd);
		char c[1024]; c[0]=0; c[1]=0;
		UTF82WStr(pCE->cText, c);
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
	
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
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


void CChapterDlg::OnSelchangedChaptertree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
	HTREEITEM  hNewItem = pNMTreeView->itemNew.hItem;
	hSelectedChapter = hNewItem;
	CHAPTER_ENTRY* ce = (CHAPTER_ENTRY*)m_Chapters.GetItemData(hNewItem);

	if (ce) {
		iSelectedChapterLanguageEntry=-1;
		m_ChapterDisplay_Lng.EnableWindow(0);
		m_ChapterDisplay_Edit.EnableWindow(0);
		int j = m_ChapterDisplay.GetItemCount();
		int disp_count = ce->c->GetChapterDisplayCount(ce->iIndex);
		j = m_ChapterDisplay.GetItemCount();

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
			} else m_ChapterDisplay.InsertItem(&lvitem);
			m_ChapterDisplay.SetItemText(i, 1, txt);

			m_ChapterDisplay.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
			iSelectedChapterLanguageEntry = 0;
			
			int x = (int)m_ChapterDisplay.GetFirstSelectedItemPosition();
			m_ChapterDisplay_Lng.EnableWindow(1);
			m_ChapterDisplay_Edit.EnableWindow(1);

			UpdateChapterDisplayLngEdit(0);

		}

		for (i=j-1; i>=disp_count+1; i--) {
			m_ChapterDisplay.DeleteItem(i);
		}
		m_ChapterDisplay.InvalidateRect(NULL);
		m_ChapterDisplay.UpdateWindow();
	}

	*pResult = 0;
}

void CChapterDlg::UpdateChapterDisplayLngEdit(int iItem)
{
	HTREEITEM h = NULL;

	if (!m_Chapters.GetRootItem()) return;

	int count = m_ChapterDisplay_Lng.GetCount();
	char* c;
	c = m_ChapterDisplay.GetItemText(iItem, 1);
	
	char* l;
	l = m_ChapterDisplay.GetItemText(iItem, 0);

	for (int i=0;i<count;i++) {
		char* lng = (char*)m_ChapterDisplay_Lng.GetItemData(i);
		if (!strcmp(lng, l)) m_ChapterDisplay_Lng.SetCurSel(i);
	}

	if (h=m_ChapterDisplay_Edit.GetRootItem()) m_ChapterDisplay_Edit.DeleteItem(h);

	TVINSERTSTRUCT	tvi;
	ZeroMemory(&tvi,sizeof(tvi));
	tvi.item.mask=TVIF_TEXT;
	tvi.item.pszText=(c?c:"");
	tvi.item.cchTextMax=1+(c?strlen(c):0);

	h = m_ChapterDisplay_Edit.InsertItem(&tvi);

	m_ChapterDisplay_Edit.InvalidateRect(NULL);
	m_ChapterDisplay_Edit.UpdateWindow();
	m_ChapterDisplay_Edit.EnsureVisible(h);
}

void CChapterDlg::OnItemchangedChapterDisplay(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	if (!m_Chapters.GetRootItem()) return;
	
	m_ChapterDisplay_Edit.DeleteAllItems();
	int iItem = pNMListView->iItem;
	int iSubItem = pNMListView->iSubItem;
	int old_state = pNMListView->uOldState;
	int new_state = pNMListView->uNewState;

	if (((old_state & LVIS_SELECTED) != LVIS_SELECTED) && ((new_state & LVIS_SELECTED) == LVIS_SELECTED)) {
		iSelectedChapterLanguageEntry = (int)m_ChapterDisplay.GetFirstSelectedItemPosition()-1;
		m_ChapterDisplay_Edit.EnableWindow(1);
		m_ChapterDisplay_Lng.EnableWindow(1);
		UpdateChapterDisplayLngEdit(iItem);
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

	if (!hSelectedChapter) {
		hSelectedChapter = m_Chapters.GetSelectedItem();
	}

	HTREEITEM hItem = hSelectedChapter;
	CHAPTER_ENTRY* ce = (CHAPTER_ENTRY*)m_Chapters.GetItemData(hItem);

	ce->c->SetChapterLng(ce->iIndex, (lng_code?lng_code:s.GetBuffer(256)), chp_lng_index);

	if (chp_lng_index == ce->c->GetChapterDisplayCount(ce->iIndex)-1
		&& m_ChapterDisplay.GetItemCount()-1 == chp_lng_index) {
		LVITEM  lvitem;
		ZeroMemory(&lvitem,sizeof(lvitem));
		lvitem.mask=LVIF_TEXT | LVIF_NORECOMPUTE;
		lvitem.iItem=chp_lng_index+1;
		lvitem.iSubItem=0;
		lvitem.pszText="<new>";

		m_ChapterDisplay.InsertItem(&lvitem);

		TVINSERTSTRUCT tvi; ZeroMemory(&tvi, sizeof(tvi));
		TVITEM* tvitem = &tvi.item;
		tvitem->mask = TVIF_TEXT;
		tvitem->pszText = "<new>";

		m_ChapterDisplay_Edit.InsertItem(&tvi);
	}
	
	m_ChapterDisplay.SetItemText(chp_lng_index, 0, ce->c->GetChapterLng(ce->iIndex, chp_lng_index));
	m_ChapterDisplay.InvalidateRect(NULL);
	m_ChapterDisplay.UpdateWindow();
	
}

void CChapterDlg::OnEndlabeleditChapterdisplayEdit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (!m_Chapters.GetRootItem()) return;

	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	int u = SendDlgItemMessage(IDC_CHAPTERTREE, TVM_GETUNICODEFORMAT);
	char s[1024]; s[0]=0; char c[1024]; c[0]=0;
	if (!u) {
		CEdit*	edit = m_ChapterDisplay_Edit.GetEditControl();
		edit->GetWindowText(s, 1024);
	} else {
		HWND edit = TreeView_GetEditControl(m_ChapterDisplay_Edit.m_hWnd);
		SendMessageW(edit, WM_GETTEXT, 1024, (unsigned int)s);
	}

	m_ChapterDisplay_Edit.toUTF8(s, c);

	TVITEM tvitem; ZeroMemory(&tvitem,sizeof(tvitem));
	tvitem.mask = TVIF_TEXT;
	tvitem.pszText = c;
	tvitem.hItem = m_ChapterDisplay_Edit.GetRootItem();

	m_ChapterDisplay_Edit.SetItem(&tvitem);
	int chp_lng_index = iSelectedChapterLanguageEntry;
	HTREEITEM hItem = hSelectedChapter;
	CHAPTER_ENTRY* ce = (CHAPTER_ENTRY*)m_Chapters.GetItemData(hItem);

	ce->c->SetChapterText(ce->iIndex, c, chp_lng_index);
	m_ChapterDisplay.SetItemText(chp_lng_index, 1, ce->c->GetChapterText(ce->iIndex, chp_lng_index));
	m_ChapterDisplay.InvalidateRect(NULL);
	m_ChapterDisplay.UpdateWindow();

	__int64 iBegin, iEnd; char cText[1024]; cText[0]=0; char cBuffer[1024]; cBuffer[0]=0;
	ce->c->GetChapter(ce->iIndex,&iBegin,&iEnd,cText);
	FormatChapterEntry(iBegin, iEnd, cText, cBuffer);

	if (!chp_lng_index) {
		if (ce->cText) delete ce->cText; ce->cText=0;
		ce->cText = new char[1+strlen(cBuffer)];
		strcpy(ce->cText, cBuffer);
		m_Chapters.InvalidateRect(NULL);
		m_Chapters.UpdateWindow();
	}
	*pResult = 0;

	m_Chapters.EnableWindow(1);
}

void CChapterDlg::OnKeydownChapterDisplay(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (!m_Chapters.GetRootItem()) return;

	HTREEITEM hItem			= hSelectedChapter;//m_Chapters.GetSelectedItem();
	CHAPTER_ENTRY* ce		= (CHAPTER_ENTRY*)m_Chapters.GetItemData(hItem);
	LV_KEYDOWN* pLVKeyDown	= (LV_KEYDOWN*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	int key = pLVKeyDown->wVKey;

	if (hItem && ce && key == VK_DELETE) {
		if (iSelectedChapterLanguageEntry > -1) {
			int j = ce->c->GetChapterDisplayCount(ce->iIndex);
			if (j>1 && iSelectedChapterLanguageEntry<j) {
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
		}
		
	}
	

	*pResult = 0;
}

void CChapterDlg::OnBeginlabeleditChapterdisplayEdit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (!m_Chapters.GetRootItem()) return;

	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen

	m_Chapters.EnableWindow(0);

	*pResult = 0;
}
