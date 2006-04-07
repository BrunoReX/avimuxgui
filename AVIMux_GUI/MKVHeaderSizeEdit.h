#if !defined(AFX_MKVHEADERSIZEEDIT_H__975F4FBA_3079_4B14_8B62_10894712FA96__INCLUDED_)
#define AFX_MKVHEADERSIZEEDIT_H__975F4FBA_3079_4B14_8B62_10894712FA96__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MKVHeaderSizeEdit.h : Header-Datei
//

#include "VerifyEdit.h"

/////////////////////////////////////////////////////////////////////////////
// Fenster CMKVHeaderSizeEdit 

class CMKVHeaderSizeEdit : public CVerifyEdit
{
// Konstruktion
public:
	CMKVHeaderSizeEdit();

	int		virtual VerifyElement();

// Attribute
public:

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CMKVHeaderSizeEdit)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CMKVHeaderSizeEdit();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CMKVHeaderSizeEdit)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CMKVHeaderSizeEdit)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_MKVHEADERSIZEEDIT_H__975F4FBA_3079_4B14_8B62_10894712FA96__INCLUDED_
