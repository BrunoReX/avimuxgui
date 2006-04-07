/*



*/

#include "stdafx.h"
#include "Buffers.h"
#include "stdio.h"
#include "memory.h"
#include "stdlib.h"
#include "string.h"
#include "matroska.h"

void reverse(char* source, char* dest, int iCount)
{
	for (int i=0;i<iCount;i++) {
		dest[iCount-i-1] = source[i];
	}
}

CBuffer::CBuffer()
{
	iRefCount=0;
	iSize=0;
	lpData=NULL;
	iAllocedSize=0;
}

CBuffer::CBuffer(int _iSize, void* _lpData, int iFlags)
{
	iRefCount = 0;
	iSize=0;
	lpData=NULL;
	iAllocedSize=0;
	SetSize(_iSize);
	if (_lpData) SetData(_lpData);
	if (iFlags & CBN_REF1) IncRefCount();
}

CBuffer::~CBuffer()
{
}

int CBuffer::GetSize(void)
{
	return iSize;
}

void CBuffer::SetSize(int _iSize)
{
	if ((iSize<_iSize) || (iSize > 2*_iSize)) {
		if (iSize && lpData) delete lpData;
		lpData= new char[_iSize+1];
		((BYTE*)lpData)[_iSize] = 0;
		iSize=_iSize;
		iAllocedSize=iSize;
	}
}

void CBuffer::IncRefCount()
{
	iRefCount++;
}

void CBuffer::SetData(void* lpsource)
{
	memcpy(GetData(),lpsource,GetSize());
}

bool CBuffer::DecRefCount()
{
	iRefCount--;
	if (!iRefCount) {
		delete lpData;
		lpData = NULL;
		iSize=0;
	}
	return !!iRefCount;
}

void* CBuffer::GetData()
{
	return lpData;
}

char* CBuffer::AsString()
{
	if (GetData()) return (char*)GetData();
	return "";
}

int CBuffer::AsInt8()
{
	return *((__int8*)GetData());
}

int CBuffer::AsInt16()
{
	return *((__int16*)GetData());
}

int CBuffer::AsInt24()
{
	int i=0;
	memcpy(&i,GetData(),3);
	return i;
}

int CBuffer::AsInt32()
{
	return *((__int32*)GetData());
}

__int64 CBuffer::AsInt40()
{
	__int64 i=0;
	memcpy(&i,GetData(),5);
	return i;
}

__int64 CBuffer::AsInt48()
{
	__int64 i=0;
	memcpy(&i,GetData(),6);
	return i;
}
__int64 CBuffer::AsInt56()
{
	__int64 i=0;
	memcpy(&i,GetData(),7);
	return i;
}

__int64 CBuffer::AsSBSWInt()
{
	__int64 i = AsBSWInt();
	char*	 c = (char*)&i;
	int		 l = GetSize();
	int		 s = !! (c[l-1] & (1<<(7)));
	
	if (s) {
		for (int j=l-1;j>=0;j--) (c[l-1] ^= 0xFF);
		return -i-1;
	} else return i;

	return 0;
}

__int64 CBuffer::AsInt()
{
	switch (GetSize())
	{
		case 1: return AsInt8();
		case 2: return AsInt16();
		case 3: return AsInt24();
		case 4: return AsInt32();
		case 5: return AsInt40();
		case 6: return AsInt48();
		case 7: return AsInt56();
		case 8: return AsInt64();
	}
	return 0;
}

__int64 CBuffer::AsBSWInt()
{
	unsigned char* d = (unsigned char*)GetData();
	__int64	result = 0;
	for (int i=0;i<GetSize();i++)
	{
		result = result*256 + *d++;
	}
	return result;
}

__int64 CBuffer::AsInt64()
{
	return *((__int64*)GetData());
}

float CBuffer::AsFloat()
{
	int	d;
	d = (int)AsInt();
	return *((float*)&d);
}

double CBuffer::AsBSWFloat()
{
	if (GetSize()==4) {
		int d;
		reverse((char*)GetData(), (char*)&d, 4);
		return *((float*)&d);
	}

	if (GetSize()==8) {
		__int64 d;
		reverse((char*)GetData(), (char*)&d, 8);
		return *((double*)&d);
	}

	if (GetSize()==10) {
		char d[10];
		reverse((char*)GetData(), (char*)d, 10);
		return *((long double*)d);
	}

	return 0;
}

