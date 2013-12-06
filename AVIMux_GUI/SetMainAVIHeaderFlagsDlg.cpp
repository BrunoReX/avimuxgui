// SetMainAVIHeaderFlagsDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "SetMainAVIHeaderFlagsDlg.h"
//#include "vfw.h"
#include "avifile.h"
#include "Languages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSetMainAVIHeaderFlagsDlg 


CSetMainAVIHeaderFlagsDlg::CSetMainAVIHeaderFlagsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetMainAVIHeaderFlagsDlg::IDD, pParent)
{
	EnableAutomation();
	bActive=false;

	//{{AFX_DATA_INIT(CSetMainAVIHeaderFlagsDlg)
		// HINWEIS: Der Klassen-Assistent f�gt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}


void CSetMainAVIHeaderFlagsDlg::OnFinalRelease()
{

	CDialog::OnFinalRelease();
}

void CSetMainAVIHeaderFlagsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetMainAVIHeaderFlagsDlg)
		// HINWEIS: Der Klassen-Assistent f�gt hier DDX- und DDV-Aufrufe ein
	//}}AFX_DATA_MAP
}

void CSetMainAVIHeaderFlagsDlg::SetData(DWORD dwNewFlags)
{
	dwData=dwNewFlags;
	if (bActive) RefreshCheckboxes();
}

void CSetMainAVIHeaderFlagsDlg::RefreshCheckboxes()
{
	CheckDlgButton(IDC_HASINDEX,(dwData&AVIF_HASINDEX)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(IDC_MUSTUSEINDEX,(dwData&AVIF_MUSTUSEINDEX)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(IDC_ISINTERLEAVED,(dwData&AVIF_ISINTERLEAVED)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(IDC_TRUSTCKTYPE,(dwData&AVIF_TRUSTCKTYPE)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(IDC_WASCAPTUREFILE,(dwData&AVIF_WASCAPTUREFILE)?BST_CHECKED:BST_UNCHECKED);
	CheckDlgButton(IDC_COPYRIGHTED,(dwData&AVIF_COPYRIGHTED)?BST_CHECKED:BST_UNCHECKED);
}

void CSetMainAVIHeaderFlagsDlg::UpdateData()
{
	dwData=0;
	dwData|=(AVIF_HASINDEX)*((IsDlgButtonChecked(IDC_HASINDEX)==BST_CHECKED)?1:0);
	dwData|=(AVIF_MUSTUSEINDEX)*((IsDlgButtonChecked(IDC_MUSTUSEINDEX)==BST_CHECKED)?1:0);
	dwData|=(AVIF_ISINTERLEAVED)*((IsDlgButtonChecked(IDC_ISINTERLEAVED)==BST_CHECKED)?1:0);
	dwData|=(AVIF_TRUSTCKTYPE)*((IsDlgButtonChecked(IDC_TRUSTCKTYPE)==BST_CHECKED)?1:0);
	dwData|=(AVIF_WASCAPTUREFILE)*((IsDlgButtonChecked(IDC_WASCAPTUREFILE)==BST_CHECKED)?1:0);
	dwData|=(AVIF_COPYRIGHTED)*((IsDlgButtonChecked(IDC_COPYRIGHTED)==BST_CHECKED)?1:0);
}

BEGIN_MESSAGE_MAP(CSetMainAVIHeaderFlagsDlg, CDialog)
	//{{AFX_MSG_MAP(CSetMainAVIHeaderFlagsDlg)
	ON_BN_CLICKED(IDC_SETFALGS_RESET, OnSetfalgsRestore)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CSetMainAVIHeaderFlagsDlg, CDialog)
	//{{AFX_DISPATCH_MAP(CSetMainAVIHeaderFlagsDlg)
		// HINWEIS - Der Klassen-Assistent f�gt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterst�tzung f�r IID_ISetMainAVIHeaderFlagsDlg zur Verf�gung, um typsicheres Binden
//  von VBA zu erm�glichen. Diese IID muss mit der GUID �bereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {AB3A79A4-6406-48ED-8DC4-048F6D187BFB}
static const IID IID_ISetMainAVIHeaderFlagsDlg =
{ 0xab3a79a4, 0x6406, 0x48ed, { 0x8d, 0xc4, 0x4, 0x8f, 0x6d, 0x18, 0x7b, 0xfb } };

BEGIN_INTERFACE_MAP(CSetMainAVIHeaderFlagsDlg, CDialog)
	INTERFACE_PART(CSetMainAVIHeaderFlagsDlg, IID_ISetMainAVIHeaderFlagsDlg, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen f�r Nachrichten CSetMainAVIHeaderFlagsDlg 

BOOL CSetMainAVIHeaderFlagsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	bActive=true;
	// TODO: Zus�tzliche Initialisierung hier einf�gen
	RefreshCheckboxes();
	
	SetWindowText(LoadString(STR_SAHF_TITLE));

	SendDlgItemMessage(IDOK,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_GEN_OK));
	SendDlgItemMessage(IDCANCEL,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_GEN_CANCEL));
	SendDlgItemMessage(IDC_SETFALGS_RESET,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_GEN_RESET));
	
	SendDlgItemMessage(IDC_SAHF_FLAGS,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_SAHF_AVAILABLEFLAGS));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zur�ckgeben
}

void CSetMainAVIHeaderFlagsDlg::OnSetfalgsRestore() 
{
	// TODO: Code f�r die Behandlungsroutine der Steuerelement-Benachrichtigung hier einf�gen

	SetData(dwData);
}

void CSetMainAVIHeaderFlagsDlg::OnOK() 
{
	// TODO: Zus�tzliche Pr�fung hier einf�gen
	UpdateData();	
	CDialog::OnOK();
}
