// RIFFChunkTreeDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "RIFFChunkTreeDlg.h"
#include "windows.h"
#include "FormatText.h"
#include "ResizeableDialog.h"
#include ".\riffchunktreedlg.h"
#include "..\Cache.h"
#include "HexViewListBox.h"
#include "FileDialogs.h"
#include "..\FileStream.h"
#include "MessageBoxHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef struct
{
	__int64 pos;
} RIFFTREEITEM_DATA;

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CRIFFChunkTreeDlg 

HTREEITEM InsertItem(HWND hTree, HTREEITEM hParent, char* cText, LPARAM data = 0);
HTREEITEM InsertItem(HWND hTree, TVINSERTSTRUCT& tvi);

bool bChunkTree_stop = false;
int iChunkTree_ThreadCount = 0;
CRITICAL_SECTION cs;

CRIFFChunkTreeDlg::CRIFFChunkTreeDlg(CWnd* pParent /*=NULL*/)
	: CResizeableDialog(CRIFFChunkTreeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRIFFChunkTreeDlg)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}


void CRIFFChunkTreeDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizeableDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRIFFChunkTreeDlg)
	DDX_Control(pDX, IDC_TREE1, m_Tree);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_WAIT_FOR_COMPLETE_TREE, m_WaitButton);
	DDX_Control(pDX, IDC_HEXVIEW_LISTBOX2, m_HexView);
}


BEGIN_MESSAGE_MAP(CRIFFChunkTreeDlg, CResizeableDialog)
	//{{AFX_MSG_MAP(CRIFFChunkTreeDlg)
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_WM_SYSCOMMAND()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, OnTvnSelchangedTree1)
	ON_BN_CLICKED(IDC_WAIT, OnBnClickedWait)
	ON_BN_CLICKED(IDC_WAIT_FOR_COMPLETE_TREE, OnBnClickedWaitForCompleteTree)
	ON_LBN_SELCHANGE(IDC_HEXVIEW_LISTBOX2, OnLbnSelchangeHexviewListbox2)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CRIFFChunkTreeDlg 



DWORD	dwMBNbr;

