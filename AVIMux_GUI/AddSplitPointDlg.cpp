// AddSplitPointDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "AddSplitPointDlg.h"
#include "Languages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld AddSplitPointDlg 


AddSplitPointDlg::AddSplitPointDlg(CWnd* pParent /*=NULL*/)
	: CDialog(AddSplitPointDlg::IDD, pParent)
{
	EnableAutomation();

	//{{AFX_DATA_INIT(AddSplitPointDlg)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}


void AddSplitPointDlg::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CDialog::OnFinalRelease();
}

void AddSplitPointDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AddSplitPointDlg)
		// HINWEIS: Der Klassen-Assistent fügt hier DDX- und DDV-Aufrufe ein
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(AddSplitPointDlg, CDialog)
	//{{AFX_MSG_MAP(AddSplitPointDlg)
	ON_WM_CREATE()
	ON_WM_CANCELMODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(AddSplitPointDlg, CDialog)
	//{{AFX_DISPATCH_MAP(AddSplitPointDlg)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_IAddSplitPointDlg zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {0BC28AF1-0C0F-4091-B8B9-B2B827FAF7BA}
static const IID IID_IAddSplitPointDlg =
{ 0xbc28af1, 0xc0f, 0x4091, { 0xb8, 0xb9, 0xb2, 0xb8, 0x27, 0xfa, 0xf7, 0xba } };

BEGIN_INTERFACE_MAP(AddSplitPointDlg, CDialog)
	INTERFACE_PART(AddSplitPointDlg, IID_IAddSplitPointDlg, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten AddSplitPointDlg 

int AddSplitPointDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Speziellen Erstellungscode hier einfügen
	
	CheckDlgButton(IDC_ALLOWDELTA,BST_UNCHECKED);

	return 0;
}

void AddSplitPointDlg::OnCancelMode() 
{
	CDialog::OnCancelMode();
	
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen
	
}

void AddSplitPointDlg::OnOK() 
{
	// TODO: Zusätzliche Prüfung hier einfügen
    GetDlgItemText(IDC_NEWSPLITPOINT,Buffer,sizeof(Buffer));
	CDialog::OnOK();
}

DWORD AddSplitPointDlg::GetSplitPos()
{
	return atoi(Buffer);
}

BOOL AddSplitPointDlg::OnInitDialog() 
{
	char	Buffer[100];

	CDialog::OnInitDialog();
	
	// TODO: Zusätzliche Initialisierung hier einfügen
	GetDlgItem(IDC_NEWSPLITPOINT)->SetFocus();
	if (dwSplitPos)
	{
		wsprintf(Buffer,"%d",dwSplitPos);
		SetDlgItemText(IDC_NEWSPLITPOINT,Buffer);
	}

	SendDlgItemMessage(IDOK,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_GEN_OK));
	SendDlgItemMessage(IDCANCEL,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_GEN_CANCEL));
	SendDlgItemMessage(IDC_ASP_NEWPART,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_ASP_S_NEWPART));
	SetWindowText(LoadString(STR_ASP_TITLE));

	return false;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void AddSplitPointDlg::SetSplitPos(DWORD _dwSplitPos)
{
	dwSplitPos=_dwSplitPos;
}
