#if !defined(AFX_VIDEOINFORMATIONDLG_H__F7EDD748_F8CE_4DBE_B7F0_489AE67BF754__INCLUDED_)
#define AFX_VIDEOINFORMATIONDLG_H__F7EDD748_F8CE_4DBE_B7F0_489AE67BF754__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VideoInformationDlg.h : Header-Datei
//
#include "avifile.h"
#include "VideoInformationDlgListbox.h"
#include "videosource.h"
#include "UnicodeTreeCtrl.h"
#include "FILE_INFO.h"
#include "AttachedWindows.h"
#include "ResizeableDialog.h"

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CVideoInformationDlg 

class CVideoInformationDlg : public CResizeableDialog
{

// Konstruktion
public:
	CVideoInformationDlg(CWnd* pParent = NULL);   // Standardkonstruktor
	union {
		AVIFILEEX*		avifile;
		MATROSKA*		mkvfile;
	};
	FILE_INFO*		lpFI;
	VIDEOSOURCE*	lpVS;
	void			SetFile(FILE_INFO*	_lpFI);
//	void			SetVideoSource(VIDEOSOURCE* _lpVS);
	FILE_INFO*		GetFile();
	VIDEOSOURCE*	GetVideoSource();
	DWORD			GetKindOfSource();
// Dialogfelddaten
	//{{AFX_DATA(CVideoInformationDlg)
	enum { IDD = IDD_VIDEOINFO };
	CButton	m_Apply_Changes;
	CButton	m_OK;
	CButton	m_SaveTree_Button;
	CUnicodeTreeCtrl	m_Tree;
	CButton	m_BuildRIFFTree;
	CVideoInformationDlgListbox	m_VideoInformationDlgListbox;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CVideoInformationDlg)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementierung
protected:
	DWORD	dwKinfOfSource;
	bool	RenderChapters(CUnicodeTreeCtrl* cTree,HTREEITEM hParent,CHAPTERS* chapters);
	void	AddTags(TAG_INDEX_LIST &pTags, HTREEITEM hParent);
//	bool	InitDialog_VideoSource();
	bool	InitDialog_Matroska();
    HTREEITEM    Insert(const CUTF8& item, HTREEITEM hParent);
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CVideoInformationDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnApplyrepairs();
	virtual void OnOK();
	afx_msg void OnRIFFChunkTree();
	afx_msg void OnSavetree();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CVideoInformationDlg)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_VIDEOINFORMATIONDLG_H__F7EDD748_F8CE_4DBE_B7F0_489AE67BF754__INCLUDED_
