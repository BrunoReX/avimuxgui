#if !defined(AFX_SUBTITLESLISTBOX_H__9E96687F_E6B7_4870_8EC7_DAE3D21396E7__INCLUDED_)
#define AFX_SUBTITLESLISTBOX_H__9E96687F_E6B7_4870_8EC7_DAE3D21396E7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SubtitlesListBox.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Fenster CSubtitlesListBox 

#include "enhancedlistbox.h"

class CSubtitlesListBox : public CEnhancedListBox
{
// Konstruktion
private:
	int		iDown;
public:
	CSubtitlesListBox();

// Attribute
public:

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSubtitlesListBox)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CSubtitlesListBox();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CSubtitlesListBox)
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CSubtitlesListBox)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_SUBTITLESLISTBOX_H__9E96687F_E6B7_4870_8EC7_DAE3D21396E7__INCLUDED_
