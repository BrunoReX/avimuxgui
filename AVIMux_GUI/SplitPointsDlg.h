#if !defined(AFX_SPLITPOINTSDLG_H__F53EBEFC_3455_4FBF_B752_8A72E5023A49__INCLUDED_)
#define AFX_SPLITPOINTSDLG_H__F53EBEFC_3455_4FBF_B752_8A72E5023A49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SplitPointsDlg.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CSplitPointsDlg 

#include "SplitPointList.h"
#include "AVIFile.h"
#include "resource.h"
#include "../dynarray.h"
#include "../chapters.h"

const int SPD_BEGIN	=	0x00000001;
const int SPD_END   =   0x00000002;
const int SPD_BCHAP =   0x00000004;
const int SPD_ECHAP =   0x00000008;
const int SPD_BFRAME=   0x00000010;
const int SPD_EFRAME=   0x00000020;


typedef struct
{
	int			iFlags;     // flags
	__int64		iBegin;     // milliseconds or chapter index
	__int64		iEnd;       // milliseconds or chapter index
	CDynIntArray* aChapBegin;
	CDynIntArray* aChapEnd;
	CChapters*  chapter;
	int			iIndex;
	CStringBuffer* cOrgText;
} SPLIT_POINT_DESCRIPTOR;

class CSplitPoints
{
	private:
		CDynIntArray* points;
	public:
		CSplitPoints();
		void Delete(int iIndex);
		void DeleteAll();
		SPLIT_POINT_DESCRIPTOR* At(int iIndex);
		void Insert(SPLIT_POINT_DESCRIPTOR* p);
		int GetCount();
		void Duplicate(CSplitPoints* pDest);
};

class CSplitPointsDlg : public CDialog
{
private:
	__int64*	lpqwDataIn;
	__int64*	lpqwDataOut;
	VIDEOSOURCE* lpVideoSource;
	CSplitPoints* points;
// Konstruktion
public:
	CSplitPointsDlg(CWnd* pParent = NULL);   // Standardkonstruktor
	DWORD	Load(CSplitPoints* points);
	DWORD	GetData(CSplitPoints* points);
	void	SetVideoSource(VIDEOSOURCE* lpSource);

// Dialogfelddaten
	//{{AFX_DATA(CSplitPointsDlg)
	enum { IDD = IDD_SPLITPOINTS };
	CEdit	m_Splitpoint;
	CSplitPointList	m_SplitPointList;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CSplitPointsDlg)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CSplitPointsDlg)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CSplitPointsDlg)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_SPLITPOINTSDLG_H__F53EBEFC_3455_4FBF_B752_8A72E5023A49__INCLUDED_
