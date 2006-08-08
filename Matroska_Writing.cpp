#include "stdafx.h"
#include "matroska.h"
#include "matroska_ids.h"
#include "matroska_writing.h"
#include "basestreams.h"
#include "integers.h"
#include "math.h"
#include "CRC.h"
#include "generateuids.h"
#include "Compression.h"

#ifdef DEBUG_NEW
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

// some global settings for the lib

int		iDontWriteBlockSizes = 0;
int		iRandomizeElementOrder = 0;

#define pFirst (pCRC32?pCRC32:pChild)

void MATROSKA_WriteBlockSizes(bool bWrite)
{
	iDontWriteBlockSizes = !bWrite;
}

void MATROSKA_RandomizeElementOrder(bool bRandomize)
{
	iRandomizeElementOrder = bRandomize;
}

EBMLElement_Writer::EBMLElement_Writer()
{
	ZeroMemory(&info,sizeof(info));
	pNext = NULL;
	pChild = NULL;
	pLastChild = NULL;
	bWriteCRC = NULL;
	pCRC32 = NULL;
}

EBMLElement_Writer::EBMLElement_Writer(STREAM* stream,char* ID,CBuffer* cBuffer)
{
	ZeroMemory(&info,sizeof(info));
	info.stream = stream;
	if (ID) SetID(ID);
	if (cBuffer) SetData(cBuffer);
	pNext = NULL;
	pChild = NULL;
	pLastChild = NULL;
	bWriteCRC = NULL;
	pCRC32 = NULL;
}

EBMLElement_Writer::~EBMLElement_Writer()
{
}

void EBMLElement_Writer::SetID(char* ID)
{
	info.ID = ID;
	info.iIDLen = GetIDLength(ID);
}

void EBMLElement_Writer::SetChild(EBMLElement_Writer* _pChild)
{
	pChild = _pChild;
}

void EBMLElement_Writer::SetNext(EBMLElement_Writer* _pNext)
{
	pNext = _pNext;
}

void EBMLElement_Writer::SetData(CBuffer* cBuffer)
{
	__int64 q = cBuffer->GetSize();
	info.cData = cBuffer;
	info.iSizeLen = Int2VSUInt(&q, info.cSize);
}

CBuffer* EBMLElement_Writer::GetData()
{
	return info.cData;
}

__int64 EBMLElement_Writer::GetSize()
{
	__int64 q;
	
	if (! (info.iFlags & EWI_SIZEFIXED)) {
		if (!pChild) {
			if (info.cData) {
				q = info.cData->GetSize();
			} else {
				// neither child elements nor cData
				q = 0;
			}
		} else q = pFirst->GetListSize();
		info.iSize = q;
	} else {
		q = info.iSize;
	}

	if (info.iFlags & EWI_SIZELENFIXED) {
		info.iSizeLen = Int2VSUInt(&q,info.cSize,info.iSizeLen);
	} else info.iSizeLen = Int2VSUInt(&q, info.cSize);

	if (info.iWriteUnknownSize) {
		return 1+ q + info.iIDLen;
	}

	return q + info.iIDLen + info.iSizeLen;
}

EBMLElement_Writer* EBMLElement_Writer::GetNext()
{
	return pNext;
}

__int64 EBMLElement_Writer::GetListSize()
{
	__int64 iRes = 0;
	EBMLElement_Writer*	pNext = this;

	while (pNext) {
		iRes = iRes + pNext->GetSize();
		pNext = pNext->GetNext();
	}

	return iRes;
}

STREAM* EBMLElement_Writer::GetDest()
{
	return info.stream;
}

int EBMLElement_Writer::Put(void* pSource, int iSize, char* pDest, int* iDest)
{
	if (!pDest && !iDest) {
		GetDest()->Write(pSource, iSize);
	} else 
	if (iDest && !pDest) {
		*iDest += iSize;
	} else {
		memcpy(((char*)pDest)+*iDest, pSource, iSize);
		*iDest+=iSize;
	}

	return iSize;
}

int EBMLElement_Writer::IsCRCEnabled()
{
	return bWriteCRC;
}

__int64 EBMLElement_Writer::Write(char* pDest, int* iDest)
{
	__int64 iRes = 0;

/*	if (info.iFlags & EWI_SIZEFIXED) {
		Sleep(1);
	}
*/
	// update size related members and write size info
	GetSize();
	iRes += Put(info.ID, info.iIDLen, pDest, iDest);
	if (info.iWriteUnknownSize) {
		unsigned char c = 0xFF;
		iRes += Put(&c, 1, pDest, iDest);
		if (!pDest) IncWriteOverhead(info.iIDLen + 1);
	} else {
		iRes += Put(info.cSize, info.iSizeLen, pDest, iDest);
		if (!pDest) IncWriteOverhead(info.iIDLen + info.iSizeLen);
	}
	// no children: write either data or zeros
	if (!pChild) {
		if (info.cData) {
			iRes += Put(info.cData->GetData(), (int)info.iSize, pDest, iDest);
		} else {
			void* lpBuffer = new char[(int)info.iSize];
			memset(lpBuffer, 0, (int)info.iSize);
			iRes += Put(lpBuffer, (int)info.iSize, pDest, iDest);
			delete lpBuffer;
		}
	} else {
	// children: -> write all child element data and padd with VOID if necessary
		EBMLElement_Writer*	e_Void = NULL;
		__int64	iDiff;
		if (info.iFlags & EWI_SIZEFIXED) {
			e_Void = new EBMLElement_Writer(GetDest(),(char*)MID_VOID,NULL);
			int i = 0;
			iDiff = info.iSize - pFirst->WriteList(NULL, &i);
			if (iDiff>=10) {
				e_Void->SetFixedSizeLen(8);
				e_Void->SetFixedSize(iDiff);
			} else {
				e_Void->SetFixedSizeLen((int)(iDiff-2));
				e_Void->SetFixedSize(iDiff);
			}
		}

		if (e_Void && iDiff > 0) {
			AppendChild(e_Void, APPEND_END | APPEND_DONT_RANDOMIZE);
		}

		if (IsCRCEnabled()) {
			unsigned char* p = new unsigned char[(int)info.iSize];
			int i = 0;
			pChild->WriteList((char*)p, &i);
			int crc = CRC32(p, i);
			memcpy(pCRC32->GetData()->AsString(), &crc, 4);
			iRes+=pCRC32->Write(pDest, iDest);
			if (!pDest) IncWriteOverhead(6);
			delete p;
		}

		iRes += pChild->WriteList(pDest, iDest);
	}


	return iRes;
}

__int64 EBMLElement_Writer::WriteList(char* pDest, int* iDest)
{
	EBMLElement_Writer*	pNext = this;
	__int64				iRes = 0;

	while (pNext) {
		iRes += pNext->Write(pDest, iDest);
		pNext = pNext->GetNext();
	}

	return iRes;
}

