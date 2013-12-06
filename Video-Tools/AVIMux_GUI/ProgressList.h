#if !defined(AFX_PROGRESSLIST_H__321B0EBE_313C_4890_A799_0FEDBDEA0CE5__INCLUDED_)
#define AFX_PROGRESSLIST_H__321B0EBE_313C_4890_A799_0FEDBDEA0CE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProgressList.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Fenster CProgressList 

const int PRAC_BYTES = 1;
const int PRAC_SCALED = 10;

class CProgressList : public CListCtrl
{
// private:
	DWORD	dwAccuracy;

	// Konstruktion
public:
	CProgressList();

// Attribute
public:

// Operationen
public:
	int		GetAccuracy(void);
	void	SetAccuracy(DWORD dwAccuracy);

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CProgressList)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CProgressList();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CProgressList)
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CProgressList)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_PROGRESSLIST_H__321B0EBE_313C_4890_A799_0FEDBDEA0CE5__INCLUDED_
