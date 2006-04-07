#ifndef I_BUFFER
#define I_BUFFER

#include "basestreams.h"

const int CBN_REF1	= 0x01;
const int CSB_ASCII = 0x04;
const int CSB_UTF8  = 0x08;
const int CSB_UTF16 = 0x10;


class CBuffer
{
	private:
		int		iRefCount;
		int		iSize;
		int		iAllocedSize;
	protected:
		void*	lpData;
	public:
		CBuffer();
		CBuffer(int iSize, void* lpData = NULL, int iFlags = 0);
		~CBuffer();
		int		virtual GetSize(void);
		bool	virtual DecRefCount();
		void	virtual IncRefCount();
		void	virtual SetRefCount(int i);
		void	virtual SetData(void* lpsource);
		void	virtual SetSize(int iSize);
		void	virtual* GetData();
		char	virtual* AsString();
		__int64 virtual AsBSWInt();
		double	virtual AsBSWFloat();
		__int64 virtual AsInt();
		__int64 virtual AsSBSWInt();

		int		virtual AsInt8();
		float	virtual AsFloat();
		int		virtual AsInt16();
		int		virtual AsInt24();
		int		virtual AsInt32();
		__int64 virtual AsInt40();
		__int64 virtual AsInt48();
		__int64 virtual AsInt56();
		__int64 virtual AsInt64();
		void	virtual Refer(CBuffer** p);
};

/*
class CMultimediaBuffer: public CBuffer
{
	private:
		__int64	qwStart_ns;
	public:
		CMultimediaBuffer();
		~CMultimediaBuffer();
		void	 virtual SetTimeStamp(__int64 qwTime);
		__int64 virtual GetTimeStamp(void);

};
*/

class CStringBuffer: public CBuffer
{
	private:
		bool	bASCII, bUTF8, bUTF16;
		CBuffer* b[3];
		int		iOutputFormat;
		int		GetOutputFormat();
		int		SetOutputFormat(int iFormat);
		void	Prepare(int iFormat);
	public:
		CStringBuffer();
		CStringBuffer(char* s, int iFlags = CBN_REF1);
		void	virtual Set(char* s, int iFlags = CSB_ASCII);
		char	virtual* Get(void);
		void	virtual* GetData();
		int		virtual GetSize(void);

		bool	virtual DecRefCount();
};

/*const int LISTINS_BEGIN  = 0x01;
const int LISTINS_END    = 0x02;


class CList
{
	private:
		CBuffer*	data;
		CList*		next;
		CList*		prev;
		bool		bEmpty;
		bool		IsEmpty();
	public:
		CList();
		~CList();
		void	virtual		Insert(CBuffer* lpBuffer,int iFlag);
		CList	virtual*	GetNext();
		CList	virtual*	GetPrev();
		void	virtual		SetData(CBuffer* lpBuffer);
		CBuffer virtual*	GetData();
		void	virtual		SetNext(CList* lpNext);
		void	virtual		SetPrev(CList* lpPrev);
};
*/

const int ATTRTYPE_INT64	= 0x01;
const int ATTRTYPE_ASCII	= 0x02;
const int ATTRTYPE_UTF8		= 0x03;
const int ATTRTYPE_BINARY	= 0x04;
const int ATTRTYPE_FLOAT	= 0x05;
const int ATTRTYPE_ATTRIBS	= 0x06;

const int FATTR_VALID = 0x01;
const int FATTR_ADDATTR_CREATE = 0x00;
const int FATTR_ADDATTR_DONTCREATE = 0x01;


typedef struct ATTRIBUTE_ENTRY
{
	union {
		void*		pData;
	};
	char*  cName;
	int	   iType;
	int    iFlags;
	struct ATTRIBUTE_ENTRY* pNext;
} ATTRIBUTE_ENTRY;

class CAttribs
{
	private:
		int				 iEntryCount;
		ATTRIBUTE_ENTRY** pEntries;
	protected:
		void				Init();
		int					Position(char* cName);
		ATTRIBUTE_ENTRY*	FindInLine(char* cName, int iLine);
		void				Add2Line(int iLine, ATTRIBUTE_ENTRY* e);
		ATTRIBUTE_ENTRY*	Find(char* cName);
		CAttribs*			Resolve(char* cPath, char** cName);
		void				DeleteLine(int iLine);
		void				DuplicateLine(CAttribs* a, int iLine);		
	public:
		CAttribs();
		CAttribs(int iSize);
		void				Add(char* cName, int iFlags, int iType, void* pData);
		void				AddInt(char* cName, int iFlags, __int64 pData);
		void				Set(char* cName, void* pData);
		void				SetInt(char* cName, __int64 pData);
		__int64				GetInt(char* cName);
		int					Exists(char* cName);
		CAttribs*			GetAttr(char* cName);
		void				GetStr(char* cName, char** cDest);
		void				Delete();
		CAttribs*			Duplicate();
		void				Export(CAttribs* a);
};

template <class T> void DecBufferRefCount(T** buffer)
{
	if (*buffer) {
		if (!(*buffer)->DecRefCount()) {
			delete *buffer;
			*buffer = NULL;
		}
	}
}


#endif