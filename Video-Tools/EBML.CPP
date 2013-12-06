#include "stdafx.h"
#include "ebml.h"
#include "matroska_ids.h"
#include "integers.h"
#include "warnings.h"
#include "stdio.h"
#include "FormatInt64.h"

#ifdef DEBUG_NEW
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif


int		bCRC32AutoCheck = 0;
int		iCRC32MinLevel = 1;

int EBMLUIntLen(__int64 x)
{
	if (x < 0x7F) return 1;
	if (x < 0x3FFF) return 2;
	if (x < 0x1FFFFF) return 3;
	if (x < 0x0FFFFFFF) return 4;
	if (x < 0x07FFFFFFFF) return 5;
	if (x < 0x03FFFFFFFFFF) return 6;
	if (x < 0x01FFFFFFFFFFFF) return 7;
	if (x < 0x00FFFFFFFFFFFFFF) return 8;
	return 9;
}

int UIntLen(__int64 x)
{
	if (x <= 0xFF) return 1;
	if (x <= 0xFFFF) return 2;
	if (x <= 0xFFFFFF) return 3;
	if (x <= 0xFFFFFFFF) return 4;
	if (x <= 0xFFFFFFFFFF) return 5;
	if (x <= 0xFFFFFFFFFFFF) return 6;
	if (x <= 0xFFFFFFFFFFFFFF) return 7;
	return 8;
}

EBML_INFO::EBML_INFO()
{
	EBMLVersion = 1;
	EBMLReadVersion = 1;
	EBMLMaxIDLengh = 4;
	EBMLMaxSizeLength = 8;
	DocTypeVersion = 1;
	DocTypeReadVersion = 1;
	DocType = NULL;
}

EBML_INFO::~EBML_INFO()
{
	if (DocType)
		free(DocType);
}

void EBML_EnableCRC32AutoCheck(int bEnable)
{
	bCRC32AutoCheck = bEnable;
}

void EBML_SetCRC32AutoCheck_MinLevel(int min_level)
{
	iCRC32MinLevel = min_level;
}

int EBMLElement::AutoCheckCRC32()
{
	if (bCRC32AutoCheck) {
		if (GetLevel() >= iCRC32MinLevel && IsMaster()) {
			__int64 j = GetSource()->GetPos();
			int check = CheckCRC();

			if (check == EBML_CRC_FAILED) {
				char cTxt[128]; cTxt[0]=0;sprintf(cTxt, "CRC check failed on element %s", GetTypeString());
				B0rked(cTxt, GetStreamPos() - GetHeaderSize());
			}
			GetSource()->Seek(j);
		}
	}

	return EBML_CRC_NOT_CHECKED;
}

void DeleteElementList(EBMLELEMENTLIST** pList)
{
	if (!pList || !*pList)
		return;

	for (size_t i=0;i<(*pList)->iCount;i++) {
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
	iCRCcalculated = 0;
	iCRC = 0;
	bLengthUndefined = -1;
	ZeroMemory(&sub_elements,sizeof(sub_elements));
}

EBMLElement::EBMLElement(STREAM* s,EBMLElement* p)
{
	SetStream(s);
	SetParent(p);
	qwElementLength = s->GetSize();
	ZeroMemory(&sub_elements,sizeof(sub_elements));
	bLengthUndefined = -1;
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
	if (IsLengthUndefined())
		return false;

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
	
//	bLengthUndefined  = -1;
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
		if (bLengthUndefined == -1)
			bLengthUndefined = 1;
		Warning("read undefined length", GetSource()->GetPos() - i);
		j=1;
	} else
		if (bLengthUndefined == -1)
			bLengthUndefined = 0;

	*(((BYTE*)&qwElementLength)+i-1) &= ~(1 << (8-i));

	if (!qwElementLength && !j) {
		Warning("element size of 0 encountered", GetSource()->GetPos()-1);
	}

	return qwElementLength;
}

bool EBMLElement::IsLengthUndefined()
{
	if (bLengthUndefined == -1) {
		DetermineLength();
		return IsLengthUndefined();
	}

	return !!bLengthUndefined;
}

void EBMLElement::DetermineLength()
{
	qwElementLength = ReadLength();
	qwStreamPos = GetSource()->GetPos();
}

