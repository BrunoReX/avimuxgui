#ifndef I_MATROSKASEGMENT
#define I_MATROSKASEGMENT

#include "ebml.h"
#include "ebml_matroska.h"
#include "matroska_clusters.h"
#include "chapters.h"

#define newz(a,b,c) c=new a[b]; ZeroMemory(c,b*sizeof(a))

const int TN2I_INVALID = -0x01;
const int U2TN_INVALID = -0x01;

const __int64 TIMECODE_UNKNOWN = -1;
const __int64 TIMECODE_UNINITIALIZED = -2;
const __int64 DURATION_UNKNOWN = -1;

const int SEEKMODE_NORMAL = 0x01;
const int SEEKMODE_IDIOT  = 0x02;

typedef struct
{
	int			iInterlaced;
	int			iStereoMode;
	int			iPixelWidth;
	int			iPixelHeight;
	int			iDisplayWidth;
	int			iDisplayHeight;
	int			iDisplayUnit;
	int			iAspectRatio;
	int			iColorSpace;
	float		fGamma;
} TRACK_INFO_VIDEO;

typedef struct
{
	float		fSamplingFrequency;
	float		fOutputSamplingFrequency;
	int			iChannels;
	CBuffer*	cChannelPositions;
	int			iBitDepth;
} TRACK_INFO_AUDIO;

const int TRACKTAGS_BITSPS         = 0x00000001;
const int TRACKTAGS_FRAMESPS       = 0x00000002;

const int COMPRESSION_NONE         = 0x00000000;
const int COMPRESSION_ZLIB         = 0x00000001;

typedef struct
{
	int		iFlags;
	__int64	iBitsPS;
	float	fFramesPS;
} TRACK_NUMERIC_TAGS;

typedef struct
{
	char*	cName;
	int		iType;
	int		iRef;
	union {
		char*	cData;
		void*	pData;
	};
} TAG;

typedef struct
{
	int		iCount;
	TAG**	tags;
} TAG_LIST;

typedef struct
{
	int		iCount;
	int*	pIndex;
} TAG_INDEX_LIST;
typedef struct
{
	__int64			qwRefTime;
	__int64			qwRefClusterPos;
	__int64			qwRefCodecStatePos;
	int				iRefNumber;
} CUE_REFERENCE;

typedef struct
{
	int				iTrack;				// track index, NOT track number
	int				iBlockNbr;
	int				iRefCount;
	__int64			qwClusterPos;
	__int64			qwCodecStatePos;
	CUE_REFERENCE**	pReferences;
} CUE_TRACKINFO;

typedef struct 
{
	__int64			qwTimecode;
	int				iTracks;
	CUE_TRACKINFO**	pTracks;
} CUE_POINT;

typedef struct
{
	int				iCuePoints;
	CUE_POINT**		pCuePoints;
} CUES;

typedef struct
{
	CBuffer*			cUID;
	CBuffer*			cFilename;
} SEGID;

typedef struct
{
	int				iCount;
	CUE_POINT**		point;
} TRACK_CUE_POINTS;

typedef struct
{
	// information embedded in file
	int			iType;
	int			iNbr;
	int			iEnabled;
	int			iDefault;
	int			iLacing;
	int			iSparse;
	int			iUID;
	__int64		qwDefaultDuration;
	__int64		qwBias;
	__int64		iDataQueued;
	int			iMinCache;
	int			iMaxCache;
	double		fTimecodeScale;
	CBuffer*	cName;
	CBuffer*	cLanguage;
	CBuffer*	cCodecID;
	CBuffer*	cCodecName;
	CBuffer*	cCodecPrivate;
	TRACK_INFO_AUDIO	audio;
	TRACK_INFO_VIDEO	video;
	TRACK_NUMERIC_TAGS	tags;
	TRACK_CUE_POINTS* cue_points;
	TAG_INDEX_LIST* pTags;
	// additional stuff
	int			iSelected;
	__int64	    iLastBlockEndTimecode;
	int			iCompression;

} TRACK_INFO;

typedef struct
{
	int				iCount;						// number of tracks
	int				iCountByType[256];			// number of tracks of type [i]
	int*			iTable;						// index -> track number
	int*			iIndexTableByType[256];     // j-th track of type i has index 
	                                            //   iIndexTableByType[i][j]
	TRACK_INFO**	track_info;
} TRACKS;

const int TAGTYPE_STRING = 0x01;
const int TAGTYPE_BINARY = 0x02;


int tagAdd(TAG_LIST** pTags, char* cName, char* cText);
int tagAddIndex(TAG_INDEX_LIST** pTagIndexList, int iIndex);


const int CLIV_POS	= 0x01;
const int CLIV_TIME = 0x02;

typedef struct
{
	EBMLELEMENTLIST* e_SegmentInfo;
	EBMLELEMENTLIST* e_Cues;
	EBMLELEMENTLIST* e_Tracks;
	EBMLELEMENTLIST* e_Tracks2;
	EBMLELEMENTLIST* e_Chapters;
	EBMLELEMENTLIST* e_Seekhead;
	EBMLELEMENTLIST* e_Tags;

} GLOBALELEMENTS;



typedef struct
{
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

	CStringBuffer*		cTitle;
	CBuffer*			cMuxingApp;
	CBuffer*			cWritingApp;
	
	TRACKS*				tracks;
	CLUSTERS*			clusters;
	EBMLM_Cluster*		pActiveCluster;
	GLOBALELEMENTS*		pGlobElem;

	SEGID				CurrSeg, PrevSeg, NextSeg;
	CHAPTERS*			chapters;
	CChapters*			cChapters;
	CUES*				cues;
	TAG_LIST*			pAllTags;
	TAG_INDEX_LIST*		pGlobalTags;
	void**				queue;
} SEGMENT_INFO;

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
		void    virtual RetrieveChapters(EBMLELEMENTLIST** e_Chapters);
		void	virtual RetrieveCues(EBMLELEMENTLIST** e_Cues);
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
	public: 
		EBMLM_Segment(STREAM* s,EBMLElement* p); 
		CUES*			GetCues();
		SEGMENT_INFO*	GetSegmentInfo();
		int				GetTrackCount();
		int	virtual		ReadBlock(READBLOCK_INFO* pInfo);
		__int64 virtual GetTrackDuration(int iTrack);
		__int64 virtual GetMasterTrackDuration();

		GETTYPSTR; 
		void			AllowObfuscatedSegments(bool bAllow);
		void			Seek(__int64 iTimecode);
		int				UID2TrackIndex(int iUID);
		void			EnableShiftFirstClusterTimecode2Zero(bool bEnable = true);
};
#endif