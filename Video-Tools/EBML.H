#ifndef I_EBML
#define I_EBML

#pragma warning(disable:4786)

#include "buffers.h"
#include "FormatInt64.h"
#include "CRC.h"
#include "EBML_IDs.h"
#include <vector>

#pragma warning(disable:4305)
#pragma warning(disable:4309)

int EBMLUIntLen(__int64 x);
int UIntLen(__int64 x);



typedef struct
{
	int			iCount;
	char**		pIDs;
	__int64*	piPositions;
} SUBELEMENT_INFO;

class EBML_INFO
{
private:
public:
	EBML_INFO();
	virtual ~EBML_INFO();
	int		EBMLVersion;
	int		EBMLReadVersion;
	int		EBMLMaxIDLengh;
	int		EBMLMaxSizeLength;
	char*	DocType;
	int		DocTypeVersion;
	int		DocTypeReadVersion;
};

const int EBML_CRC_NOT_PRESENT = 0x01;
const int EBML_CRC_FAILED      = 0x00;
const int EBML_CRC_OK          = 0x02;
const int EBML_CRC_NOT_CRC32   = 0x03;
const int EBML_CRC_NOT_CHECKED = 0x04;

const int EBML_MANDATORY_ELEMENT_NOT_FOUND = -0x01;

typedef struct
{
	char**	ids;	// NULL-terminated array of IDs to look for
	void**	dests;  // pointers to store value of first occurence at (ints and doubles only)
	int*	occ;	// array with mandatory/unique conditions: 
	                //   bit 0 -> mandatory, bit 1 -> unique, bit 2 -> almost mandatory
} SEARCHMULTIEX;

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
		int				bLengthUndefined;
		unsigned __int32 iCRC;
		int				iCRCcalculated;
		int				iCRCresult;
	// subelements
		SUBELEMENT_INFO	sub_elements;

	protected:
		CBuffer*		buffer;
		int				virtual AutoCheckCRC32();
		bool			virtual CheckIDs(char* iID,EBMLElement** p);
		bool			virtual CheckGlobalIDs(char* iID,EBMLElement** p);

		bool			virtual CompIDs(char* ID1, char* ID2);
		__int64			virtual ReadLength();
		int				virtual ReadID(char** iID);
		int				virtual ReadID(char* iID);
		void			virtual DetermineLength();
		int				virtual GetIDLength(char* _ID = NULL);
		int				virtual InsertElement(void** e, EBMLElement* seg, char* ID, EBMLElement* pos);
		int				virtual InsertElement(std::vector<EBMLElement*>& e, EBMLElement* seg, char* ID, EBMLElement* pos);

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
		virtual			~EBMLElement();
		void			virtual Delete();

		__int64			virtual GetAbsoluteHeaderPos();
		__int64			virtual GetRelativeHeaderPos();
		__int64			virtual GetStreamPos();
		__int64			virtual GetRelPos();
		int				virtual GetHeaderSize();

		CBuffer			virtual* GetData();
		__int64			virtual GetLength();
		__int64			virtual GetParentLength();

		int				virtual SeekStream(__int64 qwPos);
		bool			virtual CanBeMulti();
		bool			virtual IsMaster();
		int				virtual GetDataType();

		bool			virtual IsLengthUndefined();
		int				virtual GetLevel();
		EBMLElement		virtual* GetParent();
		int				virtual CheckCRC();
		int				virtual Verify();

		int				virtual GetType();

		char			virtual* GetTypeString();
		EBMLElement		virtual* Create(EBMLElement** p,char* ID = NULL);
		EBMLElement		virtual* Create(EBMLElement** p,char** ID);
		EBMLElement		virtual* Create1stSub(EBMLElement** p,char* ID = NULL);
		bool			virtual HasSucc();
		EBMLElement		virtual* GetSucc();
		EBMLElement		virtual* FindNext(char* ID);
		EBMLElement		virtual* Resync(EID_DESCRIPTOR* pDescr, int iDescrCount);
		void			virtual SetSearchLimit(int iLimit);

		/* get element value */
		__int64			virtual AsInt();
		__int64			virtual AsSInt();
		double			virtual AsFloat();
		char			virtual* AsString();

		/* search for multiple elements, create an array of EBMLELEMENTLIST* */
		int				virtual SearchMulti(void*** dest, char** iID, int iIDCount, char* iID2StopAt = NULL);

		/* search for multiple elements and create a vector of vectors of EBMLElements */
		int				virtual SearchMulti(std::vector<std::vector<EBMLElement*> > &dest, char** iID, char* iID2StopAt = NULL);

		/* search for multiple elements, check for occurence restrictions "mandatory" and "unique" 
		   and save value of each first occurence to the specified variable. Return -1 if a mandatory
		   element is not found */
		int				virtual SearchMulti(std::vector<std::vector<EBMLElement*> > &dest, SEARCHMULTIEX& search, char* iID2StopAt = NULL);

		/* search for elements of one type */
		int				virtual Search(void** dest,char* iID2Find,char* iID2StopAt = NULL);
		int				virtual Search(std::vector<EBMLElement*> &dest,char* iID2Find,char* iID2StopAt = NULL);
		int				virtual Search(std::vector<EBMLElement*> &dest,char* iID2Find,char* iID2StopAt, int occurence, __int64* pTarget);

		bool	operator< (EBMLElement& first);
};

typedef struct
{
	size_t			iCount;
	EBMLElement**	pElement;
	EBMLElement* operator() (size_t index = NULL);	// return i-th element of that list
	__int64 operator[] (int index);				// return integer value of i-th element of that list
	__int64 operator^ (__int64 _default);		// return value of 1st element in that list of default if there is none

} EBMLELEMENTLIST;


typedef std::vector<EBMLElement*> EBMLElementVector;
typedef std::vector<std::vector<EBMLElement*> > EBMLElementVectors;

void DeleteElementLists(EBMLELEMENTLIST*** pList, int iCount);
void DeleteElementList(EBMLELEMENTLIST** pList);

void EBML_EnableCRC32AutoCheck(int bEnabled);
void EBML_SetCRC32AutoCheck_MinLevel(int min_level);

template<class T> void DeleteEBML(T** p)
{
	if (*p) {
		((EBMLElement*)(*p))->Delete();
		delete *p;
		*p = NULL;
	}
}

template<class T> void DeleteVector(std::vector<T> p)
{
	std::vector<T>::iterator iter = p.begin();
	size_t i = p.size();

	for (size_t j=0;j<i;j++) {
		(*iter)->Delete();
		delete (*iter++);
	}
}

template<class T> void DeleteVectors(std::vector<std::vector<T> > p)
{
	std::vector<std::vector<T> >::iterator iter = p.begin();
	size_t i = p.size();

	for (size_t j=0;j<i;j++)
		DeleteVector (*iter++);
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