#include "stdafx.h"
#include "matroska.h"
#include "Matroska_Segment.h"
#include "Matroska_Block.h"
#include "warnings.h"
#include "math.h"
#include "integers.h"
#include "ebml.h"
#include "stdio.h"
//#include "stdlib.h"

const int iSizeOfSeekhead = 1024;
const int iBufsPerStream = 32;
char debugmsg[1024];

int matroska_guess_overhead(MGO_DESCR* lpMGO)
{
	int iRes = 0;
	int iLaceSize;
	int	iLaceOvh;
	int iClCount,iClCountS=0,iClCountT=0;
	int iCueCount;
	int	iOvhPF;
	int iOvhPC;
	int	iPrevSize;
	float fLaceDur;

	MGO_STREAMDESCR* d;

	iOvhPC = 0;
// clusters
		iPrevSize = 5;
		if (lpMGO->iFlags & MGOF_CLUSTERS_BYSIZE) {
			iClCountS = (int)(lpMGO->iFinalSize / lpMGO->iClusterSize);
			
			// cluster over 2 MB need 4 bytes for size indicator, instead of 3
			if (lpMGO->iClusterSize>=2<<21) iOvhPC++;
			if (lpMGO->iClusterSize<=2<<16) iPrevSize--;
		}
		if (lpMGO->iFlags & MGOF_CLUSTERS_BYTIME) {
			iClCountT = (int)(lpMGO->iDuration * 1000 / lpMGO->iClusterTime);
		}
		iClCount = max(iClCountS,iClCountT);
		iOvhPC += 11;
		if (!(lpMGO->iFlags & MGOF_CLUSTERS_NOINDEX)) iOvhPC += 18;
		iOvhPC += 6; // CRC32
		if (lpMGO->iFlags & MGOF_CLUSTERS_PREVSIZE) iOvhPC += iPrevSize;
		if (lpMGO->iFlags & MGOF_CLUSTERS_POSITION) iOvhPC += 6;

		iRes += iOvhPC * iClCount;

// cues
	iCueCount = int(lpMGO->iDuration * lpMGO->fFPS / lpMGO->iKeyframeInterval);
	iRes += 18 * iCueCount;

// video stream
	iRes += (int)(lpMGO->fFPS * lpMGO->iDuration * 13 - 3* lpMGO->fFPS * lpMGO->iDuration / lpMGO->iKeyframeInterval);
// audio streams
	for (int i=0;i<lpMGO->iStreamCount;i++) {
		d = &lpMGO->pStreams[i];

		if (!(d->iFlags & MGOSF_FRAMEDUR_IND)) {
			d->fFrameDuration = 24;
		}

		if (!(d->iFlags & MGOSF_FRAMESIZE_IND)) {
			d->iFramesize = 1000;
		}

		if (!(d->iFlags & MGOSF_FRAMECOUNT_IND)) {
			d->fFrameCountPerLace = 8;
		}

		if ((d->iFlags & MGOSF_LACE) && (d->iFramesize < 1020 || d->iLaceStyle!=LACESTYLE_XIPH) && d->fDurPerLace>1) {
		// laced audio
			if (!(d->iFlags & MGOSF_LACEDUR_IND)) {
				iLaceSize = (int)(d->iFramesize * d->fFrameCountPerLace);
				fLaceDur = d->fFrameDuration * d->fFrameCountPerLace;
			} else {
				fLaceDur = d->fDurPerLace;
				float f = (fLaceDur / d->fFrameDuration);
				d->fFrameCountPerLace = float(fLaceDur / d->fFrameDuration);
				if (d->fFrameCountPerLace - int(d->fFrameCountPerLace) > 0.0001) {
					d->fFrameCountPerLace=(float)int(d->fFrameCountPerLace+1);
				}
				iLaceSize = (int)(d->iFramesize * d->fFrameCountPerLace);
				fLaceDur = d->fFrameCountPerLace * d->fFrameDuration;
			}

			// number of frames in lace: 1 byte
			iLaceOvh = 1;

			// blockgroup, block etc
			iLaceOvh += 8 + 2*!!(iLaceSize > 250) + 2*!!(iLaceSize > 16375);
			
			if (d->iLaceStyle == LACESTYLE_XIPH) {
				iLaceOvh += (int)(d->fFrameCountPerLace * ( 1+ d->iFramesize/255));
			} else {
				if (d->iLaceStyle == LACESTYLE_AUTO) {
					if (!(d->iFlags & MGOSF_CBR)) {
						iLaceOvh += (int)((d->fFrameCountPerLace-1) * 1.3 + 2.0);
					} else {
						iLaceOvh += 0;
					}
				}
			}

			iRes += (int)(lpMGO->iDuration * 1000 * iLaceOvh / fLaceDur);

		} else {
		// no laced audio

			// blockgroup, block etc
			iOvhPF = 8 + 2*(!!(d->iFramesize > 250) + !!(d->iFramesize > 16375));
			iRes += int(lpMGO->iDuration * 1000 * iOvhPF / d->fFrameDuration);

		}
	}

	return iRes;

}

char* matroska_GetCompileTimestamp() {
	return __TIMESTAMP__;
}

char* matroska_GetCompileDate() {
	return __DATE__;
}



/*
/////////////////////////
// binomial heap stuff //
/////////////////////////

typedef struct
{
	__int64 key;
	void*   pData;
	void*	pChildren;
} BINHEAP_ELEMENT;

typedef struct
{
	int					iCount;
	int					iMinIndex;
	int					iAllocatedSize;
	int					iLastInUse;
	BINHEAP_ELEMENT**	pElements;
} BINOMIAL_HEAP;

void binheap_Init(BINOMIAL_HEAP** lpbh)
{
	*lpbh = new BINOMIAL_HEAP;
	ZeroMemory(*lpbh, sizeof(BINOMIAL_HEAP));

	(*lpbh)->iLastInUse = -1;
	(*lpbh)->pElements = new BINHEAP_ELEMENT*[20];
	(*lpbh)->iAllocatedSize = 20;
	ZeroMemory((*lpbh)->pElements, (*lpbh)->iAllocatedSize*sizeof(BINHEAP_ELEMENT*));
}

void binheap_join_left(BINOMIAL_HEAP* h1, int index)
{
	if (h1->pElements[index]) {
		if (!h1->pElements[index+1]) {
			h1->pElements[index+1] = h1->pElements[index];
			h1->pElements[index] = NULL;
		} else {
			BINOMIAL_HEAP* ch1 = (BINOMIAL_HEAP*)h1->pElements[index]->pChildren;
			ch1->pElements[ch1->iLastInUse+1] = h1->pElements[index];
			h1->pElements[index] = NULL;
			binheap_join_left(h1, index+1);
		}
	}
}


void binheap_merge_element(BINOMIAL_HEAP* h1, BINOMIAL_HEAP* h2, int index)
{
	int i = index;
	int j = i;
	__int64 m1 = h1->pElements[h1->iMinIndex]->key;
	__int64 m2 = h2->pElements[h2->iMinIndex]->key;

	if (!h1->pElements[i]) {
		h1->pElements[i] = h2->pElements[i];
		if (m2 < m1) h1->iMinIndex = h2->iMinIndex;
	} else {

		BINOMIAL_HEAP* ch1 = (BINOMIAL_HEAP*)h1->pElements[j]->pChildren;
		ch1->pElements[++ch1->iLastInUse] = h2->pElements[j];
		binheap_join_left(h1, j);
	}
}

void binheap_merge(BINOMIAL_HEAP* h1, BINOMIAL_HEAP* h2)
{
	__int64 m1 = h1->pElements[h1->iMinIndex]->key;
	__int64 m2 = h2->pElements[h2->iMinIndex]->key;
	int		j;
	// merge h2 into h1
	for (int i=h2->iLastInUse;i>=0;i--) {
		if (h2->pElements[i]) {	
			binheap_merge_element(h1, h2, i);
		}
	}

	h2->iLastInUse = max(h1->iLastInUse, h2->iLastInUse);

}

  */

MATROSKA::MATROSKA()
{
	e_EBML = NULL;
	e_Main = NULL;
	e_Segment = NULL;
	bAllowObfuscatedFiles = false;
	queue = NULL;
	iStreams2Queue = NULL;
	SegmentInfo = NULL;
	iSingleStream = -1;
	e_Tracks = NULL;
	tracks = NULL;
	e_Segments = NULL;
	e_Header = NULL;
	iTrackCount = 0;
	ZeroMemory(&ws,sizeof(ws));
	iAVImode = 1;
	e_Cluster = NULL;
	e_PrimarySeekhead = NULL;
	e_SecondarySeekhead = NULL;
	e_SegInfo = NULL;
	chapters = NULL;
	iDebugLevel = 1;
	pActiveSegInfo = NULL;
	max_chapter_duration = 0;
	iMaxBlockGapFactor = 10;
}

MATROSKA::~MATROSKA()
{
}

__int64 MATROSKA::GetTrackDuration(int iTrack)
{
	EBMLM_Segment*		seg = (EBMLM_Segment*)e_Segments->pElement[GetActiveSegment()];

	return seg->GetTrackDuration(MapT(iTrack));
}

void MATROSKA::SetMaxBlockGapFactor(int iFactor)
{
	iMaxBlockGapFactor = iFactor;
}

int MATROSKA::GetMaxBlockGapFactor()
{
	return iMaxBlockGapFactor;
}

