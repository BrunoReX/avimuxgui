// AVIMux_GUI.h : Haupt-Header-Datei f�r die Anwendung AVIMUX_GUI
//

#if !defined(AFX_AVIMUX_GUI_H__F0ACBB31_E386_4B2F_80B2_63D6481F6C9F__INCLUDED_)
#define AFX_AVIMUX_GUI_H__F0ACBB31_E386_4B2F_80B2_63D6481F6C9F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "IncResource.h"		// Hauptsymbole
#include "..\..\Common\TraceFiles\TraceFile.h"

/////////////////////////////////////////////////////////////////////////////
// CAVIMux_GUIApp:
// Siehe AVIMux_GUI.cpp f�r die Implementierung dieser Klasse
//

HINSTANCE GetInstance();
void ProcessMsgQueue(HWND hwnd);

class CAVIMux_GUIApp : public CWinApp
{
public:
	CAVIMux_GUIApp();

// �berladungen
	// Vom Klassenassistenten generierte �berladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CAVIMux_GUIApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementierung

	//{{AFX_MSG(CAVIMux_GUIApp)
		// HINWEIS - An dieser Stelle werden Member-Funktionen vom Klassen-Assistenten eingef�gt und entfernt.
		//    Innerhalb dieser generierten Quelltextabschnitte NICHTS VER�NDERN!
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ f�gt unmittelbar vor der vorhergehenden Zeile zus�tzliche Deklarationen ein.

#endif // !defined(AFX_AVIMUX_GUI_H__F0ACBB31_E386_4B2F_80B2_63D6481F6C9F__INCLUDED_)
