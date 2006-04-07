#ifndef I_TREES
#define I_TREES

#include "../DynArray.h"

template <class T> HTREEITEM Tree_Insert(T* CTree,char* cBuffer,HTREEITEM hParent = NULL, HTREEITEM hAfter = TVI_LAST)
{
	TVINSERTSTRUCT	tvi;

	tvi.hParent=hParent;
	tvi.hInsertAfter=hAfter;
	tvi.item.mask=TVIF_TEXT;
	tvi.item.pszText=cBuffer;
	
	if (cBuffer != LPSTR_TEXTCALLBACK)
		tvi.item.cchTextMax=strlen(cBuffer);

	return CTree->InsertItem(&tvi);
}

template <class T> HTREEITEM Tree_InsertCheck(T* CTree,char* cBuffer,HTREEITEM hParent = NULL, HTREEITEM hAfter = TVI_LAST)
{
	TVINSERTSTRUCT	tvi;

	tvi.hParent=hParent;
	tvi.hInsertAfter=hAfter;
	tvi.item.mask=TVIF_TEXT;
	tvi.item.pszText=cBuffer;
	tvi.item.cchTextMax=1+lstrlen(cBuffer);

	HTREEITEM hRes = CTree->InsertItem(&tvi);
	Tree_SetCheckState(CTree,hRes,true);

	return hRes;
}


template <class T> BOOL Tree_SetCheckState(T* CTree, HTREEITEM hItem, BOOL fCheck)
{
    TVITEM tvItem;

    tvItem.mask = TVIF_HANDLE | TVIF_STATE;
    tvItem.hItem = hItem;
    tvItem.stateMask = TVIS_STATEIMAGEMASK;

    /*
    Since state images are one-based, 1 in this macro turns the check off, and 
    2 turns it on.
    */
    tvItem.state = INDEXTOSTATEIMAGEMASK((fCheck ? 2 : 1));

    ((CTreeCtrl*)CTree)->SetItem(&tvItem);
	return true;
}

template <class T> BOOL Tree_GetCheckState(T* CTree, HTREEITEM hItem)
{
    TVITEM tvItem;

    // Prepare to receive the desired information.
    tvItem.mask = TVIF_HANDLE | TVIF_STATE;
    tvItem.hItem = hItem;
    tvItem.stateMask = TVIS_STATEIMAGEMASK;

    // Request the information.
    CTree->GetItem(&tvItem);

    // Return zero if it's not checked, or nonzero otherwise.
    return ((BOOL)(tvItem.state >> 12) -1);
}



template <class T> CDynIntArray* Tree_GetChecked(T* CTree, HTREEITEM hItem, CDynIntArray** indices = NULL)
{
	int	j;

	CDynIntArray* a = new CDynIntArray;
	if (indices) *indices = new CDynIntArray;

	j=0;
	while (hItem)
	{
		if (Tree_GetCheckState(CTree,hItem)) {
			a->Insert((int)hItem);
			if (indices) (*indices)->Insert(j++);
		}

		hItem = CTree->GetNextSiblingItem(hItem);
	}

	return a;
}

template <class T> CDynIntArray* Tree_GetAllRootElements(T* CTree, HTREEITEM hItem)
{
	CDynIntArray* a = new CDynIntArray;

	while (hItem)
	{
		a->Insert((int)hItem);
		hItem = CTree->GetNextSiblingItem(hItem);
	}

	return a;
}

template <class T> HTREEITEM Tree_Index2Item(T* CTree, int iIndex, int iID = -1, int* d = NULL)
{
	int counter = 0;

	HTREEITEM hItem = CTree->GetRootItem();

	while (++counter && 
		   hItem &&
		   iIndex > -1
		   ) 
	
	{
		if ((iID == -1 || ((TREE_ITEM_INFO*)CTree->GetItemData(hItem))->iID == iID)) {
			iIndex--;
		}
		if (iIndex>-1) hItem = CTree->GetNextSiblingItem(hItem);
	}

	if (d) *d = counter-2;

	return hItem;
}


#endif