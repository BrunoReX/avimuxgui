#if !defined(AFX_SETMAINAVIHEADERFLAGSDLG_H__A335D326_0196_4769_B2EB_23285DAE7BAE__INCLUDED_)
#define AFX_SETMAINAVIHEADERFLAGSDLG_H__A335D326_0196_4769_B2EB_23285DAE7BAE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetMainAVIHeaderFlagsDlg.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSetMainAVIHeaderFlagsDlg 

class CSetMainAVIHeaderFlagsDlg : public CDialog
{
// Konstruktion
public:
	CSetMainAVIHeaderFlagsDlg(CWnd* pParent = NULL); // Standardkonstruktor
	DWORD	dwData;
	bool	bActive;
	void	SetData(DWORD dwNewFlags);
	void	RefreshCheckboxes();
	void	UpdateData();
	DWORD	GetData(void) { return dwData; }

// Dialogfelddaten
	//{{AFX_DATA(CSetMainAVIHeaderFlagsDlg)
	enum { IDD = IDD_SETFLAGS };
		// HINWEIS: Der Klassen-Assistent fügt hier Datenelemente ein
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSetMainAVIHeaderFlagsDlg)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CSetMainAVIHeaderFlagsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSetfalgsRestore();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CSetMainAVIHeaderFlagsDlg)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_SETMAINAVIHEADERFLAGSDLG_H__A335D326_0196_4769_B2EB_23285DAE7BAE__INCLUDED_
