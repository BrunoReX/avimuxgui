// RIFFChunkTreeDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "RIFFChunkTreeDlg.h"
#include "windows.h"
#include "FormatText.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CRIFFChunkTreeDlg 


CRIFFChunkTreeDlg::CRIFFChunkTreeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRIFFChunkTreeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRIFFChunkTreeDlg)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}


void CRIFFChunkTreeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRIFFChunkTreeDlg)
	DDX_Control(pDX, IDC_TREE1, m_Tree);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRIFFChunkTreeDlg, CDialog)
	//{{AFX_MSG_MAP(CRIFFChunkTreeDlg)
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CRIFFChunkTreeDlg 



DWORD	dwMBNbr;

void CRIFFChunkTreeDlg::ParseAVIH(HTREEITEM hParent,CHUNKHEADER ch)
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
	wsprintf(cBuffer,"dwMicroSecPerFrame   : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	QW2Str(avih.dwMaxBytesPerSec,cQW,10);
	wsprintf(cBuffer,"dwMaxBytesPerSec     : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	QW2Str(avih.dwTotalFrames,cQW,10);
	wsprintf(cBuffer,"dwTotalFrames        : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	QW2Str(avih.dwSuggestedBufferSize,cQW,10);
	wsprintf(cBuffer,"dwSuggestedBufferSize: %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	QW2Str(avih.dwStreams,cQW,10);
	wsprintf(cBuffer,"dwStreams            : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	wsprintf(cBuffer,"dwFlags              : 0x%08X",avih.dwFlags);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	QW2Str(avih.dwWidth,cQW,10);
	wsprintf(cBuffer,"dwWidth              : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	QW2Str(avih.dwHeight,cQW,10);
	wsprintf(cBuffer,"dwHeight             : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	QW2Str(avih.dwInitialFrames,cQW,10);
	wsprintf(cBuffer,"dwInitialFrames      : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	QW2Str(avih.dwPaddingGranularity,cQW,10);
	wsprintf(cBuffer,"dwPaddingGranularity : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);
}

void CRIFFChunkTreeDlg::ParseSTRH(HTREEITEM hParent,CHUNKHEADER ch)
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

	wsprintf(cBuffer,"fccType              :             %c%c%c%c",strh.fccType&0xFF,(strh.fccType>>8)&0xFF,
		(strh.fccType>>16)&0xFF,(strh.fccType>>24)&0xFF);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	wsprintf(cBuffer,"fccHandler           :             %c%c%c%c",strh.fccHandler&0xFF,(strh.fccHandler>>8)&0xFF,
		(strh.fccHandler>>16)&0xFF,(strh.fccHandler>>24)&0xFF);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	wsprintf(cBuffer,"dwFlags              :       0x%08X",strh.dwFlags);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	QW2Str(strh.dwInitialFrames,cQW,16);
	wsprintf(cBuffer,"dwInitialFrames      : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	QW2Str(strh.dwLength,cQW,16);
	wsprintf(cBuffer,"dwLength             : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	QW2Str(strh.dwQuality,cQW,16);
	wsprintf(cBuffer,"dwQuality            : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	QW2Str(strh.dwRate,cQW,16);
	wsprintf(cBuffer,"dwRate               : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	QW2Str(strh.dwScale,cQW,16);
	wsprintf(cBuffer,"dwScale              : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	QW2Str(strh.dwSampleSize,cQW,16);
	wsprintf(cBuffer,"dwSampleSize         : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	QW2Str(strh.dwStart,cQW,16);
	wsprintf(cBuffer,"dwStart              : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	QW2Str(strh.dwSuggestedBufferSize,cQW,16);
	wsprintf(cBuffer,"dwSuggestedBufferSize: %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	QW2Str(strh.wLanguage,cQW,16);
	wsprintf(cBuffer,"wLanguage            : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	QW2Str(strh.wPriority,cQW,16);
	wsprintf(cBuffer,"wPriority            : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	wsprintf(cBuffer,"rcFrame              : %d,%d - %d,%d",*(WORD*)&strh.rcFrame,
		*((WORD*)&strh.rcFrame+1),*((WORD*)&strh.rcFrame+2),*((WORD*)&strh.rcFrame+3));
	tvi.item.cchTextMax=lstrlen(cBuffer);

	m_Tree.InsertItem(&tvi);
}

void CRIFFChunkTreeDlg::ParseINDX(HTREEITEM hParent,CHUNKHEADER ch)
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

	wsprintf(cBuffer,"fcc           : %c%c%c%c",asi.dwChunkId&0xFF,(asi.dwChunkId>>8)&0xFF,
		(asi.dwChunkId>>16)&0xFF,(asi.dwChunkId>>24)&0xFF);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	wsprintf(cBuffer,"nEntriesInUse : %4d",asi.nEntriesInUse);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	wsprintf(cBuffer,"wLongsPerEntry: %4d",asi.wLongsPerEntry);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	wsprintf(cBuffer,"bIndexType    : %4d",asi.bIndexType);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	wsprintf(cBuffer,"bIndexSubType : %4d",asi.bIndexSubType);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);

	for (i=0;i<(int)asi.nEntriesInUse;i++)
	{
		wsprintf(cBuffer,"aIndex[%d]",i);
		tvi.item.cchTextMax=lstrlen(cBuffer);
		tvi.hParent=hParent;
		tvi.hParent=m_Tree.InsertItem(&tvi);

		QW2Str(asi.aIndex[i].qwOffset,cQW,16);
		wsprintf(cBuffer,"qwOffset    : %s",cQW);
		tvi.item.cchTextMax=lstrlen(cBuffer);
		m_Tree.InsertItem(&tvi);

		QW2Str(asi.aIndex[i].dwSize,cQW,16);
		wsprintf(cBuffer,"dwSize      : %s",cQW);
		tvi.item.cchTextMax=lstrlen(cBuffer);
		m_Tree.InsertItem(&tvi);

		QW2Str(asi.aIndex[i].dwDuration,cQW,16);
		wsprintf(cBuffer,"dwDuration  : %s",cQW);
		tvi.item.cchTextMax=lstrlen(cBuffer);
		m_Tree.InsertItem(&tvi);
	}

}

void CRIFFChunkTreeDlg::ParseDMLH(HTREEITEM hParent,CHUNKHEADER ch)
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
	wsprintf(cBuffer,"dwTotalFrames : %s",cQW);
	tvi.item.cchTextMax=lstrlen(cBuffer);
	m_Tree.InsertItem(&tvi);
}

const int CHTYPE_STRING = 0x01;

void CRIFFChunkTreeDlg::InsertChunk(HTREEITEM hParent,CHUNKHEADER ch, int iType)
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
	if (iType == CHTYPE_STRING) {
		ZeroMemory(cItem, sizeof(cItem));
		GetSource()->Read(cItem, ch.dwLength);
		wsprintf(cBuffer,"%s %s %s (%s)",cFourCC,cQW,cOfs,cItem);
	} else {
		wsprintf(cBuffer,"%s %s %s ",cFourCC,cQW,cOfs);
	}
	tvi.hParent=hParent;
	tvi.hInsertAfter=TVI_LAST;
	tvi.item.mask=TVIF_TEXT;
	tvi.item.pszText=cBuffer;
	tvi.item.cchTextMax=1+lstrlen(cBuffer);

	hItem=m_Tree.InsertItem(&tvi);


	if (ch.dwFourCC==MakeFourCC("avih"))
	{
		ParseAVIH(hItem,ch);
	} else
	if (ch.dwFourCC==MakeFourCC("strh"))
	{
		ParseSTRH(hItem,ch);
	} else
	if (ch.dwFourCC==MakeFourCC("indx"))
	{
		ParseINDX(hItem,ch);
	} else
	if (ch.dwFourCC==MakeFourCC("dmlh"))
	{
		ParseDMLH(hItem,ch);
	}
    if ((qwPos>>20)!=dwMBNbr)
	{
		dwMBNbr=(DWORD)(qwPos>>20);
		GetParent()->SendMessage(WM_COMMAND,IDM_BUILDRIFFSTATE,dwMBNbr);
	};
    source->Seek(qwPos);

}

void CRIFFChunkTreeDlg::InsertList(HTREEITEM hParent,LISTHEADER plh)
{
	union
	{
		CHUNKHEADER	ch;
		LISTHEADER	lh;
	};
	DWORD	dwRelPos=0;
	TVINSERTSTRUCT	tvi;
	__int64		qwPos;
	char			cBuffer[200];
	char			cQW[20];
	HTREEITEM		hItem;
	DWORD			dwCount;

	memcpy(cBuffer,&plh,4);
	cBuffer[4]=32;
	memcpy(&(cBuffer[5]),&plh.dwFourCC,4);
	cBuffer[9]=32;
	QW2Str(plh.dwLength,cQW,16);
	wsprintf(&(cBuffer[10]),"%s",cQW);

	tvi.hParent=hParent;
	tvi.hInsertAfter=TVI_LAST;
	tvi.item.mask=TVIF_TEXT;
	tvi.item.pszText=cBuffer;
	tvi.item.cchTextMax=lstrlen(cBuffer);

	hItem=m_Tree.InsertItem(&tvi);
	dwCount = 0;

	if (plh.dwLength>=8) while (dwRelPos<plh.dwLength-8)
	{
		source->Read(&ch,8);
		if ((ch.dwFourCC==MakeFourCC("LIST"))||(ch.dwFourCC==MakeFourCC("RIFF")))
		{
			source->Read(&lh.dwFourCC,4);

			qwPos=source->GetPos();
		    InsertList(hItem,lh);
			source->Seek(qwPos+lh.dwLength-4);
		}
		else if (plh.dwLength)
		{
			dwCount++;
			if (dwCount==2000) {
				memcpy(cBuffer,"REST",4);
				cBuffer[4]=32;
				memcpy(&(cBuffer[5]),"<  >",4);
				cBuffer[9]=32;
				QW2Str(plh.dwLength,cQW,16);
				wsprintf(&(cBuffer[10]),"%s",cQW);

				tvi.hParent=hItem;
				tvi.hInsertAfter=TVI_LAST;
				tvi.item.mask=TVIF_TEXT;
				tvi.item.pszText=cBuffer;
				tvi.item.cchTextMax=lstrlen(cBuffer);

				hItem=m_Tree.InsertItem(&tvi);
				dwCount=0;
			}
			if (plh.dwFourCC == MakeFourCC("INFO")) {
				InsertChunk(hItem,ch, CHTYPE_STRING);
			} else {
				InsertChunk(hItem,ch);
			}
			source->Seek(source->GetPos()+ch.dwLength+(ch.dwLength%2));
		} else return;
		dwRelPos+=ch.dwLength+8+(ch.dwLength%2);
	}
}

BOOL CRIFFChunkTreeDlg::OnInitDialog() 
{
	__int64		qwPos;
	LISTHEADER		lh;
	
	CDialog::OnInitDialog();
	
	SendDlgItemMessage(IDOK,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_GEN_OK));
	SendDlgItemMessage(IDCANCEL,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_GEN_CANCEL));

	qwPos=source->GetPos();
	dwMBNbr=0;
	source->Seek(0);
	while (!source->IsEndOfStream())
	{
		source->Read(&lh,12);
		InsertList(NULL,lh);
	}

	source->Seek(qwPos);
	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CRIFFChunkTreeDlg::SetSource(STREAM* lpSource)
{
	source=lpSource;
}

STREAM* CRIFFChunkTreeDlg::GetSource()
{
	return source;
}

void CRIFFChunkTreeDlg::RenderItem(FILE* file, HTREEITEM _hItem, int iLevel)
{
	HTREEITEM  hItem = _hItem;
	

	while (hItem) {
		CString s = m_Tree.GetItemText(hItem);
		for (int j=0;j<2*iLevel;j++) fprintf(file, " ");
		fprintf(file, "%s%c%c", s, 13, 10);
		RenderItem(file, m_Tree.GetChildItem(hItem), iLevel+1);
		hItem = m_Tree.GetNextSiblingItem(hItem);
	}

}

void CRIFFChunkTreeDlg::OnSave() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	CFileDialog* dlg;
	
	dlg= new CFileDialog(false,"txt","",OFN_OVERWRITEPROMPT,"Text file (*.txt)|*.txt||",NULL);
	if (dlg->DoModal()==IDOK)
	{
		FILE* file = fopen(dlg->GetPathName().GetBuffer(1024), "wb");
		RenderItem(file, m_Tree.GetRootItem(), 0);
		fclose(file);
	}

}
