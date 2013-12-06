// AddSplitPointDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "AddSplitPointDlg.h"
#include "Languages.h"

#include <string>
#include <sstream>

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
		// HINWEIS: Der Klassen-Assistent f�gt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}


void AddSplitPointDlg::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse l�scht das Objekt
	// automatisch. F�gen Sie zus�tzlichen Bereinigungscode f�r Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CDialog::OnFinalRelease();
}

void AddSplitPointDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AddSplitPointDlg)
		// HINWEIS: Der Klassen-Assistent f�gt hier DDX- und DDV-Aufrufe ein
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
		// HINWEIS - Der Klassen-Assistent f�gt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterst�tzung f�r IID_IAddSplitPointDlg zur Verf�gung, um typsicheres Binden
//  von VBA zu erm�glichen. Diese IID muss mit der GUID �bereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {0BC28AF1-0C0F-4091-B8B9-B2B827FAF7BA}
static const IID IID_IAddSplitPointDlg =
{ 0xbc28af1, 0xc0f, 0x4091, { 0xb8, 0xb9, 0xb2, 0xb8, 0x27, 0xfa, 0xf7, 0xba } };

BEGIN_INTERFACE_MAP(AddSplitPointDlg, CDialog)
	INTERFACE_PART(AddSplitPointDlg, IID_IAddSplitPointDlg, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen f�r Nachrichten AddSplitPointDlg 

int AddSplitPointDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Speziellen Erstellungscode hier einf�gen
	
	CheckDlgButton(IDC_ALLOWDELTA,BST_UNCHECKED);

	return 0;
}

void AddSplitPointDlg::OnCancelMode() 
{
	CDialog::OnCancelMode();
	
	// TODO: Code f�r die Behandlungsroutine f�r Nachrichten hier einf�gen
	
}

void AddSplitPointDlg::OnOK() 
{
	// TODO: Zus�tzliche Pr�fung hier einf�gen

	const int bufSize = 128;
	TCHAR temp[bufSize];
	memset(temp, 0, sizeof(temp));
	GetDlgItemText(IDC_NEWSPLITPOINT, temp, bufSize);

	Buffer = temp;
//    GetDlgItemText(IDC_NEWSPLITPOINT,Buffer,sizeof(Buffer));
	CDialog::OnOK();
}

DWORD AddSplitPointDlg::GetSplitPos()
{
	return _ttoi(Buffer.c_str());
}

BOOL AddSplitPointDlg::OnInitDialog() 
{
//	char	Buffer[100];

	CDialog::OnInitDialog();
	
	// TODO: Zus�tzliche Initialisierung hier einf�gen
	GetDlgItem(IDC_NEWSPLITPOINT)->SetFocus();
	if (dwSplitPos)
	{
		std::basic_stringstream<TCHAR> sstrSplitPos;
		sstrSplitPos << dwSplitPos;
		std::basic_string<TCHAR> strSplitPos = sstrSplitPos.str();

		SetDlgItemText(IDC_NEWSPLITPOINT, strSplitPos.c_str());
		//wsprintf(Buffer,"%d",dwSplitPos);
		//SetDlgItemText(IDC_NEWSPLITPOINT,Buffer);
	}

	SendDlgItemMessage(IDOK,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_GEN_OK));
	SendDlgItemMessage(IDCANCEL,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_GEN_CANCEL));
	SendDlgItemMessage(IDC_ASP_NEWPART,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_ASP_S_NEWPART));
	
	CUTF8 utf8Title(LoadString(STR_ASP_TITLE, LOADSTRING_UTF8), 
		CharacterEncoding::UTF8);

	SetWindowText(utf8Title.TStr());
	//SetWindowText(LoadString(STR_ASP_TITLE));

	return false;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zur�ckgeben
}

void AddSplitPointDlg::SetSplitPos(DWORD _dwSplitPos)
{
	dwSplitPos=_dwSplitPos;
}
