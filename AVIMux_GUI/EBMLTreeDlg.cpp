// EBMLTreeDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "EBMLTreeDlg.h"
#include "EBMLTree.h"
#include "..\matroska.h"
#include "..\matroska_block.h"
#include "FormatText.h"
#include "UnicodeTreeCtrl.h"
#include "..\cache.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CEBMLTreeDlg 


char* sem_close = "semaphore_close";
int iDepth;

CEBMLTreeDlg::CEBMLTreeDlg(CWnd* pParent /*=NULL*/)
	: CResizeableDialog(CEBMLTreeDlg::IDD, pParent)
{
	EnableAutomation();

	//{{AFX_DATA_INIT(CEBMLTreeDlg)
	//}}AFX_DATA_INIT
}


void CEBMLTreeDlg::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CResizeableDialog::OnFinalRelease();
}

void CEBMLTreeDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizeableDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEBMLTreeDlg)
	DDX_Control(pDX, IDOK, m_OK);
	DDX_Control(pDX, IDC_TREE1, m_EBMLTree);
	DDX_Control(pDX, IDC_ABSOLUTE, m_Absolute);
	DDX_Control(pDX, IDC_RELATIVE, m_Relative);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEBMLTreeDlg, CResizeableDialog)
	//{{AFX_MSG_MAP(CEBMLTreeDlg)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, OnSelchangedTree1)
	ON_BN_CLICKED(IDC_ABSOLUTE, OnAbsolute)
	ON_BN_CLICKED(IDC_RELATIVE, OnRelative)
	ON_NOTIFY(TVN_SELCHANGEDW, IDC_TREE1, OnSelchangedTree1)
	ON_BN_CLICKED(IDC_FULLEXPAND, OnFullexpand)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_FONT_LARGER, OnBnClickedFontLarger)
	ON_BN_CLICKED(IDC_FONT_SMALLER, OnBnClickedFontSmaller)
//	ON_BN_CLICKED(IDOK, OnBnClickedOk)
ON_BN_CLICKED(IDOK, OnBnClickedOk)
ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CEBMLTreeDlg, CResizeableDialog)
	//{{AFX_DISPATCH_MAP(CEBMLTreeDlg)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_IEBMLTreeDlg zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {1B7FAF77-38F6-4D01-B045-1BD4EC892CF9}
static const IID IID_IEBMLTreeDlg =
{ 0x1b7faf77, 0x38f6, 0x4d01, { 0xb0, 0x45, 0x1b, 0xd4, 0xec, 0x89, 0x2c, 0xf9 } };

