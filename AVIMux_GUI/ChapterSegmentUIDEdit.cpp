// ChapterSegmentUIDEdit.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "ChapterSegmentUIDEdit.h"
#include "..\Matroska.h"
#include "..\UnicodeCalls.h"
#include "languages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChapterSegmentUIDEdit

#include "..\chapters.h"

CChapterSegmentUIDEdit::CChapterSegmentUIDEdit()
{
	EnableAutomation();
	bValid = false;
	fSegDuration = -1;
}

CChapterSegmentUIDEdit::~CChapterSegmentUIDEdit()
{
}

int CChapterSegmentUIDEdit::VerifyElement()
{
	return IsValid();
}

int CChapterSegmentUIDEdit::Validate()
{
	memset(cUID,0,sizeof(cUID));
	char t[64]; memset(t,0,sizeof(t));
	bValid = false;

	GetWindowText(t, 64);
	if (!stricmp(t, "N/A"))
		return 1;

	if (bValid = !!hex2int128(t, cUID))
		return 1;
	else
		return 0;

	return 0;
}

bool CChapterSegmentUIDEdit::IsValid()
{
	Validate();
	return (bValid);
}

void CChapterSegmentUIDEdit::GetUID(char* uid)
{
	if (uid)
		memcpy(uid, cUID, 16);
}

void CChapterSegmentUIDEdit::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CVerifyEdit::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CChapterSegmentUIDEdit, CVerifyEdit)
	//{{AFX_MSG_MAP(CChapterSegmentUIDEdit)
	ON_WM_SETFOCUS()
	ON_WM_DROPFILES()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CChapterSegmentUIDEdit, CVerifyEdit)
	//{{AFX_DISPATCH_MAP(CChapterSegmentUIDEdit)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_IChapterSegmentUIDEdit zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {7E8B0076-B405-4E65-8EB1-C30361828E2B}
static const IID IID_IChapterSegmentUIDEdit =
{ 0x7e8b0076, 0xb405, 0x4e65, { 0x8e, 0xb1, 0xc3, 0x3, 0x61, 0x82, 0x8e, 0x2b } };

BEGIN_INTERFACE_MAP(CChapterSegmentUIDEdit, CVerifyEdit)
	INTERFACE_PART(CChapterSegmentUIDEdit, IID_IChapterSegmentUIDEdit, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CChapterSegmentUIDEdit 

void CChapterSegmentUIDEdit::OnSetFocus(CWnd* pOldWnd) 
{
	CVerifyEdit::OnSetFocus(pOldWnd);
	
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen
}

void CChapterSegmentUIDEdit::OnDropFiles(HDROP hDropInfo) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	DWORD	dwCount;
	char*	lpcName;

	dwCount=DragQueryFile(hDropInfo,0xFFFFFFFF,NULL,NULL);

	if (dwCount > 1)
		return;
	else
	{
		int i=0;
		char temp[4096]; temp[0]=0;
		(*UDragQueryFile())((uint32)hDropInfo, i, temp, 4096);
		lpcName = (char*)calloc(1,4096);
		toUTF8(temp, lpcName);

		FILESTREAM* f = new FILESTREAM;
		if (f->Open(lpcName, STREAM_READ)==STREAM_OK) {
			MATROSKA* m = new MATROSKA;
			if (m->Open(f, MMODE_READ)== MOPEN_OK) {
				if (m->GetSegmentCount() == 1) {
					char c[64]; memset(c, 0, sizeof(c));
					if (m->GetSegmentUID())
						__int128hex(m->GetSegmentUID(), c, 1);
					else
						MessageBox(LoadString(STR_ERR_NOSEGMENTUID), LoadString(IDS_ERROR),
							MB_OK | MB_ICONERROR);
					fSegDuration = (double)m->GetSegmentDuration() * m->GetTimecodeScale();
					SetWindowText(c);
				}
				m->Close();
			}
			delete m;
			f->Close();
		}
		delete f;
	}
	

	CVerifyEdit::OnDropFiles(hDropInfo);
}

double CChapterSegmentUIDEdit::GetSegmentDuration()
{
	double res = fSegDuration;
	fSegDuration = -1;
	return res;
}