void EBMLElement_Writer::SetFixedSize(__int64 iSize)
{
	info.iFlags &=~ EWI_SIZEFIXED;

	if (iSize) {
		info.iFlags |= EWI_SIZEFIXED;
		info.iSize = iSize - info.iIDLen;
		GetSize();
		SetFixedSizeLen(info.iSizeLen);
		info.iSize = iSize - info.iSizeLen - info.iIDLen;
	};
}

void EBMLElement_Writer::SetFixedSizeLen(int iSize)
{
	info.iFlags &=~ EWI_SIZELENFIXED;

	if (iSize) {
		info.iFlags |= EWI_SIZELENFIXED;
		info.iSizeLen = iSize;
	} else SetData(info.cData);
}

void EBMLElement_Writer::EnableCRC32(int bEnabled)
{
	if (!bWriteCRC) {
		bWriteCRC = bEnabled;
		pCRC32 = new EBMLElement_Writer(GetDest(), (char*)MID_CRC32, new CBuffer(4, new char[4]));
		pCRC32->SetNext(pChild);
	} 	
}

void EBMLElement_Writer::SetDest(STREAM* pStream)
{
	info.stream = pStream;
}

void EBMLElement_Writer::Delete()
{
	if (pCRC32) {
		pCRC32->DeleteList();
		delete pCRC32;
		SetChild(NULL);
	} else
	if (GetChild()) {
		GetChild()->DeleteList();
		delete GetChild();
		SetChild(NULL);
	}

	if (info.cData)
		DecBufferRefCount(&info.cData);

	return;
}

void EBMLElement_Writer::DeleteList()
{
	EBMLElement_Writer*	pNext = GetNext();
	EBMLElement_Writer* pCurr;

	while (pNext) {
		pCurr = pNext;
		pCurr->Delete();
		pNext = pCurr->GetNext();
		delete pCurr;
	}
	
	Delete();
}

EBMLElement_Writer* EBMLElement_Writer::GetChild()
{
	return pChild;
}

EBMLElement_Writer* EBMLElement_Writer::AppendChild(EBMLElement_Writer* e_Child, int where)
{
	if (iRandomizeElementOrder && (!(where & APPEND_DONT_RANDOMIZE))) {
		int j = rand();
		where &=~ (APPEND_BEGIN | APPEND_END);
		where |= ((j%2)?APPEND_BEGIN:APPEND_END);
	}

	if (where & APPEND_END) {
		if (!pLastChild) {
			pChild = e_Child;
			pLastChild = pChild;
		} else {
			pLastChild->SetNext(e_Child);
			pLastChild = e_Child;
		}
	} else {
		if (!pLastChild) {
			pChild = e_Child;
			pLastChild = pChild;
		} else {
			e_Child->SetNext(pChild);
			pChild = e_Child;
		}
	}

	return e_Child;
}

void EBMLElement_Writer::SetCBuffer(CBuffer* cSource,CBuffer** cDest)
{
	if (cSource!=*cDest) {
		cSource->IncRefCount();
		DecBufferRefCount(cDest);
		*cDest = cSource;
	} 
}

EBMLElement_Writer* EBMLElement_Writer::AppendChild_UInt(char* ID,__int64 iData, __int64 iDefault, int where)
{
	if (iData!=iDefault) {
		EBMLUInt_Writer*	e_UInt = new EBMLUInt_Writer();
		e_UInt->Define(GetDest(),ID,iData,NULL);
		
		AppendChild(e_UInt, where);
		return e_UInt;
	} else return NULL;
}

EBMLElement_Writer* EBMLElement_Writer::AppendChild_SInt(char* ID,__int64 iData, __int64 iDefault)
{
	if (iData!=iDefault) {
		EBMLUInt_Writer*	e_SInt = new EBMLSInt_Writer();
		e_SInt->Define(GetDest(),ID,iData,NULL);
		AppendChild(e_SInt);
		return e_SInt;
	} else return NULL;
}

EBMLElement_Writer* EBMLElement_Writer::AppendChild_Float(char* ID,long double fData, long double fDefault)
{
	if (fabs(fData-fDefault)>0.000001) {
		EBMLFloat_Writer*	e_Float = new EBMLFloat_Writer();
		e_Float->Define(GetDest(),ID,fData,NULL);
		AppendChild(e_Float);
		return e_Float;
	} else return NULL;
}

EBMLElement_Writer* EBMLElement_Writer::AppendChild_String(char* ID, char* cData)
{
	EBMLString_Writer*	 e_String = NULL;
	if (cData && strlen(cData)) {
		(e_String = new EBMLString_Writer())->Define(GetDest(),ID,cData,NULL);
		AppendChild(e_String);
	}
	return e_String;
}

EBMLElement_Writer* EBMLElement_Writer::AppendChild_String(char* ID, CBuffer* cBuffer)
{
	if (cBuffer) return AppendChild_String(ID,cBuffer->AsString());
	return NULL;
}

EBMLElement_Writer* EBMLElement_Writer::AppendChild_Binary(char* ID, CBuffer* cBuffer)
{
	if (cBuffer) {
		EBMLElement_Writer*	 e_Binary;
		e_Binary = new EBMLElement_Writer(GetDest(),ID);
		e_Binary->SetData(cBuffer);
		AppendChild(e_Binary);
		return e_Binary;
	} else return NULL;
}

void EBMLElement_Writer::IncWriteOverhead(int iSize)
{
	info.iWriteOverhead += iSize;
}

__int64 EBMLElement_Writer::GetWriteOverhead()
{
	__int64 iRes = info.iWriteOverhead + (GetChild()?GetChild()->GetWriteOverhead_List():0);
	info.iWriteOverhead = 0;
	return iRes;
}

__int64 EBMLElement_Writer::GetWriteOverhead_List()
{
	EBMLElement_Writer* pCurr = this;
	__int64 iRes = 0;

	while (pCurr) {
		iRes += pCurr->GetWriteOverhead();
		pCurr = pCurr->GetNext();
	}

	return iRes;
}

__int64 EBMLOverheadElement_Writer::GetWriteOverhead()
{
	return GetSize();
}


	//////////////////
	// EBML Integer //
	//////////////////

EBMLUInt_Writer::EBMLUInt_Writer()
{
	CBuffer* cBuffer = new CBuffer;
	SetData(cBuffer);
	cBuffer->IncRefCount();
}

EBMLSInt_Writer::EBMLSInt_Writer()
{
	CBuffer* cBuffer = new CBuffer;
	SetData(cBuffer);
	cBuffer->IncRefCount();
}


void EBMLUInt_Writer::SetIData(__int64 iData)
{
	int  iSize = 0;
	__int64  iTemp = iData;

	while (iTemp) {
		iTemp = iTemp / 256;
		iSize++;
	}

	if (!iSize) iSize=1;

	GetData()->SetSize(iSize);
	char* cSize = GetData()->AsString();

	for (int i=iSize-1;i>=0;i--) {
		cSize[i] = (int)iData;
		iData = iData/256;
	}
}