__int64 EBMLElement::GetLength()
{
/*	if (IsLengthUndefined()) {
		return (__int64)INT_MAX * INT_MAX;
	} else*/
	return qwElementLength;
}

__int64 EBMLElement::GetParentLength()
{
	if (!GetParent())
		return 0;

	if (GetParent()->IsLengthUndefined())
		return GetParent()->GetSource()->GetSize();

	return (GetParent()->GetLength());
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
	//	printf("Could not read element ID!\n");
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
//		printf("Could not read element ID!\n");
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

	(*p)->AutoCheckCRC32();

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

	(*p)->AutoCheckCRC32();

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
	bool			bStop = false;
	char*			ID=NULL;


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
	while (!bStop && 
		    ( (GetSource()->GetPos()-qwCurrentPos<GetLength()) || IsLengthUndefined()) && 
		    (*elements)->iCount < GetSearchLimit() &&
		   !GetSource()->IsEndOfStream());
	delete ID;

	return !!(*elements)->iCount;
}

int	EBMLElement::Search(std::vector<EBMLElement*> &dest,char* iID2Find,char* iID2stopAt)
{
	char*		ID=NULL;
	__int64		qwLen;
	bool		bStop = false;
	__int64		qwCurrentPos = GetStreamPos();

	GetSource()->Seek(GetStreamPos());

	if (GetLength()) do
	{
		if (!ReadID(&ID)) return 0;
		if (CompIDs(ID,iID2Find)) {
			EBMLElement* element = NULL;
			qwLen = Create(&element, ID)->GetLength();
			dest.push_back(element);
		} else {
			qwLen=ReadLength();
		}
		
		if (CompIDs(ID,iID2stopAt)) 
			bStop = true;
		
		GetSource()->Seek(GetSource()->GetPos()+qwLen);
	}
	while (!bStop && 
		    ( (GetSource()->GetPos()-qwCurrentPos<GetLength()) || IsLengthUndefined()) && 
		    dest.size() < GetSearchLimit() &&
		   !GetSource()->IsEndOfStream());

	delete ID;

	return !!dest.size();
}

int EBMLElement::Search(std::vector<EBMLElement*> &dest,char* iID2Find,char* iID2stopAt, int occurence, __int64* pTarget)
{
	char* ids[] = { iID2Find, NULL };
	void* targets[] = { pTarget, NULL };
	int occ_restr[] = { occurence, 0 };

	SEARCHMULTIEX sme = { ids, targets, occ_restr };

	EBMLElementVectors result;

	int r = SearchMulti(result, sme, iID2stopAt);

	dest = result[0];
	
	return r;
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
		for (i=0;i<iIDCount && iIndex == -1;i++) {
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
			if ((elements)->pElement[(elements)->iCount-1]->IsLengthUndefined())
				qwLen = (__int64)INT_MAX * INT_MAX;

		}
		else {
			qwLen=ReadLength();
		}
		if (CompIDs(ID,iID2stopAt)) {
			bStop = true;
		}

		GetSource()->Seek(GetSource()->GetPos()+qwLen);
	}
	while (!bStop &&
		   !GetSource()->IsEndOfStream() &&
	   		(GetSource()->GetPos()-qwCurrentPos<GetLength() || IsLengthUndefined())
		   );

	delete ID;
	delete iMaxCurrSize;

	if (!elements) return 0;
	return !!(elements)->iCount;
}

int EBMLElement::SearchMulti(std::vector<std::vector<EBMLElement*> > &dest, char** ids, char* iID2stopAt)
{
	__int64				qwLen;
	__int64				qwCurrentPos = GetStreamPos();
	GetSource()->Seek(GetStreamPos());
	char*				ID=NULL;
	int					iIndex = -1;
	int					i;
	bool				bStop = false;
	
	// create number of entries
	int count = 0;
	while (ids[count]) {
		std::vector<EBMLElement*>	list;
		dest.push_back(list);
		count++;
	}
		
	//
	if (GetLength()) do
	{
		__int64 q = GetSource()->GetPos();
		if (!ReadID(&ID)) return 0;

		// find index
		iIndex = -1;
		for (i=0;i<count && iIndex == -1;i++) {
			if (CompIDs(ID, ids[i])) {
				iIndex = i;
			}
		}

		if (iIndex!=-1) {
			EBMLElement* element = NULL;
			qwLen = Create(&element, ID)->GetLength();
			dest[iIndex].push_back(element);
		}
		else {
			qwLen=ReadLength();
		}
		
		if (CompIDs(ID,iID2stopAt))
			bStop = true;

		GetSource()->Seek(GetSource()->GetPos()+qwLen);
	}
	while (!bStop &&
		   !GetSource()->IsEndOfStream() &&
   			(GetSource()->GetPos()-qwCurrentPos<GetLength() || IsLengthUndefined())
		   );

	delete ID;

	return count;
}

