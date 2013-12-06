#if !defined(AFX_RESOLUTIONEDIT_H__40438898_9D36_4F15_B1B9_F20308886106__INCLUDED_)
#define AFX_RESOLUTIONEDIT_H__40438898_9D36_4F15_B1B9_F20308886106__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResolutionEdit.h : Header-Datei
//

#include "VerifyEdit.h"

/////////////////////////////////////////////////////////////////////////////
// Fenster CResolutionEdit 

class CResolutionEdit : public CVerifyEdit
{
// Konstruktion
public:
	CResolutionEdit();

	int	virtual	VerifyElement();
// Attribute
public:

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CResolutionEdit)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CResolutionEdit();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CResolutionEdit)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CResolutionEdit)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_RESOLUTIONEDIT_H__40438898_9D36_4F15_B1B9_F20308886106__INCLUDED_