void MATROSKA::SelectStreams(int* iStreams)
{
	for (int i=0;i<GetTrackCount();i++) {
		pActiveSegInfo->tracks->track_info[i]->iSelected = 0;
	}
	while (*iStreams != -1) {
		pActiveSegInfo->tracks->track_info[*iStreams++]->iSelected = 1;
	}
}

int MATROSKA::IsStreamQueued(int i)
{
	if (iSingleStream > -1) {
		return i == iSingleStream;
	}
	return pActiveSegInfo->tracks->track_info[i]->iSelected;
}

int MATROSKA::StreamListLen(int* piList)
{
	int i = 0;
	if (!piList) return 0;
	while (*piList++ != -1) i++;
	return i;
}

STREAM* MATROSKA::GetSource()
{
	return stream;
}

int MATROSKA::GetActiveSegment()
{
	return info->iActiveSegment;
}

void MATROSKA::FlushQueues()
{
	int i;
	if (queue) {
		for (i=0;i<GetTrackCount();i++) {
			QUEUE_kill(&queue[i]);
		}
	}
}

int MATROSKA::SetActiveSegment(int iSegment)
{
	if (iSegment<GetSegmentCount() && iSegment > -1) {
		if (pActiveSegInfo) pActiveSegInfo->queue = (void**)queue;
		queue = (QUEUE**)SegmentInfo[iSegment]->queue;
		info->iActiveSegment = iSegment;
		pActiveSegInfo = SegmentInfo[iSegment];
		return 0;
	} else {
		return -1;
	}
}

int MATROSKA::SetActiveTrack(int iTrack, int iType)
{
	if (iType == SAT_ALL) {
		info->iActiveTrack = iTrack;
	} else {
		if (iType < 256 && iTrack < GetTrackCount(iType)) {
			info->iActiveTrack = pActiveSegInfo->tracks->iIndexTableByType[iType][iTrack];
		} else {
			return -1;
		}
	}

	pActiveTrackInfo = pActiveSegInfo->tracks->track_info[info->iActiveTrack];

	return 0;
}

int MATROSKA::TrackNumber2Index(int iNbr)
{
	SEGMENT_INFO*	seg;
	TRACKS*			tra;
	if (info->mode == MMODE_READ) {
		seg = pActiveSegInfo;
		tra = seg->tracks;
	}
	int			  i;

	if (info->mode == MMODE_READ && tra->iTable[iNbr-1] == iNbr) return iNbr-1;
	if (info->mode == MMODE_WRITE && tracks[iNbr-1]->iTrackNbr == iNbr) return iNbr-1;

	for (i=0;i<tra->iCount;i++) {
		if (info->mode == MMODE_READ) {
			if (tra->iTable[i] == iNbr) return i;
		} else if (tracks[i]->iTrackNbr == iNbr) return i; 
	}

	return TN2I_INVALID;
}

int MATROSKA::GetActiveTrack()
{
	return info->iActiveTrack;
}

int MATROSKA::EnableQueue(int iStream, int iEnable)
{
	if (iStream < 0) return -1;
	if (iStream >= GetTrackCount()) return -1;
	pActiveSegInfo->tracks->track_info[iStream]->iSelected = !!iEnable;

	return 0;
}

int MATROSKA::IsEndOfSegment()
{
	return (!!pActiveSegInfo->iEndReached);
}

int MATROSKA::ReadBlock()
{
	READBLOCK_INFO		rbi;
	EBMLM_Segment*		seg = (EBMLM_Segment*)e_Segments->pElement[GetActiveSegment()];
	int					iStreamIndex, i;
	char*				laceelement;
	CBuffer*			c;

	ZeroMemory(&rbi,sizeof(rbi));

	if (pActiveSegInfo->iEndReached == 1) {
		return READBL_ENDOFSEGMENT;
	}

	if (seg->ReadBlock(&rbi)==READBL_ENDOFSEGMENT) {
		pActiveSegInfo->iEndReached = 1;
		return READBL_ENDOFSEGMENT;
	}

/*	if (rbi.qwTimecode<0) {
		Sleep(1);
	}
*/
	if (iDebugLevel > 1) {
		sprintf(debugmsg, "ReadBlock: stream %d, timecode %I64d, duration: %d",
			rbi.iStream, rbi.qwTimecode, (int)rbi.qwDuration);
		fprintf(stderr, "%s%c",debugmsg, '\n');
	}

	if ((iStreamIndex = TrackNumber2Index(rbi.iStream)) == TN2I_INVALID) {
		B0rked("Block belongs to stream which is not described in 'Tracks'");
		return READBL_FILEB0RKED;
	}

	pActiveSegInfo->iBlockCount[iStreamIndex]++;
	pActiveSegInfo->iTotalBlockCount++;
	for (i=GetTrackCount()-1;i>=0;i--) {
		if (!IsSparse(iStreamIndex)) {
			if (i!=iStreamIndex) {
				pActiveSegInfo->iOtherBlocksThan[i]++;
			} else {
				pActiveSegInfo->iOtherBlocksThan[i]=0;
			}
		}
	}

	if (rbi.qwTimecode < GetMinTimecode()) {
		return READBL_OK;
	}

	if (iSingleStream>-1) {
		if (iSingleStream !=iStreamIndex) {
			DecBufferRefCount(&rbi.cData);
			if (rbi.iFrameCountInLace) DecBufferRefCount(&rbi.cFrameSizes);
			return READBL_OK;
		}
	}
	
	if (!IsStreamQueued(iStreamIndex)) {
		DecBufferRefCount(&rbi.cData);
		if (rbi.iFrameCountInLace) {
			DecBufferRefCount(&rbi.cFrameSizes);
		}
		return READBL_OK;
	}

	rbi.iEnqueueCount = iEnqueueCount++;
	if (rbi.iFlags & BLKHDRF_LACING) {
		laceelement = rbi.cData->AsString();
		c = new CBuffer;
		c->SetSize(sizeof(READBLOCK_INFO));
		c->SetData(&rbi);
		c->IncRefCount();
		QUEUE_enter(&queue[iStreamIndex],c);

	} else {
		c = new CBuffer;
		rbi.qwTimecode = (__int64)((double)(rbi.qwTimecode) * pActiveSegInfo->tracks->track_info[iStreamIndex]->fTimecodeScale);
		
		c->SetSize(sizeof(READBLOCK_INFO));
		c->SetData(&rbi);
		c->IncRefCount();
		QUEUE_enter(&queue[iStreamIndex],c);
	}

	__int64 idq = pActiveSegInfo->tracks->track_info[iStreamIndex]->iDataQueued += rbi.cData->GetSize();
	__int64 iTimecode = rbi.qwTimecode;
	return READBL_OK;

/*	rbi2 = (READBLOCK_INFO*)queue[iStreamIndex]->buffer->GetData();
	if (!IsSparse(iStreamIndex)) {
		if ((int)(GetTimecodeScale()*fabs((double)(rbi2->qwTimecode-rbi.qwTimecode))/1000000000) > 30) {
			MessageBox(0,"Losing queue!","Error",MB_OK | MB_ICONERROR);
			c = QUEUE_read(&queue[iStreamIndex]);
			rbi2 = (READBLOCK_INFO*)c->GetData();
			DecBufferRefCount(&rbi2->cData);
			DecBufferRefCount(&c);
		}
	}
*/
}

int MATROSKA::MapT(int iIndex)
{
	return (iIndex==-1)?GetActiveTrack():iIndex;
}

int MATROSKA::MapS(int iIndex)
{
	return (iIndex==-1)?GetActiveSegment():iIndex;
}

void MATROSKA::SelectSingleStream(int iStream)
{
	iSingleStream = iStream;
}

CHAPTERS* MATROSKA::GetChapterInfo()
{
	return pActiveSegInfo->chapters;
}

CUES* MATROSKA::GetCues()
{
	return ((EBMLM_Segment*)(e_Segments->pElement[GetActiveSegment()]))->GetCues();
}

__int64 MATROSKA::GetQueuedDataSize(int iTrack)
{
	__int64 res = 0;

	if (iTrack >= 0) {
		return pActiveSegInfo->tracks->track_info[iTrack]->iDataQueued;
	} else
		for (int j=0;j<GetTrackCount();j++) {
			res += pActiveSegInfo->tracks->track_info[j]->iDataQueued;
		}
	return res;
}

// fill queue for stream at index i
int MATROSKA::FillQueue(int i)
{
	if (!IsStreamQueued(i)) return 0;
	int j = 0, count = 0;
	__int64 total_blocks = pActiveSegInfo->iTotalBlockCount;
	float	ratio = ((float)total_blocks / (float)pActiveSegInfo->iBlockCount[i]);
	if (!total_blocks) ratio = (float)(2*GetTrackCount());
	if (ratio<2) ratio = 2;

	while ( /*( ((count++ <= 10) || (count++ < 3*ratio)) || (total_blocks<1000)) &&*/
			pActiveSegInfo->iOtherBlocksThan[i] < GetMaxBlockGapFactor()*ratio &&
		    QUEUE_empty(queue[i])	&& 
			!IsSparse(i)			&& 
			((j=(ReadBlock()))!=READBL_ENDOFSEGMENT));

	if (j==READBL_ENDOFSEGMENT && count<=1) return j;
	if (IsSparse(i) && QUEUE_empty(queue[i])) return READBL_SPARSEQUEUEEMPTY;
	if (!IsSparse() && QUEUE_empty(queue[i]) && j!=READBL_ENDOFSEGMENT /*&& count > 3*ratio && count > 10*/) {
		SetSparseFlag(i, 1);
		return READBL_STREAMNOWSPARSE;
	}

	return 0;
}

