#include "stdafx.h"
#include "ebml.h"
#include "matroska_ids.h"
#include "integers.h"
#include "warnings.h"
#include "stdio.h"
#include "FormatInt64.h"

void DeleteElementList(EBMLELEMENTLIST** pList)
{
	for (int i=0;i<(*pList)->iCount;i++) {
		(*pList)->pElement[i]->Delete();
		delete (*pList)->pElement[i];
	}
	delete (*pList)->pElement;
	delete *pList;
	*pList=NULL;
}

void DeleteElementLists(EBMLELEMENTLIST*** pList, int iCount)
{
	for (int i=0;i<iCount;DeleteElementList(&((*pList)[i++])));
	delete *pList;
	*pList=NULL;
}

const __int64 undefined_lengths[] = {
	0xFF, 0x7FFF, 0x3FFFFF, 0x1FFFFFFF, 0x0FFFFFFFFF, 0x07FFFFFFFFFF, 0x03FFFFFFFFFFFF,
	0x01FFFFFFFFFFFFFF,

	0x7F, 0x3FFF, 0x1FFFFF, 0x0FFFFFFF, 0x07FFFFFFFF, 0x03FFFFFFFFFF, 0x01FFFFFFFFFFFF,
	0x00FFFFFFFFFFFFFF
};

const int VSizeInt_Len[256] = 
	{ 
	9,8,7,7,6,6,6,6,5,5,5,5,5,5,5,5,
	4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
 };

EBMLElement::EBMLElement()
{
	stream = NULL;
	pParent = NULL;
	bCanBeMulti = false;
	iType = 0;
	iDataType = 0;
	qwHdrPos = 0;
	buffer = NULL;
	iSearchLimit = 100000;
	ZeroMemory(&sub_elements,sizeof(sub_elements));
}

EBMLElement::EBMLElement(STREAM* s,EBMLElement* p)
{
	SetStream(s);
	SetParent(p);
	qwElementLength = s->GetSize();
	ZeroMemory(&sub_elements,sizeof(sub_elements));
}

char* EBMLElement::GetTypeString()
{
	return "";
}

CBuffer* EBMLElement::GetData()
{
	if (!buffer)
	{
		int isize;
		buffer = new CBuffer;
		buffer->SetSize(isize = (int)GetLength());
		ZeroMemory(buffer->GetData(),isize);
		GetSource()->Seek(GetStreamPos());
		GetSource()->Read(buffer->GetData(),(int)GetLength());
		buffer->IncRefCount();
	}
	return buffer;
}

void EBMLElement::SetSearchLimit(int iLimit)
{
	iSearchLimit = iLimit;
}

void EBMLElement::Delete()
{
	int i;
	for (i=0;i<sub_elements.iCount;i++) {
		delete sub_elements.pIDs[i];
	}
	if (sub_elements.pIDs) delete sub_elements.pIDs;
	if (sub_elements.piPositions) delete sub_elements.piPositions;
		
	if (buffer) DecBufferRefCount(&buffer);
}

void EBMLElement::SetMulti(bool multi)
{
	bCanBeMulti = multi;
}

void EBMLElement::SetType(int type)
{
	iType = type;
}

bool EBMLElement::CanBeMulti()
{
	return bCanBeMulti;
}

EBMLElement* EBMLElement::GetParent()
{
	return pParent;
}

STREAM* EBMLElement::GetSource()
{
	return stream;
}

void EBMLElement::SetStream(STREAM* s)
{
	stream = s;
	qwStreamPos = s->GetPos();
}

void EBMLElement::SetParent(EBMLElement *p)
{
	pParent = p;
}

int EBMLElement::GetType()
{
	return iType;
}

__int64 EBMLElement::GetRelPos()
{
	EBMLElement* p = GetParent();

	if (p) return GetStreamPos()-p->GetStreamPos();
	return GetStreamPos();
}

bool EBMLElement::HasSucc()
{
	if (GetParent())
	  return (GetRelPos()+GetLength()<min(GetParentLength(),GetSource()->GetSize()));
	  else return false;
}

EBMLElement* EBMLElement::GetSucc()
{
	EBMLElement* p=NULL;
	if (HasSucc()) {
		GetSource()->Seek(GetStreamPos()+GetLength());
		GetParent()->Create(&p);
	}
	return p;
}

bool Comp_EBMLIDs(char* ID1,char* ID2)
{
	int l1 = VSizeInt_Len[(unsigned char)ID1[0]];
	int l2 = VSizeInt_Len[(unsigned char)ID2[0]];
	if (l1!=l2) return false;

	while (l1--) {
		if (ID2[l1]!=ID1[l1]) return false;
	}
	return true;

}

