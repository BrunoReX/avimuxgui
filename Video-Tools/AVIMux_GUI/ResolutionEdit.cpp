// ResolutionEdit.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "ResolutionEdit.h"
#include "FormatText.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CResolutionEdit

CResolutionEdit::CResolutionEdit()
{
	EnableAutomation();
}

CResolutionEdit::~CResolutionEdit()
{
}

int CResolutionEdit::VerifyElement()
{
	DWORD textSize = GetWindowTextLength();
	std::vector<TCHAR> text(textSize+1);
	GetWindowText(&text[0], textSize+1);

	RESOLUTION r1, r2;

	if (Str2Resolution(((std::string)CUTF8(&text[0])).c_str(), 0, 0, &r1, &r2) == STRF_OK)
		return 1;
	else
		return 0;
	
}

void CResolutionEdit::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CVerifyEdit::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CResolutionEdit, CVerifyEdit)
	//{{AFX_MSG_MAP(CResolutionEdit)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CResolutionEdit, CVerifyEdit)
	//{{AFX_DISPATCH_MAP(CResolutionEdit)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_IResolutionEdit zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {008DB436-1A63-4005-A7CD-E6E9BF6A8FB3}
static const IID IID_IResolutionEdit =
{ 0x8db436, 0x1a63, 0x4005, { 0xa7, 0xcd, 0xe6, 0xe9, 0xbf, 0x6a, 0x8f, 0xb3 } };

BEGIN_INTERFACE_MAP(CResolutionEdit, CVerifyEdit)
	INTERFACE_PART(CResolutionEdit, IID_IResolutionEdit, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CResolutionEdit 