int MATROSKA::Read(READ_INFO* pInfo, int iFlags)
{
	int iTrack = GetActiveTrack();
	int iRes = 0;
	bool bEmpty;
	bool bNext = 0;
	bool bFileorder = 0;

	// if stream is sparse, only look if there is something in the corresponging queue
	if ((iFlags & MRF_FILEORDER) != MRF_FILEORDER && ((iFlags & MRF_SPARSE) == MRF_SPARSE || IsSparse())) {
		if (QUEUE_empty(queue[iTrack])) {
			return READBL_SPARSEQUEUEEMPTY;
		}
	} else if ((bNext = ((iFlags & MRF_NEXT) == MRF_NEXT)) ||
		       (bFileorder = ((iFlags & MRF_FILEORDER) == MRF_FILEORDER))) {
		// find block with earliest timestamp at beginning of queues
		CBuffer*		b;
		READBLOCK_INFO* rbi = NULL;

		__int64	iMininumTime = (__int64)(1<<30)*(1<<30);
		__int64 iMinimumEnqueueCount = 0x7FFFFFFFFFFFFFFF;
		int		iIndex = 0, i;
		bEmpty = true;

		for (i=0;i<GetTrackCount();i++) {
			int fillqueue = FillQueue(i);
			if (!QUEUE_empty(queue[i])) {
				bEmpty = false;
				b = QUEUE_read(&queue[i],QR_DONTREMOVE);
				rbi = (READBLOCK_INFO*)b->GetData();

				if (bNext) {
					if (rbi->qwTimecode <= iMininumTime) {
						iMininumTime = rbi->qwTimecode;
						iIndex = i;
					}
				}
				if (bFileorder) {
					if (rbi->iEnqueueCount <= iMinimumEnqueueCount) {
						iMinimumEnqueueCount = rbi->iEnqueueCount;
						iIndex = i;
					}
				}
			}

		}
		
		// if all queues are still empty -> end of segment
		if (bEmpty) return READBL_ENDOFSEGMENT;

		iTrack = iIndex;

	} else {
		// if a block shall be read, look for one
		iRes = FillQueue(iTrack);
	}
	
	if (iRes==READBL_ENDOFSEGMENT) return READBL_ENDOFSEGMENT;
	CBuffer* b = QUEUE_read(&queue[iTrack]);
	
	READBLOCK_INFO*	rbi = (READBLOCK_INFO*)b->GetData();

	pInfo->iLength = rbi->cData->GetSize();

	if (rbi->iFrameCountInLace && (!IsLaced(iTrack))) {
		B0rked("Lacing used, but lacing flag is not set");
	}

	if (rbi->iFrameCountInLace) {
		pInfo->iFrameCount = rbi->iFrameCountInLace;
		newz(int, rbi->iFrameCountInLace, pInfo->iFrameSizes);
		memcpy(pInfo->iFrameSizes,rbi->cFrameSizes->GetData(),sizeof(int)*rbi->iFrameCountInLace);
		
		DecBufferRefCount(&rbi->cFrameSizes);
	} else {
		pInfo->iFrameSizes = 0;
	}
	pInfo->qwTimecode = rbi->qwTimecode;
	pInfo->pData = rbi->cData;
	pInfo->iReferences = rbi->iReferences;
	pInfo->iFlags = 0;
	pInfo->iTrack = iTrack;

	if (rbi->iFlags & RBIDUR_INDICATED) {
		pInfo->iFlags |= RIF_DURATION;
		pInfo->qwDuration = rbi->qwDuration;
	}
	if (rbi->iFlags & BLKHDRF_LACINGMASK) {
		pInfo->iFlags = RIF_LACING;
	}

	memcpy(pInfo->iReferencedFrames,rbi->iReferencedFrames,2*sizeof(int));
	__int64 idq = pActiveSegInfo->tracks->track_info[iTrack]->iDataQueued -= rbi->cData->GetSize();

	DecBufferRefCount(&b);

	return READBL_OK;
}

void MATROSKA::SetTracksCopiesCount(int iCount)
{
	ws.iTracksCopies = iCount;
}

void MATROSKA::EnableShiftFirstClusterTimecode2Zero(int bEnable)
{
	info->bShiftFirstClusterTimecode2Zero = bEnable;
}

int MATROSKA::GetTracksCopiesCount()
{
	return (int)ws.iTracksCopies;
}

bool MATROSKA::IsKeyframe()
{
	int iTrack = GetActiveTrack();
	int iRes;

	while (QUEUE_empty(queue[iTrack]) && ((iRes=ReadBlock()) != READBL_ENDOFSEGMENT));
	
	if (iRes==READBL_ENDOFSEGMENT) return false;
	CBuffer* b = QUEUE_read(&queue[iTrack],QR_DONTREMOVE);
	READBLOCK_INFO*	rbi = (READBLOCK_INFO*)b->GetData();

	return !rbi->iReferences;
}

__int64 MATROSKA::GetNextTimecode(int iTrack)
{
	iTrack = MapT(iTrack);
	int iRes = 0;
	bool bS;

	bS = !!IsSparse(iTrack);

	if (bS && QUEUE_empty(queue[iTrack])) {
		return TIMECODE_UNKNOWN;
	}

	iRes = FillQueue(iTrack);
	
	if (iRes==READBL_ENDOFSEGMENT) return TIMECODE_UNKNOWN;
	if (QUEUE_empty(queue[iTrack])) return TIMECODE_UNKNOWN;

	CBuffer* b = QUEUE_read(&queue[iTrack],QR_DONTREMOVE);
	READBLOCK_INFO*	rbi = (READBLOCK_INFO*)b->GetData();

	return rbi->qwTimecode;
}

bool MATROSKA::IsEndOfStream(int iTrack)
{
	iTrack = MapT(iTrack);
	int iRes = 0;

	if (!IsSparse(iTrack)) {
		// normal streams: try to fill queue
		while (QUEUE_empty(queue[iTrack]) && ((iRes=ReadBlock()) != READBL_ENDOFSEGMENT));
		if (iRes==READBL_ENDOFSEGMENT) return true;
		return false;
	} else {
		// sparse streams: check if recent read operation reached end of file
		if (QUEUE_empty(queue[iTrack]) && pActiveSegInfo->iEndReached) return true;

		return false;
	}
}

__int64 MATROSKA::GetSize()
{
	return GetSource()->GetSize();
}

int MATROSKA::Open(STREAM* s, int iMode)
{
	int i;
	stream = s;
	EBMLElement*		e_next=NULL;
	EBMLElement*		e_SIInfo = NULL;
	EBMLM_Segment*		segment;
	CBuffer*			cb=NULL;

	newz(MATROSKA_INFO, 1, info);
	
	if (iMode & MMODE_READ) {
		info->mode = iMode;
		e_Main = new EBML_Matroska(s,NULL);
		e_Main->Create1stSub(&e_next);
		if (!e_next || e_next->GetType()!=EBMLTYPE_EBML) {
			delete info;
			DeleteEBML(&e_next);
			DeleteEBML((EBMLElement**)&e_Main);
			return MOPEN_ERR;
		}
		DeleteEBML(&e_next);

		char* cid_Level0[] = { (char*)EID_EBML, (char*)MID_SEGMENT };
		void** pe_Level0 = NULL;
		e_Main->SearchMulti(&pe_Level0, cid_Level0, 2);
		e_EBML		= (EBMLELEMENTLIST*)pe_Level0[0];
		e_Segments	= (EBMLELEMENTLIST*)pe_Level0[1];
		delete pe_Level0;

		if (iMode & MMODE_DUMMY) {
			return MOPEN_OK;
		}
		if (!e_Segments->iCount) {
			FatalError("No Segment found!");
			return MOPEN_ERR;
		}

		newz(SEGMENT_INFO*,e_Segments->iCount, SegmentInfo);
	
		for (i=0;i<e_Segments->iCount;i++) {
			segment = ((EBMLM_Segment*)e_Segments->pElement[i]);
			segment->AllowObfuscatedSegments(bAllowObfuscatedFiles);
			if (!(SegmentInfo[i]=segment->GetSegmentInfo())) {
				printf("Error trying to read SegmentInfo!\n");
				return MOPEN_ERR;
			}
			switch (info->bShiftFirstClusterTimecode2Zero)	{
				case SFCTZ_AUTO:
					if (SegmentInfo[i]->PrevSeg.cFilename || SegmentInfo[i]->PrevSeg.cUID) {
						segment->EnableShiftFirstClusterTimecode2Zero();
					} else {
						segment->EnableShiftFirstClusterTimecode2Zero(false);
					}
					break;
				default:
					segment->EnableShiftFirstClusterTimecode2Zero(!!info->bShiftFirstClusterTimecode2Zero);
			}

		}

		// gather info
		for (i=0;i<e_Segments->iCount;i++) {
			info->qwLength+=(__int64)SegmentInfo[i]->fDuration;
		}

		EBMLElement* e = e_Segments->pElement[e_Segments->iCount-1]->GetSucc();
		if (e) {
			B0rked("end of last segment is not end of file");
		}

		int iSizeUnknown = 0;
		for (i=0;i<8;i++) {
			if (e_Segments->pElement[0]->GetLength() == undefined_lengths[i+8]) {
				iSizeUnknown = 1;
			}
		}
		if (!iSizeUnknown && (e_Segments->pElement[0]->GetLength() + e_Segments->pElement[0]->GetStreamPos() > GetSource()->GetSize())) {
			B0rked("end of first segment points beyond the end of the file");
		}

		SetActiveSegment(0);
		SetActiveTrack(0);
		SetMinTimecode(-40000);
		if (e) DeleteEBML(&e);
		iEnqueueCount = 0;

		return MOPEN_OK;
	}

	if (iMode & MMODE_WRITE) {
		srand((int)rdtsc());
		info->mode = iMode;
		ZeroMemory(&ws,sizeof(ws));

		e_Cluster = NULL;
		e_Header = new EBMLHeader_Writer(GetDest());
		
		e_Segment = new EBMLElement_Writer(GetDest());
		e_Segment->SetFixedSizeLen(8);
		e_Segment->SetID((char*)MID_SEGMENT);
		
		e_Tracks = new EBMLMTrackInfo_Writer(GetDest(), this);
		e_Tracks->SetFixedSize(131072);

		e_PrimarySeekhead = new EBMLMSeekhead_Writer(GetDest());
		e_PrimarySeekhead->SetFixedSize(iSizeOfSeekhead);
		
		e_SegInfo = new EBMLMSegmentInfo_Writer(GetDest());
	
		ws.iTimecodeScale = 1000000;
		ws.iMinimumCuePointInterval = 1995000000;
		
		ws.iMaxClusterSize = 524288;
		ws.iMaxClusterTime = 30000;
		ws.iLaceStyle = LACESTYLE_XIPH;
		ws.iClusterCount = 0;
		ws.iClusterOverhead = 0;
		ws.iCuePoints = 0;
		ws.iSeekheadSize = 0;
		ws.bDisplayWidth_Height = 1;
		ws.bCueBlockNumber = 1;
		ws.bWriteCues=1;
		ws.bWriteFlagDefault = true;
		ws.bWriteFlagEnabled = false;
		ws.bWriteFlagLacing = true;
		ws.iTracksCopies = 1;
		ws.bClusterIndex = 1;
		ws.iLatestTimecode = -1;
		ws.bRandomizeElementOrder = 0;
		
		// set default behaviour for Duration elements
		ws.duration_default.iVideo = MATROSKA_TDDM_GAP;
		ws.duration_default.iAudio = MATROSKA_TDDM_GAP;
		ws.duration_default.iSubtitles = MATROSKA_TDDM_ALWAYS;
	
		return MOPEN_OK;
	}

	return MOPEN_ERR;
}