void /*CRIFFChunkTreeDlg::*/ParseAVIH(HWND hTree, STREAM* source,
									  HTREEITEM hParent, CHUNKHEADER ch)
{
	union
	{
		MainAVIHeader	avih;
		char			cDummy[256];
	};
	TVINSERTSTRUCT	tvi;
	char			cBuffer[200];
	char			cQW[20];

	tvi.hParent=hParent;
	tvi.hInsertAfter=TVI_LAST;
	tvi.item.mask=TVIF_TEXT;
	tvi.item.pszText=cBuffer;
	ZeroMemory(cDummy,sizeof(cDummy));
	source->Read(&avih,ch.dwLength);

	QW2Str(avih.dwMicroSecPerFrame,cQW,10);
	sprintf(cBuffer,"dwMicroSecPerFrame   : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	QW2Str(avih.dwMaxBytesPerSec,cQW,10);
	sprintf(cBuffer,"dwMaxBytesPerSec     : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	QW2Str(avih.dwTotalFrames,cQW,10);
	sprintf(cBuffer,"dwTotalFrames        : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	QW2Str(avih.dwSuggestedBufferSize,cQW,10);
	sprintf(cBuffer,"dwSuggestedBufferSize: %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	QW2Str(avih.dwStreams,cQW,10);
	sprintf(cBuffer,"dwStreams            : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	wsprintf(cBuffer,"dwFlags              : 0x%08X",avih.dwFlags);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	QW2Str(avih.dwWidth,cQW,10);
	sprintf(cBuffer,"dwWidth              : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	QW2Str(avih.dwHeight,cQW,10);
	wsprintf(cBuffer,"dwHeight             : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	QW2Str(avih.dwInitialFrames,cQW,10);
	wsprintf(cBuffer,"dwInitialFrames      : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	QW2Str(avih.dwPaddingGranularity,cQW,10);
	wsprintf(cBuffer,"dwPaddingGranularity : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);
}

void /*CRIFFChunkTreeDlg::*/ParseSTRH(HWND hTree, STREAM* source,
									  HTREEITEM hParent, CHUNKHEADER ch)
{
	union
	{
		AVIStreamHeader		strh;
		char				cDummy[256];
	};
	TVINSERTSTRUCT	tvi;
	char			cBuffer[200];
	char			cQW[20];

	tvi.hParent=hParent;
	tvi.hInsertAfter=TVI_LAST;
	tvi.item.mask=TVIF_TEXT;
	tvi.item.pszText=cBuffer;
	ZeroMemory(cDummy,sizeof(cDummy));
	source->Read(&strh,ch.dwLength);

	sprintf(cBuffer,"fccType              :             %c%c%c%c",strh.fccType&0xFF,(strh.fccType>>8)&0xFF,
		(strh.fccType>>16)&0xFF,(strh.fccType>>24)&0xFF);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	sprintf(cBuffer,"fccHandler           :             %c%c%c%c",strh.fccHandler&0xFF,(strh.fccHandler>>8)&0xFF,
		(strh.fccHandler>>16)&0xFF,(strh.fccHandler>>24)&0xFF);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	sprintf(cBuffer,"dwFlags              :       0x%08X",strh.dwFlags);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	QW2Str(strh.dwInitialFrames,cQW,16);
	wsprintf(cBuffer,"dwInitialFrames      : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	QW2Str(strh.dwLength,cQW,16);
	sprintf(cBuffer,"dwLength             : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	QW2Str(strh.dwQuality,cQW,16);
	sprintf(cBuffer,"dwQuality            : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	QW2Str(strh.dwRate,cQW,16);
	sprintf(cBuffer,"dwRate               : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	QW2Str(strh.dwScale,cQW,16);
	sprintf(cBuffer,"dwScale              : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	QW2Str(strh.dwSampleSize,cQW,16);
	sprintf(cBuffer,"dwSampleSize         : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	QW2Str(strh.dwStart,cQW,16);
	sprintf(cBuffer,"dwStart              : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	QW2Str(strh.dwSuggestedBufferSize,cQW,16);
	sprintf(cBuffer,"dwSuggestedBufferSize: %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	QW2Str(strh.wLanguage,cQW,16);
	sprintf(cBuffer,"wLanguage            : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	QW2Str(strh.wPriority,cQW,16);
	sprintf(cBuffer,"wPriority            : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	sprintf(cBuffer,"rcFrame              : %d,%d - %d,%d",*(WORD*)&strh.rcFrame,
		*((WORD*)&strh.rcFrame+1),*((WORD*)&strh.rcFrame+2),*((WORD*)&strh.rcFrame+3));
	tvi.item.cchTextMax=lstrlen(cBuffer);

	InsertItem(hTree, tvi);
}

void /*CRIFFChunkTreeDlg::*/ParseINDX(HWND hTree, STREAM* source,
									  HTREEITEM hParent,CHUNKHEADER ch)
{
	union
	{
		AVISUPERINDEX		asi;
		char				cDummy[65536];
	};
	TVINSERTSTRUCT	tvi;
	char			cBuffer[200];
	int				i;
	char			cQW[20];

	tvi.hParent=hParent;
	tvi.hInsertAfter=TVI_LAST;
	tvi.item.mask=TVIF_TEXT;
	tvi.item.pszText=cBuffer;
	ZeroMemory(cDummy,sizeof(cDummy));
	source->Read(&asi.wLongsPerEntry,ch.dwLength);

	sprintf(cBuffer,"fcc           : %c%c%c%c",asi.dwChunkId&0xFF,(asi.dwChunkId>>8)&0xFF,
		(asi.dwChunkId>>16)&0xFF,(asi.dwChunkId>>24)&0xFF);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	sprintf(cBuffer,"nEntriesInUse : %4d",asi.nEntriesInUse);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	sprintf(cBuffer,"wLongsPerEntry: %4d",asi.wLongsPerEntry);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	sprintf(cBuffer,"bIndexType    : %4d",asi.bIndexType);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	sprintf(cBuffer,"bIndexSubType : %4d",asi.bIndexSubType);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);

	for (i=0;i<(int)asi.nEntriesInUse;i++)
	{
		sprintf(cBuffer,"aIndex[%d]",i);
		tvi.item.cchTextMax=lstrlen(cBuffer);
		tvi.hParent=hParent;
		tvi.hParent=InsertItem(hTree, tvi);

		QW2Str(asi.aIndex[i].qwOffset,cQW,16);
		sprintf(cBuffer,"qwOffset    : %s",cQW);
		tvi.item.cchTextMax=lstrlen(cBuffer);
		InsertItem(hTree, tvi);

		QW2Str(asi.aIndex[i].dwSize,cQW,16);
		sprintf(cBuffer,"dwSize      : %s",cQW);
		tvi.item.cchTextMax=lstrlen(cBuffer);
		InsertItem(hTree, tvi);

		QW2Str(asi.aIndex[i].dwDuration,cQW,16);
		sprintf(cBuffer,"dwDuration  : %s",cQW);
		tvi.item.cchTextMax=lstrlen(cBuffer);
		InsertItem(hTree, tvi);
	}

}

void /*CRIFFChunkTreeDlg::*/ParseDMLH(HWND hTree, STREAM* source,
									  HTREEITEM hParent,CHUNKHEADER ch)
{
	union
	{
		ODMLExtendedAVIHeader	dmlh;
		char				cDummy[256];
	};
	TVINSERTSTRUCT	tvi;
	char			cBuffer[200];
	char			cQW[20];

	tvi.hParent=hParent;
	tvi.hInsertAfter=TVI_LAST;
	tvi.item.mask=TVIF_TEXT;
	tvi.item.pszText=cBuffer;
	ZeroMemory(cDummy,sizeof(cDummy));
	source->Read(&dmlh,ch.dwLength);

	QW2Str(dmlh.dwTotalFrames,cQW,11);
	sprintf(cBuffer,"dwTotalFrames : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	InsertItem(hTree, tvi);
}

const int CHTYPE_STRING = 0x01;

void InsertChunk(HWND hTree, STREAM* source,
	HTREEITEM hParent,CHUNKHEADER ch, int iType)
{
	TVINSERTSTRUCT	tvi;
	char			cBuffer[200];
	HTREEITEM		hItem;
	__int64			qwPos;
	char			cQW[30];
	char			cOfs[30];
	char			cFourCC[5];
	char			cItem[256];

	memcpy(cFourCC,&ch.dwFourCC,4);
	cFourCC[4]=0;
	QW2Str(ch.dwLength,cQW,21);
	QW2Str(source->GetPos()-8,cOfs,14);
	qwPos=source->GetPos();

	RIFFTREEITEM_DATA* data = new RIFFTREEITEM_DATA;
	data->pos = qwPos - 8;

	if (iType == CHTYPE_STRING) {
		ZeroMemory(cItem, sizeof(cItem));
		source->Read(cItem, ch.dwLength);
		sprintf(cBuffer,"%s %s %s (%s)",cFourCC,cQW,cOfs,cItem);
	} else {
		sprintf(cBuffer,"%s %s %s ",cFourCC,cQW,cOfs);
	}
	tvi.hParent=hParent;
	tvi.hInsertAfter=TVI_LAST;
	tvi.item.mask=TVIF_TEXT | TVIF_PARAM;
	tvi.item.pszText=cBuffer;
	tvi.item.cchTextMax=1+lstrlen(cBuffer);
	tvi.item.lParam = (LPARAM)data;
	hItem=InsertItem(hTree, tvi);
	


	if (ch.dwFourCC==MakeFourCC("avih")) {
		ParseAVIH(hTree, source, hItem,ch);
	} else
	if (ch.dwFourCC==MakeFourCC("strh")) {
		ParseSTRH(hTree, source,hItem,ch);
	} else
	if (ch.dwFourCC==MakeFourCC("indx")) {
		ParseINDX(hTree, source,hItem,ch);
	} else
	if (ch.dwFourCC==MakeFourCC("dmlh")) {
		ParseDMLH(hTree, source,hItem,ch);
	}
  /*  if ((qwPos>>20)!=dwMBNbr) {
		dwMBNbr=(DWORD)(qwPos>>20);
		GetParent()->SendMessage(WM_COMMAND,IDM_BUILDRIFFSTATE,dwMBNbr);
	};*/
    source->Seek(qwPos);

}

HTREEITEM InsertItem(HWND hTree, HTREEITEM hParent, char* cText, LPARAM data)
{
	EnterCriticalSection(&cs);

	TVINSERTSTRUCT tvi;
	tvi.hParent=hParent;
	tvi.hInsertAfter=TVI_LAST;
	tvi.item.mask=TVIF_TEXT | TVIF_PARAM;
	tvi.item.pszText=cText;
	tvi.item.cchTextMax=strlen(cText);
	tvi.item.lParam = data;

	HTREEITEM hItem = TreeView_InsertItem(hTree, &tvi);

	LeaveCriticalSection(&cs);

	return hItem;
}

HTREEITEM InsertItem(HWND hTree, TVINSERTSTRUCT &tvi)
{
	HTREEITEM hItem = TreeView_InsertItem(hTree, &tvi);
	return hItem;
}

typedef struct {
	CRIFFChunkTreeDlg* dlg;
	HWND hTree;
	HTREEITEM hParent;
	LISTHEADER listhdr;
	STREAM* source;
	__int64 list_offset;
	HANDLE hSemaphore;
} INSERTLIST_THREAD_DATA;

void StartInsertListThread(CRIFFChunkTreeDlg* dlg, HTREEITEM hParent,
						   LISTHEADER* lhdr, __int64 offset, HANDLE hSemaphore);



DWORD WINAPI InsertListThread(void* pData)
{
	INSERTLIST_THREAD_DATA* itd = (INSERTLIST_THREAD_DATA*)pData;
	HTREEITEM hParent = itd->hParent;
	HWND hTree = itd->hTree;
	LISTHEADER plh = itd->listhdr;
	CRIFFChunkTreeDlg* dlg = itd->dlg;
	STREAM* source = itd->source;

	source->Seek(itd->list_offset + 12);
	iChunkTree_ThreadCount++;

	union
	{
		CHUNKHEADER	ch;
		LISTHEADER	lh;
	};
	DWORD	dwRelPos=0;
	TVINSERTSTRUCT	tvi;
	__int64		qwPos;
	char			cBuffer[200];
	char			cQW[32];
	char			cOfs[32];
	HTREEITEM		hItem;
	DWORD			dwCount;

	memcpy(cBuffer,&plh,4);
	cBuffer[4]=32;
	memcpy(&(cBuffer[5]),&plh.dwFourCC,4);
	cBuffer[9]=32;
	QW2Str(plh.dwLength,cQW,16);
	QW2Str(itd->list_offset ,cOfs,14);

	sprintf(&(cBuffer[10]),"%s %s",cQW, cOfs);

	tvi.hParent=hParent;
	tvi.hInsertAfter = TVI_LAST;
	tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
	tvi.item.pszText = cBuffer;
	tvi.item.cchTextMax = lstrlen(cBuffer);

	RIFFTREEITEM_DATA* data = new RIFFTREEITEM_DATA;
	data->pos = itd->list_offset;
	tvi.item.lParam = (LPARAM)data;

	hItem=InsertItem(hTree, tvi);
	dwCount = 0;

	EnterCriticalSection(&cs);

	if (itd->hSemaphore)
		ReleaseSemaphore(itd->hSemaphore, 1, NULL);

	if (plh.dwLength>=8) 
	while (dwRelPos<plh.dwLength-8 && !bChunkTree_stop && plh.dwLength < 0x80000000) {
		source->Seek(itd->list_offset + 12 + dwRelPos);
		source->Read(&ch,8);
		if ((ch.dwFourCC==MakeFourCC("LIST"))||(ch.dwFourCC==MakeFourCC("RIFF"))) {
			source->Read(&lh.dwFourCC,4);

			qwPos=source->GetPos();
//		    InsertList(hItem,lh);
			/*StartInsertListThread(dlg, hItem, &lh, qwPos - 12);*/

			INSERTLIST_THREAD_DATA* _itd = new INSERTLIST_THREAD_DATA;
			memcpy(_itd, itd, sizeof(*itd));
			_itd->list_offset = qwPos - 12;
			_itd->listhdr = lh;
			_itd->hParent = hItem;
			_itd->hSemaphore = NULL;
			InsertListThread(_itd);

//			source->Seek(qwPos+lh.dwLength-4);
		} else if (plh.dwLength) {
			dwCount++;
			if (dwCount==2000) {
				memcpy(cBuffer,"REST",4);
				cBuffer[4]=32;
				memcpy(&(cBuffer[5]),"<  >",4);
				cBuffer[9]=32;
				QW2Str(plh.dwLength,cQW,16);
				sprintf(&(cBuffer[10]),"%s",cQW);

				hItem=InsertItem(hTree, hItem, cBuffer); 
				dwCount=0;
			}
			if (plh.dwFourCC == MakeFourCC("INFO")) {
				InsertChunk(hTree, source, hItem, ch, CHTYPE_STRING);
			} else {
				InsertChunk(hTree, source, hItem, ch, 0);
			}
		} else {
			delete pData;
			iChunkTree_ThreadCount--;
			LeaveCriticalSection(&cs);
			return 1;
		}
		dwRelPos+=ch.dwLength+8+(ch.dwLength%2);
	}

	LeaveCriticalSection(&cs);
	iChunkTree_ThreadCount--;
	delete pData;

	return 1;
}

void StartInsertListThread(CRIFFChunkTreeDlg* dlg, HTREEITEM hParent,
						   LISTHEADER* lhdr, __int64 offset, HANDLE hSemaphore)
{
	__int64 qwPos = offset;

	INSERTLIST_THREAD_DATA* itd = new INSERTLIST_THREAD_DATA;
	itd->dlg = dlg;
	itd->hParent = hParent;
	itd->list_offset = offset;
	itd->listhdr = *lhdr;
	itd->source = dlg->GetSource();
	itd->hTree = dlg->m_Tree.m_hWnd;
	itd->hSemaphore = hSemaphore;

	DWORD dwID;
	CreateThread(NULL, 1<<20, InsertListThread, itd, NULL, &dwID);

	itd->source->Seek(offset + lhdr->dwLength + 8);
}

typedef struct
{
	CRIFFChunkTreeDlg* dlg;
	STREAM* source;
} MAIN_THREAD_DATA_STRUCT;

DWORD WINAPI MainThread(void* pData)
{
	MAIN_THREAD_DATA_STRUCT* mtds = (MAIN_THREAD_DATA_STRUCT*)pData;
	CRIFFChunkTreeDlg* dlg = mtds->dlg;
	STREAM* source = mtds->source;

	__int64 qwPos=source->GetPos();
	dwMBNbr=0;
	LISTHEADER lh;

	source->Seek(0);
	while (!source->IsEndOfStream()) {
		source->Read(&lh,12);
		HANDLE h = CreateSemaphore(NULL, 0, 1, NULL);
		StartInsertListThread(dlg, NULL, &lh, source->GetPos() - 12, h);
		WaitForSingleObject(h, INFINITE);
        CloseHandle(h);
	}

	source->Seek(qwPos);

	delete mtds;
	return 1;
}



BOOL CRIFFChunkTreeDlg::OnInitDialog() 
{
//	InitializeCriticalSection(&critical_section);
	CResizeableDialog::OnInitDialog();

	InitializeCriticalSection(&cs);
	bChunkTree_stop = false;
//	__int64		qwPos;
//	LISTHEADER		lh;
	
	SendDlgItemMessage(IDOK,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_GEN_OK));
	SendDlgItemMessage(IDCANCEL,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_GEN_CANCEL));
	SendDlgItemMessage(IDC_WAIT_FOR_COMPLETE_TREE, WM_SETTEXT, NULL,
		(LPARAM)LoadString(STR_RIFFDLG_FULL));

	DWORD dwID;
	MAIN_THREAD_DATA_STRUCT* mtds = new MAIN_THREAD_DATA_STRUCT;
	mtds->dlg = this;
	mtds->source = source;
	CreateThread(NULL, 1<<20, MainThread, mtds, NULL, &dwID);


	AttachWindow(*GetDlgItem(IDOK), ATTB_RIGHT, *this, -12);
	AttachWindow(*GetDlgItem(IDC_SAVE), ATTB_RIGHT, *GetDlgItem(IDOK));
	AttachWindow(*GetDlgItem(IDC_SAVE), ATTB_TOP, *GetDlgItem(IDOK), ATTB_BOTTOM, 2);
	AttachWindow(m_WaitButton, ATTB_TOP, *GetDlgItem(IDC_SAVE), ATTB_BOTTOM, 2);
	AttachWindow(m_WaitButton, ATTB_RIGHT, *GetDlgItem(IDOK));
	int border_x, border_y;
	GetBorder(border_x, border_y);

	AttachWindow(m_HexView, ATTB_BOTTOM, *this, -border_y);


	AttachWindow(m_Tree, ATTB_LEFT, *this, 12);
	AttachWindow(m_Tree, ATTB_BOTTOM, m_HexView, ATTB_TOP, -1);
	AttachWindow(m_Tree, ATTB_TOP, *this, border_y + 12);
	AttachWindow(m_Tree, ATTB_RIGHT, *GetDlgItem(IDOK), ATTB_LEFT, -12);

	AttachWindow(m_HexView, ATTB_RIGHT, m_Tree);
	AttachWindow(m_HexView, ATTB_LEFT, m_Tree);

	AttachWindow(*GetDlgItem(IDOK), ATTB_TOP, m_Tree);

	m_HexView.SetRange(16*256);
	m_HexView.SetDataSource(source);
	m_HexView.SetNewStartPos(0);
	m_HexView.SetMode(HWLB_MODE_RIFF);

/*	RECT r;
	GetWindowRect(&r);
	PostMessage(WM_SIZE, 0, (r.right-r.left) | ((r.bottom-r.top)<<16));
*/

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CRIFFChunkTreeDlg::SetSource(STREAM* lpSource)
{
	source=lpSource;
	source->Enable(CACHE_THREADSAFE);
}

STREAM* CRIFFChunkTreeDlg::GetSource()
{
	return source;
}

void CRIFFChunkTreeDlg::RenderItem(STREAM* file, HTREEITEM _hItem, int iLevel)
{
	HTREEITEM  hItem = _hItem;

	std::string newLine;
	newLine.push_back(0x0D);
	newLine.push_back(0x0A);

	std::string padding(2 * iLevel, ' ');
	const char* ptrPadding = padding.c_str();

	char textBuffer[1024];

	while (hItem) {
		TVITEMEX item;
		memset(textBuffer, 0, 1024);
		memset(&item, 0, sizeof(item));
		item.hItem = hItem;
		item.cchTextMax = 1023;
		item.pszText = textBuffer;
		item.mask = TVIF_TEXT | TVIF_HANDLE;
		TreeView_GetItem(m_Tree, &item);

		file->Write(const_cast<char*>(ptrPadding), padding.size());
		file->Write(const_cast<char*>(item.pszText), strlen(item.pszText));
		file->Write(const_cast<char*>(newLine.c_str()), 2);
		RenderItem(file, m_Tree.GetChildItem(hItem), iLevel+1);
		hItem = m_Tree.GetNextSiblingItem(hItem);
	}
}

void CRIFFChunkTreeDlg::OnSave() 
{
	OPENFILENAME ofn;

	PrepareSimpleDialog(&ofn, *this, "*.txt");
	ofn.lpstrFilter = "Text files (*.txt)|*.txt||";
	ofn.Flags |= OFN_OVERWRITEPROMPT;
	if (!GetOpenSaveFileNameUTF8(&ofn, false)) 
		return;

	CFileStream* destFile = new CFileStream();
	if (STREAM_OK == destFile->Open(ofn.lpstrFile, StreamMode::Write))
	{
		CACHE* cache = new CACHE(4, 1<<18);
		cache->Open(destFile, CACHE_OPEN_WRITE);
		cache->Write(cUTF8Hdr, 3);

		RenderItem(cache, m_Tree.GetRootItem(), 0);

		cache->Close();
		delete cache;
		destFile->Close();
		delete destFile;
	}
	else
	{
		MessageBoxHelper::CouldNotOpenFileForWrite(ofn.lpstrFile);
	}
}

void CleanTree(CTreeCtrl* tree, HTREEITEM hParent)
{
	HTREEITEM hChild;

	if (hParent)
		hChild = tree->GetChildItem(hParent);
	else
		hChild = tree->GetRootItem();

	while (hChild) {
		CleanTree(tree, hChild);
		hChild = tree->GetNextSiblingItem(hChild);
	}

	if (hParent) {
		TVITEM item;
		item.mask = TVIF_PARAM | TVIF_HANDLE;
		item.hItem = hParent;
		tree->GetItem(&item);
		void* p = (void*)item.lParam;
		if (p)
			delete p;
	}
}

void CRIFFChunkTreeDlg::OnDestroy()
{
	CleanTree(&m_Tree, NULL);

	source->Disable(CACHE_THREADSAFE);
//	DeleteCriticalSection(&critical_section);
	DeleteCriticalSection(&cs);

	CResizeableDialog::OnDestroy();

	// TODO: Fügen Sie hier Ihren Meldungsbehandlungscode ein.
}

void CRIFFChunkTreeDlg::OnBnClickedOk()
{
	// TODO: Fügen Sie hier Ihren Kontrollbehandlungscode für die Benachrichtigung ein.

	bChunkTree_stop = true;

	if (iChunkTree_ThreadCount > 0) {
		PostMessage(WM_COMMAND, IDOK, 0);
		Sleep(100);
		return;
	}

	CResizeableDialog::OnOK();
}

void CRIFFChunkTreeDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: Fügen Sie hier Ihren Meldungsbehandlungscode ein, und/oder benutzen Sie den Standard.
	if (nID == SC_CLOSE)
		PostMessage(WM_COMMAND, IDOK, 0);
	else
		CResizeableDialog::OnSysCommand(nID, lParam);
}

void CRIFFChunkTreeDlg::OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Fügen Sie hier Ihren Kontrollbehandlungscode für die Benachrichtigung ein.

	TVITEM item;
	item.mask = TVIF_PARAM | TVIF_HANDLE;
	item.hItem = pNMTreeView->itemNew.hItem;
	m_Tree.GetItem(&item);

	void* p = (void*)item.lParam;

	if (p) {
		RIFFTREEITEM_DATA* d = (RIFFTREEITEM_DATA*)p;
		m_HexView.SetNewStartPos(d->pos);
	}

	*pResult = 0;
}

void CRIFFChunkTreeDlg::OnBnClickedWait()
{
}

typedef struct
{
	HWND hwnd;
	DWORD idc;
} RETEST_WAIT_STATE_THREAD_DATA;

DWORD WINAPI WaitButtonTest_thread(void* pData)
{
	RETEST_WAIT_STATE_THREAD_DATA* wst = (RETEST_WAIT_STATE_THREAD_DATA*)pData;

	Sleep(250);
	PostMessage(wst->hwnd, WM_COMMAND, wst->idc, 0);

	delete pData;

	return 0;
}

void CRIFFChunkTreeDlg::OnBnClickedWaitForCompleteTree()
{
	// TODO: Fügen Sie hier Ihren Kontrollbehandlungscode für die Benachrichtigung ein.
	// TODO: Fügen Sie hier Ihren Kontrollbehandlungscode für die Benachrichtigung ein.
	m_WaitButton.EnableWindow(0);

	if (iChunkTree_ThreadCount == 0) {
		m_WaitButton.EnableWindow(1);
	} else {
		RETEST_WAIT_STATE_THREAD_DATA* wst = new RETEST_WAIT_STATE_THREAD_DATA;
		wst->hwnd = *this;
		wst->idc = IDC_WAIT_FOR_COMPLETE_TREE;
		DWORD dwID;
		CreateThread(NULL, 0, WaitButtonTest_thread, wst, NULL, &dwID);

/* the following code b0rks:
			Sleep(250);
			PostMessage(WM_COMMAND, IDC_WAIT_FOR_COMPLETE_TREE, 0);
*/
	}
}

void CRIFFChunkTreeDlg::OnLbnSelchangeHexviewListbox2()
{
	// TODO: Fügen Sie hier Ihren Kontrollbehandlungscode für die Benachrichtigung ein.
}
