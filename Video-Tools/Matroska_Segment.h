#ifndef I_MATROSKASEGMENT
#define I_MATROSKASEGMENT

#ifndef uint
#define uint unsigned int
#endif

#include "ebml.h"
#include "ebml_matroska.h"
#include "matroska_clusters.h"
#include "chapters.h"
#include "Tags.h"

#define newz(a,b,c) c=new a[b]; ZeroMemory(c,b*sizeof(a))

const int TN2I_INVALID = -0x01;
const int TUID2TI_INVALID = -0x01;


const int SEEKMODE_NORMAL = 0x01;
const int SEEKMODE_IDIOT  = 0x02;

typedef struct
{
	__int64			qwRefTime;
	__int64			qwRefClusterPos;
	__int64			qwRefCodecStatePos;
	int				iRefNumber;
} CUE_REFERENCE;

class CUE_TRACKINFO
{
	int				size;
public:
	CUE_TRACKINFO();

	int				GetSize();

	int				iTrack;				// reading: track index; writing: track number
	int				iBlockNbr;
	int				iRefCount;
	__int64			qwClusterPos;
	__int64			qwCodecStatePos;
	CUE_REFERENCE**	pReferences;
};

class CUE_POINT
{
	int				size;
public:
	CUE_POINT();
	__int64			qwTimecode;
	int				GetSize();
	std::vector<CUE_TRACKINFO> tracks;
};

typedef struct
{
	int				iCuePoints;
	CUE_POINT**		pCuePoints;
} CUES;

typedef struct
{
	__int64			id;
	__int64			position;
} SEEKHEAD_ENTRY;

typedef struct 
{
	__int64			position;
	__int64			size;
} SEEKHEAD_INFO;

typedef std::vector<SEEKHEAD_ENTRY> SEEKHEAD_ENTRIES;
typedef std::vector<SEEKHEAD_INFO> SEEKHEADS;

typedef struct
{
	int			iInterlaced;
	int			iStereoMode;
	int			iPixelWidth;
	int			iPixelHeight;
	RECT		rPixelCrop;
	int			iDisplayWidth;
	int			iDisplayHeight;
	int			iDisplayUnit;
	int			iAspectRatio;
	int			iColorSpace;
	double		fGamma;
} TRACK_INFO_VIDEO;

typedef struct
{
	double		fSamplingFrequency;
	double		fOutputSamplingFrequency;
	int			iChannels;
	CBuffer*	cChannelPositions;
	int			iBitDepth;
} TRACK_INFO_AUDIO;

const int TRACKTAGS_BITSPS         = 0x00000001;
const int TRACKTAGS_FRAMESPS       = 0x00000002;


const int TRACKCOMPRESSION_BLOCKS			= 0x01;
const int TRACKCOMPRESSION_CODECPRIVATE		= 0x02;

/* Tag Target Types */
const int TTT_TRACK                = 0x00000001;
const int TTT_CHAPTER              = 0x00000002;
const int TTT_EDITION              = 0x00000003;
const int TTT_ATTACHMENT           = 0x00000004;

typedef struct
{
	int		iFlags;
	__int64	iBitsPS;
	float	fFramesPS;
} TRACK_NUMERIC_TAGS;


class SEGID
{
public:
	SEGID();
	bool		bValid;
	char		cUID[16];
	CBuffer*	cFilename;
};

typedef struct
{
	size_t			iCount;
	CUE_POINT**		point;
} TRACK_CUE_POINTS;



class TRACK_INFO : public CHasTitles
{
public:
	TRACK_INFO();
	virtual ~TRACK_INFO();
	// information embedded in file
	int			iType;
	int			iNbr;
	int			iEnabled;
	int			iForced;
	int			iDefault;
	int			iLacing;
	int			iSparse;
	__int64		iUID;
	__int64		qwDefaultDuration;
	__int64		iTrackOffset;
	__int64		iDataQueued;
	int			iMinCache;
	int			iMaxCache;
	double		fTimecodeScale;
//	CBuffer*	cName;
	CBuffer*	cLanguage;
	CBuffer*	cCodecID;
	CBuffer*	cCodecName;
	CBuffer*	cCodecPrivate;
	TRACK_INFO_AUDIO	audio;
	TRACK_INFO_VIDEO	video;
	TRACK_NUMERIC_TAGS	tags;
	TRACK_CUE_POINTS* cue_points;
	// additional stuff
	int			iSelected;
	__int64	    iLastBlockEndTimecode;


	TRACK_COMPRESSION track_compression;
	TAG_INDEX_LIST pTags;
};

typedef struct
{
	size_t			iCount;						// number of tracks
	int				iCountByType[256];			// number of tracks of type [i]
	int*			iTable;						// index -> track number
	int*			iIndexTableByType[256];     // j-th track of type i has index 
	                                            //   iIndexTableByType[i][j]
	TRACK_INFO**	track_info;
} TRACKS;

const int FATT_POSITION = 0x01;
const int FATT_ORIGINALUID = 0x02;
const int FATT_NAME     = 0x04;
const int FATT_DESCR    = 0x08;
const int FATT_MIMETYPE = 0x10;
const int FATT_SIZE     = 0x20;
const int FATT_B0RKED   = 0x40;

class ATTACHMENT
{
public:
	ATTACHMENT();
	ATTACHMENT(const ATTACHMENT& copy);
	virtual ~ATTACHMENT();
	CStringBuffer*	file_description;
	CStringBuffer*	file_name;
	CStringBuffer*	file_mime_type;
	__int64			file_uid;
	__int64			file_pos;
	__int64			file_size;
	int				flags;

