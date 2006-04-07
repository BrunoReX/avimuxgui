#ifndef I_EBML
#define I_EBML

#include "buffers.h"
#include "FormatInt64.h"

#pragma warning(disable:4305)
#pragma warning(disable:4309)

const char EID_EBML[] = { 0x1A, 0x45, 0xDF, 0xA3 };
const char EID_EBMLVersion[] = { 0x42, 0x86 };
const char EID_EBMLReadVersion[] = { 0x42, 0xF7 };
const char EID_EBMLMaxIDLength[] = { 0x42, 0xF2 };
const char EID_EBMLMaxSizeLength[] = { 0x42, 0xF3 };
const char EID_DocType[] = { 0x42, 0x82 };
const char EID_DocTypeVersion[] = { 0x42, 0x87 };
const char EID_DocTypeReadVersion[] = { 0x42, 0x85 };

const int EBMLTYPE_UNKNOWN				= -0x01;
const int EBMLTYPE_EBML					= 0x01;
const int EBMLTYPE_EBMLVersion			= 0x02;
const int EBMLTYPE_DOCTYPE				= 0x03;
const int EBMLTYPE_EBMLReadVersion		= 0x04;
const int EBMLTYPE_EBMLMaxIDLength		= 0x05;
const int EBMLTYPE_EBMLMaxSizeLength	= 0x06;
const int EBMLTYPE_DOCTYPEVERSION		= 0x07;
const int EBMLTYPE_DOCTYPEREADVERSION	= 0x08;

typedef struct {
	char* cName;
	char* cID;
} EID_DESCRIPTOR;

extern const int VSizeInt_Len[256];
extern const __int64 undefined_lengths[];

typedef struct
{
	int			iCount;
	char**		pIDs;
	__int64*	piPositions;
} SUBELEMENT_INFO;

// generic EBML Element
class EBMLElement
{
	private:
		EBMLElement*	pParent;
		__int64			qwElementLength;
		__int64			qwStreamPos;
		__int64			qwHdrPos;
		STREAM*			stream;
	//	char*			cEBMLID;
		bool			bCanBeMulti;
		int				iType;
		int				iDataType;
		int				iSearchLimit;
	// subelements
		SUBELEMENT_INFO	sub_elements;

	protected:
		CBuffer*		buffer;
		bool			virtual CheckIDs(char* iID,EBMLElement** p);
		bool			virtual CheckGlobalIDs(char* iID,EBMLElement** p);

		bool			virtual CompIDs(char* ID1, char* ID2);
		__int64			virtual ReadLength();
		int				virtual ReadID(char** iID);
		int				virtual ReadID(char* iID);
		void			virtual DetermineLength();
		int				virtual GetIDLength(char* _ID = NULL);
		int				virtual InsertElement(void** e, EBMLElement* seg, 
									char* ID, EBMLElement* pos);
		void			virtual SetParent(EBMLElement* p);
		void			virtual SetStream(STREAM* s);
		void			virtual SetMulti(bool multi);
		void			virtual SetDataType(int data_type);
		void			virtual SetType(int type);
		void			virtual SetElementLength(__int64 size);
		int				virtual GetSearchLimit();
		void			virtual SetHeaderPos(__int64 p);
	public:
		STREAM			virtual* GetSource();
		EBMLElement();
		EBMLElement(STREAM* s,EBMLElement* p);
		~EBMLElement();
		__int64			virtual GetAbsoluteHeaderPos();
		int				virtual GetHeaderSize();
		__int64			virtual GetRelativeHeaderPos();
		int				virtual SeekStream(__int64 qwPos);
		void			virtual Delete();
		bool			virtual CanBeMulti();
		bool			virtual IsMaster();
		int				virtual GetDataType();
		CBuffer			virtual* GetData();
		__int64			virtual GetLength();
		int				virtual GetLevel();
		__int64			virtual GetParentLength();
		EBMLElement		virtual* GetParent();