void MATROSKA::SetTimecodeScale(__int64 iScale)
{
	ws.iTimecodeScale = iScale;
}

void MATROSKA::EnableClusterIndex(int bEnable)
{
	ws.bClusterIndex = bEnable;
}

int MATROSKA::IsClusterIndexEnabled()
{
	return (int)ws.bClusterIndex;
}

int MATROSKA::IsCueBlockNumberEnabled()
{
	return (int)ws.bCueBlockNumber;
}

void MATROSKA::EnableWriteFlagDefault(int bEnabled)
{
	ws.bWriteFlagDefault = bEnabled;
}

void MATROSKA::EnableWriteFlagEnabled(int bEnabled)
{
	ws.bWriteFlagEnabled = bEnabled;
}

void MATROSKA::EnableWriteFlagLacing(int bEnabled)
{
	ws.bWriteFlagLacing = bEnabled;
}

void MATROSKA::EnableRandomizeElementOrder(int bEnabled)
{
	MATROSKA_RandomizeElementOrder(!!bEnabled);
}

int MATROSKA::IsEnabled_WriteFlagDefault()
{
	return (int)ws.bWriteFlagDefault;
}

int MATROSKA::IsEnabled_WriteFlagEnabled()
{
	return (int)ws.bWriteFlagEnabled;
}

int MATROSKA::IsEnabled_WriteFlagLacing()
{
	return (int)ws.bWriteFlagLacing;
}


void MATROSKA::EnableCues(int iStreamType, int bEnable)
{
	ws.bWriteCues&=~iStreamType;
	ws.bWriteCues|=(!!bEnable)*iStreamType;
}

bool MATROSKA::IsCuesEnabled(int iStreamType)
{
	return !!(ws.bWriteCues&iStreamType);
}

void MATROSKA::EnableCueBlockNumber(int bEnabled)
{
	ws.bCueBlockNumber = bEnabled;
}

int MATROSKA::BeginWrite()
{
	e_Header->Write();
	DeleteEBML(&e_Header);

	ws.iSegPosInFile = GetDest()->GetPos();

	ws.iPosInSegment = 0;
	ws.iSeekheadPosInFile = ws.iSegPosInFile + e_Segment->Write();
	ws.iSegClientDataBegin = ws.iSeekheadPosInFile;

	ws.i1stElementInFile = ws.iSeekheadPosInFile;

	// write dummy seek head
	ws.iPosInSegment += e_PrimarySeekhead->Write();
	e_PrimarySeekhead->AddEntry((char*)MID_TRACKS,ws.iPosInSegment);
	e_Seekhead = e_PrimarySeekhead;

	// write track info
	int iSize = GetTrackCount()*256; 
	for (int i=0;i<GetTrackCount();i++) {
		if (tracks[i]->cCodecPrivate) iSize+=tracks[i]->cCodecPrivate->GetSize();
		if (tracks[i]->cName) iSize+=tracks[i]->cName->GetSize();
		if (tracks[i]->cCodecID) iSize+=tracks[i]->cCodecID->GetSize();
		if (tracks[i]->cLngCode) iSize+=tracks[i]->cLngCode->GetSize();
		tracks[i]->iBufferedBlocks = 0;
		ZeroMemory(tracks[i]->iLaceSchemes,sizeof(tracks[i]->iLaceSchemes));
	}
	ws.iTracksPosInFile = GetDest()->GetPos();
	e_Tracks->SetFixedSize(iSize);
	ws.iPosInSegment += e_Tracks->Write();

	ws.iSegInfoPosInFile = ws.iPosInSegment + ws.i1stElementInFile;
	e_PrimarySeekhead->AddEntry((char*)MID_SEGMENTINFO,ws.iPosInSegment);

	char cVersion[200]; cVersion[0]=0;
	sprintf(cVersion, "matroska muxer by Alexander Noe, build date %s", matroska_GetCompileDate());
	Str2UTF8(cVersion, cVersion);
	e_SegInfo->SetMuxingApp(new CStringBuffer(cVersion));
	e_SegInfo->SetWritingApp(new CStringBuffer("<not indicated>",0));

	e_SegInfo->SetFixedSize(2048);

	e_Cues = new EBMLMCue_Writer(GetDest());

	ws.iPosInSegment += e_SegInfo->Write();
	ws.iPosOfReservedSpace = GetDest()->GetPos();
	ws.iSizeOfReservedSpace = 16384;
	e_ReservedSpace = new EBMLElement_Writer(GetDest());
	e_ReservedSpace->SetID((char*)MID_VOID);
	e_ReservedSpace->SetFixedSize(ws.iSizeOfReservedSpace);
	ws.iPosInSegment += e_ReservedSpace->Write();

	ws.iAccumulatedOverhead = GetDest()->GetPos();

	return 0;
}

void MATROSKA::SetSegmentTitle(char* cTitle)
{
	e_SegInfo->SetTitle(new CStringBuffer(cTitle));
}

void MATROSKA::SetSegmentDuration(float fDuration)
{
	e_SegInfo->SetDuration(ws.fSetDuration=fDuration);	
}

STREAM* MATROSKA::GetDest()
{
	return stream;
}

