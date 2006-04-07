#ifndef I_MATROSKAWRITING
#define I_MATROSKAWRITING

#include "ebml.h"
#include "chapters.h"

// set some global Settings
void MATROSKA_WriteBlockSizes(bool bWrite = true);
void MATROSKA_RandomizeElementOrder(bool bRandomize = true);

typedef struct
{
	char*					ID;
	char					cSize[8];	// size coded as variable size big endian
	__int64					iSize;		// real size; ususally GetData()->GetSize()
	int						iIDLen;		// length of ID
	int						iSizeLen;	// length of 'size'
	int						iFlags;
	CBuffer*				cData;
	STREAM*					stream;
	__int64					qwStreamPos;
	__int64					iWriteOverhead;
	int						iWriteUnknownSize;
} EBMLWRITER_INFO;

const int	EWI_SIZELENFIXED		= 0x01;
const int   EWI_SIZEFIXED			= 0x02;
const int	APPEND_BEGIN			= 0x01;
const int	APPEND_END				= 0x02;
const int   APPEND_DONT_RANDOMIZE	= 0x04;

void reverse(char* source, char* dest, int iCount);
void SetEBMLFloatMode(int imode);

class EBMLElement_Writer: public EBMLElement
{
	private:
		EBMLElement_Writer*		pNext;
		EBMLElement_Writer*		pChild;
		int						bWriteCRC;

	protected:
		EBMLElement_Writer*		pLastChild;
		EBMLWRITER_INFO			info;
		EBMLElement_Writer*		GetNext();
		EBMLElement_Writer*		GetChild();
		STREAM*					GetDest();
		void					SetCBuffer(CBuffer* cSource,CBuffer** cDest);
		void					IncWriteOverhead(int iSize);
		int						Put(void* pSource, int iSize, char* pDest = NULL, int* iDest = NULL);
		int						IsCRCEnabled();
		EBMLElement_Writer*		pCRC32;
	public:
		EBMLElement_Writer();
		EBMLElement_Writer(STREAM* stream,char* ID = NULL,CBuffer* cBuffer = NULL);
		~EBMLElement_Writer();
		void					SetID(char* ID);
		void					SetChild(EBMLElement_Writer* pChild);
		void					SetNext(EBMLElement_Writer* pNext);
		void					SetData(CBuffer* cBuffer);
		void					SetFixedSizeLen(int iSize);
		void					SetFixedSize(__int64 iSize);
		void					SetDest(STREAM* pStream);
		void					EnableCRC32(int bEnabled = true);
		void			virtual Delete();
		void			virtual DeleteList();

		// updates size related members of the info structure and returns the overall size of the element
		__int64			virtual	GetSize();

		CBuffer			virtual* GetData();
		__int64					GetListSize();

		// writes (1) one single element including all child elements  (2) as well as all elements linked through pNext
		__int64		virtual		Write(char* pDest = NULL, int* iDest = NULL);
		__int64		virtual		WriteList(char* pDest = NULL, int* iDest = NULL);
		__int64		virtual		GetWriteOverhead();
		__int64		virtual		GetWriteOverhead_List();

		// child elements
		EBMLElement_Writer*		AppendChild(EBMLElement_Writer* e_Child, int where = APPEND_END);
		EBMLElement_Writer*		AppendChild_UInt(char* ID, __int64 iData, __int64 iDefault = 0, int where = APPEND_END);
		EBMLElement_Writer*		AppendChild_SInt(char* ID, __int64 iData, __int64 iDefault = 0);
		EBMLElement_Writer*		AppendChild_Float(char* ID, long double fData, long double fDefault = 0);
		EBMLElement_Writer*		AppendChild_String(char* ID, char* cData);
		EBMLElement_Writer*		AppendChild_String(char* ID, CBuffer* cBuffer);
		EBMLElement_Writer*		AppendChild_Binary(char* ID, CBuffer* cBuffer);
};


class EBMLOverheadElement_Writer: public EBMLElement_Writer
{
	public:
			__int64		virtual		GetWriteOverhead();

};