bool EBMLElement::CompIDs(char* ID1,char* ID2)
{
	int l1 = GetIDLength(ID1);
	int l2 = GetIDLength(ID2);
	if (l1!=l2) return false;

	while (l1--) {
		if (ID2[l1]!=ID1[l1]) return false;
	}
	return true;

}

EBMLElement::~EBMLElement()
{
	return;
}


__int64 EBMLElement::GetStreamPos()
{
	return qwStreamPos;
}

int EBMLElement::GetLevel()
{
	return (GetParent()?GetParent()->GetLevel()+1:-1);
}

int EBMLElement::GetIDLength(char* _ID)
{
	char* id = _ID;
	if (!id) return 0;
	unsigned char  c=id[0];
	return VSizeInt_Len[c];	
}

__int64 EBMLElement::ReadLength()
{
	unsigned char  c,i,j;
	unsigned char	cSize[8];
	int  iSizeLen=1;
	__int64 qwElementLength = 0;
	j=0;
	
	GetSource()->Read(&c,1);

	i = iSizeLen = VSizeInt_Len[c];
	qwElementLength = c;

	if (iSizeLen > 8) {
		i = iSizeLen = 8;
	}

	if (iSizeLen>1) {
		GetSource()->Read(cSize,i-1);
		int j=0;
		while (--iSizeLen) {
			qwElementLength = qwElementLength * 256 + cSize[j++];
		}
	}

	if (qwElementLength == undefined_lengths[i-1]) {
		Warning("read undefined length", GetSource()->GetPos() - i);
		j=1;
	}

	*(((BYTE*)&qwElementLength)+i-1) &= ~(1 << (8-i));

	if (!qwElementLength && !j) {
		Warning("element size of 0 encountered", GetSource()->GetPos()-1);
	}

	return qwElementLength;
}

void EBMLElement::DetermineLength()
{
	qwElementLength = ReadLength();
	qwStreamPos = GetSource()->GetPos();
}

__int64 EBMLElement::GetLength()
{
	return qwElementLength;
}

__int64 EBMLElement::GetParentLength()
{
	return (GetParent()?GetParent()->GetLength():GetSource()?GetSource()->GetSize():0);
}


bool EBMLElement::CheckIDs(char* iID,EBMLElement** p)
{
	if (CompIDs(iID,(char*)EID_EBML)) {
		*p = (EBMLElement*)new EBML_EBML(GetSource(),this);
	} else return false;

	return true;
}

bool EBMLElement::CheckGlobalIDs(char* iID,EBMLElement** p)
{
	return false;
}

void EBMLElement::SetElementLength(__int64 size)
{
	qwElementLength = size;
}

// allocate memory for *iID
int EBMLElement::ReadID(char** iID)
{
	if (*iID) free (*iID);
	unsigned char c,d;
	int	 iLen=1;
	STREAM* s = GetSource();

	if (!s->Read(&c,1)) {
		printf("Fehler beim Lesen!\n");
		*iID = (char*)calloc(sizeof(char),6);
		(*iID)[0] = 10; (*iID)[1] = 0; (*iID)[2] = 0; (*iID)[3] = 0; (*iID)[4] = 0;
		return 0;
	}

	d=c;
	iLen = VSizeInt_Len[c];

	if (iLen>4) {
		B0rked("1st byte of EBML-ID indicates size larger than 4", GetSource()->GetPos());
	}

	*iID = (char*)calloc(sizeof(char),iLen+1);
	(*iID)[0]=d;

	if (iLen>1) {
		s->Read(&((*iID)[1]),iLen-1);
	}

	return iLen;
}

// do NOT allocate memory
int EBMLElement::ReadID(char* iID)
{
	unsigned char c,d;
	int	 iLen=1;
	STREAM* s = GetSource();

	if (!s->Read(&c,1)) {
		printf("Fehler beim Lesen!\n");
		iID[0] = 10; iID[1] = 0; iID[2] = 0; iID[3] = 0; iID[4] = 0;

		return 0;
	}

	d=c;
	iLen = VSizeInt_Len[c];
	*iID = d;
	if (iLen>4) {
		B0rked("1st byte of EBML-ID indicates size larger than 4");
	}

	if (iLen>1) {
		s->Read(&((iID)[1]),iLen-1);
	}

	return iLen;
}

__int64 EBMLElement::GetAbsoluteHeaderPos()
{
	return qwHdrPos;
}