void EBMLSInt_Writer::SetIData(__int64 iData)
{
	int  iSize = 0;
	__int64  iTemp = iData*2;

	while (iTemp) {
		iTemp = iTemp / 256;
		iSize++;
	}

	if (!iSize) iSize=1;

	GetData()->SetSize(iSize);
	char* cSize = GetData()->AsString();
	if (iData>0) {
		for (int i=iSize-1;i>=0;i--) {
			cSize[i] = (int)iData;
			iData = iData/256;
		}
	} else {
		iData = -iData-1;
		for (int i=iSize-1;i>=0;i--) {
			cSize[i] = (int)iData ^ 0xFF;
			iData = iData/256;
		}
	}
}

void EBMLUInt_Writer::Define(STREAM* pStream,char* ID,__int64 iData, EBMLElement_Writer* pNext)
{
	SetID(ID);
	SetIData(iData);
	SetNext(pNext);
	SetDest(pStream);
	SetChild(NULL);
	pLastChild = NULL;
}

	////////////////
	// EBML Float //
	////////////////

int ebmlfloatmode = 32;

void SetEBMLFloatMode(int imode) {
	ebmlfloatmode = imode;
}

EBMLFloat_Writer::EBMLFloat_Writer()
{
	CBuffer* cBuffer = new CBuffer;
	SetData(cBuffer);
	cBuffer->SetSize(ebmlfloatmode/8);
}
/*
void EBMLFloat_Writer::Delete()
{
	CBuffer* cBuffer = GetData();
	DecBufferRefCount(&cBuffer);
}
*/
void EBMLFloat_Writer::SetFData(long double ldData)
{
	float  fData = (float)ldData;
	double dData = (double)ldData;
	
	switch (GetData()->GetSize()) {
		case  4: reverse((char*)&fData, (char*)GetData()->GetData(), 4);
				 break;
		case  8: reverse((char*)&dData, (char*)GetData()->GetData(), 8);
                 break;
		case 10: reverse((char*)&ldData, (char*)GetData()->GetData(), 10);
			     break;
	}
}

void EBMLFloat_Writer::Define(STREAM* pStream,char* ID,long double fData, EBMLElement_Writer* pNext)
{
	SetID(ID);
	SetFData(fData);
	SetNext(pNext);
	SetDest(pStream);
}

	/////////////////
	// EBML String //
	/////////////////

EBMLString_Writer::EBMLString_Writer()
{
	SetData(new CStringBuffer);
}

__int64 EBMLString_Writer::GetSize()
{
	__int64 q;
	
	q = info.cData->GetSize()-1;
	info.iSize = q;
	if (info.iFlags & EWI_SIZELENFIXED) {
		info.iSizeLen = Int2VSUInt(&q, info.cSize, info.iSizeLen);
	} else info.iSizeLen = Int2VSUInt(&q, info.cSize);

	return q + info.iIDLen + info.iSizeLen;
}
/*
void EBMLString_Writer::Delete()
{
	CBuffer* cBuffer = GetData();
	DecBufferRefCount(&cBuffer);
}*/

void EBMLString_Writer::SetSData(char* cData)
{
	((CStringBuffer*)GetData())->Set(cData);
	__int64 q = strlen(cData);
	info.iSizeLen = Int2VSUInt(&q,info.cSize);
}

void EBMLString_Writer::Define(STREAM* pStream,char* ID,char* cData, EBMLElement_Writer *pNext)
{
	SetID(ID);
	SetSData(cData);
	SetNext(pNext);
	SetDest(pStream);
}


	/////////////////
	// EBML Header //
	/////////////////

EBMLHeader_Writer::EBMLHeader_Writer(void)
{
	ZeroMemory(&info,sizeof(info));
}

EBMLHeader_Writer::EBMLHeader_Writer(STREAM* pStream)
{
	ZeroMemory(&info,sizeof(info));
	info.iMaxSizeLength = 8;
	info.iVersion = 1;
	info.iReadVersion = 1;
	info.iVersion = 1;
	info.iReadVersion = 1;
	info.iMaxIDLength = 4;
	info.cDocType = new char[9];
	strcpy(info.cDocType,"matroska");
	SetDest(pStream);
	SetID((char*)EID_EBML);
//	info.cDocType = (char*)calloc(sizeof(char),9);
//	strcpy(info.cDocType,"matroska");

	e_Version = new EBMLUInt_Writer();
//	SetFixedSizeLen(5);
	SetChild(e_Version);

	e_Version->Define(GetDest(),(char*)EID_EBMLVersion,1, e_ReadVersion = new EBMLUInt_Writer());
	e_ReadVersion->Define(GetDest(),(char*)EID_EBMLReadVersion,1, e_MaxIDLength = new EBMLUInt_Writer());
	e_MaxIDLength->Define(GetDest(),(char*)EID_EBMLMaxIDLength,4, e_MaxSizeLength = new EBMLUInt_Writer());
	e_MaxSizeLength->Define(GetDest(),(char*)EID_EBMLMaxSizeLength,8,e_DocTypeVersion = new EBMLUInt_Writer());
	e_DocTypeVersion->Define(GetDest(),(char*)EID_DocTypeVersion,1,e_DocTypeReadVersion = new EBMLUInt_Writer());
	e_DocTypeReadVersion->Define(GetDest(),(char*)EID_DocTypeReadVersion,1,e_DocType = new EBMLString_Writer());
	e_DocType->Define(GetDest(),(char*)EID_DocType,"matroska",NULL);
}

void EBMLHeader_Writer::SetDocTypeVersion(int version)
{
	e_DocTypeVersion->SetIData(version);
	e_DocTypeReadVersion->SetIData(version);
}

void EBMLHeader_Writer::Delete()
{
	GetChild()->DeleteList();
	delete GetChild();
}

EBMLHeader_Writer::~EBMLHeader_Writer()
{

}

	/////////////////
	// SegmentInfo //
	/////////////////

EBMLMSegmentInfo_Writer::EBMLMSegmentInfo_Writer()
{
	ZeroMemory(&info,sizeof(info));
	info.iTimecodeScale = 1000000;
	SetID((char*)MID_SEGMENTINFO);
}

EBMLMSegmentInfo_Writer::EBMLMSegmentInfo_Writer(STREAM* pStream)
{
	ZeroMemory(&info,sizeof(info));
	SetDest(pStream);
	info.iTimecodeScale = 1000000;

	SetID((char*)MID_SEGMENTINFO);
}

