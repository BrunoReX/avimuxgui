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
	: CDialog(CEBMLTreeDlg::IDD, pParent)
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

	CDialog::OnFinalRelease();
}

void CEBMLTreeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEBMLTreeDlg)
	DDX_Control(pDX, IDC_TREE1, m_EBMLTree);
	DDX_Control(pDX, IDC_ABSOLUTE, m_Absolute);
	DDX_Control(pDX, IDC_RELATIVE, m_Relative);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEBMLTreeDlg, CDialog)
	//{{AFX_MSG_MAP(CEBMLTreeDlg)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, OnSelchangedTree1)
	ON_BN_CLICKED(IDC_ABSOLUTE, OnAbsolute)
	ON_BN_CLICKED(IDC_RELATIVE, OnRelative)
	ON_NOTIFY(TVN_SELCHANGEDW, IDC_TREE1, OnSelchangedTree1)
	ON_BN_CLICKED(IDC_FULLEXPAND, OnFullexpand)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CEBMLTreeDlg, CDialog)
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

BEGIN_INTERFACE_MAP(CEBMLTreeDlg, CDialog)
	INTERFACE_PART(CEBMLTreeDlg, IID_IEBMLTreeDlg, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CEBMLTreeDlg 

#include "trees.h"
#include "..\ebml.h"
#include "..\matroska.h"
 
typedef struct
{
	CEBMLTree*   tree;
	HTREEITEM    hParent;
	EBMLElement* eParent;
	bool*		 pbDoClose;
	HANDLE		 hSemaphore_1;
	char*		 cSemaphore_Finished;
	int			 iFlag;
} ADD_CHILDREN_DATA;

int AddChildren_Thread(void* pData);

char* sem_name[] = 
	{ "add_children_thread_semaphore_1", "add_children_thread_semaphore_2",
	  "add_children_thread_semaphore_3"
 };


void AddChildren(CEBMLTree* tree, HTREEITEM hParent, EBMLElement* eParent, bool* pbDoClose, HANDLE hSemaphore_1,
				 char* cSemaphore_Finished)
{
	EBMLITEM_DESCRIPTOR* d;
	HTREEITEM	hThis;
	HANDLE		hSem_Loop;//, hSem;
//	int			k;
	HANDLE		hSemaphore_Finished;
	if (cSemaphore_Finished) {
		hSemaphore_Finished = OpenSemaphore(SEMAPHORE_ALL_ACCESS,false, cSemaphore_Finished);
	}

	char	msg[50];
	msg[0]=0;
	bool		bExpanded = false;

	iDepth++;
	if (!(hSem_Loop = OpenSemaphore(SEMAPHORE_ALL_ACCESS, NULL, sem_name[1]))) {
		hSem_Loop = CreateSemaphore(NULL, 1, 1, sem_name[1]);
	}

	if (!tree->GetChildItem(hParent) && !*pbDoClose) {
		EBMLElement* e = NULL;

		if (eParent && eParent->IsMaster() && !*pbDoClose) {
			WaitForSingleObject(hSem_Loop, INFINITE);
			eParent->Create1stSub(&e);
			ReleaseSemaphore(hSem_Loop,1,NULL);
		}

		while (e && !*pbDoClose) {
		  int iWaitResult = -1;
		  while (!*pbDoClose && iWaitResult!=WAIT_OBJECT_0) {
			  iWaitResult = WaitForSingleObject(hSem_Loop, 100);
		  }
		  
		  if (iWaitResult == WAIT_OBJECT_0) {
			d = new EBMLITEM_DESCRIPTOR;
			d->pElement = e;
			d->iItemPosition = e->GetAbsoluteHeaderPos();
			d->iRelPosition = e->GetRelativeHeaderPos();
			d->iHeaderSize = e->GetHeaderSize();
			if (!*pbDoClose) {
				tree->SetItemData(hThis = Tree_Insert(tree,(char*)LPSTR_TEXTCALLBACK,hParent),
				(DWORD)d);
			}
			if (e->GetType() == ETM_CLBLOCK) {
				EBMLM_CLBlock* block = (EBMLM_CLBlock*)e;
				CLBLOCKHEADER hdr;
	
				CBuffer* buffer = block->Read(&hdr);
				sprintf(msg,"stream: %d",hdr.iStream);
				Tree_Insert(tree,msg,hThis);				
				if (hdr.iFrameCountInLace) {
					sprintf(msg,"number of frames: %d (%s)",hdr.iFrameCountInLace,
						((hdr.iFlags & BLKHDRF_LACINGNEW) == BLKHDRF_LACINGEBML)?"EBML":
						((hdr.iFlags & BLKHDRF_LACINGNEW) == BLKHDRF_LACINGCONST)?"const":
						((hdr.iFlags & BLKHDRF_LACINGNEW) == BLKHDRF_LACING)?"xiph":"");
					
					int* ifs = (int*)(hdr.cFrameSizes->GetData());
					HTREEITEM h = Tree_Insert(tree,msg,hThis);				
					for (int i=0;i<hdr.iFrameCountInLace;i++) {
						char cSize[20]; QW2Str(*ifs++,cSize, 1);
						sprintf(msg,"frame size: %s bytes", cSize);
						Tree_Insert(tree, msg, h);
					}
				}
				sprintf(msg,"time code, relative to cluster: %d", (__int16)(hdr.iTimecode),0);
				Tree_Insert(tree,msg,hThis);	
				char cTC[20]; cTC[0]=0; QW2Str((__int64)(__int16)hdr.iTimecode +((EBMLM_Cluster*)e->GetParent()->GetParent())->GetTimecode(), cTC, 1);
				sprintf(msg,"absolute time code: %s", cTC);
				Tree_Insert(tree,msg,hThis);	
			}

			ReleaseSemaphore(hSem_Loop, 1, NULL);
			WaitForSingleObject(hSem_Loop, INFINITE);
			
			ADD_CHILDREN_DATA* a = NULL;
			switch (e->GetType()) {
				case ETM_CLBLOCKGROUP:
				case ETM_SEGMENTINFO:
				//AddChildren(tree, hThis, e, pbDoClose, hSemaphore);
					a = new ADD_CHILDREN_DATA;
					ZeroMemory(a,sizeof(*a));
					a->eParent = eParent;
					a->hParent = hParent;
					a->tree = tree;
					a->pbDoClose = pbDoClose;
					a->iFlag = 0;
					AfxBeginThread((AFX_THREADPROC)AddChildren_Thread, a);
					if (!*pbDoClose) {
						tree->Expand(hThis,TVE_EXPAND);
					}
			}

			switch (e->GetType()) {
				case ETM_CLBLOCK:
					if (!*pbDoClose) tree->Expand(hThis,TVE_EXPAND);
					break;
			}

			e->SeekStream(0);
			e = e->GetSucc();
			ReleaseSemaphore(hSem_Loop, 1, NULL);
			ReleaseSemaphore(hSemaphore_1, 1, NULL);
			if (!bExpanded) {
				if (!*pbDoClose) tree->Expand(hParent,TVE_EXPAND);
				bExpanded = true;
			}
		  }
		}
	}

	if (!*pbDoClose) tree->Expand(hParent,TVE_EXPAND);
	if (cSemaphore_Finished) {
		ReleaseSemaphore(hSemaphore_Finished, 1, NULL);
		CloseHandle(hSemaphore_Finished);
	}
	CloseHandle(hSem_Loop);

	iDepth--;
}


int AddChildren_Thread(void* pData)
{
	ADD_CHILDREN_DATA* acd = (ADD_CHILDREN_DATA*)pData;
//	HANDLE hSem3 = CreateSemaphore(NULL, 0, 1, sem_name[2]);

	if (acd->eParent->GetLength()) {
		HANDLE hSem = OpenSemaphore(SEMAPHORE_ALL_ACCESS, false, sem_name[0]);
		if (!hSem) {
			hSem = CreateSemaphore(NULL, 0, 1, sem_name[0]);
		} else {
			WaitForSingleObject(hSem, INFINITE);
		}

		acd->hSemaphore_1 = hSem;

		AddChildren(acd->tree, acd->hParent, acd->eParent, acd->pbDoClose, acd->hSemaphore_1, 
			acd->cSemaphore_Finished);

		CloseHandle(acd->hSemaphore_1);
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
		}
		CleanChildren(hItem);
		hNext = m_EBMLTree.GetNextSiblingItem(hItem);
		m_EBMLTree.DeleteItem(hItem);
		if (d) {
			delete d->pElement;
			delete d;
		}
		hItem = hNext;
	}
}

