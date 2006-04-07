#if !defined(AFX_SETFRAMERATEDLG_H__D1D92668_53A1_4D4F_B385_927C6DF08057__INCLUDED_)
#define AFX_SETFRAMERATEDLG_H__D1D92668_53A1_4D4F_B385_927C6DF08057__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetFramerateDlg.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSetFramerateDlg 

#define UNIT_FPS	0x01
#define UNIT_MSPF	0x02
#define UNIT_NSPF	0x03

#include "SplitPointsDlg.h"

typedef struct {
	int		nom;
	int		den;
	double  frate;
} FRAME_RATE;

class CSetFramerateDlg : public CDialog
{
// Konstruktion
public:
	FRAME_RATE	fr;

	CSetFramerateDlg(CWnd* pParent = NULL);   // Standardkonstruktor
	void		SetData(FRAME_RATE* f);
	void		GetData(FRAME_RATE* result);
	void		Refresh(void);
	bool		bAllowEditUpdate;
	DWORD		dwUnit;
// Dialogfelddaten
	//{{AFX_DATA(CSetFramerateDlg)
	enum { IDD = IDD_SETFRAMERATE };
		// HINWEIS: Der Klassen-Assistent fügt hier Datenelemente ein
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSetFramerateDlg)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CSetFramerateDlg)
	afx_msg void OnFrFps();
	afx_msg void OnFrMspf();
	afx_msg void OnFrNspf();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CSetFramerateDlg)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_SETFRAMERATEDLG_H__D1D92668_53A1_4D4F_B385_927C6DF08057__INCLUDED_