int MATROSKA::Close()
{
	int i;
	int j = (e_Segments)?GetSegmentCount():-1;
	int	iTrash;

	for (i=0;i<j;i++) {
		if (SegmentInfo && SegmentInfo[i]->queue) {
			queue = (QUEUE**)SegmentInfo[i]->queue;
			FlushQueues();
		}
	}
	
	if (e_Segments) DeleteElementList(&e_Segments);
	if (e_EBML) DeleteElementList(&e_EBML);
	if (e_Main) DeleteEBML(&e_Main);
	if (iStreams2Queue) delete iStreams2Queue;
	if (SegmentInfo) delete SegmentInfo;
//	if (chapters);

	if (info->mode == MMODE_WRITE) {
		FlushWriteBuffer();
		SetSegmentDuration((float)ws.iLatestTimecode);
		if (e_Cluster) {
			if (IsClusterIndexEnabled()) {
				e_Seekhead->AddEntry((char*)MID_CLUSTER,ws.iPosInSegment,1);
			}
			ws.iPosInSegment += e_Cluster->Write();
			ws.iAccumulatedOverhead += e_Cluster->GetWriteOverhead();
			DeleteEBML(&e_Cluster);
		}

		if (e_Cues && e_Cues->GetSize()>8) {
			e_PrimarySeekhead->AddEntry((char*)MID_CUES,ws.iPosInSegment,1);
			e_Cues->EnableCRC32();
			ws.iPosInSegment += (i=(int)e_Cues->Write());
			ws.iAccumulatedOverhead += i;
			DeleteEBML(&e_Cues);
		}

		// write chapters
		bool bEndNotIndcated = false;
		if (chapters && chapters->GetChapterCount()) {
			if (chapters->GetChapterEnd(CHAP_LAST) == -1) {
				chapters->SetChapterEnd(CHAP_LAST,(__int64)(ws.fSetDuration*GetTimecodeScale()));
				bEndNotIndcated = true;
			}

			EBMLMChapter_Writer* e_Chapters = new EBMLMChapter_Writer(GetDest(),chapters,
				(max_chapter_duration==-1)?(__int64)(ws.fSetDuration * GetTimecodeScale()):max_chapter_duration);

			if (e_Chapters->GetSize() > ws.iSizeOfReservedSpace) {
				e_PrimarySeekhead->AddEntry((char*)MID_CHAPTERS,ws.iPosInSegment,1);
				ws.iPosInSegment += (i=(int)e_Chapters->Write());
				ws.iAccumulatedOverhead += i;
				DeleteEBML(&e_Chapters);
			} else {
				__int64 q = GetDest()->GetPos();
				GetDest()->Seek(ws.iPosOfReservedSpace);
				e_PrimarySeekhead->AddEntry((char*)MID_CHAPTERS,ws.iPosOfReservedSpace - ws.iSegClientDataBegin,1);
				i=(int)e_Chapters->Write();
				ws.iPosOfReservedSpace += i;
				ws.iSizeOfReservedSpace -= i;
				e_ReservedSpace->SetFixedSize(ws.iSizeOfReservedSpace);
				e_ReservedSpace->Write();
				GetDest()->Seek(q);
			}
			if (bEndNotIndcated) chapters->SetChapterEnd(CHAP_LAST,-1);
		}

		// build track info (-> generate Track UIDs!)
		for (i=0;i<GetTrackCount();i++) e_Tracks->SetTrackProperties(i,tracks[i]);
		e_Tracks->Build();
		e_Tracks->EnableCRC32();

		if (GetTracksCopiesCount() == 2) {
			e_PrimarySeekhead->AddEntry((char*)MID_TRACKS,ws.iPosInSegment,1);
			ws.iPosInSegment += (i=(int)e_Tracks->Write());
			ws.iAccumulatedOverhead += i;
		}

		// compose tags
		EBMLElement_Writer*	e_Tags = new EBMLElement_Writer(GetDest(),(char*)MID_TAGS);

		for (int k=0;k<iTrackCount;k++) {
			__int64 iBitsPS = (__int64)((double)tracks[k]->iTotalSize * 8000000000 / ws.fSetDuration / GetTimecodeScale());
			double dFPS = (double)((double)tracks[k]->iTotalFrameCount * 1000000000 / ws.fSetDuration / GetTimecodeScale());
			EBMLElement_Writer* e_Tag = e_Tags->AppendChild(new EBMLElement_Writer(GetDest(),(char*)MID_TG_TAG));
			EBMLElement_Writer* e_Target = e_Tag->AppendChild(new EBMLElement_Writer(GetDest(),(char*)MID_TG_TARGET));
			e_Target->AppendChild_UInt((char*)MID_TG_TRACKUID,tracks[k]->iTrackUID,-1);
			EBMLElement_Writer* e_SimpleTag = e_Tag->AppendChild(new EBMLElement_Writer(GetDest(),(char*)MID_TG_SIMPLETAG));
			char cSize[20]; cSize[0]=0; itoa((int)iBitsPS, cSize, 10);
			e_SimpleTag->AppendChild_String((char*)MID_TG_TAGNAME,"BITSPS");
			e_SimpleTag->AppendChild_String((char*)MID_TG_TAGSTRING,cSize);

			e_SimpleTag = e_Tag->AppendChild(new EBMLElement_Writer(GetDest(),(char*)MID_TG_SIMPLETAG));		
			e_SimpleTag->AppendChild_String((char*)MID_TG_TAGNAME,"BPS");
			e_SimpleTag->AppendChild_String((char*)MID_TG_TAGSTRING,cSize);

			e_SimpleTag = e_Tag->AppendChild(new EBMLElement_Writer(GetDest(),(char*)MID_TG_SIMPLETAG));		
			e_SimpleTag->AppendChild_String((char*)MID_TG_TAGNAME,"FPS");
			sprintf(cSize, "%7.4f", dFPS);
			e_SimpleTag->AppendChild_String((char*)MID_TG_TAGSTRING,cSize);
		}

		e_Tags->EnableCRC32();

		if (ws.iSizeOfReservedSpace < e_Tags->GetSize()) {
			e_PrimarySeekhead->AddEntry((char*)MID_TAGS,ws.iPosInSegment,1);
			ws.iAccumulatedOverhead += (i=(int)e_Tags->Write());
			ws.iPosInSegment += i;
		} else {
			__int64 q = GetDest()->GetPos();
			GetDest()->Seek(ws.iPosOfReservedSpace);
			e_PrimarySeekhead->AddEntry((char*)MID_TAGS,ws.iPosOfReservedSpace - ws.iSegClientDataBegin,1);
			i=(int)e_Tags->Write();
			ws.iPosOfReservedSpace += i;
			ws.iSizeOfReservedSpace -= i;
			e_ReservedSpace->SetFixedSize(ws.iSizeOfReservedSpace);
			e_ReservedSpace->Write();
			GetDest()->Seek(q);
		}

		// write secondary seekhead, if there is one
		if (e_SecondarySeekhead) {
			e_PrimarySeekhead->AddEntry((char*)MID_MS_SEEKHEAD,ws.iPosInSegment,1);
			e_SecondarySeekhead->EnableCRC32();
			ws.iPosInSegment += (i=(int)e_SecondarySeekhead->Write());
			ws.iAccumulatedOverhead += i;
			ws.iSeekheadSize += i;
			DeleteEBML(&e_SecondarySeekhead);
		} 

		// set segment info
		e_SegInfo->SetDuration(ws.fSetDuration);
		e_SegInfo->SetTimecodeScale(GetTimecodeScale());
		e_SegInfo->Build();
		e_SegInfo->EnableCRC32();

		e_SegInfo->SetFixedSize(2048);
		GetDest()->Seek(ws.iSegInfoPosInFile);
		iTrash = (int)e_SegInfo->Write();

		GetDest()->Seek(ws.iTracksPosInFile);
		e_Tracks->Write();

		// write primary seekhead
		GetDest()->Seek(ws.iSeekheadPosInFile);
		e_PrimarySeekhead->EnableCRC32();
		ws.iSeekheadSize += e_PrimarySeekhead->Write();
		DeleteEBML(&e_PrimarySeekhead);
		
		GetDest()->Seek(ws.iSegPosInFile+4);
		char t[8];
		Int2VSUInt(&ws.iPosInSegment,t,8);
		GetDest()->Write(t,8);

		DeleteEBML(&e_Segment);
		DeleteEBML(&e_SegInfo);
		DeleteEBML(&e_Tracks);
		delete write_buffer.block;
		delete write_buffer.used;
		delete write_buffer.block_of_stream;
		
	}

	if (info) delete info;

	return 0;
}


__int64 MATROSKA::GetDuration()
{
	return info->qwLength;
}

__int64 MATROSKA::GetSegmentDuration()
{
	if (info->mode == MMODE_READ) {
		return (__int64)pActiveSegInfo->fDuration;
	} else {
		return (__int64)ws.fSetDuration;
	}
}

__int64 MATROSKA::GetSegmentSize()
{
	return e_Segments->pElement[GetActiveSegment()]->GetLength();
}

void* MATROSKA::GetCodecPrivate(int iTrack)
{
	CBuffer* cB = pActiveSegInfo->tracks->track_info[MapT(iTrack)]->cCodecPrivate;

	if (cB) return cB->GetData();
	return NULL;
}

int MATROSKA::GetCodecPrivateSize(int iTrack)
{
	CBuffer* cB = pActiveSegInfo->tracks->track_info[MapT(iTrack)]->cCodecPrivate;
	if (cB) return cB->GetSize();
	return 0;
}


int MATROSKA::GetTrackType(int iTrack)
{
	return pActiveSegInfo->tracks->track_info[MapT(iTrack)]->iType;
}

int MATROSKA::GetColorSpace(int iTrack)
{
	return pActiveSegInfo->tracks->track_info[MapT(iTrack)]->video.iColorSpace;
}

char* MATROSKA::GetTrackName(int iIndex)
{
	CBuffer*	C = pActiveSegInfo->tracks->track_info[MapT(iIndex)]->cName;
	if (C) {
		return C->AsString();
	} else return "";
}

char* MATROSKA::GetSegmentUID()
{
	CBuffer*	C = pActiveSegInfo->CurrSeg.cUID;
	if (C) {
		return C->AsString();
	} else return "";
}

int MATROSKA::IsLaced(int iTrack)
{
	return pActiveSegInfo->tracks->track_info[MapT(iTrack)]->iLacing;
}

int MATROSKA::IsSparse(int iTrack)
{
	return !!pActiveSegInfo->tracks->track_info[MapT(iTrack)]->iSparse;
}


int MATROSKA::IsDefault(int iIndex)
{
	return pActiveSegInfo->tracks->track_info[MapT(iIndex)]->iDefault;
}

int MATROSKA::IsEnabled(int iIndex)
{
	return pActiveSegInfo->tracks->track_info[MapT(iIndex)]->iEnabled;
}

int MATROSKA::GetMinCache(int iTrack)
{
	return pActiveSegInfo->tracks->track_info[MapT(iTrack)]->iMinCache;
}

int MATROSKA::GetMaxCache(int iTrack)
{
	return pActiveSegInfo->tracks->track_info[MapT(iTrack)]->iMaxCache;
}

char* MATROSKA::GetLanguage(int iTrack)
{
	return pActiveSegInfo->tracks->track_info[MapT(iTrack)]->cLanguage->AsString();
}

int MATROSKA::GetTrackNumber(int iIndex)
{
	return pActiveSegInfo->tracks->iTable[MapT(iIndex)];
}

