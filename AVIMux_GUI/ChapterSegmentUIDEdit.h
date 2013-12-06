#if !defined(AFX_CHAPTERSEGMENTUIDEDIT_H__19D2258D_AEDB_45B5_A0E5_DA323D6485E4__INCLUDED_)
#define AFX_CHAPTERSEGMENTUIDEDIT_H__19D2258D_AEDB_45B5_A0E5_DA323D6485E4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChapterSegmentUIDEdit.h : Header-Datei
//

#include "VerifyEdit.h"

/////////////////////////////////////////////////////////////////////////////
// Fenster CChapterSegmentUIDEdit 

class CChapterSegmentUIDEdit : public CVerifyEdit
{
// Konstruktion
public:
	CChapterSegmentUIDEdit();

	int		virtual	VerifyElement();

	bool	bValid;
	char	cUID[17];
	bool	IsValid();
	void	GetUID(char* uid);
	int		Validate();
	double  fSegDuration;
	double  GetSegmentDuration();
// Attribute
public:

// Operationen
public:

// �berschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktions�berschreibungen
	//{{AFX_VIRTUAL(CChapterSegmentUIDEdit)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CChapterSegmentUIDEdit();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CChapterSegmentUIDEdit)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CChapterSegmentUIDEdit)
		// HINWEIS - Der Klassen-Assistent f�gt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ f�gt unmittelbar vor der vorhergehenden Zeile zus�tzliche Deklarationen ein.

#endif // AFX_CHAPTERSEGMENTUIDEDIT_H__19D2258D_AEDB_45B5_A0E5_DA323D6485E4__INCLUDED_