__int64 EBMLElement::GetRelativeHeaderPos()
{
	return qwHdrPos-(GetParent()?GetParent()->GetStreamPos():0);
}

int EBMLElement::GetHeaderSize()
{
	return (int)(GetStreamPos () - GetAbsoluteHeaderPos());
}

// create first sub element of a parent element
EBMLElement* EBMLElement::Create1stSub(EBMLElement** p,char* ID)
{
	if (!GetLength()) {
		*p = NULL;
		return NULL;
	}
	GetSource()->Seek(GetStreamPos());
	return Create(p,ID);
}

// create current sub element and supply the ID or read it from the file
EBMLElement* EBMLElement::Create(EBMLElement** p,char* ID)
{
	char* iID = NULL; bool bAlloced=false; __int64 i;
	if (ID) {
		iID=ID;
		i = GetSource()->GetPos() - GetIDLength(ID);
	} else {
		i = GetSource()->GetPos();
		if (!ReadID(&iID)) {
			*p = NULL;
			return *p;
		}
		bAlloced=true; 
	}
	if (!CheckIDs(iID,p) && !CheckGlobalIDs(iID,p)) {
		*p = new EBML_UNKNOWN(GetSource(),this);
	}
	(*p)->SetHeaderPos(i);
	if (bAlloced) delete iID;

	return *p;
}

// create current sub element, always read ID from the file, and return it in *ID. Do not allocate
EBMLElement* EBMLElement::Create(EBMLElement** p,char** ID)
{
	char* iID=NULL; bool bAlloced=false;
	__int64 i = GetSource()->GetPos();
	ReadID(*ID);
	iID = *ID;

	if (!CheckIDs(iID,p) && !CheckGlobalIDs(iID,p)) {
		*p = new EBML_UNKNOWN(GetSource(),this);
	}
	(*p)->SetHeaderPos(i);

	return *p;
}

void EBMLElement::SetHeaderPos(__int64 p)
{
	qwHdrPos = p;
}

int EBMLElement::GetSearchLimit()
{
	return iSearchLimit;
}

int EBMLElement::SeekStream(__int64 qwPos)
{
	return GetSource()->Seek(qwPos + GetStreamPos());
}

int EBMLElement::Search(void** dest,char* iID2Find,char* iID2stopAt)
{
	__int64		qwLen;
	__int64		qwCurrentPos = GetStreamPos();
	EBMLELEMENTLIST** elements = (EBMLELEMENTLIST**)dest;
	int				iMaxCurrSize = 0;
	GetSource()->Seek(GetStreamPos());
	char*			ID=NULL;
	bool			bStop = false;


	if (*elements) DeleteElementList(elements);
	(*elements) = (EBMLELEMENTLIST*)calloc(1,sizeof(EBMLELEMENTLIST));

	if (GetLength()) do
	{
		if (!ReadID(&ID)) return 0;
		if (CompIDs(ID,iID2Find)) {
			if ((*elements)->iCount==iMaxCurrSize) {
				(*elements)->pElement = (EBMLElement**)realloc(
					(*elements)->pElement,sizeof(EBMLElement*)*(iMaxCurrSize+=10));
			}
			qwLen=Create(&(*elements)->pElement[(*elements)->iCount++],ID)->GetLength();
		}
		else {
			qwLen=ReadLength();
		}
		if (CompIDs(ID,iID2stopAt)) {
			bStop = true;
		}
		GetSource()->Seek(GetSource()->GetPos()+qwLen);
	}
	while (!bStop && GetSource()->GetPos()-qwCurrentPos<GetLength() && (*elements)->iCount < GetSearchLimit());
	delete ID;

	return !!(*elements)->iCount;
}