void EBMLMSegmentInfo_Writer::Build()
{
	if (GetChild()) GetChild()->DeleteList();
	
	if (!info.bSegmentUID) {
		generate_uid(info.cSegmentUID, 16);
		info.bSegmentUID = true;
	}

	if (info.bSegmentUID)
		AppendChild(new EBMLElement_Writer(GetDest(), (char*)MID_SI_SEGMENTUID, new CBuffer(16, info.cSegmentUID, CBN_REF1)));

	if (info.bPrevUID)
		AppendChild(new EBMLElement_Writer(GetDest(), (char*)MID_SI_PREVUID, new CBuffer(16, info.cPrevUID, CBN_REF1)));

	if (info.bNextUID)
		AppendChild(new EBMLElement_Writer(GetDest(), (char*)MID_SI_NEXTUID, new CBuffer(16, info.cNextUID, CBN_REF1)));
	
	if (info.bSegmentFamily)
		AppendChild(new EBMLElement_Writer(GetDest(), (char*)MID_SI_SEGMENTFAMILY, new CBuffer(16, info.cSegmentFamily, CBN_REF1)));

	AppendChild_Float((char*)MID_SI_DURATION,info.fDuration,-1);
    AppendChild_String((char*)MID_SI_WRITINGAPP,info.cWritingApp);
	AppendChild_String((char*)MID_SI_MUXINGAPP,info.cMuxingApp);
	AppendChild_String((char*)MID_SI_TITLE,info.cTitle);
	AppendChild_UInt((char*)MID_SI_TIMECODESCALE,info.iTimecodeScale,1000000);
	return;
}

char* EBMLMSegmentInfo_Writer::GetTitle()
{
	if (info.cTitle)
		return info.cTitle->AsString();
	else
		return ("");
}

void EBMLMSegmentInfo_Writer::SetSegmentUID(char* cSUID) 
{
	if (cSUID) {
		memcpy(info.cSegmentUID, cSUID, 16);
		info.bSegmentUID = true;
	} else
		info.bSegmentUID = false;
}

void EBMLMSegmentInfo_Writer::SetNextSegmentUID(char* cNUID) 
{
	if (cNUID) {
		memcpy(info.cNextUID, cNUID, 16);
		info.bNextUID = true;
	} else
		info.bNextUID = false;

}
void EBMLMSegmentInfo_Writer::SetPrevSegmentUID(char* cPUID) 
{
	if (cPUID) {
		memcpy(info.cPrevUID, cPUID, 16);
		info.bPrevUID = true;
	} else
		info.bPrevUID = false;
}

void EBMLMSegmentInfo_Writer::SetSegmentFamily(char* cUID)
{
	if (cUID) {
		memcpy(info.cSegmentFamily, cUID, 16);
		info.bSegmentFamily = true;
	} else
		info.bSegmentFamily = false;
}

void EBMLMSegmentInfo_Writer::SetUID(int uidtype, char* cUID)
{
	switch (uidtype) {
		case UIDTYPE_NEXTUID: SetNextSegmentUID(cUID); break;
		case UIDTYPE_PREVUID: SetPrevSegmentUID(cUID); break;
		case UIDTYPE_SEGMENTFAMILY: SetSegmentFamily(cUID); break;
		case UIDTYPE_SEGMENTUID: SetSegmentUID(cUID); break;
	}
}

void EBMLMSegmentInfo_Writer::EnableNextUID(bool bEnabled)
{
	info.bWriteNextUID = true;
}

void EBMLMSegmentInfo_Writer::SetTimecodeScale(__int64 iScale)
{
	info.iTimecodeScale = iScale;
}

void EBMLMSegmentInfo_Writer::Delete()
{
//	if (GetChild()) 
//		GetChild()->DeleteList();
	EBMLElement_Writer::Delete();	

	DecBufferRefCount(&info.cMuxingApp);
	DecBufferRefCount(&info.cTitle);
	DecBufferRefCount(&info.cWritingApp);
}

void EBMLMSegmentInfo_Writer::SetWritingApp(CBuffer* cApp)
{
	SetCBuffer(cApp,&info.cWritingApp);
}

void EBMLMSegmentInfo_Writer::SetMuxingApp(CBuffer* cApp)
{
	SetCBuffer(cApp,&info.cMuxingApp);
}

void EBMLMSegmentInfo_Writer::SetTitle(CBuffer* cTitle)
{
	SetCBuffer(cTitle,&info.cTitle);
}

void EBMLMSegmentInfo_Writer::SetDuration(float fDuration)
{
	info.fDuration = fDuration;
}

	/////////////////////
	// Matroska Tracks //
	/////////////////////

EBMLMTrackInfo_Writer::EBMLMTrackInfo_Writer()
{
	iTrackCount = 0;
	tracks = NULL;
	SetID((char*)MID_TRACKS);
}

EBMLMTrackInfo_Writer::EBMLMTrackInfo_Writer(STREAM* pStream, void* p)
{
	iTrackCount = 0;
	tracks = NULL;
	SetDest(pStream);
	SetID((char*)MID_TRACKS);
	pParent = p;
}

void EBMLMTrackInfo_Writer::SetTrackCount(int iCount)
{
	tracks = (TRACK_DESCRIPTOR**)calloc(iCount,sizeof(TRACK_DESCRIPTOR*));

	if (iCount>iTrackCount) {
		for (int i=iTrackCount;i<iCount;tracks[i++]=(TRACK_DESCRIPTOR*)calloc(1,sizeof(TRACK_DESCRIPTOR)));
	}

	iTrackCount = iCount;
}

void EBMLMTrackInfo_Writer::SetTrackProperties(int iNbr,TRACK_DESCRIPTOR* track)
{
	// generate TrackUID if it is not present
	if (!track->iTrackUID) 
		generate_uid((char*)&track->iTrackUID, 4);
	
	if (iNbr<iTrackCount)
		memcpy(tracks[iNbr],track,sizeof(TRACK_DESCRIPTOR));
	
}

