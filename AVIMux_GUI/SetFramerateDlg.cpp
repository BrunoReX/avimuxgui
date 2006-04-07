// SetFramerateDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "SetFramerateDlg.h"
#include "..\Basestreams.h"
#include "Languages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSetFramerateDlg 


CSetFramerateDlg::CSetFramerateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetFramerateDlg::IDD, pParent)
{
	EnableAutomation();
	bAllowEditUpdate=false;
	
	//{{AFX_DATA_INIT(CSetFramerateDlg)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}


void CSetFramerateDlg::OnFinalRelease()
{

	CDialog::OnFinalRelease();
}

void CSetFramerateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetFramerateDlg)
		// HINWEIS: Der Klassen-Assistent fügt hier DDX- und DDV-Aufrufe ein
	//}}AFX_DATA_MAP
}

void CSetFramerateDlg::SetData(FRAME_RATE* f)
{
	memcpy(&fr, f, sizeof(*f));

	//float dFramerate=dData;
	char		Buffer[50];
	//fr.frate = (double)(100*(round(dFramerate/100)));

	if (bAllowEditUpdate)
	{
		if (IsDlgButtonChecked(IDR_FR_NSPF))
		{
			itoa((int)(1000000000./fr.frate),Buffer,10);
			dwUnit=UNIT_NSPF;
		}
		else
		if (IsDlgButtonChecked(IDR_FR_MSPF))
		{	
			gcvt(1000000./fr.frate,9,Buffer);
			dwUnit=UNIT_MSPF;
		}
		else
		if (IsDlgButtonChecked(IDR_FR_FPS))
		{
			if (fr.den == 0) {
				_gcvt(fr.frate,6,Buffer);
				dwUnit=UNIT_FPS;
			} else {
				sprintf(Buffer, "%d/%d", fr.nom, fr.den);
			}
		}
		SendDlgItemMessage(IDC_NEWFRAMERATE,WM_SETTEXT,0,(LPARAM)Buffer);
	}
}

void CSetFramerateDlg::GetData(FRAME_RATE* result)
{
	result->frate = fr.frate;
	result->den = fr.den;
	result->nom = fr.nom;
}

void split_at(char split_char, char* c1, char** c2, char** c3);

void split_equal(char* c1, char** c2, char** c3)
{
	split_at('=', c1, c2, c3);
}

void split_at(char split_char, char* c1, char** c2, char** c3)
{
	*c2 = c1;

	while (*c1) {
		if (*c1 == split_char) {
			*c1 = 0;
			*c3 = ++c1;
			return;
		}
		c1++;
	}
}


void CSetFramerateDlg::Refresh()
{
	char		Buffer[50]; memset(Buffer,0,sizeof(Buffer));
	char		*p1; p1=(char*)calloc(1, 50);
	char		*p2; p2=(char*)calloc(1, 50);
	char        *q1 = p1;
	char        *q2 = p2;

	SendDlgItemMessage(IDC_NEWFRAMERATE,WM_GETTEXT,50,(LPARAM)Buffer);

	split_at('/', Buffer, (char**)&p1, (char**)&p2);
	if (*p1) 
		fr.nom = atoi(p1);
	
	if (*p2) {
		fr.den = atoi(p2);
		fr.frate = (double)fr.nom / (double)fr.den;
	} else {
		fr.den = 0;
		fr.frate = atof(p1);
	}

	if (dwUnit==UNIT_FPS)
	{ } else if (dwUnit==UNIT_MSPF) {
		fr.den = 0;
		fr.frate = 1000000./atof(Buffer);
	} else if (dwUnit==UNIT_NSPF) {
		fr.den = 0;
		fr.frate = 1000000000./atof(Buffer);
	}

	SetData(&fr);

	free(q1);
	free(q2);
}

BEGIN_MESSAGE_MAP(CSetFramerateDlg, CDialog)
	//{{AFX_MSG_MAP(CSetFramerateDlg)
	ON_BN_CLICKED(IDR_FR_FPS, OnFrFps)
	ON_BN_CLICKED(IDR_FR_MSPF, OnFrMspf)
	ON_BN_CLICKED(IDR_FR_NSPF, OnFrNspf)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CSetFramerateDlg, CDialog)
	//{{AFX_DISPATCH_MAP(CSetFramerateDlg)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_ISetFramerateDlg zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {658025D0-E43A-49BB-B278-38BE52807A49}
static const IID IID_ISetFramerateDlg =
{ 0x658025d0, 0xe43a, 0x49bb, { 0xb2, 0x78, 0x38, 0xbe, 0x52, 0x80, 0x7a, 0x49 } };

BEGIN_INTERFACE_MAP(CSetFramerateDlg, CDialog)
	INTERFACE_PART(CSetFramerateDlg, IID_ISetFramerateDlg, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CSetFramerateDlg 

void CSetFramerateDlg::OnFrFps() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	Refresh();
	
}

void CSetFramerateDlg::OnFrMspf() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	Refresh();
	
}

void CSetFramerateDlg::OnFrNspf() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	Refresh();
	
}

BOOL CSetFramerateDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	char	Buffer[50];
	
	// TODO: Zusätzliche Initialisierung hier einfügen
	dwUnit=UNIT_FPS;
	CheckDlgButton(IDR_FR_FPS,BST_CHECKED);
	bAllowEditUpdate=true;	
	_gcvt(fr.frate,6,Buffer);
	SendDlgItemMessage(IDC_NEWFRAMERATE,WM_SETTEXT,0,(LPARAM)Buffer);

	SetWindowText(LoadString(STR_SFR_TITLE));
	SendDlgItemMessage(IDOK,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_GEN_OK));
	SendDlgItemMessage(IDCANCEL,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_GEN_CANCEL));

	SendDlgItemMessage(IDC_SFR_UNIT,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_SFR_UNIT));
	SendDlgItemMessage(IDR_FR_FPS,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_SFR_FPS));
	SendDlgItemMessage(IDR_FR_MSPF,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_SFR_MSPF));
	SendDlgItemMessage(IDR_FR_NSPF,WM_SETTEXT,NULL,(LPARAM)LoadString(STR_SFR_NSPF));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CSetFramerateDlg::OnOK() 
{
	// TODO: Zusätzliche Prüfung hier einfügen
	Refresh();
	
	CDialog::OnOK();
}