int EBMLID2string(char* id, char* dest)
{
	int len = VSizeInt_Len[(unsigned char)id[0]];
	dest[0]=0;

	for (int i=0;i<len;i++) {
		sprintf(dest," %02X", id[i] & 0xFF);
		dest+=strlen(dest);
	}

	return 1;		
}

int EBMLElement::SearchMulti(std::vector<std::vector<EBMLElement*> > &dest, SEARCHMULTIEX& search, char* iID2stopAt)
{
	int error = 0;
	int count = SearchMulti(dest, search.ids, iID2stopAt);

	if (!count)
		return count;

	if (search.dests) {
		for (int i=0;i<count;i++) {
			if (dest[i].size() && search.dests[i]) {
				if (dest[i][0]->GetDataType() == EBMLDATATYPE_INT ||
					dest[i][0]->GetDataType() == EBMLDATATYPE_HEX) {

					if (dest[i][0]->GetLength() > 9)
						memcpy(search.dests[i], dest[i][0]->AsString(), 16);
					else {
						unsigned __int64 v = dest[i][0]->AsInt();
						if (v > UINT_MAX) 
							* ((__int64*)search.dests[i]) = dest[i][0]->AsInt();
						else
							* ((int*)search.dests[i]) = dest[i][0]->AsInt();
					}
				} else
				if (dest[i][0]->GetDataType() == EBMLDATATYPE_FLOAT) 
					* ((double*)search.dests[i]) = dest[i][0]->AsFloat();
				else
				if (dest[i][0]->GetDataType() == EBMLDATATYPE_UTF8 ||
					dest[i][0]->GetDataType() == EBMLDATATYPE_ASCII) {
					if (*((char**)search.dests[i])) {
						memcpy(*((char**)search.dests[i]), dest[i][0]->GetData(), dest[i][0]->GetLength());
						*((char**)search.dests[i])[strlen((char*)*((char**)search.dests[i]))] = 0;
					} else {
						*((char**)search.dests[i]) = _strdup(dest[i][0]->AsString());
					}
				}
					

			}
		}
	}

	if (search.occ) {
		for (int i=0;i<count;i++) {
			if ((search.occ[i] & 1) && (dest[i].empty())) { // mandatory element missing
				char c[256]; c[0]=0;
				char id[32]; id[0]=0;
				EBMLID2string(search.ids[i], id);
				sprintf(c, "Missing mandatory element %s in %s", id, GetTypeString());
				B0rked(c, GetStreamPos());
				error = 1;
			}
			if ((search.occ[i] & 4) && (dest[i].empty())) { // almost-mandatory element missing
				char c[256]; c[0]=0;
				char id[32]; id[0]=0;
				EBMLID2string(search.ids[i], id);
				sprintf(c, "Element %s should be present in %s, but wasn't found", id, GetTypeString());
				Warning(c, GetStreamPos());
				error = 1;
			}
			if ((search.occ[i] & 2) && (dest[i].size() > 1)) { // unique element more than once
				char c[256]; c[0]=0;
				char id[32]; id[0]=0;
				EBMLID2string(search.ids[i], id);
				sprintf(c, "Multiple occurences of unique element %s in %s", id, GetTypeString());
				B0rked(c, GetStreamPos());
			}
		}
	}

	if (error)
		return EBML_MANDATORY_ELEMENT_NOT_FOUND;

	return count;
}