int EBMLElement::SearchMulti(void*** dest, char **iIDs, int iIDCount, char* iID2stopAt)
{
	__int64				qwLen;
	int					i;
	__int64				qwCurrentPos = GetStreamPos();
	EBMLELEMENTLIST*	 elements = NULL;
	int*				iMaxCurrSize = 0;
	GetSource()->Seek(GetStreamPos());
	char*				ID=NULL;
	int					iIndex = -1;
	bool				bStop = false;

	if (*dest) delete *dest;
	*dest = (void**)new EBMLELEMENTLIST*[iIDCount];
	for (i=0;i<iIDCount;(*dest)[i++] = (EBMLELEMENTLIST*)calloc(1,sizeof(EBMLELEMENTLIST)));
	iMaxCurrSize = (int*)calloc(sizeof(int),iIDCount);

	if (GetLength()) do
	{
		__int64 q = GetSource()->GetPos();
		if (!ReadID(&ID)) return 0;

		// find index
		iIndex = -1;
		for (i=0;i<iIDCount;i++) {
			if (CompIDs(ID, iIDs[i])) {
				iIndex = i;
			}
		}

		if (iIndex!=-1) {
			elements = (EBMLELEMENTLIST*)((*dest)[iIndex]);
			if ((elements)->iCount==iMaxCurrSize[iIndex]) {
				(elements)->pElement = (EBMLElement**)realloc(
					(elements)->pElement,sizeof(EBMLElement*)*(iMaxCurrSize[iIndex]+=10));
			}
			qwLen=Create(&(elements)->pElement[(elements)->iCount++],ID)->GetLength();
		}
		else {
			qwLen=ReadLength();
		}
		if (CompIDs(ID,iID2stopAt)) {
			bStop = true;
		}

		GetSource()->Seek(GetSource()->GetPos()+qwLen);
	}
	while (GetSource()->GetPos()-qwCurrentPos<GetLength() && !bStop);

	delete ID;
	delete iMaxCurrSize;

	if (!elements) return 0;
	return !!(elements)->iCount;
}

EBMLElement* EBMLElement::FindNext(char* iID2Find)
{
	__int64		qwLen;
	__int64		qwCurrentPos = GetStreamPos();
	GetSource()->Seek(GetStreamPos()+GetLength());
	bool			bStop = false;
	EBMLElement*	res = NULL;
	char*			ID = NULL;

	do
	{
		ReadID(&ID);
		if (CompIDs(ID,iID2Find)) {
			qwLen=GetParent()->Create(&res,ID)->GetLength();
		}
		else {
			qwLen=ReadLength();
		}
		GetSource()->Seek(GetSource()->GetPos()+qwLen);
	}
	while (GetSource()->GetPos()-qwCurrentPos<GetLength() && !res);
	delete ID;
	return res;
}

EBMLElement* EBMLElement::Resync(EID_DESCRIPTOR* pDescr, int iDescrCount) // not yet implemented
{
	EBMLElement*	res = NULL;

	unsigned __int32 id;

	GetSource()->Read(((char*)&id)+1, 3);

	while (GetSource()->Read(&id, 1)) {
		for (int j = 0; j<iDescrCount; j++) {
			if (CompIDs((char*)&id, pDescr[j].cID)) {
				return Create(&res,(char*)&id);
			}
			id <<= 8;
		}
	}


	return res;
}

void EBMLElement::SetDataType(int data_type)
{
	iDataType = data_type;
}

bool EBMLElement::IsMaster()
{
	return !!(iDataType==EBMLDATATYPE_MASTER);
}

int EBMLElement::GetDataType()
{
	return iDataType;
}

__int64 EBMLElement::AsInt()
{
	return (GetData()->AsInt());
}

__int64 EBMLElement::AsSInt()
{
	return 0;
}

double EBMLElement::AsFloat()
{
	return (GetData()->AsBSWFloat());
}

////////////////
// unbekannte //
////////////////

EBML_UNKNOWN::EBML_UNKNOWN(STREAM* s,EBMLElement* p)
{
	SetStream(s);
	SetParent(p);
	DetermineLength();
	SetType(EBMLTYPE_UNKNOWN);
	SetDataType(EBMLDATATYPE_BIN);
	return;
}

bool EBML_UNKNOWN::CheckIDs(char* iID,EBMLElement** p)
{
	return false;
}

char* EBML_UNKNOWN:: GetTypeString()
{
	return "UNKNOWN";
}



		//////////
		// EBML //
		//////////

EBML_EBML::EBML_EBML(STREAM* s,EBMLElement* p)
{
	SetStream(s);
	SetParent(p);
	DetermineLength();
	SetMulti(true);
	SetType(EBMLTYPE_EBML);
	SetDataType(EBMLDATATYPE_MASTER);
	return;
}

