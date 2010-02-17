#include "stdafx.h"
#include "Buffers.h"
#include "stdio.h"
#include "memory.h"
#include "stdlib.h"
#include "strings.h"
//#include "matroska.h"
#include "assert.h"
#include "Finalizer.h"
#ifdef DEBUG_NEW
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif


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
	external = 0;
}

CBuffer::CBuffer(int _iSize, void* _lpData, int iFlags)
{
	iRefCount = 0;
	iSize=0;
	lpData=NULL;
	external = 0;
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
		if (iSize && lpData) 
			free(lpData);
		
		if (_iSize)
		{
			lpData=(char*)malloc(_iSize+1);
			((BYTE*)lpData)[_iSize] = 0;
		}
		else
		{
			printf("???");
		}

		iSize=_iSize;
		
		iAllocedSize=iSize;
	}
}

void CBuffer::Resize(int new_size)
{
	if (!iSize) {
		SetSize(new_size);
		return;
	}

	lpData = realloc(lpData, 1+new_size);


	((BYTE*)lpData)[new_size] = 0;

	iSize = new_size;
	iAllocedSize = new_size;
}

void CBuffer::Prepend(int pos, void* data, int len)
{
	int old_size = iSize;

	Resize(iSize + len);
	memmove(((char*)lpData)+len+pos, ((char*)lpData)+pos, iSize-len-pos);
	memmove(((char*)lpData)+pos, data, len);
}

void CBuffer::Cut(int pos, int len)
{
	if (pos >= iSize)
		return;

	int bytes_to_copy = iSize - pos - len;

	memmove(((BYTE*)lpData)+pos, ((BYTE*)lpData)+pos+len, bytes_to_copy);

	iSize -= len;
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
		if (!external) { 
			if (lpData)
				free(lpData);
		}
		else
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
	double result;

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
		__asm {
			lea eax, d
			fld tbyte ptr [eax]
			lea eax, result
			fstp qword ptr [eax]
		}
//		return *((long double*)d);
	}

	return result;
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

void CBuffer::SetExternal(void* lpsource, int size)
{
	iSize = size;
	lpData = lpsource;
	external = 1;
}