BEGIN_INTERFACE_MAP(CEBMLTreeDlg, CResizeableDialog)
	INTERFACE_PART(CEBMLTreeDlg, IID_IEBMLTreeDlg, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CEBMLTreeDlg 

#include "trees.h"
#include "..\ebml.h"
#include "..\matroska.h"
#include ".\ebmltreedlg.h"

typedef struct
{
	CEBMLTree*   tree;
	HTREEITEM    hParent;
	EBMLElement* eParent;
	bool*		 pbDoClose;
//  HANDLE		 hSemaphore_1;
//	char*		 cSemaphore_Finished;
	int			 iFlag;
} ADD_CHILDREN_DATA;

DWORD WINAPI AddChildren_Thread(void* pData);

char* sem_name[] = 
	{ "add_children_thread_semaphore_1", "add_children_thread_semaphore_2",
	  "add_children_thread_semaphore_3"
 };


void AddChildren(CEBMLTree* tree, HTREEITEM hParent, EBMLElement* eParent, 
				 bool* pbDoClose)
{
	EBMLITEM_DESCRIPTOR* d;
	HTREEITEM	hThis;
	
	HANDLE		hSem_Loop;

	char		msg[128];
	msg[0]=0;
	
	bool		bExpanded = false;

	iDepth++;

//	if (iDepth >= 4) {
//		MessageBox(0, "B0rked!", "Fatal error", MB_OK | MB_ICONERROR);
//		while (iDepth >= 4 && !*pbDoClose) Sleep(100);
//	}
	
	if (!(hSem_Loop = OpenSemaphore(SEMAPHORE_ALL_ACCESS, NULL, sem_name[1]))) {
		hSem_Loop = CreateSemaphore(NULL, 1, 1, sem_name[1]);
		if (!hSem_Loop)
			MessageBox(0, "Internal error: OpenSemaphore and CreateSemaphore failed!", "B0rked!", MB_OK | MB_ICONERROR);
	}

	if (!tree->GetChildItem(hParent) && !*pbDoClose) {
		EBMLElement* e = NULL;

		int wait_result = -1;
		while (wait_result != WAIT_OBJECT_0 && !*pbDoClose)
			wait_result = WaitForSingleObject(hSem_Loop, 100);
		
		if (wait_result == WAIT_OBJECT_0) {
			if (eParent && eParent->IsMaster() && !*pbDoClose)
				eParent->Create1stSub(&e);
		
			ReleaseSemaphore(hSem_Loop,1,NULL);
		}

		while (e && !*pbDoClose) {
			wait_result = -1;
			while (!*pbDoClose && wait_result != WAIT_OBJECT_0) {
				wait_result = WaitForSingleObject(hSem_Loop, 100);
			}
			  
			if (wait_result == WAIT_OBJECT_0) {
				d = new EBMLITEM_DESCRIPTOR;
				d->pElement = e;
				d->iItemPosition = e->GetAbsoluteHeaderPos();
				d->iRelPosition = e->GetRelativeHeaderPos();
				d->iHeaderSize = e->GetHeaderSize();
				
				if (e->GetLevel() >= 1) 
					e->CheckCRC();

				if (!*pbDoClose) 
					tree->SetItemData(hThis = Tree_Insert(tree,
						(char*)LPSTR_TEXTCALLBACK,hParent),
						(DWORD)d);
				
				if (e->GetType() == IDVALUE(MID_CL_BLOCK) ||
					e->GetType() == IDVALUE(MID_CL_SIMPLEBLOCK)) {
					EBMLM_CLBlock* block = (EBMLM_CLBlock*)e;
					CLBLOCKHEADER hdr;

					CBuffer* buffer = block->Read(&hdr);
					sprintf(msg,"stream: %d %s%s",hdr.iStream,
						((hdr.iFlags & BLKHDRF_KEYFRAME) == BLKHDRF_KEYFRAME)?"keyframe ":"",
						((hdr.iFlags & BLKHDRF_DISCARDABLE) == BLKHDRF_DISCARDABLE)?"discardable ":"");
					Tree_Insert(tree,msg,hThis);				
					if (hdr.frame_sizes.size() > 1) {
						sprintf(msg,"number of frames: %d %s ",hdr.frame_sizes.size(),
							((hdr.iFlags & BLKHDRF_LACINGNEW) == BLKHDRF_LACINGEBML)?"(EBML lacing)":
							((hdr.iFlags & BLKHDRF_LACINGNEW) == BLKHDRF_LACINGCONST)?"(constant lacing)":
							((hdr.iFlags & BLKHDRF_LACINGNEW) == BLKHDRF_LACING)?"(xiph lacing)":"");
						
						HTREEITEM h = Tree_Insert(tree,msg,hThis);				
						for (size_t i=0;i<hdr.frame_sizes.size();i++) {
							char cSize[20]; QW2Str(hdr.frame_sizes[i],cSize, 1);
							sprintf(msg,"frame size: %s bytes", cSize);
							Tree_Insert(tree, msg, h);
						}
					}
					char cTC[64]; cTC[0]=0; 
					QW2Str((__int64)((__int16)(hdr.iTimecode)), cTC, 1);

					sprintf(msg,"time code, relative to cluster: %s", 
						cTC,0);
					Tree_Insert(tree,msg,hThis);	
					cTC[0]=0;
					if (e->GetType() == IDVALUE(MID_CL_BLOCK)) {
						QW2Str((__int64)(__int16)hdr.iTimecode +((EBMLM_Cluster*)e->GetParent()->GetParent())->GetTimecode(), cTC, 1);
					} else {
						QW2Str((__int64)(__int16)hdr.iTimecode +((EBMLM_Cluster*)e->GetParent())->GetTimecode(), cTC, 1);
					}
					sprintf(msg,"absolute time code: %s", cTC);
					Tree_Insert(tree,msg,hThis);	
					e->Delete();
				}

				ADD_CHILDREN_DATA* a = NULL;

				if (e->GetType() == IDVALUE(MID_CL_BLOCKGROUP) /*||
					e->GetType() == IDVALUE(MID_SEGMENTINFO)*/) {
						a = new ADD_CHILDREN_DATA;
						ZeroMemory(a,sizeof(*a));
						a->eParent = e;
						a->hParent = hThis;
						a->tree = tree;
						a->pbDoClose = pbDoClose;
						a->iFlag = 0;
						DWORD dwID;
						CreateThread(NULL, 1<<20, AddChildren_Thread, a, 
							0, &dwID);
						
						RECT r;
						tree->GetItemRect(hParent, &r, false);
						tree->InvalidateRect(&r, false);
						tree->UpdateWindow();
						//if (!*pbDoClose)
						//	tree->Expand(hThis,TVE_EXPAND);
						
				}

				if (e->GetType() == IDVALUE(MID_CL_BLOCK) ||
					e->GetType() == IDVALUE(MID_CL_SIMPLEBLOCK)) {
					
/*					if (!*pbDoClose) 
						tree->Expand(hThis,TVE_EXPAND);*/
				}

				e->SeekStream(0);
				e = e->GetSucc();

				if (!bExpanded) {
					if (!*pbDoClose) {
						HTREEITEM hSelected = tree->GetSelectedItem();
						tree->Expand(hParent,TVE_EXPAND);
						tree->EnsureVisible(hSelected);
					}
				
					bExpanded = true;
				}

				ReleaseSemaphore(hSem_Loop, 1, NULL);
			}
		}
	}

//	if (!*pbDoClose) 
//		tree->Expand(hParent,TVE_EXPAND);


	RECT r;
	tree->GetItemRect(hParent, &r, false);
	tree->InvalidateRect(&r, false);
	tree->UpdateWindow();

	CloseHandle(hSem_Loop);

	iDepth--;
}


DWORD WINAPI AddChildren_Thread(void* pData)
{
	ADD_CHILDREN_DATA* acd = (ADD_CHILDREN_DATA*)pData;

	if (acd->eParent->GetLength()) {

		AddChildren(acd->tree, acd->hParent, acd->eParent, 
			acd->pbDoClose);

	}
	acd->iFlag = 1;
	delete acd;
	return 1;
}


void CEBMLTreeDlg::CleanChildren(HTREEITEM hParent)
{
	HTREEITEM hItem = m_EBMLTree.GetChildItem(hParent);
	HTREEITEM hNext;

	while (hItem) {
		EBMLITEM_DESCRIPTOR* d = (EBMLITEM_DESCRIPTOR*)m_EBMLTree.GetItemData(hItem);
		m_EBMLTree.Expand(hItem,TVE_COLLAPSE);
		if (d) {
			d->pElement->Delete();
			delete d->pElement;
			d->pElement = NULL;
		}
		
		CleanChildren(hItem);
		hNext = m_EBMLTree.GetNextSiblingItem(hItem);
		m_EBMLTree.DeleteItem(hItem);
		
		if (d) {
			delete d;
		}
		
		hItem = hNext;
	}
}

void CEBMLTreeDlg::RecreateTreeFont()
{
	CFont* font = new CFont;
	
	if (!font->CreateFont(-font_size, 0, 0, 0, FW_NORMAL, false, false,
		false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, FF_ROMAN, "Microsoft Sans Serif") &&
		!font->CreateFont(-font_size, 0, 0, 0, FW_NORMAL, false, false,
		false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, FF_ROMAN, "Microsoft Sans Serif"))
			return;
		
	GetAttribs()->SetInt("tree_font_size", font_size);

	CFont* old_font = m_EBMLTree.GetFont();
	m_EBMLTree.SetFont(font);
	delete old_font;
}

BOOL CEBMLTreeDlg::OnInitDialog() 
{
	CResizeableDialog::OnInitDialog();
	
	// TODO: Zusätzliche Initialisierung hier einfügen
	
	m_EBMLTree.InitUnicode();
	
	source->Seek(0);
	fThreadsafe = source->IsEnabled(CACHE_THREADSAFE);
	source->Enable(CACHE_THREADSAFE);
	e_matroska = new EBML_Matroska(source,0);

	bDoClose = false;
	iDepth = 0;
	font_size = 12;
	
	ADD_CHILDREN_DATA* a = new ADD_CHILDREN_DATA;
	ZeroMemory(a,sizeof(*a));
	a->hParent = NULL;
	a->tree = &m_EBMLTree;
	a->eParent = e_matroska;
	a->pbDoClose = &bDoClose;
//	a->hSemaphore_1 = NULL;
	a->iFlag = 0;

	//AddChildren(&m_EBMLTree, 0, e_matroska, &bDoClose, 0, "semamphore_close");
	DWORD dwID;
	CreateThread(NULL, 1<<20, AddChildren_Thread, a, NULL, &dwID);
	m_Absolute.SetCheck(1);
	OnAbsolute();
	m_Relative.SetCheck(0);

	int w, h;
	GetBorder(w, h);

	AttachWindow(m_OK, ATTB_RIGHT, m_hWnd, -16);
	AttachWindow(*GetDlgItem(IDC_FONT_LARGER), ATTB_RIGHT, m_OK);
	AttachWindow(*GetDlgItem(IDC_FONT_LARGER), ATTB_TOP, m_OK, ATTB_BOTTOM, 1);
	AttachWindow(*GetDlgItem(IDC_FONT_SMALLER), ATTB_RIGHT, *GetDlgItem(IDC_FONT_LARGER), ATTB_LEFT, -1);
	AttachWindow(*GetDlgItem(IDC_FONT_SMALLER), ATTB_TOPBOTTOM, *GetDlgItem(IDC_FONT_LARGER));

	AttachWindow(m_EBMLTree, ATTB_TOP, m_hWnd, h + 16);
	AttachWindow(m_EBMLTree, ATTB_BOTTOM, m_hWnd, -16);
	AttachWindow(m_Relative, ATTB_RIGHT, m_OK);
	AttachWindow(m_Absolute, ATTB_RIGHT, m_OK);
	AttachWindow(m_Relative, ATTB_LEFT, m_OK);
	AttachWindow(m_Absolute, ATTB_LEFT, m_OK);
	AttachWindow(m_EBMLTree, ATTB_RIGHT, m_OK, ATTB_LEFT, -16);
	AttachWindow(m_EBMLTree, ATTB_LEFT, m_hWnd, 16);

	CFont* font = new CFont;
	font->CreateFont(-font_size, 0, 0, 0, FW_NORMAL, false, false,
		false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH, "Microsoft Sans Serif");
	
	m_EBMLTree.SetFont(font);

	font_size = (int)GetAttribs()->GetIntWithDefault("tree_font_size", 12);

	RecreateTreeFont();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CEBMLTreeDlg::SetSource(STREAM* lpSource)
{
	source = lpSource;
}

STREAM* CEBMLTreeDlg::GetSource()
{
	return source;
}

void CEBMLTreeDlg::OnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	TVITEM* item = &(pNMTreeView->itemNew);
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
	EBMLITEM_DESCRIPTOR* d = (EBMLITEM_DESCRIPTOR*)m_EBMLTree.GetItemData(item->hItem);
	if (d && item && !bDoClose) {

	//	if (d->pElement->GetType() == ETM_SEGMENT) {
		ADD_CHILDREN_DATA* a = new ADD_CHILDREN_DATA;
		ZeroMemory(a,sizeof(*a));
		a->tree = &m_EBMLTree;
		a->hParent = item->hItem;
		a->eParent = d->pElement;
		a->pbDoClose = &bDoClose;
		a->iFlag = 0;
		AfxBeginThread((AFX_THREADPROC)AddChildren_Thread, a);

//			m_EBMLTree.InvalidateRect(NULL);
//			m_EBMLTree.UpdateWindow();
//			m_EBMLTree.Expand(item->hItem, TVE_EXPAND);
	/*	} else {
			AddChildren(&m_EBMLTree, item->hItem, d->pElement, &bDoClose, NULL, NULL);
		}*/
	}
	*pResult = 0;
}

void CEBMLTreeDlg::OnOK() 
{
	// TODO: Zusätzliche Prüfung hier einfügen
	bDoClose = true;
//	hSem_close = OpenSemaphore(SEMAPHORE_ALL_ACCESS, false, sem_close);
//	WaitForSingleObject(hSem_close, INFINITE);
/*	while (iDepth) {
		Sleep(100);
	}
	*/
	if (iDepth) {  // don't hang here, otherwise it will completely hang
		Sleep(100);
		PostMessage(WM_COMMAND, IDOK, 0);
	} else {
	//	CloseHandle(hSem_close);
		CleanChildren(0);
		e_matroska->Delete();
		delete e_matroska;

		delete m_EBMLTree.GetFont();

		CResizeableDialog::OnOK();
	}
}

void CEBMLTreeDlg::OnAbsolute() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	m_EBMLTree.SetMode(1);
}