void CBuffer::Refer(CBuffer** p)
{
	if (*p) DecBufferRefCount(p);
	*p = this;
	IncRefCount();
}

void CBuffer::SetRefCount(int i)
{
	iRefCount = i;
}
/*
CMultimediaBuffer::CMultimediaBuffer()
{
	qwStart_ns = 0;
}

CMultimediaBuffer::~CMultimediaBuffer()
{
}

void CMultimediaBuffer::SetTimeStamp(__int64 qwTime)
{
	qwStart_ns = qwTime;
}

__int64 CMultimediaBuffer::GetTimeStamp(void)
{
	return qwStart_ns;
}
*/	
void CStringBuffer::Set(char* s, int iFlags)
{
	bASCII = false;
	bUTF8 = false;
	bUTF16 = false;

	if ((iFlags & 0x1C) == 0) iFlags|=CSB_ASCII;

	if (iFlags & CSB_ASCII) {
		b[0]->SetSize(1+strlen(s));
		b[0]->SetData(s);
		bASCII = true;
		iOutputFormat = CSB_ASCII;
	}

	if (iFlags & CSB_UTF8) {
		b[1]->SetSize(1+strlen(s));
		b[1]->SetData(s);
		bUTF8 = true;
		iOutputFormat = CSB_UTF8;
	}

	if (iFlags & CSB_UTF16) {
		b[2]->SetSize(2+2*wcslen((unsigned short*)s));
		b[2]->SetData(s);
		bUTF16 = true;
		iOutputFormat = CSB_UTF16;
	}
}

int CStringBuffer::GetSize() 
{
	Prepare(GetOutputFormat());

	switch (GetOutputFormat()) {
		case CSB_ASCII:
			return b[0]->GetSize();
		case CSB_UTF8:
			return b[1]->GetSize();
		case CSB_UTF16:
			return b[2]->GetSize();
	}

	return 0;
}

int CStringBuffer::GetOutputFormat()
{
	return iOutputFormat;
}

int CStringBuffer::SetOutputFormat(int iFormat)
{
	iOutputFormat = iFormat;
	return iFormat;
}

bool CStringBuffer::DecRefCount()
{
	DecBufferRefCount(&b[0]);
	DecBufferRefCount(&b[1]);
	DecBufferRefCount(&b[2]);
	
	return !!b[0];
}

void* CStringBuffer::GetData()
{
	return Get();
}

void CStringBuffer::Prepare(int iFormat)
{
	char* d;

	// if only UTF16 is present, set UTF8 as well
	if (!bUTF8 && !bASCII && bUTF16) {
		WStr2UTF8(b[2]->AsString(),&d);
		b[1]->SetSize(1+strlen(d));
		b[1]->SetData(d);
		bUTF8 = true;
	}

	switch (iFormat) {
		case CSB_ASCII:
			if (bUTF16) {
				WStr2Str(b[2]->AsString(), &d);
				b[0]->SetSize(1+strlen(d));
				b[0]->SetData(d);
				bASCII = true;
			}
			if (bUTF8) {
				UTF82Str(b[2]->AsString(), &d);
				b[0]->SetSize(strlen(d)+1);
				b[0]->SetData(d);
				bASCII = true;
			}
			break;
		case CSB_UTF8:
			if (bASCII) {
				Str2UTF8(b[1]->AsString(),&d);
				b[0]->SetSize(strlen(d)+1);
				b[0]->SetData(d);
				bUTF8 = true;
			}
			break;
	}
}

char* CStringBuffer::Get(void)
{
	Prepare(GetOutputFormat());

	switch (GetOutputFormat()) {
		case CSB_ASCII:
			return b[0]->AsString();
		case CSB_UTF8:
			return b[1]->AsString();
	}

	return NULL;
}

CStringBuffer::CStringBuffer()
{
	lpData = NULL;
	bASCII = false;
	bUTF8 = false;
	bUTF16 = false;

	b[0] = new CBuffer(CBN_REF1);
	b[1] = new CBuffer(CBN_REF1);
	b[2] = new CBuffer(CBN_REF1);
}

CStringBuffer::CStringBuffer(char* s, int iFlags)
{
	b[0] = new CBuffer(CBN_REF1);
	b[1] = new CBuffer(CBN_REF1);
	b[2] = new CBuffer(CBN_REF1);

	Set(s, iFlags);
	
	if (iFlags & CBN_REF1) SetRefCount(1);
}