void CStringBuffer::Set(char* s, int iFlags)
{
//	_ASSERT((iFlags | CSB_FLAGS) != CSB_FLAGS);

	bASCII = false;
	bUTF8 = false;
	bUTF16 = false;

	if ((iFlags & 0x03) == 0) iFlags|=CSB_ASCII;

	if ((iFlags & 0x03) == CSB_ASCII) {
		b[0]->SetSize(1+(int)strlen(s));
		b[0]->SetData(s);
		bASCII = true;
		iOutputFormat = CSB_ASCII;
	}

	if ((iFlags & 0x03) == CSB_UTF8) {
		b[1]->SetSize(1+(int)strlen(s));
		b[1]->SetData(s);
		bUTF8 = true;
		iOutputFormat = CSB_UTF8;
	}

	if ((iFlags & 0x03) == CSB_UTF16) {
		b[2]->SetSize(2+2*(int)wcslen((const wchar_t*)s));
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


void CStringBuffer::IncRefCount()
{
	b[0]->IncRefCount();
	b[1]->IncRefCount();
	b[2]->IncRefCount();
	
	CBuffer::IncRefCount();
}

bool CStringBuffer::DecRefCount()
{
	DecBufferRefCount(&b[0]);
	DecBufferRefCount(&b[1]);
	DecBufferRefCount(&b[2]);
	
	return !!CBuffer::DecRefCount();
}

void* CStringBuffer::GetData()
{
	return Get();
}

void CStringBuffer::Prepare(int iFormat)
{
	char* d = NULL;

	// if only UTF16 is present, set UTF8 as well
	if (!bUTF8 && !bASCII && bUTF16) {
		WStr2UTF8(b[2]->AsString(),&d);
		b[1]->SetSize(1+(int)strlen(d));
		b[1]->SetData(d);
		bUTF8 = true;
	}

	switch (iFormat) {
		case CSB_ASCII:
			if (bUTF16) {
				WStr2Str(b[2]->AsString(), &d);
				b[0]->SetSize(1+(int)strlen(d));
				b[0]->SetData(d);
				bASCII = true;
				free(d);
				d = NULL;
			}
			if (bUTF8) {
				UTF82Str(b[1]->AsString(), &d);
				b[0]->SetSize((int)strlen(d)+1);
				b[0]->SetData(d);
				bASCII = true;
				free(d);
				d = NULL;
			}
			break;
		case CSB_UTF8:
			if (bASCII) {
				Str2UTF8(b[0]->AsString(),&d);
				b[1]->SetSize((int)strlen(d)+1);
				b[1]->SetData(d);
				bUTF8 = true;
				free(d);
				d = NULL;
			}
			break;
	}

	if (d)
		delete d;
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

	b[0] = new CBuffer(0, NULL, CBN_REF1);
	b[1] = new CBuffer(0, NULL, CBN_REF1);
	b[2] = new CBuffer(0, NULL, CBN_REF1);

	SetRefCount(1);
}

CStringBuffer::CStringBuffer(const char* s, int iFlags)
{
	lpData = NULL;

	b[0] = new CBuffer(0, NULL, CBN_REF1);
	b[1] = new CBuffer(0, NULL, CBN_REF1);
	b[2] = new CBuffer(0, NULL, CBN_REF1);

	Set((char*)s, iFlags);

	if (iFlags & CBN_REF1) SetRefCount(1);
}


CAttribs::CAttribs()
{
	iEntryCount = 32;
	Init();
}

CAttribs::CAttribs(int iSize)
{
	iEntryCount = iSize;
	Init();
}

CAttribs::~CAttribs()
{
	Delete();
}

void CAttribs::Init()
{
	pEntries = new ATTRIBUTE_ENTRY*[iEntryCount];
	ZeroMemory(pEntries, sizeof(ATTRIBUTE_ENTRY*)*iEntryCount);
}


int CAttribs::Position(char* cName)
{
	__ASSERT(iEntryCount > 0 && iEntryCount < 64, "CAttribs::Position(): Bad iEntryCount")
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

	if (!r)
		return NULL;

	ATTRIBUTE_ENTRY* ae = r->FindInLine(n, Position(n));

	return ae;
}

CAttribs* CAttribs::GetAttr(char* cName)
{
	char* t = (char*)malloc(1024); t[0]=0; int i=0; int changed = 0; 
	Finalizer<char, void, free> tGuard(t);

	while (*cName) {
	if (*cName != ' ')
			t[i++]=*cName++;
		else {
			changed = 1;
			t[i++]='_';
			cName++;
		}
	}
	t[i++]=0;

    ATTRIBUTE_ENTRY* e = Find(t);

//	free(t);

	if (e) 
		return (CAttribs*)e->pData;
	
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
	Finalizer<char, char, delete_array> cGuard(c);

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

	if (cName) *cName = &cPath[(size_t)d-(size_t)c];
//	delete c[];

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
	for (int i=0;i<iEntryCount;i++)
		DeleteLine(i);

	if (pEntries)
		delete pEntries;

	pEntries = NULL;
	iEntryCount = 0;
}

void CAttribs::SetInt(char* cName, __int64 pData)
{
	char* t = (char*)malloc(1024); t[0]=0; int i=0; int changed = 0; 
	Finalizer<char, void, free> tGuard(t);

	while (*cName) {
	if (*cName != ' ')
			t[i++]=*cName++;
		else {
			changed = 1;
			t[i++]='_';
			cName++;
		}
	}
	t[i++]=0;

	cName = t;


	ATTRIBUTE_ENTRY*	entry = Find(cName);

	if (!entry) {
		AddInt(cName, 0, pData);
	} else {
		if (entry->iType == ATTRTYPE_INT64) {
			memcpy(entry->pData, &pData, sizeof(__int64));
		} else {
			char msg[200];
			msg[0]=0;
			sprintf(msg, "Wrong type in CAttribs::Set for Element %s", cName);
			MessageBoxA(0, msg, "Error", MB_OK | MB_ICONERROR);
		}
	}

//	free(t);
}

void CAttribs::SetStr(char* cName, char* value)
{
	char* t = (char*)malloc(1024); t[0]=0; int i=0; int changed = 0; 
	Finalizer<char, void, free> tGuard(t);

	while (*cName) {
	if (*cName != ' ')
			t[i++]=*cName++;
		else {
			changed = 1;
			t[i++]='_';
			cName++;
		}
	}
	t[i++]=0;

	cName = t;


	ATTRIBUTE_ENTRY*	entry = Find(cName);

	if (!entry) {
		Add(cName, 0, ATTRTYPE_UTF8, value);
	} else {
		if (entry->iType == ATTRTYPE_UTF8 || entry->iType == ATTRTYPE_ASCII) {
			entry->pData = realloc(entry->pData, 1+strlen(value));
			memcpy(entry->pData, value, strlen(value));
		} else {
			char msg[200];
			msg[0]=0;
			sprintf(msg, "Wrong type in CAttribs::Set for Element %s", cName);
			MessageBoxA(0, msg, "Error", MB_OK | MB_ICONERROR);
		}
	}

//	free(t);
}

__int64 CAttribs::GetInt(char* cName) 
{
	char* t = (char*)malloc(1024); t[0]=0; int i=0; int changed = 0; 
	Finalizer<char, void, free> tGuard(t);

	while (*cName) {
	if (*cName != ' ')
			t[i++]=*cName++;
		else {
			changed = 1;
			t[i++]='_';
			cName++;
		}
	}
	t[i++]=0;

	cName = t;
	
	ATTRIBUTE_ENTRY*	entry = Find(cName);

	if (entry && entry->pData) {
		if (entry->iType == ATTRTYPE_INT64) {
	//		free(t);
			__int64 d = *(__int64*)entry->pData; 
			return (d);
		}

		if (entry->iType == ATTRTYPE_ASCII && entry->iFlags == ATTRTYPE_UTF8) {
	//		free(t);
			__int64 d = _atoi64((char*)entry->pData);
			return (d);
		}
	}

	char msg[200];
	msg[0]=0;
	sprintf(msg, "Requested uninitialized integer:%c%c GetInt for Element %s", 13,10,cName);
	MessageBoxA(0, msg, "Error", MB_OK | MB_ICONERROR);

//	free(t);

	return 0;
}

int CAttribs::GetStr(char* cName, char** cDest) 
{
	char* t = (char*)malloc(1024); t[0]=0; int i=0; int changed = 0; 
	Finalizer<char, void, free> tGuard(t);

	while (*cName) {
	if (*cName != ' ')
			t[i++]=*cName++;
		else {
			changed = 1;
			t[i++]='_';
			cName++;
		}
	}
	t[i++]=0;

	cName = t;
	
	ATTRIBUTE_ENTRY*	entry = Find(cName);

	if (entry && entry->pData) {
//		free(t);

		if (entry->iType == ATTRTYPE_ASCII || entry->iType == ATTRTYPE_UTF8) {
			int l = (int)strlen((char*)entry->pData);
			(*cDest) = (char*)calloc(1, 1+l);
			memcpy(*cDest, entry->pData, l);
			return l;
		}

		if (entry->iType == ATTRTYPE_INT64) {
			char c[32]; c[0]=0;
			_snprintf(c, 32, "%I64d", *((__int64*)entry->pData));
			int l = (int)strlen(c);
			(*cDest) = (char*)calloc(1, 1+l);
			memcpy(*cDest, entry->pData, l);
			return l;
		}

		*cDest = NULL;
		return 0;
	}

	char msg[200];
	msg[0]=0;
	sprintf(msg, "Requested uninitialized string:%c%c GetInt for Element %s", 13,10,cName);
	MessageBoxA(0, msg, "Error", MB_OK | MB_ICONERROR);

//	free(t);


	return 0;
}
__int64 CAttribs::GetIntWithDefault(char* cName, __int64 _default)
{
	if (!Exists(cName))
		return _default;

	return GetInt(cName);
}

int CAttribs::Exists(char* cName)
{
	char* t = (char*)malloc(1024); t[0]=0; int i=0; int changed = 0; 
	Finalizer<char, void, free> tGuard(t);

	while (*cName) {
	if (*cName != ' ')
			t[i++]=*cName++;
		else {
			changed = 1;
			t[i++]='_';
			cName++;
		}
	}
	t[i++]=0;

	ATTRIBUTE_ENTRY*	entry = Find(t);

//	free(t);

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
			case ATTRTYPE_UTF8:
			case ATTRTYPE_ASCII:
				a->Add(e->cName, FATTR_ADDATTR_CREATE, e->iType, e->pData);
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

void CAttribs::CopyLine(CAttribs* a, int iLine)
{
	ATTRIBUTE_ENTRY* e = pEntries[iLine];

	while (e) {
		switch (e->iType) {
			case ATTRTYPE_INT64: 
				a->SetInt(e->cName, *(__int64*)e->pData);
				break;
			case ATTRTYPE_UTF8:
			case ATTRTYPE_ASCII:
				a->SetStr(e->cName, (char*)e->pData);
				break;
			case ATTRTYPE_ATTRIBS:
				if (!a->Exists(e->cName)) {
					a->Add(e->cName, FATTR_ADDATTR_DONTCREATE, ATTRTYPE_ATTRIBS, 
						((CAttribs*)e->pData)->Duplicate());
				} else {
					GetAttr(e->cName)->CopyTo(a->GetAttr(e->cName));
				}
				break;
		}
		e = e->pNext;
	}
}


void CAttribs::CopyTo(CAttribs* target)
{
	for (int i=0;i<iEntryCount;i++) {
		CopyLine(target, i);	
	}

}

void CAttribs::Export(CAttribs* a)
{
	for (int i=0;i<<iEntryCount;i++) {
		DuplicateLine(a, i);	
	}

}

void CAttribs::Add(const char* cName, int iFlags, int iType, void* pData)
{
	ATTRIBUTE_ENTRY*	entry;
	CAttribs* a;
	int position, size;
	char* n;

	a = Resolve((char*)cName, &n);

	position = Position(n);
	entry = new ATTRIBUTE_ENTRY;
	ZeroMemory(entry,sizeof(*entry));
	entry->cName = new char[size = (int)strlen(n)+1];
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

//	free(t);
}

CAttribs::operator XMLNODE*() {
	XMLNODE* pNode = NULL;

	if (!iEntryCount)
		return NULL;

	for (int j=0;j<iEntryCount;j++) {
		ATTRIBUTE_ENTRY* e = pEntries[j];

		while (e) {
			if (e->iType == ATTRTYPE_ATTRIBS) {
				XMLNODE* node = xmlAddSibling(&pNode, e->cName, "", false);
				node->pChild = *(CAttribs*)e->pData;
				
			} else
			if (e->iType == ATTRTYPE_INT64) {
				char c[16]; c[0]=0;
				sprintf(c, "%I64d", *(__int64*)e->pData);
				xmlAddSibling(&pNode, e->cName, c, false);
			} else
			if (e->iType == ATTRTYPE_ASCII || e->iType == ATTRTYPE_UTF8) {
				xmlAddSibling(&pNode, e->cName, (char*)e->pData, false);
			}
			e = e->pNext;
		}
	}

	return pNode;
}

int CAttribs::Import(XMLNODE* xml)
{
	if (!xml)
		return 1;

	if (xml->pChild || (xml->cValue.empty() && !xml->ValuePresent())) {
		CAttribs* a = GetAttr((char*)xml->cNodeName.c_str());
		if (!a) {
			Add(xml->cNodeName.c_str(), FATTR_ADDATTR_CREATE, ATTRTYPE_ATTRIBS, NULL);
			a = GetAttr((char*)xml->cNodeName.c_str());
		}
		a->Import((XMLNODE*)xml->pChild);
	} else {
		if (isint(xml->cValue.c_str())) {
			int i = atoi(xml->cValue.c_str());
			SetInt((char*)xml->cNodeName.c_str(), i);
		} else {
			SetStr((char*)xml->cNodeName.c_str(), (char*)xml->cValue.c_str());
		}
	}

	Import((XMLNODE*)xml->pNext);

	return 1;
}