	bool operator ==(ATTACHMENT& att);
	bool operator <(ATTACHMENT& att);
};

typedef struct {
	std::vector<ATTACHMENT> attachments;
	int count;
	int uid_recreated;
} ATTACHMENTS;

const int TAGTYPE_STRING = 0x01;
const int TAGTYPE_BINARY = 0x02;


int tagAdd(TAG_LIST** pTags, char* cName, char* cText, char* cLng);
int tagAddIndex(TAG_INDEX_LIST &pTagIndexList, int iIndex);


const int CLIV_POS	= 0x01;
const int CLIV_TIME = 0x02;

class GLOBALELEMENTS
{
public:
	GLOBALELEMENTS();
	virtual ~GLOBALELEMENTS();

	EBMLELEMENTLIST* e_SegmentInfo;
	EBMLELEMENTLIST* e_Cues;
	EBMLELEMENTLIST* e_CuePoints;

	EBMLELEMENTLIST* e_Tracks;
	EBMLELEMENTLIST* e_Tracks2;
	
	EBMLElementVector e_Chapters;
	EBMLElementVector e_Seekhead;

	EBMLElementVector e_Tags;
	EBMLElementVector e_Attachments;
} ;

const int PARSECUEPOINTS_INDEX = 0x01;
const int PARSECUEPOINTS_TIME  = 0x02;

class SEGMENT_INFO : public CHasTitles
{
public:
	SEGMENT_INFO();

	EBML_INFO			ebml_info;
	double				fDuration;				// segment duration
	__int64				qwBias;					
	__int64				iFirstClusterTimecode;	// timecode of first cluster
	int					iEndReached;			// end-of-segment reached
	int					iActiveClusterIndex;
	int					iTimecodeScale;			// timecode scale of segment
	int					iMasterStream;
	int					iSeekMode;

	__int64*			iBlockCount;		// number of blocks of Track [i]
	__int64				iTotalBlockCount;	// total number of blocks
	__int64*			iOtherBlocksThan;	// blocks of tracks[j!=i] read since the last block of track i

//	CStringBuffer*		cTitle;
	CBuffer*			cMuxingApp;
	CBuffer*			cWritingApp;
	
	TRACKS*				tracks;
	CLUSTERS*			clusters;
	EBMLM_Cluster*		pActiveCluster;
	GLOBALELEMENTS*		pGlobElem;

	SEGID				CurrSeg, PrevSeg, NextSeg, SegmentFamily;
	CHAPTERS*			chapters;
	CChapters*			pChapters;
	CUES*				cues;
	TAG_LIST*			pAllTags;
	void**				queue;

	TAG_INDEX_LIST		pGlobalTags;
	ATTACHMENTS			attachments;
	SEEKHEAD_ENTRIES	seekhead_entries;
	SEEKHEADS			seekheads;
};

typedef struct
{
	int					bShiftFirstClusterToZero;
	int					bInfoRetrieved;
} SEGMENT_INFO_PRIVATE;

class EBMLM_Segment : public EBML_MatroskaElement { 
	private:
		SEGMENT_INFO*		SegmentInfo;
		SEGMENT_INFO_PRIVATE InfoPrivate;
		bool				bAllowObfuscatedSegments;
		EBMLM_Cluster*  SetActiveCluster(EBMLM_Cluster* pCluster);
	// return cluster timecode, either unmodified or shifted by -(timecode of 1st cluster)
		__int64			GetShiftedClusterTimecode(EBMLM_Cluster* pCluster);
		void	virtual RetrieveSegmentInfo(EBMLELEMENTLIST* e_SIList);
		void	virtual RetrieveSubChapters(EBMLElement* pMaster,CHAPTER_INFO** pInfo);
		void    virtual RetrieveChapters(EBMLElementVector& e_Chapters);
		void	virtual RetrieveCues(EBMLELEMENTLIST** e_Cues, __int64 time_start = 0, __int64 time_end = -1);
		void	virtual RetrieveAttachments(EBMLElementVector& attachments, ATTACHMENTS& dest);
		CUE_POINT	virtual* GetCuePoint(int index);
		void	virtual ParseCuePoints(CUES* cues, __int64 start, __int64 end, int mode);
		int     virtual RetrieveLastCuepoint(int iTrack, int* index2);
		__int64	virtual RetrieveLastBlockEndTimecode(int iTrack);
	protected: 
		CHECKIDs; 
		void	virtual Delete();
		int				RetrieveInfo();
		EBMLM_Cluster*	LocateCluster_MetaSeek(__int64 qwTime);
		EBMLM_Cluster*	LocateCluster_Cues(__int64 qwTime);
		EBMLM_Cluster*  GetActiveCluster();
		int				GetActiveClusterIndex();
		int				NextCluster(EBMLM_Cluster** p);
		int				TrackNumber2Index(int iNbr);
		int		virtual CheckCRC();
	public:
		EBMLM_Segment(STREAM* s,EBMLElement* p);
		virtual ~EBMLM_Segment();
		CUES*			GetCues(__int64 time_start = 0, __int64 time_end = -1);
		SEGMENT_INFO*	GetSegmentInfo();
		int				GetTrackCount();
		int	virtual		ReadBlock(READBLOCK_INFO* pInfo);
		__int64 virtual GetTrackDuration(int iTrack);
		__int64 virtual GetMasterTrackDuration();

		GETTYPSTR; 
		void			AllowObfuscatedSegments(bool bAllow);
		void			Seek(__int64 iTimecode);
		int				TrackUID2TrackIndex(__int64 iUID);
		void			EnableShiftFirstClusterTimecode2Zero(bool bEnable = true);
		bool			VerifyCuePoint(int index);
};
#endif