void EBMLMTrackInfo_Writer::Build()
{
	EBMLElement_Writer*	e_Track;
	EBMLElement_Writer*	e_video;
	EBMLElement_Writer*	e_audio;
	MATROSKA* m = (MATROSKA*)pParent;

	int					i,j;

	for (i=0;i<iTrackCount;i++) {

		e_Track = new EBMLElement_Writer();
		e_Track->SetDest(GetDest());
		e_Track->SetID((char*)MID_TR_TRACKENTRY);
		AppendChild(e_Track, APPEND_END | APPEND_DONT_RANDOMIZE);

		e_Track->AppendChild_UInt((char*)MID_TR_TRACKNUMBER,tracks[i]->iTrackNbr);
		e_Track->AppendChild_UInt((char*)MID_TR_TRACKUID,tracks[i]->iTrackUID);

		if (m->IsEnabled_WriteFlagDefault())
			e_Track->AppendChild_UInt((char*)MID_TR_FLAGDEFAULT,!!tracks[i]->iDefault,-1);
		
		if (m->IsEnabled_WriteFlagEnabled()) 
			e_Track->AppendChild_UInt((char*)MID_TR_FLAGENABLED,!!tracks[i]->iEnabled,-1);
		
		if (m->IsEnabled_WriteFlagLacing()) 
			e_Track->AppendChild_UInt((char*)MID_TR_FLAGLACING,!!tracks[i]->iLacing,-1);

		e_Track->AppendChild_UInt((char*)MID_TR_TRACKTYPE,tracks[i]->iTrackType,0);
		e_Track->AppendChild_UInt((char*)MID_TR_DEFAULTDURATION,tracks[i]->iDefaultDuration,0);
		e_Track->AppendChild_UInt((char*)MID_TR_MINCACHE,tracks[i]->iMinCache,-1);
		e_Track->AppendChild_UInt((char*)MID_TR_MAXCACHE,tracks[i]->iMaxCache,-1);
		e_Track->AppendChild_String((char*)MID_TR_CODECID,tracks[i]->cCodecID);
		if (tracks[i]->cCodecPrivate)
			tracks[i]->cCodecPrivate->IncRefCount();
		e_Track->AppendChild_Binary((char*)MID_TR_CODECPRIVATE,tracks[i]->cCodecPrivate);
		e_Track->AppendChild_String((char*)MID_TR_NAME,tracks[i]->cName);
		e_Track->AppendChild_String((char*)MID_TR_LANGUAGE,tracks[i]->cLngCode);
		e_Track->AppendChild_Float((char*)MID_TR_TRACKTIMECODESCALE,tracks[i]->fTrackTimecodeScale,-1);
		switch (tracks[i]->iTrackType) {
			case MSTRT_VIDEO:
				e_video = e_Track->AppendChild(new EBMLElement_Writer());
				e_video->SetID((char*)MID_TR_VIDEO);
				e_video->SetDest(GetDest());
				e_video->AppendChild_UInt((char*)MID_TRV_PIXELWIDTH,tracks[i]->video.iX1,-1);
				e_video->AppendChild_UInt((char*)MID_TRV_PIXELHEIGHT,tracks[i]->video.iY1,-1);

				e_video->AppendChild_UInt((char*)MID_TRV_PIXELCROPBOTTOM,tracks[i]->video.rCrop.bottom,-1);
				e_video->AppendChild_UInt((char*)MID_TRV_PIXELCROPTOP,tracks[i]->video.rCrop.top,-1);
				e_video->AppendChild_UInt((char*)MID_TRV_PIXELCROPLEFT,tracks[i]->video.rCrop.left,-1);
				e_video->AppendChild_UInt((char*)MID_TRV_PIXELCROPRIGHT,tracks[i]->video.rCrop.right,-1);

				e_video->AppendChild_UInt((char*)MID_TRV_DISPLAYWIDTH,tracks[i]->video.iX2,
					m->IsDisplayWidth_HeightEnabled()?-1:tracks[i]->video.iX1);
				e_video->AppendChild_UInt((char*)MID_TRV_DISPLAYHEIGHT,tracks[i]->video.iY2,
					m->IsDisplayWidth_HeightEnabled()?-1:tracks[i]->video.iY2);
				e_video->AppendChild_UInt((char*)MID_TRV_DISPLAYUNIT,tracks[i]->video.iDU,0);
				e_video->AppendChild_UInt((char*)MID_TRV_COLORSPACE,tracks[i]->video.iColorSpace,0);
				e_video->AppendChild_Float((char*)MID_TRV_GAMMAVALUE,tracks[i]->video.fGamma,0);
				break;
			case MSTRT_AUDIO:
				e_audio = new EBMLElement_Writer();
				e_audio->SetID((char*)MID_TR_AUDIO);
				e_audio->SetDest(GetDest());
				e_Track->AppendChild(e_audio);
				e_audio->AppendChild_Float((char*)MID_TRA_SAMPLINGFREQUENCY,tracks[i]->audio.fSamplingFrequency,8000);
				e_audio->AppendChild_Float((char*)MID_TRA_OUTPUTSAMPLINGFREQUENCY,tracks[i]->audio.fOutputSamplingFrequency,tracks[i]->audio.fSamplingFrequency);
				e_audio->AppendChild_UInt((char*)MID_TRA_CHANNELS,tracks[i]->audio.iChannels,1);
				for (j=0;j<tracks[i]->audio.iChannelPositionCount;j++) {
					e_audio->AppendChild_SInt((char*)MID_TRA_CHANNELPOSITIONS,tracks[i]->audio.iChannelPositions[j],0);
				}
				e_audio->AppendChild_UInt((char*)MID_TRA_BITDEPTH,tracks[i]->audio.iBitDepth,0);
				break;
		}
		if (!tracks[i]->track_compression.empty()) {
			EBMLElement_Writer* e_CENCs = new EBMLElement_Writer;
			e_CENCs->SetID((char*)MID_TR_CONTENTENCODINGS);
			e_CENCs->SetDest(GetDest());
			
			TRACK_COMPRESSION::iterator compression;
			for (compression=tracks[i]->track_compression.begin();
				compression != tracks[i]->track_compression.end(); compression++) {
				
				EBMLElement_Writer* e_CENC = new EBMLElement_Writer;
				e_CENC->SetID((char*)MID_TRCE_CONTENTENCODING);
				e_CENC->SetDest(GetDest());

				e_CENC->AppendChild_UInt((char*)MID_TRCE_CONTENTENCODINGTYPE, 0, -1);
				e_CENC->AppendChild_UInt((char*)MID_TRCE_CONTENTENCODINGSCOPE, 1, -1);
				e_CENC->AppendChild_UInt((char*)MID_TRCE_CONTENTENCODINGORDER, 
					compression->order, -1);

				EBMLElement_Writer* e_CCPR= new EBMLElement_Writer;
				e_CCPR->SetID((char*)MID_TRCE_CONTENTCOMPRESSION);
				e_CCPR->SetDest(GetDest());

				if (compression->compression == COMPRESSION_ZLIB) {
					e_CCPR->AppendChild_UInt((char*)MID_TRCE_CONTENTCOMPALGO, 0, -1);
				}

				if (compression->compression == COMPRESSION_HDRSTRIPPING) {
					e_CCPR->AppendChild_UInt((char*)MID_TRCE_CONTENTCOMPALGO, 3, -1);
					e_CCPR->AppendChild_Binary((char*)MID_TRCE_CONTENTCOMPSETTINGS,
						new CBuffer(compression->compression_private_size, 
							compression->compression_private,
							CBN_REF1));
				}

				e_CENC->AppendChild(e_CCPR);
				e_CENCs->AppendChild(e_CENC);
			}			
			
			e_Track->AppendChild(e_CENCs);
		}
		e_Track->EnableCRC32();
	}
}


	///////////////////////
	// Matroska Seekhead //
	///////////////////////

EBMLMSeekhead_Writer::EBMLMSeekhead_Writer()
{
	SetID((char*)MID_SEEKHEAD);
	iSpaceLeft = 1<<30;
}

EBMLMSeekhead_Writer::EBMLMSeekhead_Writer(STREAM* pStream)
{
	SetDest(pStream);
	SetID((char*)MID_SEEKHEAD);
	iSpaceLeft = 1<<30;
}