EBMLElement* EBMLElement::FindNext(char* iID2Find)
{
	__int64		qwLen;
	__int64		qwCurrentPos = GetStreamPos();
	GetSource()->Seek(GetStreamPos()+GetLength());
	bool			bStop = false;
	EBMLElement*	res = NULL;
	char*			ID = NULL;

	__int64 parent_size = (GetParent())?GetParent()->GetLength():0;

	do
	{
		if (!ReadID(&ID))
			return NULL;

		if (CompIDs(ID,iID2Find)) {
			qwLen=GetParent()->Create(&res,ID)->GetLength();
		}
		else {
			qwLen=ReadLength();
		}
		GetSource()->Seek(GetSource()->GetPos()+qwLen);
	}
	while (GetSource()->GetPos()-qwCurrentPos<parent_size && !res);
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

char* EBMLElement::AsString()
{
	return GetData()->AsString();
}

__int64 EBMLElement::AsSInt()
{
	return 0;
}

double EBMLElement::AsFloat()
{
	return (GetData()->AsBSWFloat());
}

// perform a CRC check on an element
int EBMLElement::CheckCRC()
{
	EBMLELEMENTLIST* e_CRC = NULL;
	EBMLElement* pCRC;

	if (!iCRCcalculated) {
		iCRCcalculated = 1;
		Search((void**)&e_CRC, (char*)MID_CRC32, (char*)MID_CRC32);
		if (!e_CRC->iCount)
			return (iCRCresult = EBML_CRC_NOT_PRESENT);

		pCRC = e_CRC->pElement[0];
		if (pCRC->GetLength() != 4)
			return (iCRCresult = EBML_CRC_NOT_CRC32);

		if (pCRC->GetRelPos() != pCRC->GetHeaderSize()) 
			return (iCRCresult = EBML_CRC_NOT_CHECKED);
	
		int i = !!buffer;
		unsigned __int32 crc = CRC32((unsigned char*)(GetData()->AsString()+6), (int)(GetLength()-6));
		memcpy(&iCRC, pCRC->GetData()->GetData(), 4);

//		CBuffer* b = GetData();
		if (!i) DecBufferRefCount(&buffer);

		if (crc != iCRC)
			return (iCRCresult = EBML_CRC_FAILED);

		DeleteElementList(&e_CRC);

		return (iCRCresult = EBML_CRC_OK);
	} else
		return iCRCresult;
}

// returns true if the element is definitely OK
int EBMLElement::Verify()
{
	return (CheckCRC() == EBML_CRC_OK || (GetParent() && GetParent()->Verify()));
}
#define newz(a,b,c) c=new a[b]; ZeroMemory(c,b*sizeof(a))
// pointer to list and element description
int EBMLElement::InsertElement(void** _e, EBMLElement* seg, char* ID, EBMLElement* pos)
{
	EBMLELEMENTLIST** e = (EBMLELEMENTLIST**)_e;
	char* created_id;
	created_id = new char[5];
	ZeroMemory(created_id,sizeof(created_id));

	if (!*e) {
		newz(EBMLELEMENTLIST, 1, *e);
	}
	seg->SeekStream(pos->AsInt());
	(*e)->pElement = (EBMLElement**)realloc((*e)->pElement,
		((*e)->iCount+1)*sizeof(EBMLElement));

	seg->Create(&(*e)->pElement[(*e)->iCount++], (char**)&created_id);

	if (pos->CompIDs(ID, (char*)created_id)) {
		delete created_id;
		return 1;
	} else {
		delete created_id;
		(*e)->iCount--;
		return -1;
	}
}

// pointer to list and element description
int EBMLElement::InsertElement(EBMLElementVector& e, EBMLElement* seg, char* ID, EBMLElement* pos)
{
	char* created_id;
	created_id = new char[5];
	ZeroMemory(created_id,sizeof(created_id));

	seg->SeekStream(pos->AsInt());

	EBMLElement* element;

	seg->Create(&element, (char**)&created_id);

	if (pos->CompIDs(ID, (char*)created_id)) {
		delete created_id;
		e.push_back(element);
		return 1;
	} else {
		delete created_id;
		return -1;
	}
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

EBMLElement* EBMLELEMENTLIST::operator ()(int index)
{
	if (index < 0 || index >= iCount)
		return 0;

	return pElement[index];
}

__int64 EBMLELEMENTLIST::operator [](int index)
{
	if (index == -1)
		return iCount;

	if (index < 0 || index >= iCount)
		return -1;

	return pElement[index]->AsInt();
}

__int64 EBMLELEMENTLIST::operator ^(__int64 _default)
{
	if (iCount == 0)
		return _default;


	return pElement[0]->AsInt();
}

bool EBMLElement::operator <(EBMLElement& first)
{
	return GetStreamPos() < first.GetStreamPos();
}