bool EBML_EBML::CheckIDs(char* iID,EBMLElement** p)
{
	if (CompIDs(iID,(char*)EID_EBMLVersion)) {
		*p = (EBMLElement*)new EBML_EBMLVersion(GetSource(),this);
	} else
	if (CompIDs(iID,(char*)EID_DocType)) {
		*p = (EBMLElement*)new EBML_DocType(GetSource(),this);
	} else 
	if (CompIDs(iID,(char*)EID_EBMLReadVersion)) {
		*p = (EBMLElement*)new EBML_EBMLReadVersion(GetSource(),this);
	} else		
	if (CompIDs(iID,(char*)EID_EBMLMaxIDLength)) {
		*p = (EBMLElement*)new EBML_EBMLMaxIDLength(GetSource(),this);
	} else		
	if (CompIDs(iID,(char*)EID_EBMLMaxSizeLength)) {
		*p = (EBMLElement*)new EBML_EBMLMaxSizeLength(GetSource(),this);
	} else		
	if (CompIDs(iID,(char*)EID_DocTypeVersion)) {
		*p = (EBMLElement*)new EBML_DocTypeVersion(GetSource(),this);
	} else		
	if (CompIDs(iID,(char*)EID_DocTypeReadVersion)) {
		*p = (EBMLElement*)new EBML_DocTypeReadVersion(GetSource(),this);
	} else		

		return false;
	return true;
}

char* EBML_EBML:: GetTypeString()
{
	return "EBML";
}

		/////////////////
		// EBMLVersion //
		/////////////////

EBML_EBMLVersion::EBML_EBMLVersion(STREAM* s,EBMLElement* p)
{
	SetStream(s);
	SetParent(p);
	DetermineLength();
	SetMulti(false);
	SetType(EBMLTYPE_EBMLVersion);
	SetDataType(EBMLDATATYPE_INT);
	return;
}

char* EBML_EBMLVersion::GetTypeString()
{
	return "EBMLVersion";
}

		/////////////////////
		// EBMLReadVersion //
		/////////////////////


EBML_EBMLReadVersion::EBML_EBMLReadVersion(STREAM* s,EBMLElement* p)
{
	SetStream(s);
	SetParent(p);
	DetermineLength();
	SetMulti(false);
	SetType(EBMLTYPE_EBMLReadVersion);
	SetDataType(EBMLDATATYPE_INT);
	return;
}

char* EBML_EBMLReadVersion::GetTypeString()
{
	return "EBMLReadVersion";
}

		/////////////////////
		// EBMLMaxIDLength //
		/////////////////////

EBML_EBMLMaxIDLength::EBML_EBMLMaxIDLength(STREAM* s,EBMLElement* p)
{
	SetStream(s);
	SetParent(p);
	DetermineLength();
	SetMulti(false);
	SetType(EBMLTYPE_EBMLMaxIDLength);
	SetDataType(EBMLDATATYPE_INT);
	return;
}

char* EBML_EBMLMaxIDLength::GetTypeString()
{
	return "EBMLMaxIDLength";
}

		///////////////////////
		// EBMLMaxSizeLength //
		///////////////////////

EBML_EBMLMaxSizeLength::EBML_EBMLMaxSizeLength(STREAM* s,EBMLElement* p)
{
	SetStream(s);
	SetParent(p);
	DetermineLength();
	SetMulti(false);
	SetType(EBMLTYPE_EBMLMaxSizeLength);
	SetDataType(EBMLDATATYPE_INT);
	return;
}

char* EBML_EBMLMaxSizeLength::GetTypeString()
{
	return "EBMLMaxSizeLength";
}


		/////////////////
		// EBMLDocType //
		/////////////////

EBML_DocType::EBML_DocType(STREAM* s,EBMLElement* p)
{
	SetStream(s);
	SetParent(p);
	DetermineLength();
	SetMulti(false);
	SetType(EBMLTYPE_DOCTYPE);
	SetDataType(EBMLDATATYPE_ASCII);
	return;
}

char* EBML_DocType:: GetTypeString()
{
	return "DocType";
}

		////////////////////////
		// EBMLDocTypeVersion //
		////////////////////////

EBML_DocTypeVersion::EBML_DocTypeVersion(STREAM* s,EBMLElement* p)
{
	SetStream(s);
	SetParent(p);
	DetermineLength();
	SetMulti(false);
	SetType(EBMLTYPE_DOCTYPEVERSION);
	SetDataType(EBMLDATATYPE_INT);
	return;
}

char* EBML_DocTypeVersion:: GetTypeString()
{
	return "DocTypeVersion";
}


		////////////////////////////
		// EBMLDocTypeReadVersion //
		////////////////////////////

EBML_DocTypeReadVersion::EBML_DocTypeReadVersion(STREAM* s,EBMLElement* p)
{
	SetStream(s);
	SetParent(p);
	DetermineLength();
	SetMulti(false);
	SetType(EBMLTYPE_DOCTYPEREADVERSION);
	SetDataType(EBMLDATATYPE_INT);
	return;
}


char* EBML_DocTypeReadVersion:: GetTypeString()
{
	return "DocTypeReadVersion";
}