int EBMLMSeekhead_Writer::AddEntry(char* ID,__int64 iPosition, int iFlags)
{
	union {
		char _ID[4];
		int  _i;
	};
	int i,j;

	if ((iSpaceLeft>256) || (iFlags & 0x01)) {
		for (i=j=GetIDLength(ID)-1;i>=0;_ID[j-i--] = ID[i]);

		EBMLElement_Writer* e_SeekEntry = AppendChild(new EBMLElement_Writer(), APPEND_END | APPEND_DONT_RANDOMIZE);
		e_SeekEntry->SetDest(GetDest());
		e_SeekEntry->SetID((char*)MID_MS_SEEK);
		e_SeekEntry->AppendChild_UInt((char*)MID_MS_SEEKID,_i,0);
	
		EBMLUInt_Writer* e_Pos = (EBMLUInt_Writer*)e_SeekEntry->AppendChild_UInt((char*)MID_MS_SEEKPOSITION,iPosition,-1);

		iSpaceLeft -= (int)e_SeekEntry->GetSize();

		return ESHAE_OK;
	}
	else
		return ESHAE_FULL;
}

void EBMLMSeekhead_Writer::SetFixedSize(__int64 iSize)
{
	info.iFlags &=~ EWI_SIZEFIXED;

	if (iSize) {
		info.iFlags |= EWI_SIZEFIXED;
		info.iSize = iSize - info.iIDLen;
		GetSize();
		SetFixedSizeLen(info.iSizeLen);
		info.iSize = iSize - info.iSizeLen - info.iIDLen;
	};
	iSpaceLeft = (int)info.iSize;
}


	//////////////////////
	// Matroska Cluster //
	//////////////////////

EBMLMCluster_Writer::EBMLMCluster_Writer()
{
	Init();
	SetID((char*)MID_CLUSTER);
	iCurrentSize = 0;
}

EBMLMCluster_Writer::EBMLMCluster_Writer(STREAM* pStream,void* p, __int64 _iTimecode,
										 __int64 _iPrevSize,__int64 _iPosition)
{
	MATROSKA* m = (MATROSKA*)p;

	SetID((char*)MID_CLUSTER);
	SetDest(pStream);
	iPrevSize = _iPrevSize;
	iPosition = _iPosition;
	pParent = p;
	iMaxClusterSize = m->GetMaxClusterSize();
	iMaxClusterTime = m->GetMaxClusterTime();
	
	if (iMaxClusterTime <= 30000 || (m->Is1stClusterLimited() && !_iTimecode)) {
		iTimecode = _iTimecode;
	} else {
		iTimecode = _iTimecode + iMaxClusterTime - 30000;
	}

	AppendChild_UInt((char*)MID_CL_TIMECODE,iTimecode,-1);

	if (m->IsPrevClusterSizeEnabled()) {
		AppendChild_UInt((char*)MID_CL_PREVSIZE,iPrevSize,0);
	}
	
	if (m->IsClusterPositionEnabled()) {
		AppendChild_UInt((char*)MID_CL_POSITION,iPosition,-1);
	}

	EnableCRC32();
	iCurrentSize = GetSize();
}

void EBMLMCluster_Writer::Init()
{
	iTimecode = 0;
	iPrevSize = 0;
	iPosition = 0;
}

int EBMLMCluster_Writer::AddBlock(ADDBLOCK* a, BLOCK_INFO* lpInfo)
{
	MATROSKA* m = (MATROSKA*)pParent;

	int  i, iDiff;

	iDiff = (int)(a->iTimecode-iTimecode);
	if (iDiff > min(iMaxClusterTime,32000)) {
		return ABR_CLUSTERFULL;
	}

	if (iDiff < -32000)
		return ABR_CLUSTERFULL;

	if (iCurrentSize > 1024 && (iCurrentSize + a->cData->GetSize() > iMaxClusterSize)) {
		return ABR_CLUSTERFULL;
	}

	a->iTimecode -= iTimecode;
	
	if (m->GetMatroskaVersion() == 1 || a->iDuration) {
		lpInfo->discardable = 0;
		lpInfo->key_frame = 0;
		lpInfo->block_type = BLOCKTYPE_BLOCK;

		EBMLElement_Writer*  e_BG = AppendChild(new EBMLElement_Writer(GetDest(),(char*)MID_CL_BLOCKGROUP),
			APPEND_END | APPEND_DONT_RANDOMIZE);
		e_BG->AppendChild_UInt((char*)MID_CL_BLOCKDURATION,a->iDuration,0);

		for (i=0;i<a->iRefCount;i++) {
			e_BG->AppendChild_SInt((char*)MID_CL_REFERENCEBLOCK,a->iReferences[i],0);
		}

		e_BG->AppendChild(new EBMLMBlock_Writer(GetDest(),a, pParent, lpInfo));

		iCurrentSize += e_BG->GetSize();
	} else {
		lpInfo->discardable = 0;
		lpInfo->key_frame = (!a->iRefCount);
		lpInfo->block_type = BLOCKTYPE_SIMPLEBLOCK;

		EBMLElement_Writer* e;

		AppendChild(e=new EBMLMBlock_Writer(GetDest(),a, pParent, lpInfo), APPEND_END | APPEND_DONT_RANDOMIZE);

		iCurrentSize += e->GetSize();
	}

	return ABR_OK;
}
	
__int64 EBMLMCluster_Writer::GetTimecode()
{
	return iTimecode;
}


	////////////////////
	// Matroska Block //
	////////////////////

EBMLMBlock_Writer::EBMLMBlock_Writer()
{
	SetID((char*)MID_CL_BLOCK);
}