int MATROSKA::GetTrackUID(int iIndex)
{
	if (info->mode == MMODE_READ) {
		return pActiveSegInfo->tracks->track_info[MapT(iIndex)]->iUID;
	} else {
		return tracks[MapT(iIndex)]->iTrackUID;
	}
}


int MATROSKA::GetSegmentCount()
{
	return e_Segments->iCount;
}

int MATROSKA::GetTrackCount(int iType)
{
	if (info->mode == MMODE_READ) {
		if (iType == GTC_ALL) {
			return pActiveSegInfo->tracks->iCount;
		} else {
			if (iType < 256) {
				return pActiveSegInfo->tracks->iCountByType[iType];
			} else {
				return 0;
			}
		}
	}
	if (info->mode == MMODE_WRITE) return iTrackCount;
	return 0;
}

char* MATROSKA::GetSegmentMuxingApp(int iSegment)
{
	CBuffer*	C = SegmentInfo[MapS(iSegment)]->cMuxingApp;
	if (C) {
		return C->AsString();
	} else return "";
}

char* MATROSKA::GetSegmentWritingApp(int iSegment)
{
	CBuffer*	C = SegmentInfo[MapS(iSegment)]->cWritingApp;
	if (C) {
		return C->AsString();
	} else return "";
}

int MATROSKA::GetResolution(int *iPX, int* iPY, int* iDX, int* iDY, int* iDU)
{
	TRACK_INFO*	t = pActiveTrackInfo;

	if (iPX) *iPX = t->video.iPixelWidth;
	if (iPY) *iPY = t->video.iPixelHeight;
	if (iDX) *iDX = t->video.iDisplayWidth;
	if (iDY) *iDY = t->video.iDisplayHeight;
	if (iDU) *iDU = t->video.iDisplayUnit;
	return 0;
}

float MATROSKA::GetSamplingFrequency(int iTrack)
{
	return  pActiveSegInfo->tracks->track_info[MapT(iTrack)]->audio.fSamplingFrequency;
}

float MATROSKA::GetOutputSamplingFrequency(int iTrack)
{
	return  pActiveSegInfo->tracks->track_info[MapT(iTrack)]->audio.fOutputSamplingFrequency;
}

int MATROSKA::GetChannelCount(int iTrack)
{
	return  pActiveSegInfo->tracks->track_info[MapT(iTrack)]->audio.iChannels;
}

int MATROSKA::GetBitDepth(int iTrack)
{
	return  pActiveSegInfo->tracks->track_info[MapT(iTrack)]->audio.iBitDepth;
}

char* MATROSKA::GetCodecID(int iTrack)
{
	return  pActiveSegInfo->tracks->track_info[MapT(iTrack)]->cCodecID->AsString();
}

char* MATROSKA::GetCodecName(int iTrack)
{
	CBuffer* cName = pActiveSegInfo->tracks->track_info[MapT(iTrack)]->cCodecName;
	if (cName) {
		return cName->AsString();
	} else return "";
}

void MATROSKA::GetChannelPositions(unsigned char* pos)
{
	CBuffer* positions = pActiveTrackInfo->audio.cChannelPositions;
	memcpy(pos,positions->GetData(),positions->GetSize());
}

void MATROSKA::AllowObfuscatedFiles(bool bAllow)
{
	bAllowObfuscatedFiles = bAllow;
}

__int64 MATROSKA::GetTimecodeScale()
{
	if (info->mode == MMODE_READ) {
		return pActiveSegInfo->iTimecodeScale;
	} else {
		return ws.iTimecodeScale;
	}
}

__int64 MATROSKA::GetDefaultDuration(int iTrack)
{
	__int64 i = pActiveSegInfo->tracks->track_info[MapT(iTrack)]->qwDefaultDuration;
	if (i>0) return i;

	if (!strcmp(GetCodecID(iTrack),"A_MPEG/L3")) {
		switch ((int)(GetSamplingFrequency(iTrack)+0.5)) {
			case 48000: return 24000000; break;
			case 44100: return 26122450; break;
		}
	} else
	if (!strcmp(GetCodecID(iTrack),"A_AC3")) {
		switch ((int)(GetSamplingFrequency(iTrack)+0.5)) {
			case 48000: return 32000000; break;
		}
	}
	if (!strcmp(GetCodecID(iTrack),"A_DTS")) {
		switch ((int)(GetSamplingFrequency(iTrack)+0.5)) {
			case 48000: return 10666667; break;
		}
	}

	return 0;
}

int MATROSKA::GetAspectRatioType(int iTrack)
{
	return pActiveSegInfo->tracks->track_info[MapT(iTrack)]->video.iAspectRatio;
}

float MATROSKA::GetGamma(int iTrack)
{
	return pActiveSegInfo->tracks->track_info[MapT(iTrack)]->video.fGamma;
}

__int64 MATROSKA::GetMinTimecode()
{
	return info->iMinTimecode;
}

void MATROSKA::SetMinTimecode(__int64 iTimecode)
{
	info->iMinTimecode = iTimecode;
}

__int64 MATROSKA::GetMasterTrackDuration()
{
	EBMLM_Segment*	seg = (EBMLM_Segment*)e_Segments->pElement[GetActiveSegment()];

	return seg->GetMasterTrackDuration();
}

int MATROSKA::GetTrackCompression(int iTrack)
{
	return pActiveSegInfo->tracks->track_info[MapT(iTrack)]->iCompression;
}

			//////////////////////
			// Matroska Writing //
			//////////////////////

void MATROSKA::SetTrackCount(int iCount)
{
	tracks = (TRACK_DESCRIPTOR**)realloc(tracks,sizeof(*tracks)*iCount);
	if (iCount>iTrackCount) {
		for (int i=iTrackCount;i<iCount;i++) {
			newz(TRACK_DESCRIPTOR, 1, tracks[i]);
			tracks[i]->fTrackTimecodeScale = 1.0;
			tracks[i]->iMinCache = 1;
			tracks[i]->iMaxCache = 1;

		}
	}
	e_Tracks->SetTrackCount(iTrackCount = iCount);
	ws.iLastCuePointTimecode = new __int64[iCount+1];
	for (int j=0;j<=iCount;ws.iLastCuePointTimecode[j++]=-ws.iMinimumCuePointInterval-1);
	
	j = write_buffer.iCount = iBufsPerStream * iCount;
	write_buffer.block = new ADDBLOCK[j];
	ZeroMemory(write_buffer.block, sizeof(ADDBLOCK)*j);
	write_buffer.used = new int[j];
	ZeroMemory(write_buffer.used, sizeof(int)*j);
	
	
	write_buffer.block_of_stream = new int[iCount];
	ZeroMemory(write_buffer.block_of_stream, sizeof(int)*iCount);
}

void MATROSKA::SetCodecID(int iTrack, char* cID)
{
	TRACK_DESCRIPTOR* t = tracks[iTrack];
	CStringBuffer* cNew = new CStringBuffer(cID);

	if (t->cCodecID) DecBufferRefCount(&t->cCodecID);
	t->cCodecID = cNew;

	if (!strcmp(cID, "S_VOBSUB")) {
		tracks[iTrack]->iMinCache = 0;
		tracks[iTrack]->iMaxCache = 0;
	}

}

void MATROSKA::SetCodecPrivate(int iTrack, void* pData, int iSize)
{
	TRACK_DESCRIPTOR* t = tracks[iTrack];
	CBuffer*			cNew = NULL;
	if (iSize) {
		cNew = new CBuffer (iSize);
		cNew->SetData(pData);
		cNew->IncRefCount();
	}
	if (t->cCodecPrivate) DecBufferRefCount(&t->cCodecPrivate);
	if (iSize) {
		t->cCodecPrivate = cNew;
	}
}

void MATROSKA::SetTrackName(int iTrack,char* cName)
{
	TRACK_DESCRIPTOR* t = tracks[iTrack];

	if (t->cName) DecBufferRefCount(&t->cName);
	if (cName) {
		t->cName = new CStringBuffer (cName);
	}
}

void MATROSKA::SetTrackLanguageCode(int iTrack,char* cLngCode)
{
	TRACK_DESCRIPTOR* t = tracks[iTrack];

	if (t->cLngCode) DecBufferRefCount(&t->cLngCode);
	if (cLngCode) {
		t->cLngCode = new CStringBuffer (cLngCode);
	}
}

void MATROSKA::SetLaceStyle(int iStyle)
{
	ws.iLaceStyle = iStyle;
}

void MATROSKA::SetResolution(int iTrack,int X, int Y, int X2, int Y2)
{
	TRACK_DESCRIPTOR* t = tracks[iTrack];

	t->video.iX1 = X; 
	t->video.iX2 = (X2==-1)?X:X2;
	t->video.iDU = 0;
	t->video.iY1 = Y;
	t->video.iY2 = (Y2==-1)?Y:Y2;
}

void MATROSKA::SetFlags(int iTrack,int iEnabled, int iLacing, int iDefault)
{
	TRACK_DESCRIPTOR* t = tracks[iTrack];

	t->iDefault = iDefault; t->iLacing = iLacing; t->iEnabled = iEnabled;
}

void MATROSKA::SetTrackType(int iTrack, int iType)
{
	tracks[iTrack]->iTrackType = iType;
	switch (iType) {
		case MSTRT_VIDEO: tracks[iTrack]->iDurationMode = ws.duration_default.iVideo; break;
		case MSTRT_AUDIO: tracks[iTrack]->iDurationMode = ws.duration_default.iAudio; break;
		case MSTRT_SUBT : tracks[iTrack]->iDurationMode = ws.duration_default.iSubtitles; break;
	}
}