void CEBMLTreeDlg::OnRelative() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	m_EBMLTree.SetMode(0);

}

void CEBMLTreeDlg::OnFullexpand() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
//	HTREEITEM* hItem = m_EBMLTree.GetRootItem();
	

}

void CEBMLTreeDlg::OnBnClickedFontLarger()
{
	// TODO: Fügen Sie hier Ihren Kontrollbehandlungscode für die Benachrichtigung ein.

	font_size++;
	RecreateTreeFont();
}

void CEBMLTreeDlg::OnBnClickedFontSmaller()
{
	// TODO: Fügen Sie hier Ihren Kontrollbehandlungscode für die Benachrichtigung ein.
	if (font_size > 1) 
		font_size--;
	
	RecreateTreeFont();
}


void CEBMLTreeDlg::OnBnClickedOk()
{
	bDoClose = true;

	if (iDepth) {  // don't hang here, otherwise it will completely hang
		Sleep(100);
		PostMessage(WM_COMMAND, IDOK, 0);
	} else {
		source->SetFlag(CACHE_THREADSAFE, fThreadsafe);
		CleanChildren(0);
		e_matroska->Delete();
		delete e_matroska;

		delete m_EBMLTree.GetFont();

		CResizeableDialog::OnOK();
	}
}

void CEBMLTreeDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: Fügen Sie hier Ihren Meldungsbehandlungscode ein, und/oder benutzen Sie den Standard.
	if (nID == SC_CLOSE) {
		PostMessage(WM_COMMAND, IDOK, 0);
		return;
	}

	CResizeableDialog::OnSysCommand(nID, lParam);
}
