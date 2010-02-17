// SplitPointsDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "SplitPointsDlg.h"
#include "FormatText.h"
#include "Languages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSplitPointsDlg 


CSplitPointsDlg::CSplitPointsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSplitPointsDlg::IDD, pParent)
{
	lpqwDataIn=NULL;
	lpqwDataOut=NULL;
	lpVideoSource=NULL;
	points = NULL;
	EnableAutomation();

	//{{AFX_DATA_INIT(CSplitPointsDlg)
	//}}AFX_DATA_INIT
}

void CSplitPointsDlg::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CDialog::OnFinalRelease();
}

void CSplitPointsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSplitPointsDlg)
	DDX_Control(pDX, IDC_ESPLITPOINT, m_Splitpoint);
	DDX_Control(pDX, IDC_SPLITPOINTLIST, m_SplitPointList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSplitPointsDlg, CDialog)
	//{{AFX_MSG_MAP(CSplitPointsDlg)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CSplitPointsDlg, CDialog)
	//{{AFX_DISPATCH_MAP(CSplitPointsDlg)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_ISplitPointsDlg zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {7943B35E-59E0-467E-A949-A5A1A82B97E6}
static const IID IID_ISplitPointsDlg =
{ 0x7943b35e, 0x59e0, 0x467e, { 0xa9, 0x49, 0xa5, 0xa1, 0xa8, 0x2b, 0x97, 0xe6 } };

BEGIN_INTERFACE_MAP(CSplitPointsDlg, CDialog)
	INTERFACE_PART(CSplitPointsDlg, IID_ISplitPointsDlg, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CSplitPointsDlg 

void SplitPointDescriptor2String(SPLIT_POINT_DESCRIPTOR* d,char* pDest);

int CSplitPointsDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Speziellen Erstellungscode hier einfügen

	return 0;
}

void BubbleSort (__int64* lpqwData)
{
	__int64 iCount=*lpqwData++;
	bool	bChanged=true;

	while (bChanged)
	{
		bChanged=false;
		for (int i=0;i<iCount-1;i++)
		{
			if (lpqwData[i]>lpqwData[i+1])
			{
				bChanged=true;
				__int64 qwTemp=lpqwData[i];
				lpqwData[i]=lpqwData[i+1];
				lpqwData[i+1]=qwTemp;
			}
		}
	}
}

DWORD CSplitPointsDlg::Load(CSplitPoints* p)
{
	if (!points) points = new CSplitPoints;
	p->Duplicate(points);

	return 0;
}

BOOL CSplitPointsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Zusätzliche Initialisierung hier einfügen
 
	m_SplitPointList.ResetContent();
	char	Buffer[50]; 

	if (points)
	{
		for (int i=0;i<points->GetCount();i++)
		{
			SplitPointDescriptor2String(points->At(i),Buffer);
			SendDlgItemMessage(IDC_SPLITPOINTLIST,LB_ADDSTRING,0,(LPARAM)Buffer);
		}
	}
	m_SplitPointList.SetVideoSource(lpVideoSource);
	
	CUTF8 title(LoadString(STR_SSP_TITLE));
	SetWindowText(title.TStr());

	SendDlgItemMessage(IDOK,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_GEN_OK));
	SendDlgItemMessage(IDCANCEL,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_GEN_CANCEL));

	m_Splitpoint.SetFocus();
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben

}

void CSplitPointsDlg::OnOK() 
{
	// TODO: Zusätzliche Prüfung hier einfügen

	CDialog::OnOK();
}

DWORD CSplitPointsDlg::GetData(CSplitPoints* p)
{
	points->Duplicate(p);
	return 0;
}

void CSplitPointsDlg::SetVideoSource(VIDEOSOURCE* lpSource)
{
	lpVideoSource=lpSource;
}

void SplitPointDescriptor2String(SPLIT_POINT_DESCRIPTOR* d,char* pDest)
{
	*pDest = 0;
	
	if (d->iFlags & SPD_BEGIN) {
		if (d->iFlags & SPD_BCHAP) {
			strcat(pDest, "chapter ");
			char cNbr[10];
			for (int i=0;i<d->aChapBegin->GetCount()-1;i++) {
				if (d->aChapBegin->At(i) != -2) {
					sprintf(cNbr,"%d.",d->aChapBegin->At(i)+1);
				} else {
					sprintf(cNbr,"%s", "*.");
				}
				strcat(pDest,cNbr);
			}
			int j = d->aChapBegin->At(d->aChapBegin->GetCount()-1);
			if (j>-2) {
				sprintf(cNbr,"%d",j+1);
			} else 
			if (j==-2) {
				sprintf(cNbr,"%c", '*');
			} else *cNbr=0;

			strcat(pDest,cNbr);
		} else 
		if (d->iFlags & SPD_BFRAME) {
			strcat(pDest, "frame ");
			char cNbr[10];
			sprintf(cNbr,"%d",d->iBegin);
			strcat(pDest,cNbr);
		} else {
			char c[20];
			Millisec2Str(d->iBegin / 1000000,c);
			strcat(pDest,c);
		}
	}
}

void CSplitPointsDlg::OnAdd() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	SPLIT_POINT_DESCRIPTOR* d = new SPLIT_POINT_DESCRIPTOR;

	CString c;
	m_Splitpoint.GetWindowText(c);

	ZeroMemory(d,sizeof(*d));
	if (String2SplitPointDescriptor(c.GetBuffer(255),d)!=-1) {
		points->Insert(d);
		char cLabel[128]; cLabel[0]=0;
		SplitPointDescriptor2String(d,cLabel);
		m_SplitPointList.SetItemData(m_SplitPointList.AddString(cLabel),(int)d);
		m_Splitpoint.SetWindowText("");
	}
}


CSplitPoints::CSplitPoints()
{
	points = NULL;
}

void CSplitPoints::Insert(SPLIT_POINT_DESCRIPTOR* p)
{
	if (!points) points = new CDynIntArray;
	points->Insert((int)p);
}

void CSplitPoints::Delete(int iIndex)
{
	delete (void*)points->At(iIndex);
	points->Delete(iIndex);
}

int CSplitPoints::GetCount()
{
	return (points)?points->GetCount():0;
}

void CSplitPoints::DeleteAll()
{
	for (int i=GetCount()-1;i>=0;i--) {
		delete (void*)points->At(i);
	}
	if (points) {
		points->DeleteAll();
		delete points;
		points = NULL;
	}
}

SPLIT_POINT_DESCRIPTOR* CSplitPoints::At(int iIndex)
{
	return (points)?(SPLIT_POINT_DESCRIPTOR*)points->At(iIndex):0;
}

void CSplitPoints::Duplicate(CSplitPoints* pDest)
{
	pDest->DeleteAll();

	for (int i=0;i<GetCount();i++) {
		SPLIT_POINT_DESCRIPTOR* p = new SPLIT_POINT_DESCRIPTOR;
		memcpy(p,At(i),sizeof(*p));
		pDest->Insert(p);
	}
}

void CSplitPointsDlg::OnDelete() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	int iIndex = m_SplitPointList.GetCurSel();

	if (iIndex!=LB_ERR) {
		m_SplitPointList.DeleteString(iIndex);
		points->Delete(iIndex);
	}

}