void MATROSKA::SetDefaultDuration(int iTrack, __int64 iDuration)
{
	tracks[iTrack]->iDefaultDuration = iDuration;
}

void MATROSKA::SetCacheData(int iTrack, int iMin, int iMax)
{
	TRACK_DESCRIPTOR* t = tracks[iTrack];

	t->iMinCache = iMin; t->iMaxCache = iMax;
}

void MATROSKA::SetSamplingFrequency(int iTrack, float fFreq, float fOutFreq)
{
	TRACK_DESCRIPTOR* t = tracks[iTrack];

	t->audio.fOutputSamplingFrequency = fOutFreq;
	t->audio.fSamplingFrequency = fFreq;
}

void MATROSKA::SetChannelCount(int iTrack, int iCount)
{
	tracks[iTrack]->audio.iChannels = iCount;
}

void MATROSKA::SetBitDepth(int iTrack, int iDepth)
{
	tracks[iTrack]->audio.iBitDepth = iDepth;
}

void MATROSKA::SetTrackNumber(int iTrack, int iNbr)
{
	tracks[iTrack]->iTrackNbr = iNbr;
}

void MATROSKA::SetTrackInfo(int iTrack,MATROSKA *m,int iSourceTrack)
{
	TRACK_DESCRIPTOR* t = tracks[iTrack];

	if (m) {
		m->SetActiveTrack(iSourceTrack);	
		t->iDefault = m->IsDefault();
		t->iEnabled = m->IsEnabled();
		t->iLacing = m->IsLaced();
		t->iMinCache = m->GetMinCache();
		t->iMaxCache = m->GetMaxCache();
		t->iTrackNbr = m->GetTrackNumber();
		t->iTrackUID = m->GetTrackUID();
		t->iTrackType = m->GetTrackType();
		t->fTrackTimecodeScale = m->GetTrackTimecodeScale();
		t->iDefaultDuration = m->GetDefaultDuration();
		t->audio.fSamplingFrequency = m->GetSamplingFrequency();
		t->audio.fOutputSamplingFrequency = m->GetOutputSamplingFrequency();
		t->audio.fOutputSamplingFrequency = t->audio.fSamplingFrequency;
		t->audio.iBitDepth = m->GetBitDepth();
		t->audio.iChannels = m->GetChannelCount();
	
		m->GetResolution(&t->video.iX1,&t->video.iY1,&t->video.iX2,&t->video.iY2,&t->video.iDU);
		t->video.iColorSpace = m->GetColorSpace();
		t->video.iAspectRatioType = m->GetAspectRatioType();
		t->video.fGamma = m->GetGamma();
		t->iCompression = m->GetTrackCompression(iSourceTrack);
		SetCodecID(iTrack,m->GetCodecID());
		SetCodecPrivate(iTrack,m->GetCodecPrivate(),m->GetCodecPrivateSize());
		SetTrackName(iTrack,m->GetTrackName());
		SetTrackLanguageCode(iTrack,m->GetLanguage());
	}
}

void MATROSKA::SetTrackCompression(int iTrack, int iCompression)
{
	tracks[iTrack]->iCompression = iCompression;
}


int MATROSKA::StoreBlock(ADDBLOCK *a)
{
//	printf("storing block...\n");
	int  iTrackIndex;
	BLOCK_INFO block_info;
	ZeroMemory(&block_info,sizeof(block_info));

	EBMLMCluster_Writer*	e_NextCluster;
	if ((a->iFlags & ABSSM_MASK) == ABSSM_INDEX) {
		iTrackIndex = a->iStream;
	} else {
		iTrackIndex = TrackNumber2Index(a->iStream);
	}

	if (a->iFrameCountInLace>1 && !tracks[iTrackIndex]->iLacing) {
		tracks[iTrackIndex]->iLacing = 1;
	}

	if ((a->iFlags & ABTC_MASK) == ABTC_UNSCALED) {
		a->iFlags &=~ABTC_MASK;
		a->iFlags |= ABTC_SCALED;
		a->iTimecode/=GetTimecodeScale();
		a->iDuration/=GetTimecodeScale();
	}

	__int64	iTimecode = a->iTimecode;

	if (!e_Cluster) {
		printf("creating first cluster...\n");
		e_Cluster = new EBMLMCluster_Writer(GetDest(),this,(max(0,a->iTimecode)),0,ws.iPosInSegment);
		ws.iClusterCount++;
		ws.iClusterOverhead += e_Cluster->GetSize();
		ws.iBlocksInCluster = 1;
	}

	if ((a->iFlags & ABSSM_MASK) == ABSSM_INDEX) {
		a->iStream = tracks[a->iStream]->iTrackNbr;
		a->iFlags &=~ ABSSM_MASK;
		a->iFlags |= ABSSM_NUMBER;
	}

	// keep duration?
	if (tracks[iTrackIndex]->iBufferedBlocks == 1 && 
		tracks[iTrackIndex]->iDurationMode == MATROSKA_TDDM_GAP ||
		tracks[iTrackIndex]->iDurationMode == MATROSKA_TDDM_ALWAYS) {
	} else {
		a->iDuration = 0;
	}

	if (a->iTimecode + a->iDuration > ws.iLatestTimecode) {
		ws.iLatestTimecode = a->iTimecode + a->iDuration;
	}

	if (e_Cluster->AddBlock(a, &block_info)==ABR_CLUSTERFULL) {
		int iSize = (int)e_Cluster->Write();
		ws.iAccumulatedOverhead += e_Cluster->GetWriteOverhead();
		if (IsClusterIndexEnabled()) {
			if (e_Seekhead->AddEntry((char*)MID_CLUSTER,ws.iPosInSegment) == ESHAE_FULL) {
				e_SecondarySeekhead = new EBMLMSeekhead_Writer(GetDest());
				e_SecondarySeekhead->AddEntry((char*)MID_CLUSTER,ws.iPosInSegment);
				e_Seekhead = e_SecondarySeekhead;
			}
		}
		ws.iPosInSegment += iSize;
		e_NextCluster = new EBMLMCluster_Writer(GetDest(),this,(max(0,a->iTimecode)),iSize,ws.iPosInSegment);
		ws.iClusterOverhead += e_NextCluster->GetSize();
		ws.iClusterCount++;
		DeleteEBML(&e_Cluster);
		e_Cluster = e_NextCluster;
		ws.iBlocksInCluster = 1;
		return StoreBlock(a);
	}

	if (!a->iRefCount) {
		if (tracks[iTrackIndex]->iTrackType == MSTRT_VIDEO && IsCuesEnabled(CUE_VIDEO) ||
			tracks[iTrackIndex]->iTrackType == MSTRT_AUDIO && IsCuesEnabled(CUE_AUDIO)) {
			ws.iCuePoints += AddCuepoint(a->iStream, ws.iPosInSegment,iTimecode, 
				(IsCueBlockNumberEnabled()?ws.iBlocksInCluster:0));
		}
	}


	tracks[iTrackIndex]->iLaceSchemes[block_info.iLaceStyle]++;
	tracks[iTrackIndex]->iBlocksWritten++;
	tracks[iTrackIndex]->iFramesWritten[block_info.iLaceStyle]+=a->iFrameCountInLace;
	tracks[iTrackIndex]->iLaceOverhead[block_info.iLaceStyle]+=block_info.iLaceOverhead;

	tracks[iTrackIndex]->iTotalSize += a->cData->GetSize();
	tracks[iTrackIndex]->iTotalFrameCount += (a->iFrameCountInLace)?a->iFrameCountInLace:1;

	ws.iBlocksInCluster++;

	if (a->iFrameCountInLace>1) {

	}

	return 0;
}

int MATROSKA::Write(ADDBLOCK* a)
{
	//StoreBlock(a);
	PrebufferBlock(a);
	return 0;
}

int MATROSKA::FindEarliestBlockIndexOfStream(int s)
{
	int iMinIndex = 0x7FFFFFFF, iMinIndexInStream = 0x7FFFFFFF;

	int iBegin = iBufsPerStream*s;
	for (int j=0; j<iBufsPerStream; j++) {
		if (write_buffer.used[iBegin] < iMinIndexInStream && write_buffer.used[iBegin]) {
			iMinIndex = iBegin;
			iMinIndexInStream = write_buffer.used[iBegin];
		}
		iBegin++;
	}

	return ((iMinIndex>(s+1)*iBufsPerStream)?(s*iBufsPerStream):iMinIndex);
}

int MATROSKA::FindEarliestBlockIndex()
{
	__int64 iMinTC = 0x7FFFFFFFFFFFFFFF;
	int iMinIndex = 0x7FFFFFFF;
	int bUsed = 0;
	int i;

	for (int j=0;j<GetTrackCount();j++) {
		if (write_buffer.used[i=FindEarliestBlockIndexOfStream(j)]) {
			bUsed = 1;
			if (iMinTC > write_buffer.block[i].iTimecode) {
				iMinTC = write_buffer.block[i].iTimecode;
				iMinIndex = i;
			}
		}
	}
	if (!bUsed) return -1;
	return iMinIndex;
}

int MATROSKA::StoreEarliestBlock()
{
	int iMinIndex = FindEarliestBlockIndex();
	if (iMinIndex < write_buffer.iCount && iMinIndex >= 0) {
		write_buffer.used[iMinIndex] = 0;
		int i = write_buffer.block[iMinIndex].iStream; 
		StoreBlock(&write_buffer.block[iMinIndex]);
		tracks[i]->iBufferedBlocks--;
		DecBufferRefCount(&write_buffer.block[iMinIndex].cData);
		if (write_buffer.block[iMinIndex].iFrameCountInLace) {
			delete write_buffer.block[iMinIndex].iFrameSizes;
		}
		return iMinIndex;
	}
	return -1;
}

