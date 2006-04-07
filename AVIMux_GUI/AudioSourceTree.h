#if !defined(AFX_AUDIOSOURCETREE_H__DBEF7E65_CA96_491A_9D46_FDA849441A13__INCLUDED_)
#define AFX_AUDIOSOURCETREE_H__DBEF7E65_CA96_491A_9D46_FDA849441A13__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AudioSourceTree.h : Header-Datei
//

//#include "audiosourcelist.h"
#include "subtitles.h"
#include "dynarray.h"
#include "trees.h"
#include "audiosource.h"
#include "unicodetreectrl.h"

typedef struct 
{
	DWORD				dwType;
	DWORD				dwFlags;
	AVIStreamHeader*	lpASH;
	void*				lpFormat;
	int					iFormatSize;
	AUDIOSOURCE*		audiosource;
	int					iDelay;
	__int64				iSize;

	bool				bNameFromFormatTag;
	DWORD*				lpdwFiles;

} AUDIO_STREAM_INFO;

const int ASIF_ALLOCATED =	0x01;
const int ASIF_AVISOURCE =	0x02;

typedef struct {
	int		iID;
	int		iOrgPos;
	int		iCurrPos;

	union
	{
		void*                  pData;
		AUDIO_STREAM_INFO*     pASI;
		SUBTITLE_STREAM_INFO*  pSSI;
		char*                  pText;
	};

} TREE_ITEM_INFO;

const int TIIID_ASI		= 0x01;
const int TIIID_SSI		= 0x02;
const int TIIID_LNGCODE = 0x10;
const int TIIID_STRNAME = 0x11;

TREE_ITEM_INFO*	BuildTIIfromASI(AUDIO_STREAM_INFO* asi);
TREE_ITEM_INFO*	BuildTIIfromSSI(SUBTITLE_STREAM_INFO* ssi);

/////////////////////////////////////////////////////////////////////////////
// Fenster CAudioSourceTree 

class CAudioSourceTree : public CUnicodeTreeCtrl
{
private:
	bool	bDragging;
	TVITEM	dragged_item;
	CPoint	drag_source_point;
// Konstruktion
public:
	CAudioSourceTree();
	void	OpenContextMenu(CPoint point);
	void	Sort();
	int		GetRootCount();

// Attribute
public:

// Operationen
public:
	HTREEITEM	FindID(HTREEITEM hItem, int iID, TREE_ITEM_INFO** tii = NULL);
	CDynIntArray* GetItems(HTREEITEM hItem, int iID, int iCheck, CDynIntArray** indices = NULL);
	TREE_ITEM_INFO* GetItemInfo(HTREEITEM hItem);

// �berschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktions�berschreibungen
	//{{AFX_VIRTUAL(CAudioSourceTree)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CAudioSourceTree();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CAudioSourceTree)
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generierte OLE-Dispatch-Zuordnungsfunktionen
	//{{AFX_DISPATCH(CAudioSourceTree)
		// HINWEIS - Der Klassen-Assistent f�gt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ f�gt unmittelbar vor der vorhergehenden Zeile zus�tzliche Deklarationen ein.

#endif // AFX_AUDIOSOURCETREE_H__DBEF7E65_CA96_491A_9D46_FDA849441A13__INCLUDED_