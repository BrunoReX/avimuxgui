// protokolliert den Index beim Schreiben von AVIs
// und baut Legacy-, Standard- und Superindices auf

#ifndef I_AVIINDICES
#define I_AVIINDICES

//#include "stdafx.h"
#include "..\basestreams.h"
#include "rifffile.h"
#include "avistructs.h"

const int AFE_OK = 0x01;
const int AFE_INVALIDPARAM = -0x01;
const int AFE_ENDOFSTREAM = -0x02;
const int AFE_INVALIDCALL = -0x03;
const int AFE_CANTREADFROMSOURCE = -0x04;

const int AVI_INDEX_OF_INDEXES	= 0x00;
const int AVI_INDEX_OF_CHUNKS	= 0x01;
const int AVI_INDEX_IS_DATA		= 0x80;
const int AVI_INDEX_2FIELD		= 0x01;

const int AVI_VIDEOSTREAM = 0x01;
const int AVI_AUDIOSTREAM = 0x02;
const int AVI_TEXTSTREAM  = 0x03;
const int INDEXTYPE_UNCHANGED = 0x00;

bool IsMP3SampleCount(int x);

#pragma pack(push,4)
class INDEX
{
	private:
		DWORD			dwStream;
		DWORD			dwKind;
		DWORD			dwFlags;
		_int64			qwOffset;
		DWORD			dwSize;
		DWORD			dwTest;
		__int64		qwBegin;
		__int64		qwEnd;
	public:
		INDEX*			lpNext;
		INDEX(void);
		~INDEX(void);
		void	virtual	SetData(DWORD dwStream,DWORD dwFlags,__int64 qwOffset,DWORD dwSize,DWORD dwKind);
		__int64		GetOffset(void);
		DWORD	virtual	GetChunkSize(void);
		DWORD	virtual	GetFlags(void);
		__int64 virtual GetBegin(void);
		__int64 virtual GetEnd(void);
		DWORD	virtual GetKindOfStream(void);
		DWORD   virtual GetStream(void);
		DWORD	virtual	SelectStream(DWORD dwStreamNbr);
		DWORD	virtual SetRange(__int64 _qwBegin,__int64 _qwEnd);
		DWORD	virtual	Store(void* lpDest);
		bool	virtual Valid(void);
		void	virtual Delete(void);
		DWORD	virtual	GetSize(void);
};

class EXTENDEDINDEX : public INDEX
{
	private:
		DWORD			dwSelectedStream;
	public:
		EXTENDEDINDEX(void);
		~EXTENDEDINDEX(void);
		DWORD			GetSelectedStream(void);
		DWORD	virtual	SelectStream(DWORD dwStreamNbr);
		DWORD	virtual GetDWORDsperEntry(void);
		DWORD	virtual	GetSize(void);
};

class SUPERINDEX : public EXTENDEDINDEX
{
	private:
		void*		lpstrf;
		__int64	qwCurrStreamSize;
		AVIStreamHeader* lpHeader;
	public:
		SUPERINDEX(void);
		~SUPERINDEX(void);
		DWORD	virtual GetDWORDsperEntry(void);
		DWORD			Store(void* lpDest);
		DWORD	virtual SetStreamFormat(void* lpFormat);
		DWORD	virtual	SetStreamHeader(AVIStreamHeader* lpHeader);
		DWORD			SetCurrentStreamSize(__int64 qwCSS);
		__int64			GetCurrentStreamSize(void);
		DWORD	virtual	GetSize(void);

};
#pragma pack(pop)

class STANDARDINDEX : public EXTENDEDINDEX
{
	public:
		STANDARDINDEX(void);
		~STANDARDINDEX(void);
		DWORD	virtual GetDWORDsperEntry(void);
		DWORD			Store(void* lpDest);
};

#endif