class EBMLUInt_Writer: public EBMLOverheadElement_Writer
{
	public: 
		EBMLUInt_Writer();
//		void	virtual			Delete();
		void	virtual			SetIData(__int64 iData);
		void	virtual			Define(STREAM* pStream,char* ID,__int64 iData, EBMLElement_Writer* pNext);
};

class EBMLSInt_Writer: public EBMLUInt_Writer
{
	public: 
		EBMLSInt_Writer();
		void	virtual			SetIData(__int64 iData);
};

class EBMLFloat_Writer: public EBMLElement_Writer
{
	public: 
		EBMLFloat_Writer();
		void					Delete();
		void					SetFData(long double ldData);
		void					Define(STREAM* pStream,char* ID,long double fData, EBMLElement_Writer* pNext);
};

class EBMLString_Writer: public EBMLElement_Writer
{
	public: 
		EBMLString_Writer();
		void					Delete();
		void					SetSData(char* cData);
		void					Define(STREAM* pStream,char* ID,char* cData, EBMLElement_Writer* pNext);
		__int64			virtual	GetSize();
};

typedef struct {
	int		iVersion;
	int		iReadVersion;
	int		iMaxIDLength;
	int		iMaxSizeLength;
	char*	cDocType;
	int		iDocTypeVersion;
	int		iDocTypeReadVersion;
} EBMLHEADER_INFO;

typedef struct {
	__int64		iTimecodeScale;
	CBuffer*	cSegmentUID;
	CBuffer*	cTitle;
	CBuffer*	cMuxingApp;
	CBuffer*	cWritingApp;
	float		fDuration;
} SEGMENTHEADER_INFO;

class EBMLHeader_Writer: public EBMLElement_Writer
{
	private:
		EBMLHEADER_INFO		info;
	protected:
		EBMLElement_Writer *e_Main;
		EBMLString_Writer  *e_DocType;
		EBMLUInt_Writer    *e_Version, *e_ReadVersion, *e_MaxIDLength, *e_MaxSizeLength, \
			               *e_DocTypeVersion, *e_DocTypeReadVersion; 

	public:
		EBMLHeader_Writer();
		EBMLHeader_Writer(STREAM* pStream);

		~EBMLHeader_Writer();
		void					virtual Delete();
};

class EBMLMSegmentInfo_Writer: public EBMLElement_Writer
{
	private:
		SEGMENTHEADER_INFO		info;
	protected:
	public:
		EBMLMSegmentInfo_Writer();
		EBMLMSegmentInfo_Writer(STREAM* pStream);
		void					Build();
		void					Delete();

		void					SetWritingApp(CBuffer* cApp);
		void					SetMuxingApp(CBuffer* cApp);
		void					SetTitle(CBuffer* cTitle);
		void					SetDuration(float fDuration);
		void					SetTimecodeScale(__int64 iScale);
};

typedef struct
{
	int		iX1, iY1, iX2, iY2, iDU;
	int		iAspectRatioType;
	int		iColorSpace;
	float   fGamma;
} VIDEOTRACK_SPECIFIC;

typedef struct
{
	float	fSamplingFrequency;
	float   fOutputSamplingFrequency;
	int		iChannels;
	int		iChannelPositionCount;
	int*	iChannelPositions;
	int		iBitDepth;
} AUDIOTRACK_SPECIFIC;

const int MATROSKA_TDDM_NEVER  = 0x01;
const int MATROSKA_TDDM_GAP    = 0x02;
const int MATROSKA_TDDM_ALWAYS = 0x03;

typedef struct 
{
	int				iLacing;
	int				iEnabled;
	int				iDefault;
	int				iTrackNbr;
	int				iTrackUID;
	int				iTrackType;
	int				iMinCache;
	int				iMaxCache;
	int				iLaceSchemes[5];
	int				iBlocksWritten;
	int				iFramesWritten[5];
	int				iLaceOverhead[5];
	__int64			iTotalSize;
	__int64			iTotalFrameCount;
	__int64			iDefaultDuration;
	int				iDurationMode;
	int				iBufferedBlocks;
	float			fTrackTimecodeScale;
	CStringBuffer*	cCodecID;
	CBuffer*		cCodecPrivate;
	CStringBuffer*	cName;
	CStringBuffer*	cLngCode;
	int				i;
	int				iCompression;
	VIDEOTRACK_SPECIFIC	video;
	AUDIOTRACK_SPECIFIC audio;
} TRACK_DESCRIPTOR;