CAttribs::CAttribs()
{
	iEntryCount = 32;
	Init();
}

CAttribs::CAttribs(int iSize)
{
	iEntryCount = 32;
	Init();
}

void CAttribs::Init()
{
	pEntries = new ATTRIBUTE_ENTRY*[iEntryCount];
	ZeroMemory(pEntries, sizeof(ATTRIBUTE_ENTRY*)*iEntryCount);
}

int CAttribs::Position(char* cName)
{
	int j=0;

	while (*cName) {
		j+=*cName++;
	}

	return j%=iEntryCount;
}

ATTRIBUTE_ENTRY* CAttribs::FindInLine(char* cName, int iLine)
{
	if (iLine >= iEntryCount || iLine < 0) return NULL;

	ATTRIBUTE_ENTRY* e = pEntries[iLine];
	while (e) {
		if (!strcmp(e->cName, cName)) return e;
		e = e->pNext;
	}

	return e;
}

ATTRIBUTE_ENTRY* CAttribs::Find(char* cName)
{
	char* n;
	CAttribs* r = Resolve(cName, &n);
	return r->FindInLine(n, Position(n));
}

CAttribs* CAttribs::GetAttr(char* cName)
{
	ATTRIBUTE_ENTRY* e = Find(cName);

	if (e) return (CAttribs*)e->pData;
	
	return NULL;
}

void CAttribs::Add2Line(int iLine, ATTRIBUTE_ENTRY* e)
{
	ATTRIBUTE_ENTRY* l = pEntries[iLine];

	if (!pEntries[iLine]) {
		pEntries[iLine] = e;
	} else {
		while (l->iFlags & FATTR_VALID && l->pNext) {
			l = l->pNext;			
		}
		l->pNext = e;
	}
}

CAttribs* CAttribs::Resolve(char* cPath, char** cName)
{
	if (!strstr(cPath,"/")) {
		if (cName) *cName = cPath;
		return this;
	}

	char* next;
	char* c = new char[1+strlen(cPath)];
	strcpy(c,cPath);
	char* d=c;
	CAttribs* a = this, *b = NULL;

	while (a && (next = strchr(d, '/'))) {
		*next=0;
		if (!(b=a->GetAttr(d))) {
			a->Add(d,0,ATTRTYPE_ATTRIBS,NULL);
			a = a->GetAttr(d);
		} else {
			a = b;
		}
		d=next+1;
	}

	if (cName) *cName = &cPath[(DWORD)d-(DWORD)c];
	delete c;

	return a;
}

void CAttribs::Set(char* cName, void* pData)
{
	ATTRIBUTE_ENTRY*	entry = Find(cName);


}

void CAttribs::DeleteLine(int iLine)
{
	ATTRIBUTE_ENTRY*	e, *enext;

	e = pEntries[iLine];

	while (e) {
		switch (e->iType) {
			case ATTRTYPE_INT64:
				delete e->pData;
				e->pData = NULL;
				delete e->cName;
				e->cName = NULL;
				break;
			case ATTRTYPE_ATTRIBS:
				((CAttribs*)(e->pData))->Delete();
				delete e->pData;
				e->pData = NULL;
				delete e->cName;
				e->cName = NULL;
				break;
		}
		enext = e->pNext;
		delete e;
		e=enext;		
	}

	pEntries[iLine] = NULL;
}

void CAttribs::Delete()
{
	for (int i=0;i<iEntryCount;i++) {
		DeleteLine(i);
	}
	delete pEntries;
	pEntries = NULL;
}

void CAttribs::SetInt(char* cName, __int64 pData)
{
	ATTRIBUTE_ENTRY*	entry = Find(cName);

	if (!entry) {
		AddInt(cName, 0, pData);
	} else {
		if (entry->iType == ATTRTYPE_INT64) {
			memcpy(entry->pData, &pData, sizeof(__int64));
		} else {
			char msg[200];
			msg[0]=0;
			sprintf(msg, "Wrong type in CAttribs::Set: for Element %s", cName);
			MessageBox(0, msg, "Error", MB_OK | MB_ICONERROR);
		}
	}
}

__int64 CAttribs::GetInt(char* cName) 
{
	ATTRIBUTE_ENTRY*	entry = Find(cName);

	if (entry && entry->pData) {
		return (*(__int64*)entry->pData);
	}

	char msg[200];
	msg[0]=0;
	sprintf(msg, "Requested uninitialized integer:%c%c GetInt for Element %s", 13,10,cName);
	MessageBox(0, msg, "Error", MB_OK | MB_ICONERROR);

	return 0;
}

