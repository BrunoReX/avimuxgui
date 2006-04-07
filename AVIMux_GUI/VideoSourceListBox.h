#if !defined(AFX_VIDEOSOURCELISTBOX_H__FC7B00A6_F91C_4FEB_A907_4CB3AE9330FD__INCLUDED_)
#define AFX_VIDEOSOURCELISTBOX_H__FC7B00A6_F91C_4FEB_A907_4CB3AE9330FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VideoSourceListBox.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Fenster CVideoSourceListBox 

#include "enhancedlistbox.h"

class CVideoSourceListBox : public CEnhancedListBox
{
// Konstruktion
public:
	CVideoSourceListBox();

// Attribute
public:

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CVideoSourceListBox)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CVideoSourceListBox();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CVideoSourceListBox)
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CVideoSourceListBox)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_VIDEOSOURCELISTBOX_H__FC7B00A6_F91C_4FEB_A907_4CB3AE9330FD__INCLUDED_
