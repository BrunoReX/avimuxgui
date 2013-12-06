#if !defined(AFX_CHAPTERDLGLIST_H__4036B2E5_808D_47D3_A949_3FEB85CECBED__INCLUDED_)
#define AFX_CHAPTERDLGLIST_H__4036B2E5_808D_47D3_A949_3FEB85CECBED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChapterDlgList.h : Header-Datei
//

#include "UnicodeListControl.h"

/////////////////////////////////////////////////////////////////////////////
// Fenster CChapterDlgList 

class CChapterDlgList : public CUnicodeListCtrl
{
// Konstruktion
public:
	CChapterDlgList();

// Attribute
public:

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CChapterDlgList)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CChapterDlgList();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CChapterDlgList)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CChapterDlgList)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_CHAPTERDLGLIST_H__4036B2E5_808D_47D3_A949_3FEB85CECBED__INCLUDED_
