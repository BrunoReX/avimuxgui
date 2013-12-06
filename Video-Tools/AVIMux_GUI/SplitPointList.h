#if !defined(AFX_SPLITPOINTLIST_H__ADFFF899_7C08_4A68_97B3_2406F8DFD8E5__INCLUDED_)
#define AFX_SPLITPOINTLIST_H__ADFFF899_7C08_4A68_97B3_2406F8DFD8E5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SplitPointList.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Fenster CSplitPointList 

#include "AVIFile.h"
#include "videosource.h"

typedef struct
{
	int		iFlags;
	__int64 iBegin;
	__int64 iEnd;

} SPLIT_POINT;

class CSplitPointList : public CListBox
{
// Konstruktion
public:
	CMenu*	cmPopupMenu;
	VIDEOSOURCE* lpVideoSource;

	CSplitPointList();
	void	AddSplitPoint(DWORD dwSplitPos);
	bool	CheckSplitPoint(DWORD dwSplitPos);

// Attribute
public:
	void SetVideoSource(VIDEOSOURCE* lpSource);

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSplitPointList)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CSplitPointList();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CSplitPointList)
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CSplitPointList)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_SPLITPOINTLIST_H__ADFFF899_7C08_4A68_97B3_2406F8DFD8E5__INCLUDED_
