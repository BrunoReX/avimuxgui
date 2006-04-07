#if !defined(AFX_SOURCEFILELISTBOX_H__7D185088_2C6F_4FF1_8836_772604B2F7D9__INCLUDED_)
#define AFX_SOURCEFILELISTBOX_H__7D185088_2C6F_4FF1_8836_772604B2F7D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SourceFileListBox.h : Header-Datei
//

#include "EnhancedListbox.h"
#include "File_Info.h"

/////////////////////////////////////////////////////////////////////////////
// Fenster CSourceFileListBox 

class CSourceFileListBox : public CEnhancedListBox
{
// Konstruktion
public:
	CSourceFileListBox();

	FILE_INFO*	GetFileInfo(int i = -1);

// Attribute
public:

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSourceFileListBox)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CSourceFileListBox();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CSourceFileListBox)
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CSourceFileListBox)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_SOURCEFILELISTBOX_H__7D185088_2C6F_4FF1_8836_772604B2F7D9__INCLUDED_
