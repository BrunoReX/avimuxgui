#include "stdafx.h"
#include "dynarray.h"
#include "stdlib.h"

CDynArray::CDynArray()
{
	iCount = 0;
	iMaxCount = 0;
	iElementSize = 0;
}

CDynArray::~CDynArray()
{
}

int CDynArray::GetCount()
{
	return iCount;
}

CDynIntArray::CDynIntArray()
{
	iElementSize = sizeof(int);
	iCount = 0;
	pData = NULL;
}

int CDynIntArray::Insert(int iItem)
{
	if (iCount == iMaxCount) {
		iMaxCount = iMaxCount*3/2+1;
		pData = (int*)realloc(pData,iMaxCount*iElementSize);
	}

	pData[iCount++] = iItem;

	return iItem;
}

int CDynIntArray::Insert(int* iItems, int iCount)
{
	for (int i=0;i<iCount;i++) Insert(iItems[i]);

	return GetCount();
}

int CDynIntArray::At(int iIndex)
{
	if (iIndex<iCount) return pData[iIndex];
	return 0;
}

CDynIntArray* CDynIntArray::Duplicate(int count)
{
	int i;
	CDynIntArray* x = new CDynIntArray;
	if (count == -2) for (i=0;i<GetCount();x->Insert(At(i++)));
	if (count != -1) for (i=0;i<=count;x->Insert(At(i++)));
	return x;
}

int CDynIntArray::Find(int iItem)
{
	int i;

	for (i=0;i<GetCount();i++) {
		if (pData[i] == iItem) return i;
	}

	return -1;
}

void CDynIntArray::Set(int iIndex, int iItem)
{
	if (iIndex<iCount) pData[iIndex]=iItem;
	
}

void CDynIntArray::Delete(int iIndex)
{
	int c = GetCount();

	for (int i=iIndex;i<c-1;i++) {
		pData[i] = pData[i+1];
	}

	iCount--;
}

void CDynIntArray::DeleteAll()
{
	iCount = 0;
	iMaxCount = 0;
	if (pData) {
		delete pData;
		pData = NULL;
	}
}

/*CHashIntArray::CHashIntArray()
{
	iElementSize = sizeof(CDynIntArray*);
}

void CHashIntArray::SetSize(int iSize)
{
	iCount = iSize;
	iMaxCount = iSize;
	pData = (CDynIntArray**)realloc(pData,iCount*iElementSize);
}

void CHashIntArray::DeleteAll()
{
	for (int i=0;i<iCount;i++) {
		pData[i]->DeleteAll();
		delete pData[i];
	}
	if (pData) delete pData;
}*/