EBMLMBlock_Writer::EBMLMBlock_Writer(STREAM* pStream,ADDBLOCK* a, void* p, BLOCK_INFO* lpInfo)
{
	unsigned char	cBlockHdr[2048];
	unsigned char*   pFlag;
	unsigned char	cBlockHdrNew[2048];
	bool	bFixed = false;
	int		iHdrPos=0;
	int     iSize;
	int		iLaceHdrBegin;
	int		iHdrPosNew=0;
	__int64	iTrack = a->iStream;
	MATROSKA* m = (MATROSKA*)p;
	int		iLaceStyle = m->GetLaceStyle();

	int		kf_flag = (lpInfo->key_frame && lpInfo->block_type == BLOCKTYPE_SIMPLEBLOCK)?BLKHDRF_KEYFRAME:0;

	SetDest(pStream);
	iHdrPos += Int2VSUInt(&iTrack,(char*)(cBlockHdr+iHdrPos));
	iHdrPosNew += Int2VSUInt(&iTrack,(char*)(cBlockHdrNew+iHdrPosNew));
	
	// add timestamp
	cBlockHdr[iHdrPos++] = (int)((((unsigned int)a->iTimecode) / 256) & 0xFF);
	cBlockHdr[iHdrPos++] = (int)(((unsigned int)a->iTimecode) & 0xFF);

	cBlockHdrNew[iHdrPosNew++] = (int)((((unsigned int)a->iTimecode) / 256) & 0xFF);
	cBlockHdrNew[iHdrPosNew++] = (int)(((unsigned int)a->iTimecode) & 0xFF);

	iLaceHdrBegin = iHdrPos+1;
	pFlag = &cBlockHdr[iHdrPos];

	// laces of 1 frame are not allowed; only use lacing for 2 or more frames
	if (a->iFrameCountInLace<2) {
		cBlockHdr[iHdrPos++] = kf_flag;
		if (lpInfo) lpInfo->iLaceStyle = 0;
	} else {
		// xiph lacing or automatic
		if (iLaceStyle == LACESTYLE_XIPH || iLaceStyle == 0 || iLaceStyle == LACESTYLE_AUTO) {
			cBlockHdr[iHdrPos++] = 2 | kf_flag;
			cBlockHdr[iHdrPos++] = a->iFrameCountInLace-1;
			for (int i=0;(i<a->iFrameCountInLace-1)&&(iHdrPos<2040);i++) {
				int iSize = a->iFrameSizes[i];
				if (!iSize) {
					cBlockHdr[iHdrPos++]=0;
				} else while (iSize&&(iHdrPos<2040)) {
					if (iSize>=255) {
						cBlockHdr[iHdrPos++] = 255;
						iSize-=255; 
						if (!iSize) cBlockHdr[iHdrPos++] = 0;
					} else {
						cBlockHdr[iHdrPos++] = iSize;
						iSize = 0;
					}
				}
			}
		}

		// embl/fixed lacing or automatic
		if (iLaceStyle == LACESTYLE_EBML || iLaceStyle == LACESTYLE_AUTO) {
			int bEqual = true;
			for (int i=1;i<a->iFrameCountInLace;i++) {
				bEqual &= (a->iFrameSizes[i]==a->iFrameSizes[0]);
			}

			if (bEqual) {
				cBlockHdrNew[iHdrPosNew++] = 4 | kf_flag;
				cBlockHdrNew[iHdrPosNew++] = a->iFrameCountInLace-1;
				bFixed = true;
			} else {

				cBlockHdrNew[iHdrPosNew++] = 6 | kf_flag;
				cBlockHdrNew[iHdrPosNew++] = a->iFrameCountInLace-1;
				__int64 f = 0;
				for (int i=0;i<a->iFrameCountInLace-1;i++) {
					__int64 iSize = a->iFrameSizes[i]-f;
					char cSize[8];
					int j = (int)((i)?Int2VSSInt(&iSize,cSize):Int2VSUInt(&iSize,cSize));
					f=a->iFrameSizes[i];
					memcpy(cBlockHdrNew+iHdrPosNew,cSize,j);
					iHdrPosNew+=j;
				}
			}
		}

		if (iLaceStyle == LACESTYLE_AUTO) {
			if (iHdrPosNew < iHdrPos) {
				memcpy(cBlockHdr,cBlockHdrNew,iHdrPosNew);
				iHdrPos = iHdrPosNew;
				if (lpInfo) lpInfo->iLaceStyle = (bFixed)?LACESTYLE_FIXED:LACESTYLE_EBML;
			} else if (lpInfo) lpInfo->iLaceStyle = LACESTYLE_XIPH;
		} else
		if (iLaceStyle == LACESTYLE_EBML) {
			memcpy(cBlockHdr,cBlockHdrNew,iHdrPosNew);
			iHdrPos = iHdrPosNew;
			if (lpInfo) lpInfo->iLaceStyle = (bFixed)?LACESTYLE_FIXED:LACESTYLE_EBML;
		} else	if (lpInfo) lpInfo->iLaceStyle = LACESTYLE_XIPH;
	}

	IncWriteOverhead(iHdrPos);
	if (lpInfo) lpInfo->iLaceOverhead = iHdrPos-iLaceHdrBegin;
	CBuffer* cB = new CBuffer(a->cData->GetSize()+iHdrPos);
	cB->IncRefCount();

	char* cData = cB->AsString();
	memcpy(cData,cBlockHdr,iHdrPos);
	if (iSize = a->cData->GetSize()) {
		memcpy(cData+iHdrPos,a->cData->AsString(),iSize);
	}

	if (lpInfo->block_type == BLOCKTYPE_BLOCK)
		SetID((char*)MID_CL_BLOCK);
	else 
		SetID((char*)MID_CL_SIMPLEBLOCK);

	SetData(cB);

	if (iDontWriteBlockSizes) info.iWriteUnknownSize = 1;
}

	/////////////////////////
	// Matroska Cue Points //
	/////////////////////////

EBMLMCue_Writer::EBMLMCue_Writer()
{
	SetID((char*)MID_CUES);
	iLastTimecode = -100000;
}

EBMLMCue_Writer::EBMLMCue_Writer(STREAM* pStream)
{
	SetID((char*)MID_CUES);
	SetDest(pStream);
	iLastTimecode = -100000;
}

int EBMLMCue_Writer::AddCuePoint(int iTrack, __int64 iTimecode, __int64 iClusterPosition, __int64 iBlock)
{
	EBMLElement_Writer*	e_CuePoint;

	if (iTimecode != iLastTimecode) {
		e_CuePoint = AppendChild(new EBMLElement_Writer(GetDest(),(char*)MID_CU_CUEPOINT),
			APPEND_END | APPEND_DONT_RANDOMIZE);
		e_CuePoint->AppendChild_UInt((char*)MID_CU_CUETIME,iTimecode,-1);
	} else {
		e_CuePoint = pLastCuePoint;
	}

	EBMLElement_Writer* e_CueTrackPosition = e_CuePoint->AppendChild(new EBMLElement_Writer(GetDest(),
		(char*)MID_CU_CUETRACKPOSITIONS));
	e_CueTrackPosition->AppendChild_UInt((char*)MID_CU_CUECLUSTERPOSITION,iClusterPosition,-1);
	e_CueTrackPosition->AppendChild_UInt((char*)MID_CU_CUETRACK,iTrack,-1);
	e_CueTrackPosition->AppendChild_UInt((char*)MID_CU_CUEBLOCKNUMBER, iBlock, 0);

	pLastCuePoint = e_CuePoint;
	iLastTimecode = iTimecode;

	return 1;
}

int EBMLMCue_Writer::AddCuePoint(CUE_POINT& cue_point)
{
	EBMLElement_Writer*	e_CuePoint;

	e_CuePoint = AppendChild(new EBMLElement_Writer(GetDest(),(char*)MID_CU_CUEPOINT),
		APPEND_END | APPEND_DONT_RANDOMIZE);
	e_CuePoint->AppendChild_UInt((char*)MID_CU_CUETIME,cue_point.qwTimecode,-1);

	for (size_t i=0;i<cue_point.tracks.size();i++) {
		EBMLElement_Writer* e_CueTrackPosition = e_CuePoint->AppendChild(new EBMLElement_Writer(GetDest(),
			(char*)MID_CU_CUETRACKPOSITIONS));
		CUE_TRACKINFO& cti = cue_point.tracks[i];
		e_CueTrackPosition->AppendChild_UInt((char*)MID_CU_CUECLUSTERPOSITION,cti.qwClusterPos,-1);
		e_CueTrackPosition->AppendChild_UInt((char*)MID_CU_CUETRACK,cti.iTrack, -1);
		e_CueTrackPosition->AppendChild_UInt((char*)MID_CU_CUEBLOCKNUMBER, cti.iBlockNbr, 0);
	}

	return 1;
}

	///////////////////////
	// Matroska Chapters //
	///////////////////////

