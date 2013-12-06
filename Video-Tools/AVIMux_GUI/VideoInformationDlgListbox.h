#if !defined(AFX_VIDEOINFORMATIONDLGLISTBOX_H__AB9D4ED3_4E25_4577_A213_2612264D6892__INCLUDED_)
#define AFX_VIDEOINFORMATIONDLGLISTBOX_H__AB9D4ED3_4E25_4577_A213_2612264D6892__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VideoInformationDlgListbox.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Fenster CVideoInformationDlgListbox 

#include "avifile.h"
#include "VideoInformationDlgTypes.h"

#define REPAIRS_ODML				0x00000001
#define REPAIRS_FRAMERATE			0x00000002
#define	REPAIRS_CHANGEFRAMERATE		0x00000004
#define REPAIRS_MAHFLAGS			0x00000008
#define REPAIRS_TOTALFRAMES			0x00000010

const int KOS_AVIFILEEX		= 0x01;
const int KOS_VIDEOSOURCE	= 0x02;
const int KOS_MATROSKA		= 0x03;


class CVideoInformationDlgListbox : public CListBox
{
// Konstruktion
public:
	AVIFILEEX*			avifile;
	CHANGEAVIHEADER*	lpcahCurr, *lpcahFirst;
	DWORD				dwDone;


	CVideoInformationDlgListbox();
	void		SetFile(AVIFILEEX* lpAFE) { avifile=lpAFE; };
	CHANGEAVIHEADER*	GetRepairs(void) { return lpcahFirst; }
	void				ClearRepairs(bool bStartNew = true);
	void				SetUnavailableRepairs(DWORD dwUnavailable);
	void				NewRepairList();

// Attribute
public:

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CVideoInformationDlgListbox)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CVideoInformationDlgListbox();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CVideoInformationDlgListbox)
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CVideoInformationDlgListbox)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_VIDEOINFORMATIONDLGLISTBOX_H__AB9D4ED3_4E25_4577_A213_2612264D6892__INCLUDED_
