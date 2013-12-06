// MKVHeaderSizeEdit.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "MKVHeaderSizeEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMKVHeaderSizeEdit

CMKVHeaderSizeEdit::CMKVHeaderSizeEdit()
{
	EnableAutomation();
}

CMKVHeaderSizeEdit::~CMKVHeaderSizeEdit()
{
}

int CMKVHeaderSizeEdit::VerifyElement()
{
	char t[16]; memset(t, 0, sizeof(t));

	GetWindowText(t, 16);
	int j = atoi(t);

	if ((j<2 || j>1024) && j!=0)
		return 0;

	return 1;
}

void CMKVHeaderSizeEdit::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse l�scht das Objekt
	// automatisch. F�gen Sie zus�tzlichen Bereinigungscode f�r Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CVerifyEdit::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CMKVHeaderSizeEdit, CVerifyEdit)
	//{{AFX_MSG_MAP(CMKVHeaderSizeEdit)
		// HINWEIS - Der Klassen-Assistent f�gt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CMKVHeaderSizeEdit, CVerifyEdit)
	//{{AFX_DISPATCH_MAP(CMKVHeaderSizeEdit)
		// HINWEIS - Der Klassen-Assistent f�gt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterst�tzung f�r IID_IMKVHeaderSizeEdit zur Verf�gung, um typsicheres Binden
//  von VBA zu erm�glichen. Diese IID muss mit der GUID �bereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {5A3218F4-C6BB-4416-B84C-0E15B07FC63B}
static const IID IID_IMKVHeaderSizeEdit =
{ 0x5a3218f4, 0xc6bb, 0x4416, { 0xb8, 0x4c, 0xe, 0x15, 0xb0, 0x7f, 0xc6, 0x3b } };

BEGIN_INTERFACE_MAP(CMKVHeaderSizeEdit, CVerifyEdit)
	INTERFACE_PART(CMKVHeaderSizeEdit, IID_IMKVHeaderSizeEdit, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen f�r Nachrichten CMKVHeaderSizeEdit 