BOOL CEBMLTreeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Zusätzliche Initialisierung hier einfügen
	
	m_EBMLTree.InitUnicode();
	
	source->Seek(0);
	e_matroska = new EBML_Matroska(source,0);

	bDoClose = false;
	iDepth = 0;
	
	ADD_CHILDREN_DATA* a = new ADD_CHILDREN_DATA;
	ZeroMemory(a,sizeof(*a));
	a->hParent = NULL;
	a->tree = &m_EBMLTree;
	a->eParent = e_matroska;
	a->pbDoClose = &bDoClose;
	a->hSemaphore_1 = NULL;
	a->iFlag = 0;

	//AddChildren(&m_EBMLTree, 0, e_matroska, &bDoClose, 0, "semamphore_close");
	AfxBeginThread((AFX_THREADPROC)AddChildren_Thread, a);
	m_Absolute.SetCheck(1);
	OnAbsolute();
	m_Relative.SetCheck(0);

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
	if (d && item) {

	//	if (d->pElement->GetType() == ETM_SEGMENT) {
			ADD_CHILDREN_DATA* a = new ADD_CHILDREN_DATA;
			ZeroMemory(a,sizeof(*a));
			a->tree = &m_EBMLTree;
			a->hParent = item->hItem;
			a->eParent = d->pElement;
			a->pbDoClose = &bDoClose;
			a->iFlag = 0;
			AfxBeginThread((AFX_THREADPROC)AddChildren_Thread, a);

			m_EBMLTree.InvalidateRect(NULL);
			m_EBMLTree.UpdateWindow();
			m_EBMLTree.Expand(item->hItem, TVE_EXPAND);
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
		CDialog::OnOK();
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