EBMLMChapter_Writer::EBMLMChapter_Writer()
{
	chapters = NULL;
}

EBMLMChapter_Writer::~EBMLMChapter_Writer()
{
	chapters->Delete();
	delete chapters;
}


// if bSetEnd == 0  ->  end of chapters is not manipulated, chapter is always written

int EBMLMChapter_Writer::RenderChapters(EBMLElement_Writer *pParent, CChapters* c, 
										CChapters* cParent, int parent_index,
										int bSetEnd, __int64 iMax)
{
	EBMLElement_Writer* e_ChapterAtom;
	__int64 iBegin; __int64 iEnd; 
	int bHidden; int bEnabled; int bSegUID;
	int last_chapter_found = 0;
	int first_chapter_found = 0;
	int drop_chapter = 0;

	for (int i=0;i<c->GetChapterCount();i++) {
		SINGLE_CHAPTER_DATA scd;
		c->GetChapter(i,&scd);
		iBegin = scd.iBegin; iEnd = scd.iEnd; 
		bHidden = scd.bHidden; bEnabled = scd.bEnabled; bSegUID = scd.bSegmentUIDValid;

		// chapters don't begin earlier than the segment
	//	if (iBegin < -1) iBegin = 0;

		if (bSegUID) {
		//	if (i == 0 && i != c->GetChapterCount() - 1)
//				drop_chapter = 1;
		}

		if (!drop_chapter) if (iBegin < iMaxEnd && iEnd > 0 || bSegUID || !bSetEnd) {
			e_ChapterAtom = pParent->AppendChild(new EBMLElement_Writer(GetDest(),(char*)MID_CH_CHAPTERATOM),
				APPEND_END | APPEND_DONT_RANDOMIZE);
			e_ChapterAtom->AppendChild_UInt((char*)MID_CH_CHAPTERUID,c->GetUID(i),0);
			
			e_ChapterAtom->AppendChild_UInt((char*)MID_CH_CHAPTERTIMESTART,iBegin,-1);
			e_ChapterAtom->AppendChild_UInt((char*)MID_CH_CHAPTERTIMEEND,iEnd,-1);
			e_ChapterAtom->AppendChild_UInt((char*)MID_CH_CHAPTERFLAGENABLED, bEnabled, 1);
			e_ChapterAtom->AppendChild_UInt((char*)MID_CH_CHAPTERFLAGHIDDEN, bHidden, 0);
			e_ChapterAtom->AppendChild_UInt((char*)MID_CH_CHAPTERPHYSICALEQUIV, scd.iPhysicalEquiv, 0);
			if (bSegUID)
				e_ChapterAtom->AppendChild_Binary((char*)MID_CH_CHAPTERSEGMENTUID, 
					new CBuffer(16, scd.cSegmentUID, CBN_REF1));
			
			int chdspcount = c->GetChapterDisplayCount(i);
			for (int j=0;j<chdspcount;j++) {
				EBMLElement_Writer* e_Display = new EBMLElement_Writer(GetDest(),(char*)MID_CH_CHAPTERDISPLAY);
				e_ChapterAtom->AppendChild(e_Display, APPEND_END | APPEND_DONT_RANDOMIZE);
				e_Display->AppendChild_String((char*)MID_CH_CHAPSTRING,new CStringBuffer(c->GetChapterText(i, j)));
				e_Display->AppendChild_String((char*)MID_CH_CHAPLANGUAGE,new CStringBuffer(c->GetChapterLng(i, j)));
			}

			if (c->HasSubChapters(i)) {
				RenderChapters(e_ChapterAtom,c->GetSubChapters(i),c,i, bSetEnd, iMax);
			}

			e_ChapterAtom->EnableCRC32();
		}
		
	}

	return 0;
}

/* replaces <end> markers of chapters with the real timecode,
   i.e. end of parent chapter / end of segment 
*/
void EBMLMChapter_Writer::UpdateChapterTimes(CChapters* c, int parent_index)
{
	int start = 0;
	int end = c->GetChapterCount();

	for (int j=start;j<end;j++) {

		SINGLE_CHAPTER_DATA scd;
		c->GetChapter(j, &scd);

		if (!scd.bIsEdition) {
			if (scd.iBegin < 0)
				c->SetChapterBegin(j, - c->GetBias(BIAS_UNSCALED));

			if (scd.iEnd == -1 && !scd.bSegmentUIDValid) {
				__int64 next_chapter_begin = c->GetChapterBegin(j+1);
	
				if (next_chapter_begin != CHAP_INVALIDINDEX)
					c->SetChapterEnd(j, next_chapter_begin);
				else {
					CChapters* P = c->GetParent(0);
					if (P->IsEdition(0)) {
						c->SetChapterEnd(j, iMaxEnd - c->GetBias(BIAS_UNSCALED));
					} else {
						c->SetChapterEnd(j, P->GetChapterEnd(parent_index));
					}
				}
			}	

			if (c->GetChapterEnd(j) > iMaxEnd && !scd.bSegmentUIDValid) {
				c->SetChapterEnd(j, iMaxEnd - c->GetBias(BIAS_UNSCALED));
			}

			if (c->GetChapterBegin(j) > iMaxEnd && !scd.bSegmentUIDValid) {
				c->DeleteChapter(j--);
				end--;
			}
		}

		if (c->HasSubChapters(j))
			UpdateChapterTimes(c->GetSubChapters(j), j);
	}
}

EBMLMChapter_Writer::EBMLMChapter_Writer(STREAM* p, CChapters* c, __int64 iMax, int bSetEnd)
{
	EBMLElement_Writer*	e_EditionEntry;
	SetDest(p);
	chapters = new CChapters();
	chapters->Import(c);


	SetID((char*)MID_CHAPTERS);
	if (iMax == -1) iMaxEnd = (_int64)(1<<30)*(1<<30); else	iMaxEnd = iMax;
	if (bSetEnd)
		UpdateChapterTimes(chapters, 0);

	c = chapters;

	for (int j=0;j<c->GetChapterCount();j++) {
		e_EditionEntry = AppendChild(new EBMLElement_Writer(GetDest(),(char*)MID_CH_EDITIONENTRY),
			APPEND_DONT_RANDOMIZE | APPEND_END);
		e_EditionEntry->AppendChild_UInt((char*)MID_CH_EDITIONFLAGDEFAULT, c->IsDefault(j), 0);
		e_EditionEntry->AppendChild_UInt((char*)MID_CH_EDITIONFLAGHIDDEN, c->IsHidden(j), 0);
		e_EditionEntry->AppendChild_UInt((char*)MID_CH_EDITIONFLAGORDERED, c->IsOrdered(j), 0);
		e_EditionEntry->AppendChild_UInt((char*)MID_CH_EDITIONUID, c->GetUID(j), 0);
		RenderChapters(e_EditionEntry, c->GetSubChapters(j), c, 0, bSetEnd, iMax);
		e_EditionEntry->EnableCRC32();
	}
	EnableCRC32();
}