int MATROSKA::PrebufferBlock(ADDBLOCK* a)
{
	int j;
	if ((j=FindEmptyBufferIndex(a->iStream)) >= write_buffer.iCount) {
		StoreEarliestBlock();
		PrebufferBlock(a);
	} else {
		CopyAddBlock(j,a);
		a->cData->IncRefCount();
	}

	return 0;
}

int MATROSKA::CopyAddBlock(int j, ADDBLOCK *a)
{
	tracks[a->iStream]->iBufferedBlocks++;
	memcpy(&write_buffer.block[j],a,sizeof(*a));
	if (a->iFrameCountInLace) {
		write_buffer.block[j].iFrameSizes = new int[a->iFrameCountInLace];
		memcpy(write_buffer.block[j].iFrameSizes, a->iFrameSizes, sizeof(int)*a->iFrameCountInLace);
	}
	write_buffer.used[j] = ++write_buffer.block_of_stream[a->iStream];

	return 0;
}

int MATROSKA::FlushWriteBuffer()
{
	for (int j=0;j<write_buffer.iCount;j++) {
		StoreEarliestBlock();
	}

	return 0;
}

int MATROSKA::FindEmptyBufferIndex(int s)
{
	int bStop = 0;

	int j=s*iBufsPerStream;
	while (!bStop && j<(s+1)*iBufsPerStream) {
		if (!write_buffer.used[j]) {
			return j;
		}
		j++;
	}

	return write_buffer.iCount;
}

__int64 MATROSKA::GetOverhead()
{
	__int64 iRes = ws.iAccumulatedOverhead;
	ws.iAccumulatedOverhead = 0;
	return iRes;
}

int MATROSKA::AddCuepoint(int iTrack, __int64 iClusterPosition, __int64 iTimecode, __int64 iBlock)
{
	iTimecode *= GetTimecodeScale();
	if (iTimecode - ws.iLastCuePointTimecode[iTrack] > ws.iMinimumCuePointInterval) {
		e_Cues->AddCuePoint(iTrack, iTimecode/GetTimecodeScale(), iClusterPosition, iBlock);
		ws.iLastCuePointTimecode[iTrack] = iTimecode;
		return 1;
	}

	return 0;
}

void MATROSKA::SetMaxClusterSize(__int64 iSize)
{
	ws.iMaxClusterSize = iSize * 1024;
}

__int64 MATROSKA::GetMaxClusterSize()
{
	return ws.iMaxClusterSize;
}

void MATROSKA::SetMaxClusterTime(__int64 iTime, int bLimit1stCluster)
{
	ws.iMaxClusterTime = iTime;
	ws.iLimit1stCluster = bLimit1stCluster;
}

__int64 MATROSKA::GetMaxClusterTime()
{
	return ws.iMaxClusterTime;
}

void MATROSKA::EnableClusterPosition(int bEnable)
{
	ws.bClusterPosition = bEnable;
}

bool MATROSKA::IsClusterPositionEnabled()
{
	return !!ws.bClusterPosition;
}

bool MATROSKA::Is1stClusterLimited()
{
	return !!ws.iLimit1stCluster;
}

bool MATROSKA::IsDisplayWidth_HeightEnabled()
{
	return !!ws.bDisplayWidth_Height;
}

void MATROSKA::EnablePrevClusterSize(int bEnable)
{
	ws.bPrevClusterSize = !!bEnable;
}

void MATROSKA::EnableDisplayWidth_Height(int bEnable)
{
	ws.bDisplayWidth_Height = !!bEnable;
}

bool MATROSKA::IsPrevClusterSizeEnabled()
{
	return !!ws.bPrevClusterSize;
}

void MATROSKA::SetAppName(char* cName)
{
	e_SegInfo->SetWritingApp(new CStringBuffer(cName));
}

void MATROSKA::SetChapters(CChapters* c, __int64 iDuration)
{
	chapters = c;
	max_chapter_duration = iDuration;
}

float MATROSKA::GetTrackTimecodeScale(int iIndex)
{
	return (float)pActiveSegInfo->tracks->track_info[MapT(iIndex)]->fTimecodeScale;
}

void MATROSKA::Deb0rkReferences(bool bDeb0rk)
{
	ws.bDeb0rkReferences = bDeb0rk;
}

void MATROSKA::Seek(__int64 iTimecode)
{
	EBMLM_Segment*	seg = (EBMLM_Segment*)e_Segments->pElement[GetActiveSegment()];
	int				j;

//	if (!iTimecode) iTimecode = -33000;
	seg->Seek(iTimecode);
	FlushQueues();
	queue = new QUEUE*[j=GetTrackCount()];
	ZeroMemory(queue,j*sizeof(QUEUE*));

	for (j=0;j<GetTrackCount();j++)
		if (pActiveSegInfo->tracks->track_info[j]->iType!=MSTRT_SUBT) pActiveSegInfo->tracks->track_info[j]->iSparse=0;

	SetMinTimecode(iTimecode);
	pActiveSegInfo->iEndReached = 0;
}

float MATROSKA::GetAvgFramesize(int iTrack)
{
	iTrack = MapT(iTrack);

	int		iSize = 0;
	int		iCount = 0;

	while (QUEUE_empty(queue[iTrack]) && ReadBlock() != READBL_ENDOFSEGMENT);
	QUEUE*	q = queue[iTrack];
	QUEUE*	ql = q->pLast;

	do {
		READBLOCK_INFO*	r = (READBLOCK_INFO*)q->buffer->GetData();
		iSize += r->cData->GetSize();
		iCount += max(1,r->iFrameCountInLace);
		q = q->pLast;
	} while (q != q->pLast);

	return (float)iSize/float(iCount);
}

__int64 MATROSKA::GetTrackSize(int iTrack)
{
	iTrack = MapT(iTrack);

	int		iSize = 0;
	int		iCount = 0;
	float	fTime = 0;
	TRACK_INFO* t = pActiveSegInfo->tracks->track_info[iTrack];

	if (t->tags.iFlags & TRACKTAGS_BITSPS) {
		return (__int64)((double)GetSegmentDuration() * GetTimecodeScale() * t->tags.iBitsPS / 8000000000);
	}

	while ((QUEUE_empty(queue[iTrack]) || QUEUE_empty(queue[iTrack]->pNext)) && ReadBlock() != READBL_ENDOFSEGMENT);
	QUEUE*	q = queue[iTrack];
	if (q) {
		QUEUE*	ql = q->pLast;

		do {
			READBLOCK_INFO*	r1 = (READBLOCK_INFO*)q->buffer->GetData();
			READBLOCK_INFO* r2 = (READBLOCK_INFO*)q->pNext->buffer->GetData();

			fTime += r2->qwTimecode - r1->qwTimecode;
			if (fTime>0.01) {
				iSize += r1->cData->GetSize();
			}
			q = q->pLast;

		} while (q != q->pLast);

		return (__int64)(min(GetSource()->GetSize(),(float)iSize*GetSegmentDuration()/fTime));
	} else {
		return 0;
	}
}

float MATROSKA::GetTrackBitrate(int iTrack) 
{
	iTrack = MapT(iTrack);
	TRACK_INFO* t = pActiveSegInfo->tracks->track_info[iTrack];

	if (IsBitrateIndicated()) {
		return (float)(t->tags.iBitsPS);
	} else return 0;
}

int MATROSKA::IsBitrateIndicated(int iTrack)
{
	return !!(pActiveSegInfo->tracks->track_info[MapT(iTrack)]->tags.iFlags & TRACKTAGS_BITSPS);
}

int MATROSKA::GetLaceStyle()
{
	return (int)(ws.iLaceStyle);
}

__int64 MATROSKA::GetLaceStatistics(int iTrack, int iIndex, LACE_STATS* lpData)
{
	iTrack = MapT(iTrack);
	lpData->iCount = tracks[iTrack]->iLaceSchemes[iIndex];
	lpData->iTotalHdrSize = tracks[iTrack]->iLaceOverhead[iIndex];
	lpData->iFrameCount = tracks[iTrack]->iFramesWritten[iIndex];

	return 1;
}

void MATROSKA::GetClusterStats(CLUSTER_STATS* lpInfo)
{
	lpInfo->iCount = (int)ws.iClusterCount;
	lpInfo->iOverhead = (int)ws.iClusterOverhead;
}

int MATROSKA::GetCueCount()
{
	return (int)ws.iCuePoints;
}

__int64 MATROSKA::GetSeekheadSize()
{
	return ws.iSeekheadSize;
}

char* MATROSKA::GetSegmentTitle(int iSegment)
{
	CStringBuffer* c = NULL;

	if (iSegment == -1) {
		c = pActiveSegInfo->cTitle;
	} else {
		if (iSegment < GetSegmentCount()) {
			c = SegmentInfo[iSegment]->cTitle;
		} 
	}

	return (c)?c->Get():"";
}

int MATROSKA::SetSparseFlag(int iTrack, int bFlag)
{

	if (iTrack<0) return -1;
	if (iTrack>=GetTrackCount()) return -1;

	TRACK_INFO* t= pActiveSegInfo->tracks->track_info[MapT(iTrack)];

	t->iSparse = !!bFlag;
	return 0;	
}