class EBMLMTrackInfo_Writer: public EBMLElement_Writer
{
	private:
		int					iTrackCount;
		TRACK_DESCRIPTOR**	tracks;
		void*		pParent;

	protected:

	public:
		EBMLMTrackInfo_Writer();
		EBMLMTrackInfo_Writer(STREAM* pStream, void* p);

		void				SetTrackCount(int iCount);
		void				SetTrackProperties(int iNbr, TRACK_DESCRIPTOR* track);
		void				Build();
};

const int ESHAE_OK		= 0x01;
const int ESHAE_FULL	= 0x02;

class EBMLMSeekhead_Writer: public EBMLElement_Writer
{
	private:
		int					iSpaceLeft;
	public:
		EBMLMSeekhead_Writer();
		EBMLMSeekhead_Writer(STREAM* pStream);

		int					AddEntry(char* ID, __int64 iPosition, int iFlags = 0);
		void				SetFixedSize(__int64 iSize);
};

typedef struct
{
	CBuffer*	cData;
	int			iStream;
	int			iFlags;
	int			iRefCount;
	__int64		iTimecode;
	__int64		iDuration;
	__int64		iReferences[2];
// laced input
	int			iFrameCountInLace;
	int*		iFrameSizes;
} ADDBLOCK;

const int ABR_OK			= 0x00;
const int ABR_CLUSTERFULL	= 0x01;

const int ABSSM_INDEX		= 0x00;
const int ABSSM_NUMBER		= 0x01;
const int ABSSM_MASK        = 0x01;

const int ABTC_SCALED		= 0x00;
const int ABTC_UNSCALED		= 0x02;
const int ABTC_MASK         = 0x02;

const int ABTC_GAP          = 0x04;

typedef struct {
	int		iLaceStyle;
	int		iLaceOverhead;
} BLOCK_INFO;

class EBMLMCluster_Writer: public EBMLElement_Writer
{
	private:
		__int64		iTimecode;
		__int64		iPrevSize;
		__int64		iPosition;
		__int64		iMaxClusterSize;
		__int64		iMaxClusterTime;
		__int64		iCurrentSize;
		void*		pParent;
	public:
		void Init();
		EBMLMCluster_Writer();
		EBMLMCluster_Writer(STREAM* pStream, void* p, __int64 iTimecode = 0, __int64 iPrevSize = 0, __int64 iPosition = 0);
		int			 AddBlock(ADDBLOCK* a, BLOCK_INFO* lpInfo);
		__int64		 GetTimecode();
};

class EBMLMBlock_Writer: public EBMLElement_Writer
{
	private:
	public:
		EBMLMBlock_Writer();
		EBMLMBlock_Writer(STREAM* pStream,ADDBLOCK* a, void* m, BLOCK_INFO* lpInfo);
};

class EBMLMCue_Writer: public EBMLElement_Writer
{
	private:
		__int64	iLastTimecode;
		EBMLElement_Writer* pLastCuePoint;
	public:
		EBMLMCue_Writer();
		EBMLMCue_Writer(STREAM* pStream);
		int		AddCuePoint(int iTrack, __int64 iTimecode, __int64 iClusterPosition, __int64 iBlock = 0);
};

class EBMLMChapter_Writer: public EBMLElement_Writer {
	private:
		CChapters* chapters;
		__int64	   iMaxEnd;
	public:
		EBMLMChapter_Writer();
		int RenderChapters(EBMLElement_Writer* pParent, CChapters* c, CChapters* cParent, int parent_index);
		EBMLMChapter_Writer(STREAM* pStream, CChapters* c, __int64 iMax);

};

#endif