int CAttribs::Exists(char* cName)
{
	ATTRIBUTE_ENTRY*	entry = Find(cName);

	return (!!entry);
}

void CAttribs::AddInt(char* cName, int iFlags, __int64 pData)
{
	Add(cName, iFlags, ATTRTYPE_INT64, &pData);
}

void CAttribs::DuplicateLine(CAttribs* a, int iLine)
{
	ATTRIBUTE_ENTRY* e = pEntries[iLine];

	while (e) {
		switch (e->iType) {
			case ATTRTYPE_INT64: 
				a->AddInt(e->cName, 0, *(__int64*)e->pData);
				break;
			case ATTRTYPE_ATTRIBS:
				a->Add(e->cName, FATTR_ADDATTR_DONTCREATE, ATTRTYPE_ATTRIBS, 
					((CAttribs*)e->pData)->Duplicate());
				break;
		}
		e = e->pNext;
	}
}

CAttribs* CAttribs::Duplicate()
{
	CAttribs* d = new CAttribs(iEntryCount);

	for (int i=0;i<iEntryCount;i++) {
		DuplicateLine(d, i);	
	}

	return d;
}

void CAttribs::Export(CAttribs* a)
{
	for (int i=0;i<<iEntryCount;i++) {
		DuplicateLine(a, i);	
	}

}

void CAttribs::Add(char* cName, int iFlags, int iType, void* pData)
{
	ATTRIBUTE_ENTRY*	entry;
	CAttribs* a;
	int position, size;
	char* n;

	a = Resolve(cName, &n);

	position = Position(n);
	entry = new ATTRIBUTE_ENTRY;
	ZeroMemory(entry,sizeof(*entry));
	entry->cName = new char[size = strlen(n)+1];
	memcpy(entry->cName,n,size);
	entry->iFlags = FATTR_VALID;
	entry->iType = iType;

	switch (iType) {
		case ATTRTYPE_ASCII:
		case ATTRTYPE_UTF8:
			entry->pData = new char[1+strlen((char*)pData)];
			strcpy((char*)entry->pData,(char*)pData);
			break;

		case ATTRTYPE_ATTRIBS:
			switch (iFlags) {
				case FATTR_ADDATTR_CREATE:
					entry->pData = new CAttribs(iEntryCount);
					break;
				case FATTR_ADDATTR_DONTCREATE:
					entry->pData = pData;
					break;
			}
			break;
		//case ATTRTYPE_BINARY:
		case ATTRTYPE_FLOAT:
			entry->pData = new float;
			memcpy(entry->pData,pData,sizeof(float));
			break;
		case ATTRTYPE_INT64:
			entry->pData = new __int64;
			memcpy(entry->pData,pData,sizeof(__int64));
			break;
		default:
			break;
	}

	a->Add2Line(position,entry);
}

/*
CList::CList()
{
	data=NULL;
	next=NULL;
	bEmpty=true;
}

bool CList::IsEmpty()
{
	return bEmpty;
}

CList* CList::GetNext()
{
	return next;
}

CList::~CList()
{
}

void CList::SetNext(CList* list)
{
	next = list;
}

void CList::SetData(CBuffer* lpBuffer)
{
	CBuffer* olddata = data;
	data = lpBuffer;
	data->IncRefCount();
	DecBufferRefCount(&olddata);
	bEmpty=false;
}

CBuffer* CList::GetData()
{
	return data;
}

CList* CList::GetPrev()
{
	return prev;
}

void CList::SetPrev(CList* lpPrev)
{
	prev = lpPrev;
}

void CList::Insert(CBuffer* lpBuffer,int dwFlag)
{
	switch (dwFlag)
	{
		case LISTINS_BEGIN:	{
				CList*  lnew = new CList;
				GetNext()->prev=lnew;
				lnew->prev=this;
				lnew->SetData(lpBuffer);
				lnew->SetNext(GetNext());
				SetNext(lnew);
			}
		case LISTINS_END:	{
				CList*  lnew = new CList;
				GetNext()->prev=lnew;
				lnew->prev=this;
				lnew->SetData(lpBuffer);
				lnew->SetNext(GetNext());
				SetNext(lnew);
			}
	}
}
*/