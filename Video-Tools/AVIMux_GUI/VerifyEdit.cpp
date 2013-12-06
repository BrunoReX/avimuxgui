// VerifyEdit.cpp: Implementierungsdatei
//

/*
   EDIT element which turns red when VerifyElement() fails
*/

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "VerifyEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVerifyEdit

CVerifyEdit::CVerifyEdit()
{
	EnableAutomation();

	colors[0] = RGB(255,0,0);
	colors[1] = RGB(0,0,0);
}

CVerifyEdit::~CVerifyEdit()
{
}

void CVerifyEdit::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse l�scht das Objekt
	// automatisch. F�gen Sie zus�tzlichen Bereinigungscode f�r Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CUserDrawEdit::OnFinalRelease();
}

int CVerifyEdit::GetColorIndex()
{
	if (!VerifyElement())
		return 0;
	else
		return 1;
}

COLORREF CVerifyEdit::GetTxtColor()
{
	return colors[GetColorIndex()];
}

int CVerifyEdit::VerifyElement()
{
	return 0;
}

BEGIN_MESSAGE_MAP(CVerifyEdit, CUserDrawEdit)
	//{{AFX_MSG_MAP(CVerifyEdit)
		// HINWEIS - Der Klassen-Assistent f�gt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CVerifyEdit, CUserDrawEdit)
	//{{AFX_DISPATCH_MAP(CVerifyEdit)
		// HINWEIS - Der Klassen-Assistent f�gt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterst�tzung f�r IID_IVerifyEdit zur Verf�gung, um typsicheres Binden
//  von VBA zu erm�glichen. Diese IID muss mit der GUID �bereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {7F328F52-636B-4D4F-839E-5A72B8977A1D}
static const IID IID_IVerifyEdit =
{ 0x7f328f52, 0x636b, 0x4d4f, { 0x83, 0x9e, 0x5a, 0x72, 0xb8, 0x97, 0x7a, 0x1d } };

BEGIN_INTERFACE_MAP(CVerifyEdit, CUserDrawEdit)
	INTERFACE_PART(CVerifyEdit, IID_IVerifyEdit, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen f�r Nachrichten CVerifyEdit 
