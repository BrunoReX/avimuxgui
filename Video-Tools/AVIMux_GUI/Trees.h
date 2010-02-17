#ifndef I_TREES
#define I_TREES

#include <vector>

template <class T> 
HTREEITEM Tree_Insert(T* CTree, const char* cBuffer,HTREEITEM hParent = NULL, HTREEITEM hAfter = TVI_LAST)
{
	//std::string itemString(cBuffer);

	TVINSERTSTRUCT	tvi;

	tvi.hParent=hParent;
	tvi.hInsertAfter=hAfter;
	tvi.item.mask=TVIF_TEXT;
	tvi.item.pszText = const_cast<char*>(cBuffer); //(char*)itemString.c_str(); //cBuffer;
	
	if (cBuffer != LPSTR_TEXTCALLBACK)
		tvi.item.cchTextMax=strlen(cBuffer);

	return CTree->InsertItem(&tvi);
}

template <class T> 
HTREEITEM Tree_InsertCheck(T* CTree, const CUTF8& buffer,
						   HTREEITEM hParent = NULL, HTREEITEM hAfter = TVI_LAST)
{
	TVINSERTSTRUCT	tvi;

	std::vector<TCHAR> tempBuffer(buffer.TStr(), buffer.TStr() + _tcslen(buffer.TStr()));

	tvi.hParent=hParent;
	tvi.hInsertAfter=hAfter;
	tvi.item.mask=TVIF_TEXT;
	tvi.item.pszText = &tempBuffer[0];
	tvi.item.cchTextMax = 1 + _tcslen(buffer.TStr());

	HTREEITEM hRes = CTree->InsertItem(&tvi);
	Tree_SetCheckState(CTree,hRes,true);

	return hRes;
}

template <class T> 
HTREEITEM Tree_InsertCheck_Callback(T* CTree, 
						   HTREEITEM hParent = NULL, HTREEITEM hAfter = TVI_LAST)
{
	TVINSERTSTRUCT	tvi;

	tvi.hParent=hParent;
	tvi.hInsertAfter=hAfter;
	tvi.item.mask=TVIF_TEXT;
	tvi.item.pszText = LPSTR_TEXTCALLBACK;
	tvi.item.cchTextMax = 0;

	HTREEITEM hRes = CTree->InsertItem(&tvi);
	Tree_SetCheckState(CTree,hRes,true);

	return hRes;
}


template <class T> 
BOOL Tree_SetCheckState(T* CTree, HTREEITEM hItem, BOOL fCheck)
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

template <class T> 
BOOL Tree_GetCheckState(T* CTree, HTREEITEM hItem)
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



template <class T> 
std::vector<HTREEITEM> Tree_GetChecked(T* CTree, HTREEITEM hItem, std::vector<int>* indices = NULL)
{
	int	j;

	//CDynIntArray* a = new CDynIntArray;
	std::vector<HTREEITEM> result;

//	if (indices) *indices = new CDynIntArray;

	j=0;
	while (hItem)
	{
		if (Tree_GetCheckState(CTree,hItem)) {
	//		a->Insert((int)hItem);
			result.push_back(hItem);

			if (indices) 
				indices->push_back(j++); 
			//(*indices)->Insert(j++);
		}

		hItem = CTree->GetNextSiblingItem(hItem);
	}

	return a;
}

template <class T> 
std::vector<HTREEITEM> Tree_GetAllRootElements(T* CTree, HTREEITEM hItem)
{
	std::vector<HTREEITEM> result;
//	CDynIntArray* a = new CDynIntArray;

	while (hItem)
	{
//		a->Insert((int)hItem);
		result.push_back(hItem);

		hItem = CTree->GetNextSiblingItem(hItem);
	}

	//return a;
	return result;
}

/*
 * \brief Finds an element in a tree of TREE_ITEM_INFO elements
 *
 * \param tree the tree to look in
 * \param index index of items with ID \a id to find
 * \param id iID member of item to look for
 * \param pIndexInTree pointer to variable that receives the real index of the
 *        item in the tree
 */
template <class T> 
HTREEITEM Tree_Index2Item(T* tree, int index, int id = -1, int* pIndexInTree = NULL)
{
	int counter = 0;
	int matchCounter = 0;
	HTREEITEM hItem = tree->GetRootItem();

	while (hItem)
	{
		TREE_ITEM_INFO* itemInfo = (TREE_ITEM_INFO*)tree->GetItemData(hItem);
		if (!itemInfo)
			continue;

		if (id == -1 || itemInfo->iID == id)
		{
			// ID matches
			if (index == matchCounter)
			{
				// item was found
				if (pIndexInTree)
					*pIndexInTree = counter;
				return hItem;
			}

			matchCounter++;
		}

		counter++;
		hItem = tree->GetNextSiblingItem(hItem);
	}

	return NULL;
}

// This code is not readable and could return a hItem even if the item is not found
/*template <class T> 
HTREEITEM Tree_Index2Item(T* CTree, int iIndex, int iID = -1, int* d = NULL)
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
}*/


#endif