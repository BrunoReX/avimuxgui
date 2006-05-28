#if !defined(AFX_RESIZEABLEDIALOG_H__45CCA68C_8282_4EB6_B029_0737FF1A9789__INCLUDED_)
#define AFX_RESIZEABLEDIALOG_H__45CCA68C_8282_4EB6_B029_0737FF1A9789__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResizeableDialog.h : Header-Datei
//

#include "resource.h"
#include "../buffers.h"

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CResizeableDialog 

#include "AttachedWindows.h"

class CResizeableDialog : public CDialog
{
private:
	ATTACHED_WINDOWS	attached_windows;
	int					being_destroyed;
	int					resize_in_progress;
	int					has_been_initialized;
	bool				bMinimized;
#ifdef I_ATTRIBUTES
	CAttribs*			attribs;
#endif
	CFont*	user_font;
	void	ApplyFont(CFont* f);

protected:
#ifdef I_ATTRIBUTES
	CAttribs* GetAttribs() { return attribs; }
	void	ReinitFont(CFont* f);
	void	ReinitPosition();
#endif
// Konstruktion
public:
	CResizeableDialog(UINT nIDTemplate, CWnd* pParent = NULL);   // Standardkonstruktor
	virtual ~CResizeableDialog();

	void AttachWindow(HWND hWnd, int border, HWND hWndAttachTo, int target_border, int distance);
	void AttachWindow(HWND hWnd, int border, HWND hWndAttachTo, int distance = NULL);
	void AttachWindow(HWND hWnd, HWND hWndAttachTo, int flags, float width_ratio = 1., float height_ratio = 1.);
	void AttachLabel(HWND hWnd, HWND hWndLabel);
	void AttachRow(HWND* hWnd, int distance, int additional_alignment = 0);
	void AttachUpDown(HWND hWnd, HWND hUpDown);
	void AttachVCenterAndLeftBorder(HWND hWnd, HWND hVCenterTo, HWND hLeftBorderTo, int indent = 0);
	void AttachWindowBeneath(HWND hWnd, HWND hWndTo, int distance,
		int addition_alignment = 0, int indent = 0);
	void AttachRowBeneath(HWND* hWnd_row, HWND hWndTo, int first_distance,
		int distance, int additional_alignment, int indent);
	
	void ReorderWindows(int &redo);
	void GetBorder(int &w, int &h);
#ifdef I_ATTRIBUTES
	void Attribs(CAttribs* a);
#endif
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);


// Dialogfelddaten
	//{{AFX_DATA(CResizeableDialog)
	enum { IDD = IDD_CHAPTERDIALOG };
		// HINWEIS: Der Klassen-Assistent fügt hier Datenelemente ein
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CResizeableDialog)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CResizeableDialog)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnDeleteItem(int nIDCtl, LPDELETEITEMSTRUCT lpDeleteItemStruct);
	afx_msg void OnDestroy();
	afx_msg void OnMove(int x, int y);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CResizeableDialog)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_RESIZEABLEDIALOG_H__45CCA68C_8282_4EB6_B029_0737FF1A9789__INCLUDED_