		__int64			virtual GetStreamPos();
		__int64			virtual GetRelPos();
		int				virtual GetType();
		__int64			virtual AsInt();
		__int64			virtual AsSInt();
		double			virtual AsFloat();
		char			virtual* GetTypeString();
		EBMLElement		virtual* Create(EBMLElement** p,char* ID = NULL);
		EBMLElement		virtual* Create(EBMLElement** p,char** ID);
		EBMLElement		virtual* Create1stSub(EBMLElement** p,char* ID = NULL);
		bool			virtual HasSucc();
		EBMLElement		virtual* GetSucc();
		EBMLElement		virtual* FindNext(char* ID);
		EBMLElement		virtual* Resync(EID_DESCRIPTOR* pDescr, int iDescrCount);
		void			virtual SetSearchLimit(int iLimit);
		int				virtual SearchMulti(void*** dest, char** iID, int iIDCount, char* iID2StopAt = NULL);
		int				virtual Search(void** dest,char* iID,char* iID2StopAt = NULL);
};

typedef struct
{
	int				iCount;
	EBMLElement**	pElement;
} EBMLELEMENTLIST;

void DeleteElementLists(EBMLELEMENTLIST*** pList, int iCount);
void DeleteElementList(EBMLELEMENTLIST** pList);

template<class T> void DeleteEBML(T** p)
{
	((EBMLElement*)(*p))->Delete();
	delete *p;
	*p = NULL;
}

// unbekanntes EBML-Element
class EBML_UNKNOWN: public EBMLElement
{
	private:
		bool			virtual CheckIDs(char* iID,EBMLElement** p);
	public:
		char			virtual* GetTypeString();
		EBML_UNKNOWN(STREAM* s,EBMLElement* p);
};

// EBML
class EBML_EBML: public EBMLElement
{
	private:
		bool			virtual CheckIDs(char* iID,EBMLElement** p);
	public:
		char			virtual* GetTypeString();
		EBML_EBML(STREAM* s,EBMLElement* p);
};

// EBMLVersion
class EBML_EBMLVersion: public EBMLElement
{
	public:
		char			virtual* GetTypeString();
		EBML_EBMLVersion(STREAM* s,EBMLElement* p);
};

// EMBLReadVersion
class EBML_EBMLReadVersion: public EBMLElement
{
	public:
		char			virtual* GetTypeString();
		EBML_EBMLReadVersion(STREAM* s,EBMLElement* p);
};

// EMBLMaxIDLength
class EBML_EBMLMaxIDLength: public EBMLElement
{
	public:
		char			virtual* GetTypeString();
		EBML_EBMLMaxIDLength(STREAM* s,EBMLElement* p);
};

// EMBLMaxIDLength
class EBML_EBMLMaxSizeLength: public EBMLElement
{
	public:
		char			virtual* GetTypeString();
		EBML_EBMLMaxSizeLength(STREAM* s,EBMLElement* p);
};

// DocType
class EBML_DocType: public EBMLElement
{
	public:
		char			virtual* GetTypeString();
		EBML_DocType(STREAM* s,EBMLElement* p);
};

// DocTypeVersion
class EBML_DocTypeVersion: public EBMLElement
{
	public:
		char			virtual* GetTypeString();
		EBML_DocTypeVersion(STREAM* s,EBMLElement* p);
};

// DocTypeReadVersion
class EBML_DocTypeReadVersion: public EBMLElement
{
	public:
		char			virtual* GetTypeString();
		EBML_DocTypeReadVersion(STREAM* s,EBMLElement* p);
};

const int EBMLDATATYPE_MASTER	= 0x01;
const int EBMLDATATYPE_INT		= 0x02;
const int EBMLDATATYPE_SINT		= 0x08;
const int EBMLDATATYPE_FLOAT	= 0x03;
const int EBMLDATATYPE_ASCII	= 0x04;
const int EBMLDATATYPE_UTF8		= 0x05;
const int EBMLDATATYPE_BIN		= 0x06;
const int EBMLDATATYPE_HEX		= 0x07;

bool Comp_EBMLIDs(char* ID1,char* ID2);

#endif