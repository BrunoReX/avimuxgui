#include "stdafx.h"
#include "matroska.h"
#include "Matroska_Segment.h"
#include "Matroska_Block.h"
#include "warnings.h"
#include "math.h"
#include "integers.h"
#include "ebml.h"
#include "stdio.h"

#ifdef DEBUG_NEW
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

const int iSizeOfSeekhead = 1024;
const int iBufsPerStream = 32;
char debugmsg[1024];

char* MKV_LINKTYPE_NAMES[] = { "medium linking", "hard linking", "" };
char* MKV_UID_NAMES[] = { "SegmentUID", "PrevUID", "NextUID", "SegmentFamily" };

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
	int basic_block_overhead = 8;
	int overhead_per_keyframe = 10;
	int overhead_per_deltaframe = 13;
	int frame_type_overhead_difference = 3;
	int audio_frame_base_overhead = 8;
	int audio_frame_incremental_overhead = 2;

	MGO_STREAMDESCR* d;
	if (lpMGO->iMatroskaVersion == 2) {
		basic_block_overhead = 5;
		overhead_per_keyframe = 7;
		overhead_per_deltaframe = 7;
		frame_type_overhead_difference = 0;
		audio_frame_base_overhead = 6;
		audio_frame_incremental_overhead = 1;
	}

//	iRes = 20*1024;

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

		int iClusterOverhead = iOvhPC * iClCount;

		iRes += iClusterOverhead;
		printf("predicted cluster overhead: %d bytes\n", iClusterOverhead);

// cues
	if (!(lpMGO->iFlags & MGOF_NOCUEOVERHEAD)) {
		iCueCount = int(lpMGO->iDuration * lpMGO->fFPS / lpMGO->iKeyframeInterval);
		iRes += 18 * iCueCount;

		printf("cue points: %d predicted using %d bytes", iCueCount, 18*iCueCount);
	}

// video stream
	iRes += (int)(lpMGO->fFPS * lpMGO->iDuration * overhead_per_deltaframe - 
		frame_type_overhead_difference * lpMGO->fFPS * lpMGO->iDuration / lpMGO->iKeyframeInterval);
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
			iLaceOvh += audio_frame_base_overhead + 
				audio_frame_incremental_overhead*!!(iLaceSize > 125) + 2*!!(iLaceSize > 16375);
			
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
			iOvhPF = audio_frame_base_overhead + 
				audio_frame_incremental_overhead *(!!(d->iFramesize > 125) + !!(d->iFramesize > 16375));
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
//	iAVImode = 1;
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

READ_INFO::READ_INFO()
{
	iTrack = 0;
	qwTimecode = 0;
	qwDuration = NULL;
	iFlags = 0;
	iLength = 0;
	reference_types = 0;

	pData = NULL;
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
	for (int j=0;j<GetSegmentCount();j++) {
		SetActiveSegment(j);
		if (queue) {
			for (int i=0;i<GetTrackCount();i++) {
				QUEUE_kill(&queue[i]);
			}
		}
	}
}

int MATROSKA::SetActiveSegment(int iSegment)
{
	if (iSegment<GetSegmentCount() && iSegment > -1) {
		if (pActiveSegInfo) 
			pActiveSegInfo->queue = (void**)queue;
		
		if (SegmentInfo)
			queue = (QUEUE**)SegmentInfo[iSegment]->queue;
		
		info->iActiveSegment = iSegment;
		if (SegmentInfo)
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
	size_t	i;

	if (info->mode == MMODE_READ && tra->iTable[iNbr-1] == iNbr) return iNbr-1;
	if (info->mode == MMODE_WRITE && tracks[iNbr-1]->iTrackNbr == iNbr) return iNbr-1;

	for (i=0;i<tra->iCount;i++) {
		if (info->mode == MMODE_READ) {
			if (tra->iTable[i] == iNbr) return (int)i;
		} else if (tracks[i]->iTrackNbr == iNbr) return (int)i; 
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
	READBLOCK_INFO*		rbi = new READBLOCK_INFO;
	EBMLM_Segment*		seg = (EBMLM_Segment*)e_Segments->pElement[GetActiveSegment()];
	int					iStreamIndex, i;
	char*				laceelement;
	CBuffer*			c;

//	ZeroMemory(&rbi,sizeof(rbi));

	if (pActiveSegInfo->iEndReached == 1) {
		delete rbi;
		return READBL_ENDOFSEGMENT;
	}

	if (seg->ReadBlock(rbi)==READBL_ENDOFSEGMENT) {
		pActiveSegInfo->iEndReached = 1;
		delete rbi;
		return READBL_ENDOFSEGMENT;
	}

	if (iDebugLevel > 1) {
		sprintf(debugmsg, "ReadBlock: stream %d, timecode %I64d, duration: %d",
			rbi->iStream, rbi->qwTimecode, (int)rbi->qwDuration);
		fprintf(stderr, "%s%c",debugmsg, '\n');
	}

	if ((iStreamIndex = TrackNumber2Index(rbi->iStream)) == TN2I_INVALID) {
		B0rked("Block belongs to stream which is not described in 'Tracks'");
		DecBufferRefCount(&rbi->cData);
		delete rbi;
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

	if (rbi->qwTimecode < GetMinTimecode()) {
		DecBufferRefCount(&rbi->cData);
		delete rbi;
		return READBL_OK;
	}

	if (iSingleStream>-1) {
		if (iSingleStream !=iStreamIndex) {
			DecBufferRefCount(&rbi->cData);
			delete rbi;
			return READBL_OK;
		}
	}
	
	if (!IsStreamQueued(iStreamIndex)) {
		DecBufferRefCount(&rbi->cData);
		delete rbi;
		return READBL_OK;
	}

	rbi->iEnqueueCount = iEnqueueCount++;
	if (rbi->iFlags & BLKHDRF_LACING) {
		laceelement = rbi->cData->AsString();
		c = new CBuffer;
		c->SetExternal(rbi, sizeof(*rbi));
		c->IncRefCount();
		QUEUE_enter(&queue[iStreamIndex],c);

	} else {
		c = new CBuffer;
		rbi->qwTimecode = (__int64)((double)(rbi->qwTimecode) * pActiveSegInfo->tracks->track_info[iStreamIndex]->fTimecodeScale);
		
		c->SetExternal(rbi,sizeof(*rbi));
		c->IncRefCount();
		QUEUE_enter(&queue[iStreamIndex],c);
	}

	__int64 idq = pActiveSegInfo->tracks->track_info[iStreamIndex]->iDataQueued += rbi->cData->GetSize();
	__int64 iTimecode = rbi->qwTimecode;
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
	return ((EBMLM_Segment*)(e_Segments->pElement[GetActiveSegment()]))->GetCues(0, -1);
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
		if (bEmpty) 
			return READBL_ENDOFSEGMENT;

		iTrack = iIndex;

	} else {
		// if a block shall be read, look for one
		bEmpty = true;
		iRes = FillQueue(iTrack);

		for (int i=0;i<GetTrackCount();i++) 
			if (!QUEUE_empty(queue[i])) 
				bEmpty = false;

		if (bEmpty) 
			return READBL_ENDOFSEGMENT;

	}
	
	if (QUEUE_empty(queue[iTrack])) {
		return READBL_ENDOFTRACK;
	}

	CBuffer* b = QUEUE_read(&queue[iTrack]);
	READBLOCK_INFO*	rbi = (READBLOCK_INFO*)b->GetData();

	pInfo->iLength = rbi->cData->GetSize();

	if (rbi->frame_sizes.size() > 1 && (!IsLaced(iTrack)))
		B0rked("Lacing used, but lacing flag is not set");

	pInfo->qwTimecode = rbi->qwTimecode;
	pInfo->pData = rbi->cData;

	pInfo->reference_types = rbi->reference_types;
	pInfo->iFlags = 0;
	pInfo->iTrack = iTrack;

	if (rbi->iFlags & RBIF_DURATION) {
		pInfo->iFlags |= RIF_DURATION;
		pInfo->qwDuration = rbi->qwDuration;
	}

	if (rbi->iFlags & RBIF_DISCARDABLE) {
		pInfo->iFlags |= RIF_DISCARDABLE;
	}
	
//	if (rbi->iFlags & BLKHDRF_LACINGMASK)
//		pInfo->iFlags = RIF_LACING;

	pInfo->references = rbi->references;

	__int64 idq = pActiveSegInfo->tracks->track_info[iTrack]->iDataQueued -= rbi->cData->GetSize();

	int last_compression = GetTrackCompressionDescriptorCount(iTrack) - 1;

	if (GetTrackCompression(iTrack, last_compression) == COMPRESSION_HDRSTRIPING) {
		
		if (rbi->frame_sizes.empty()) {
			rbi->cData->Prepend(0, GetTrackCompressionPrivate(iTrack, last_compression),
				GetTrackCompressionPrivateSize(iTrack, 0));
		} else {
			int hdr_size = GetTrackCompressionPrivateSize(iTrack, last_compression);
			int total_hdr_size = 0;
			int total_pos = 0;
			void* hdr = GetTrackCompressionPrivate(iTrack, last_compression);
			std::vector<int>::iterator frame_sizes = rbi->frame_sizes.begin();
	
			for (; frame_sizes != rbi->frame_sizes.end(); frame_sizes++) {
				rbi->cData->Prepend(total_pos, hdr, hdr_size);

				total_hdr_size += hdr_size;
				(*frame_sizes) += hdr_size;

				total_pos += *frame_sizes;
			}
		}
	}

	if (rbi->frame_sizes.size() > 1) {
		pInfo->frame_sizes = rbi->frame_sizes;
//		pInfo->iFlags |= BLKHDRF_LACING;
	} else {
		pInfo->frame_sizes.clear();
	}


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

	return !rbi->references.size();
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

__int64 MATROSKA::GetSegmentFilePos() 
{
	return e_Segments->pElement[GetActiveSegment()]->GetStreamPos();
}

bool MATROSKA::CanRead()
{
	if (info->mode & MMODE_READ)
		return true;
	else
		return false;
}

bool MATROSKA::CanWrite()
{
	if (info->mode & MMODE_WRITE)
		return true;
	else
		return false;
}

bool MATROSKA::ParseEBMLHeader(EBML_INFO& ebml_info, EBMLElement* parent)
{
	if (!parent)
		return false;

	/* parse EBML header */
//	EBML_INFO	ebml_info;
	char* ebml_ids[] = { (char*)EID_EBMLVersion, (char*)EID_EBMLReadVersion,
		(char*)EID_EBMLMaxIDLength, (char*)EID_EBMLMaxSizeLength,
		(char*)EID_DocType, (char*)EID_DocTypeVersion, (char*)EID_DocTypeReadVersion,NULL };
	void* ebml_target[] = {
		&ebml_info.EBMLVersion, &ebml_info.EBMLReadVersion, &ebml_info.EBMLMaxIDLengh, 
		&ebml_info.EBMLMaxSizeLength, &ebml_info.DocType, &ebml_info.DocTypeVersion,
		&ebml_info.DocTypeReadVersion, NULL };
	int ebml_occ_restr[] = {
		2, 2, 2, 2, 3, 2, 2, NULL };
	SEARCHMULTIEX ebml_sme = { ebml_ids, ebml_target, ebml_occ_restr };
	EBMLElementVectors ebml_srch;
	int result = parent->SearchMulti(ebml_srch, ebml_sme);
//	DeleteEBML(&e_next);
	DeleteVectors(ebml_srch);

	if (result < 0)
		return false;

	return true;
}

int MATROSKA::Open(STREAM* s, int iMode)
{
	size_t i;
	stream = s;
	EBMLElement*		e_next=NULL;
	EBMLElement*		e_SIInfo = NULL;
	EBMLM_Segment*		segment;
	CBuffer*			cb=NULL;

	info = new MATROSKA_INFO;
	memset(info, 0, sizeof(MATROSKA_INFO));
//	newz(MATROSKA_INFO, 1, info);
	
	if (iMode & MMODE_READ) {
		info->mode = iMode;
		e_Main = new EBML_Matroska(s,NULL);
		
		/* first EBML element must be an EBML header */
		e_Main->Create1stSub(&e_next);
		if (!e_next || e_next->GetType()!=EBMLTYPE_EBML) {
			Close();
			DeleteEBML(&e_next);
			DeleteEBML((EBMLElement**)&e_Main);
			return MOPEN_ERR;
		}

		/* parse EBML header */
/*		EBML_INFO	ebml_info;
		char* ebml_ids[] = { (char*)EID_EBMLVersion, (char*)EID_EBMLReadVersion,
			(char*)EID_EBMLMaxIDLength, (char*)EID_EBMLMaxSizeLength,
			(char*)EID_DocType, (char*)EID_DocTypeVersion, (char*)EID_DocTypeReadVersion,NULL };
		void* ebml_target[] = {
			&ebml_info.EBMLVersion, &ebml_info.EBMLReadVersion, &ebml_info.EBMLMaxIDLengh, 
			&ebml_info.EBMLMaxSizeLength, &ebml_info.DocType, &ebml_info.DocTypeVersion,
			&ebml_info.DocTypeReadVersion, NULL };
		int ebml_occ_restr[] = {
			2, 2, 2, 2, 3, 2, 2, NULL };
		SEARCHMULTIEX ebml_sme = { ebml_ids, ebml_target, ebml_occ_restr };
		EBMLElementVectors ebml_srch;
		e_next->SearchMulti(ebml_srch, ebml_sme);
		DeleteEBML(&e_next);
		DeleteVectors(ebml_srch);
*/
		EBML_INFO ebml_info;
		ParseEBMLHeader(ebml_info, e_next);
		DeleteEBML(&e_next);
		char* cid_Level0[] = { (char*)EID_EBML, (char*)MID_SEGMENT };
		void** pe_Level0 = NULL;
		e_Main->SearchMulti(&pe_Level0, cid_Level0, 2);
		e_EBML		= (EBMLELEMENTLIST*)pe_Level0[0];
		e_Segments	= (EBMLELEMENTLIST*)pe_Level0[1];
		delete pe_Level0;
		if (strcmp("matroska", ebml_info.DocType)) {
			FatalError("Unknown DocType");
			DeleteElementList(&e_EBML);
			return MOPEN_ERR;
		}

		if (iMode & MMODE_DUMMY) {
			DeleteElementList(&e_EBML);
			return MOPEN_OK;
		}

		if (!e_Segments->iCount) {
			FatalError("No Segment found!");
			DeleteElementList(&e_EBML);
			return MOPEN_ERR;
		}

		char ver[32]; ver[0]=0; 
		sprintf(ver, "File type: Matroska v%d", ebml_info.DocTypeVersion);
		Note(ver);

		if (ebml_info.DocTypeVersion > 2) {
			FatalError("This matroska version is not supported");
			return MOPEN_ERR;
		}
//		newz(SEGMENT_INFO*,e_Segments->iCount, SegmentInfo);
		SegmentInfo = new SEGMENT_INFO*[e_Segments->iCount];
	
		for (i=0;i<e_Segments->iCount;i++) {
			segment = ((EBMLM_Segment*)e_Segments->pElement[i]);
//			segment->AllowObfuscatedSegments(bAllowObfuscatedFiles);
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

			size_t count = e_EBML->iCount - 1;
			bool found = false;
			for (; count >= 0 && !found; count-- )
				if (e_EBML->pElement[count]->GetStreamPos() <
					segment->GetStreamPos())
					found = true;

			count++;
			ParseEBMLHeader(segment->GetSegmentInfo()->ebml_info, e_EBML->pElement[count]);
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
		if (e) 
			DeleteEBML(&e);
		iEnqueueCount = 0;
		DeleteElementList(&e_EBML);

		return MOPEN_OK;
	}

	if (iMode & MMODE_WRITE) {
		srand((int)rdtsc());
		info->mode = iMode;
		ZeroMemory(&ws,sizeof(ws)-sizeof(ws.cues));
		memset(&write_buffer, 0, sizeof(write_buffer));

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
		ws.min_cue_point_interval = 1995000000; // 2 seconds
		
		ws.iMaxClusterSize = 524288;
		ws.iMaxClusterTime = 30000;
		ws.iLaceStyle = LACESTYLE_XIPH;
		ws.iClusterCount = 0;
		ws.iClusterOverhead = 0;
		ws.iCuePointsCreated = 0;
		ws.iCuePointsWritten = 0;
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
		ws.iEarliestTimecode = (__int64)INT_MAX * INT_MAX;
		ws.iTargetSeekhead = 1;
		ws.bRandomizeElementOrder = 0;
		ws.iSizeOfReservedSpace = 16384;
		ws.iCueAutosize = 0;
		ws.max_cue_point_interval = 30005000000; // 30 seconds
		ws.iCueTargetSizeRatio = 0.98;
		memset(ws.bUIDs, 0, sizeof(ws.bUIDs));
		
		// set default behaviour for Duration elements
		ws.duration_default.iVideo = MATROSKA_TDDM_GAP;
		ws.duration_default.iAudio = MATROSKA_TDDM_GAP;
		ws.duration_default.iSubtitles = MATROSKA_TDDM_ALWAYS;
		ws.matroska_version = 1;
	
		return MOPEN_OK;
	}

	return MOPEN_ERR;
}

void MATROSKA::SetUID(int uid_type, char* cUID)
{
	if (cUID) {
		ws.bUIDs[uid_type] = true;
		memcpy(ws.cUIDs[uid_type], cUID, 16);
	} else
		ws.bUIDs[uid_type] = false;
}

void MATROSKA::SetTimecodeScale(__int64 iScale)
{
	ws.iTimecodeScale = iScale;
}

void MATROSKA::SetInitialHeaderSize(int size)
{
	ws.iSizeOfReservedSpace = size;
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
	e_Header->SetDocTypeVersion((int)ws.matroska_version);
	e_Header->Write();
	DeleteEBML(&e_Header);

	ws.iSegPosInFile = GetDest()->GetPos();

	ws.iPosInSegment = 0;
	ws.iSeekheadPosInFile = ws.iSegPosInFile + e_Segment->Write();
	ws.iSegClientDataBegin = ws.iSeekheadPosInFile;

	ws.i1stElementInFile = ws.iSeekheadPosInFile;

	// write dummy seek head
	if (!ws.bClusterIndex)
		e_PrimarySeekhead->SetFixedSize(512);

	ws.iPosInSegment += e_PrimarySeekhead->Write();
	e_PrimarySeekhead->AddEntry((char*)MID_TRACKS,ws.iPosInSegment);
	e_Seekhead = e_PrimarySeekhead;

	// write track info
	int iSize = GetTrackCount()*192; 
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

	int title_len = (int)strlen(e_SegInfo->GetTitle());
	e_SegInfo->SetFixedSize(256 + strlen(cVersion) + 128
		+ title_len 
		+ 20 * (!!ws.bUIDs[0] + !!ws.bUIDs[1] + !!ws.bUIDs[2] + !!ws.bUIDs[3])); 
	// 256+128 = <normal stuff> + muxing app + 20 for each 128 bit UID

	e_Cues = new EBMLMCue_Writer(GetDest());

	ws.iPosInSegment += e_SegInfo->Write();
	ws.iPosOfReservedSpace = GetDest()->GetPos();
	e_ReservedSpace = new EBMLElement_Writer(GetDest());
	e_ReservedSpace->SetID((char*)MID_VOID);

	__int64 k = ws.iSizeOfReservedSpace - e_SegInfo->GetSize() - e_Tracks->GetSize() - e_PrimarySeekhead->GetSize();
	k = max(k, 16);

	ws.iSizeOfReservedSpace = k;
	e_ReservedSpace->SetFixedSize(k);
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

void MATROSKA::RenderChapterSegmentUIDTags(EBMLElement_Writer* pParent, CChapters* c)
{
	for (int k=0;k<c->GetChapterCount();k++) {
		SINGLE_CHAPTER_DATA scd; memset(&scd, 0, sizeof(scd));
		
		c->GetChapter(k, &scd);
		if (scd.bSegmentUIDValid) {
			EBMLElement_Writer* pTag = pParent->AppendChild(new EBMLElement_Writer(GetDest(),(char*)MID_TG_TAG));
			pTag->AppendChild(new EBMLElement_Writer(GetDest(), (char*)MID_TG_TARGET))
				->AppendChild_UInt((char*)MID_TG_CHAPTERUID, c->GetUID(k));

			EBMLElement_Writer* pSimpleTag = pTag->AppendChild(new EBMLElement_Writer(GetDest(), 
				(char*)MID_TG_SIMPLETAG));
			pSimpleTag->AppendChild_String((char*)MID_TG_TAGNAME, "SegmentUID");
			char uid[64]; memset(uid,0,sizeof(uid));
			__int128hex(scd.cSegmentUID, uid, 1, 0);
			pSimpleTag->AppendChild_String((char*)MID_TG_TAGSTRING, uid);

			pTag->EnableCRC32();
		}

		if (c->HasSubChapters(k))
			RenderChapterSegmentUIDTags(pParent, c->GetSubChapters(k));
	}


}

int MATROSKA::Close()
{
	int i;
	int j = (e_Segments)?GetSegmentCount():-1;
	int	iTrash;

	for (i=0;i<j;i++) {
		SetActiveSegment(i);
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

	//if (info && info->mode == MMODE_WRITE) {
	if (CanWrite()) {
		EBMLMSeekhead_Writer* e_TargetSeekhead = NULL;
		e_TargetSeekhead = e_PrimarySeekhead;

		if (ws.iTargetSeekhead == 0 && e_SecondarySeekhead)
			e_TargetSeekhead = e_SecondarySeekhead;		

		FlushWriteBuffer();
		SetSegmentDuration((float)ws.iLatestTimecode - (float)ws.iEarliestTimecode);
		if (e_Cluster) {
			if (IsClusterIndexEnabled()) {
				e_Seekhead->AddEntry((char*)MID_CLUSTER,ws.iPosInSegment,1);
			}
			ws.iPosInSegment += e_Cluster->Write();
			ws.iAccumulatedOverhead += e_Cluster->GetWriteOverhead();
			DeleteEBML(&e_Cluster);
		} else {
			e_Cluster = new EBMLMCluster_Writer(GetDest(), this, 0, 0, 0);
			ws.iPosInSegment += e_Cluster->Write();
			ws.iAccumulatedOverhead = e_Cluster->GetWriteOverhead();
			e_Cluster->Delete();
			delete e_Cluster;
			e_Cluster = NULL;
		}

		// write chapters
		bool bEndNotIndcated = false;
		if (chapters && chapters->GetChapterCount()) {
			if (chapters->GetChapterEnd(CHAP_LAST) == -1) {
				chapters->SetChapterEnd(CHAP_LAST,(__int64)(ws.fSetDuration*GetTimecodeScale()));
				bEndNotIndcated = true;
			}

			EBMLMChapter_Writer* e_Chapters = new EBMLMChapter_Writer(GetDest(),chapters,
				(max_chapter_duration==-1)?(__int64)(ws.fSetDuration * GetTimecodeScale()):max_chapter_duration,
				(max_chapter_duration==-2)?0:1);

			if (e_Chapters->GetSize() > ws.iSizeOfReservedSpace) {
				e_TargetSeekhead->AddEntry((char*)MID_CHAPTERS,ws.iPosInSegment,1);
				ws.iPosInSegment += (i=(int)e_Chapters->Write());
				ws.iAccumulatedOverhead += i;
				DeleteEBML(&e_Chapters);
			} else {
				__int64 q = GetDest()->GetPos();
				GetDest()->Seek(ws.iPosOfReservedSpace);
				e_TargetSeekhead->AddEntry((char*)MID_CHAPTERS,ws.iPosOfReservedSpace - ws.iSegClientDataBegin,1);
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
			e_TargetSeekhead->AddEntry((char*)MID_TRACKS,ws.iPosInSegment,1);
			ws.iPosInSegment += (i=(int)e_Tracks->Write());
			ws.iAccumulatedOverhead += i;
		}

		// compose tags
		EBMLElement_Writer*	e_Tags = new EBMLElement_Writer(GetDest(),(char*)MID_TAGS);
		int k;
		for (k=0;k<iTrackCount;k++) {
			__int64 iBitsPS = (__int64)((double)tracks[k]->iTotalSize * 8000000000 / ws.fSetDuration / GetTimecodeScale());
			double  dFPS    = (double) ((double)tracks[k]->iTotalFrameCount * 1000000000 / ws.fSetDuration / GetTimecodeScale());

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
			e_SimpleTag->EnableCRC32();
			e_Tag->EnableCRC32();
		}

		for (k=0;k<chapters->GetChapterCount();k++) {
			int l;
			EBMLElement_Writer* e_Tag = NULL;
			
			if (chapters->GetChapterDisplayCount(k)) {

				e_Tag = e_Tags->AppendChild(new EBMLElement_Writer(GetDest(),(char*)MID_TG_TAG),
					APPEND_END | APPEND_DONT_RANDOMIZE);
				EBMLElement_Writer* e_Target = e_Tag->AppendChild(new EBMLElement_Writer(GetDest(),(char*)MID_TG_TARGET));
				e_Target->AppendChild_UInt((char*)MID_TG_EDITIONUID,chapters->GetUID(k),-1);
			}

			for (l=0;l<chapters->GetChapterDisplayCount(k);l++) {

				char* cTitle = NULL; cTitle = chapters->GetChapterText(k, l);
				char* cLng   = NULL; cLng   = chapters->GetChapterLng (k, l);
				if (cTitle && *cTitle || cLng && *cLng) {

					if (cTitle && *cTitle) {
						EBMLElement_Writer* e_SimpleTag = e_Tag->AppendChild(new EBMLElement_Writer(GetDest(),(char*)MID_TG_SIMPLETAG),
							APPEND_END | APPEND_DONT_RANDOMIZE);
						e_SimpleTag->AppendChild_String((char*)MID_TG_TAGNAME,"TITLE");
						e_SimpleTag->AppendChild_String((char*)MID_TG_TAGSTRING,cTitle);
						if (cLng && *cLng)
							e_SimpleTag->AppendChild_String((char*)MID_TG_TAGLANGUAGE,cLng);
						e_SimpleTag->EnableCRC32();
					}
				}
			}

			if (e_Tag)
				e_Tag->EnableCRC32();
		}

		RenderChapterSegmentUIDTags(e_Tags, chapters);

		e_Tags->EnableCRC32();

		if (ws.iSizeOfReservedSpace < e_Tags->GetSize()) {
			e_TargetSeekhead->AddEntry((char*)MID_TAGS,ws.iPosInSegment,1);
			ws.iAccumulatedOverhead += (i=(int)e_Tags->Write());
			ws.iPosInSegment += i;
		} else {
			__int64 q = GetDest()->GetPos();
			GetDest()->Seek(ws.iPosOfReservedSpace);
			e_TargetSeekhead->AddEntry((char*)MID_TAGS,ws.iPosOfReservedSpace - ws.iSegClientDataBegin,1);
			i=(int)e_Tags->Write();
			ws.iPosOfReservedSpace += i;
			ws.iSizeOfReservedSpace -= i;
			e_ReservedSpace->SetFixedSize(ws.iSizeOfReservedSpace);
			e_ReservedSpace->Write();
			GetDest()->Seek(q);
		}

		/* if Cue size is set to auto, try to fill the space reserved before
		   the first cluster with cues. If it is not set to auto, use all Cue
		   points and store them whereever possible */
		if (ws.iCueAutosize) {	
			int available_size = (int)((double)ws.iSizeOfReservedSpace  * ws.iCueTargetSizeRatio);

			/* randomly select as many cue points as probably necessary to
			   create Cues which use the reserved space without being too large */
			int retries = 0;
			do {
				if (e_Cues) {
					e_Cues->Delete();
					delete e_Cues;
				}
				e_Cues = new EBMLMCue_Writer(GetDest());

				ws.iCuePointsWritten = 0;

				int total_size = 0;
				for (int i=(int)ws.cues.size()-1;i>=0;i--) 
					total_size += ws.cues[i].GetSize();

				float point_ratio = (float)available_size / (float)total_size;

				/* if cue points don't fit, drop enough of them to fill the space
				   before the 1st cluster, otherwise use all of them */
				if (total_size > available_size && retries < 9/*&& minmax_ratio < point_ratio*/) {
			
					std::vector<CUE_POINT>::iterator cue_point = ws.cues.begin();

					for (; cue_point != ws.cues.end(); cue_point++) {
						int random = rand();
						int accept_max = (int)((float)RAND_MAX * point_ratio);
						__int64 nanosec = cue_point->qwTimecode * GetTimecodeScale();

						// cue points in the first 5 seconds stay all
						if (random < accept_max || nanosec < 5000000000)
							ws.iCuePointsWritten += e_Cues->AddCuePoint(*cue_point);
					}

					ws.iCueTargetSizeRatio -= 0.001f; // reduce ratio for next try

				} else {
					std::vector<CUE_POINT>::iterator cue_point = ws.cues.begin();
					for (;cue_point != ws.cues.end(); cue_point++)
						ws.iCuePointsWritten += e_Cues->AddCuePoint(*cue_point);
				}
			} while (e_Cues->GetSize() >= available_size && retries++ < 10);
		} else {
			std::vector<CUE_POINT>::iterator cue_point = ws.cues.begin();
			for (;cue_point != ws.cues.end(); cue_point++)
				ws.iCuePointsWritten += e_Cues->AddCuePoint(*cue_point);
		}
				
		if (e_Cues && e_Cues->GetSize()>8) {
			e_Cues->EnableCRC32();
			if (ws.iSizeOfReservedSpace > e_Cues->GetSize()) {
				__int64 q = GetDest()->GetPos();
				GetDest()->Seek(ws.iPosOfReservedSpace);
				e_TargetSeekhead->AddEntry((char*)MID_CUES,ws.iPosOfReservedSpace - ws.iSegClientDataBegin,1);
				i=(int)e_Cues->Write();
				ws.iPosOfReservedSpace += i;
				ws.iSizeOfReservedSpace -= i;
				e_ReservedSpace->SetFixedSize(ws.iSizeOfReservedSpace);
				e_ReservedSpace->Write();
				GetDest()->Seek(q);
			} else {
				e_TargetSeekhead->AddEntry((char*)MID_CUES,ws.iPosInSegment,1);
				ws.iPosInSegment += (i=(int)e_Cues->Write());
				ws.iAccumulatedOverhead += i;
			}
			DeleteEBML(&e_Cues);
		}

		// write secondary seekhead, if there is one
		if (e_SecondarySeekhead) {
			e_PrimarySeekhead->AddEntry((char*)MID_SEEKHEAD,ws.iPosInSegment,1);
			e_SecondarySeekhead->EnableCRC32();
			ws.iPosInSegment += (i=(int)e_SecondarySeekhead->Write());
			ws.iAccumulatedOverhead += i;
			ws.iSeekheadSize += i;
		} 

		// set segment info
		for (int uidtype=0;uidtype<4;uidtype++) 
			if (ws.bUIDs[uidtype])
				e_SegInfo->SetUID(uidtype, ws.cUIDs[uidtype]);

		e_SegInfo->SetDuration(ws.fSetDuration);
		e_SegInfo->SetTimecodeScale(GetTimecodeScale());
		e_SegInfo->Build();
		e_SegInfo->EnableCRC32();

//		e_SegInfo->SetFixedSize(2048);
		GetDest()->Seek(ws.iSegInfoPosInFile);
		iTrash = (int)e_SegInfo->Write();

		GetDest()->Seek(ws.iTracksPosInFile);
		e_Tracks->Write();

		// write primary seekhead
		GetDest()->Seek(ws.iSeekheadPosInFile);
		e_PrimarySeekhead->EnableCRC32();
		ws.iSeekheadSize += e_PrimarySeekhead->Write();
		
		GetDest()->Seek(ws.iSegPosInFile+4);
		char t[8];
		Int2VSUInt(&ws.iPosInSegment,t,8);
		GetDest()->Write(t,8);

		DeleteEBML(&e_PrimarySeekhead);
		if (e_SecondarySeekhead) DeleteEBML(&e_SecondarySeekhead);
		DeleteEBML(&e_Segment);
		DeleteEBML(&e_SegInfo);
		DeleteEBML(&e_Tracks);
		DeleteEBML(&e_Tags);

		if (write_buffer.block) 
			delete write_buffer.block;
		if (write_buffer.used)
			delete write_buffer.used;
		if (write_buffer.block_of_stream)
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
/*	if (info->mode == MMODE_READ) {
		return (__int64)pActiveSegInfo->fDuration;
	} else {
		return (__int64)ws.fSetDuration;
	}
*/
	if (CanRead() && !CanWrite())
		return (__int64)pActiveSegInfo->fDuration;

	if (!CanRead() && CanWrite())
		return (__int64)ws.fSetDuration;

	return 0;
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
	if (CanWrite()) {
		if (tracks[iTrack]->cCodecPrivate)
			return tracks[iTrack]->cCodecPrivate->GetSize();
		else
			return 0;
	} else {
		CBuffer* cB = pActiveSegInfo->tracks->track_info[MapT(iTrack)]->cCodecPrivate;
		if (cB) return cB->GetSize();
	}
	return 0;
}


int MATROSKA::GetTrackType(int iTrack)
{
	iTrack = MapT(iTrack);
	if (iTrack >= GetTrackCount())
		return 0;

	return pActiveSegInfo->tracks->track_info[MapT(iTrack)]->iType;
}

int MATROSKA::GetTrackCompressionDescriptorCount(int iTrack)
{
	iTrack = MapT(iTrack);
	if (iTrack >= GetTrackCount())
		return 0;

	if (CanRead())
		return (int)pActiveSegInfo->tracks->track_info[iTrack]->track_compression.size();
	if (CanWrite())
		return (int)tracks[iTrack]->track_compression.size();

	return -1;
}

int MATROSKA::GetTrackCompressionPrivateSize(int iTrack, int index)
{
	iTrack = MapT(iTrack);
	if (iTrack >= GetTrackCount())
		return 0;
	if (index < 0 || index >= GetTrackCompressionDescriptorCount(iTrack))
		return 0;

	if (CanRead())
		return pActiveSegInfo->tracks->track_info[iTrack]->track_compression[index].compression_private_size;
	if (CanWrite())
		return tracks[iTrack]->track_compression[index].compression_private_size;

	return 0;
}

int MATROSKA::GetTrackCompression(int iTrack, int index)
{
	iTrack = MapT(iTrack);
	if (iTrack >= GetTrackCount())
		return 0;
	int count = GetTrackCompressionDescriptorCount(iTrack);
	
	if (!count)
		return COMPRESSION_NONE;

	if (index < 0 || index >= count)
		return COMPRESSION_ERROR;

	if (CanRead())
		return pActiveSegInfo->tracks->track_info[iTrack]->track_compression[index].compression;
	if (CanWrite())
		return tracks[iTrack]->track_compression[index].compression;

	return 0;
}

int MATROSKA::GetTrackCompression(int iTrack, TRACK_COMPRESSION &target)
{
	iTrack = MapT(iTrack);
	if (iTrack >= GetTrackCount())
		return 0;

	if (CanRead())
		target = pActiveSegInfo->tracks->track_info[iTrack]->track_compression;
	if (CanWrite())
		target = tracks[iTrack]->track_compression;

	return 1;
}

int MATROSKA::GetTrackCompressionPrivate(int iTrack, int index, void* pDest)
{
	iTrack = MapT(iTrack);
	if (iTrack >= GetTrackCount())
		return 0;
	if (index < 0 || index >= GetTrackCompressionDescriptorCount(iTrack))
		return 0;

	int size = GetTrackCompressionPrivateSize(iTrack, index);

	if (CanRead())
		memcpy(pDest, pActiveSegInfo->tracks->track_info[iTrack]->track_compression[index].compression_private,
			size);
	if (CanWrite())
		memcpy(pDest, tracks[iTrack]->track_compression[index].compression_private,
			size);

	return size;
}

void* MATROSKA::GetTrackCompressionPrivate(int iTrack, int index)
{
	iTrack = MapT(iTrack);
	if (iTrack >= GetTrackCount())
		return 0;
	if (index < 0 || index >= GetTrackCompressionDescriptorCount(iTrack))
		return 0;

	return pActiveSegInfo->tracks->track_info[MapT(iTrack)]->track_compression[index].compression_private;
}

int MATROSKA::GetTrackCompressionPrivate(int iTrack, int index, void** pDest)
{
	iTrack = MapT(iTrack);
	if (iTrack >= GetTrackCount())
		return 0;
	if (index < 0 || index >= GetTrackCompressionDescriptorCount(index))
		return 0;

	if (!GetTrackCompressionPrivateSize(iTrack, index)) {
		*pDest = 0;
		return 0;
	}

	*pDest = malloc(GetTrackCompressionPrivateSize(iTrack, index));

	return GetTrackCompressionPrivate(iTrack, index, *pDest);
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
	return GetUID(UIDTYPE_SEGMENTUID);
}

char* MATROSKA::GetUID(int uid_type)
{
	SEGID*	id = NULL;

	switch (uid_type) {
		case UIDTYPE_NEXTUID: id = &pActiveSegInfo->NextSeg; break;
		case UIDTYPE_PREVUID: id = &pActiveSegInfo->PrevSeg; break;
		case UIDTYPE_SEGMENTUID: id = &pActiveSegInfo->CurrSeg; break;
		case UIDTYPE_SEGMENTFAMILY: id = &pActiveSegInfo->SegmentFamily; break;
		default: return NULL; break;
	}
	
	if (id->bValid)
		return id->cUID;

	return NULL;
}

TAG_INDEX_LIST& MATROSKA::GetGlobalTags()
{
	return pActiveSegInfo->pGlobalTags;
}

int MATROSKA::GetTag(uint iIndex, char* cTagName, char* cTagValue, char* cTagLng)
{
	if (iIndex >= (uint)pActiveSegInfo->pAllTags->iCount) return 0;

	if (cTagName) strcpy(cTagName, pActiveSegInfo->pAllTags->pTags[iIndex]->cName);
	if (cTagValue) strcpy(cTagValue, pActiveSegInfo->pAllTags->pTags[iIndex]->cData);
	if (cTagLng) strcpy(cTagLng, pActiveSegInfo->pAllTags->pTags[iIndex]->cLanguage);

	return 1;
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

int MATROSKA::IsForced(int iIndex)
{
	return pActiveSegInfo->tracks->track_info[MapT(iIndex)]->iForced;
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

__int64 MATROSKA::GetTrackUID(int iIndex)
{
	if (info->mode == MMODE_READ) {
		return pActiveSegInfo->tracks->track_info[MapT(iIndex)]->iUID;
	} else {
		return tracks[MapT(iIndex)]->iTrackUID;
	}
}


int MATROSKA::GetSegmentCount()
{
	return (int)e_Segments->iCount;
}

int MATROSKA::GetTrackCount(int segment, int iType)
{
	if (segment != -1)
		SetActiveSegment(segment);

	if (info->mode == MMODE_READ) {
		if (iType == GTC_ALL) {
			return (int)pActiveSegInfo->tracks->iCount;
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

TAG_INDEX_LIST& MATROSKA::GetTrackTags(int iTrack)
{
	iTrack = MapT(iTrack);

	return pActiveSegInfo->tracks->track_info[iTrack]->pTags;
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

int MATROSKA::GetCropping(RECT* cropping)
{
	TRACK_INFO*	t = pActiveTrackInfo;

	if (cropping) memcpy(cropping, &t->video.rPixelCrop, sizeof(RECT));
	
	return 0;
}

float MATROSKA::GetSamplingFrequency(int iTrack)
{
	return  (float)pActiveSegInfo->tracks->track_info[MapT(iTrack)]->audio.fSamplingFrequency;
}

float MATROSKA::GetOutputSamplingFrequency(int iTrack)
{
	return  (float)pActiveSegInfo->tracks->track_info[MapT(iTrack)]->audio.fOutputSamplingFrequency;
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
	return (float)pActiveSegInfo->tracks->track_info[MapT(iTrack)]->video.fGamma;
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


			//////////////////////
			// Matroska Writing //
			//////////////////////

void MATROSKA::SetMinimumCueInterval(__int64 interval_ns)
{
	ws.min_cue_point_interval = interval_ns;
}

void MATROSKA::SetCueTargetSizeRatio(double ratio)
{
	if (ratio > 0.999)
		ratio = 0.999;

	if (ratio < 0.7)
		ratio = 0.7;

	ws.iCueTargetSizeRatio = ratio;
}

double MATROSKA::GetCueTargetSizeRatio(void)
{
	return ws.iCueTargetSizeRatio;
}

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
	for (int j=0;j<=iCount;ws.iLastCuePointTimecode[j++]=-ws.min_cue_point_interval-1);
	
	j = write_buffer.iCount = iBufsPerStream * iCount;
	write_buffer.block = new ADDBLOCK[j];
	ZeroMemory(write_buffer.block, sizeof(ADDBLOCK)*j);
	write_buffer.used = new int[j];
	ZeroMemory(write_buffer.used, sizeof(int)*j);
	
	
	write_buffer.block_of_stream = new int[iCount];
	ZeroMemory(write_buffer.block_of_stream, sizeof(int)*iCount);
}

void MATROSKA::EnableCueAutosize(int enabled)
{
	ws.iCueAutosize = enabled;
}

bool MATROSKA::IsCueAutoSizeEnabled(void)
{
	return !!ws.iCueAutosize;
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

void MATROSKA::SetCropping(int iTrack, RECT* r)
{
	TRACK_DESCRIPTOR* t = tracks[iTrack];

	if (r) memcpy(&t->video.rCrop, r, sizeof(RECT));
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

void MATROSKA::SetTrackInfo(int iTrack, MATROSKA *m, int iSourceTrack)
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


//		t->compression = m->GetTrackCompression(iSourceTrack);
//		t->compression_private_size = m->GetTrackCompressionPrivate(iSourceTrack, &t->compression_private);
		m->GetTrackCompression(iSourceTrack, t->track_compression);

		SetCodecID(iTrack,m->GetCodecID());
		SetCodecPrivate(iTrack,m->GetCodecPrivate(),m->GetCodecPrivateSize());
		SetTrackName(iTrack,m->GetTrackName());
		SetTrackLanguageCode(iTrack,m->GetLanguage());
	}
}

int MATROSKA::GetTrackCompressionOrder(int iTrack, std::vector<int>& order)
{
	order.clear();
	iTrack = MapT(iTrack);
	if (iTrack >= GetTrackCount())
		return 0;

	int last = GetTrackCompressionDescriptorCount(iTrack) - 1;

	if (CanRead())
		for (int i=0; i<last; i++)
			order.push_back(pActiveSegInfo->tracks->track_info[iTrack]->track_compression[i].order);

	if (CanWrite())
		for (int i=0; i<last; i++)
			order.push_back(tracks[iTrack]->track_compression[i].order);
	
	return 1;
}

void MATROSKA::AddTrackCompression(int iTrack, int compression, 
								   void* compression_private,
								   size_t compression_private_size)
{
	if (iTrack < 0 || iTrack >= GetTrackCount())
		return;

	if (compression_private_size && !compression_private)
		return;
		
	TRACK_COMPRESSION_DESCRIPTOR tcd;
	tcd.compression_private_size = compression_private_size;
	tcd.compressed_elements = TRACKCOMPRESSION_BLOCKS;
	if (compression_private_size) {
		tcd.compression_private = malloc(tcd.compression_private_size);
		memcpy(tcd.compression_private, compression_private, tcd.compression_private_size);
	}
	tcd.compression = compression;
	tcd.is_decompressed = false;

	if (!GetTrackCompressionDescriptorCount(iTrack))
		tcd.order = 0;
	else {
		std::vector<int> order;
		GetTrackCompressionOrder(iTrack, order);
		int m = -1;
		std::vector<int>::iterator iter = order.begin();
		for (; iter != order.end(); iter++)
			m = max(m, *iter);

		tcd.order = 1 + m;
	}
		
	tracks[iTrack]->track_compression.push_back(tcd);
}
/*
void MATROSKA::SetTrackCompression(int iTrack, int iCompression, void* compression_private, int compression_private_size)
{
	tracks[iTrack]->compression = iCompression;
	if (!compression_private_size) {_
		if (tracks[iTrack]->compression_private)
			free (tracks[iTrack]->compression_private);
		tracks[iTrack]->compression_private_size = 0;
		return;
	}

	tracks[iTrack]->compression_private = malloc(compression_private_size);
	tracks[iTrack]->compression_private_size = compression_private_size;
	memcpy(tracks[iTrack]->compression_private, compression_private, compression_private_size);

}
*/
void MATROSKA::SetMatroskaVersion(int version)
{
	if (version > 0 && version < 3)
		ws.matroska_version = version;
}

int MATROSKA::GetMatroskaVersion()
{
	if (CanWrite() && !CanRead())
		return (int)ws.matroska_version;

	if (CanRead() && !CanWrite())
		return pActiveSegInfo->ebml_info.DocTypeVersion;

	return 0;
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

	if (a->iTimecode + a->iDuration > ws.iLatestTimecode) 
		ws.iLatestTimecode = a->iTimecode + a->iDuration;
	
	if (a->iTimecode < ws.iEarliestTimecode)
		ws.iEarliestTimecode = a->iTimecode;

//	ASSERT(iTimecode != 7808);

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

	/* if frame is keyframe, add a CuePoint if Cues are enabled for that stream type */
	if (!a->iRefCount) {
		if (tracks[iTrackIndex]->iTrackType == MSTRT_VIDEO && IsCuesEnabled(CUE_VIDEO) ||
			tracks[iTrackIndex]->iTrackType == MSTRT_AUDIO && IsCuesEnabled(CUE_AUDIO) ||
			tracks[iTrackIndex]->iTrackType == MSTRT_SUBT && IsCuesEnabled(CUE_SUBS)) {
			AddCuepoint(a->iStream, ws.iPosInSegment,iTimecode, 
				(IsCueBlockNumberEnabled()?ws.iBlocksInCluster:0));
		}
	}

	tracks[iTrackIndex]->iLaceSchemes[block_info.iLaceStyle]++;
	tracks[iTrackIndex]->iBlocksWritten++;
	tracks[iTrackIndex]->iFramesWritten[block_info.iLaceStyle]+=a->iFrameCountInLace;
	tracks[iTrackIndex]->iLaceOverhead[block_info.iLaceStyle]+=block_info.iLaceOverhead;
	tracks[iTrackIndex]->iTotalSize += a->cData->GetSize();
	if (GetTrackCompression(iTrackIndex, 0) == COMPRESSION_HDRSTRIPING)
		tracks[iTrackIndex]->iTotalSize += GetTrackCompressionPrivateSize(iTrackIndex, 0);

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

	return ((iMinIndex>(s+1)*iBufsPerStream)?(-1):iMinIndex);
}

int MATROSKA::FindEarliestBlockIndex()
{
	__int64 iMinTC = 0x7FFFFFFFFFFFFFFF;
	int iMinIndex = 0x7FFFFFFF;
	int bUsed = 0;
	int i;

	for (int j=0;j<GetTrackCount();j++) {
		i = FindEarliestBlockIndexOfStream(j);
		if (i >= 0 && write_buffer.used[i]) {
			bUsed = 1;
			if (iMinTC > write_buffer.block[i].iTimecode) {
				iMinTC = write_buffer.block[i].iTimecode;
				iMinIndex = i;
			}
		}
	}
	
	if (!bUsed)
		return -1;
	
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
			delete[] write_buffer.block[iMinIndex].iFrameSizes;
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
	__int64 iTimecode_unscaled = iTimecode * GetTimecodeScale();

	if (iTimecode_unscaled - ws.iLastCuePointTimecode[iTrack] > ws.min_cue_point_interval) {

		if (ws.cues.size() && ws.cues[ws.cues.size()-1].qwTimecode == iTimecode) {
		}
		else {
			
			int s = ws.cues.size()?ws.cues[ws.cues.size()-1].GetSize():0;
			CUE_POINT p;
			ws.cues.push_back(p);
			ws.iCuePointsCreated++;
		}
		
				
		CUE_POINT&e = ws.cues[ws.cues.size()-1];
		e.qwTimecode = iTimecode;
		
		CUE_TRACKINFO ct;
		ct.iBlockNbr = (int)iBlock;
		ct.iRefCount = 0;
		ct.iTrack    = iTrack;
		ct.pReferences = NULL;
		ct.qwClusterPos = iClusterPosition;
		ct.qwCodecStatePos = 0;

		e.tracks.push_back(ct);
		ws.iLastCuePointTimecode[iTrack] = iTimecode_unscaled;

/*		e_Cues->AddCuePoint(iTrack, iTimecode/GetTimecodeScale(), iClusterPosition, iBlock);
		
		return 1;
*/	}

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
		iCount += max(1,(int)r->frame_sizes.size());
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

int MATROSKA::GetCueCount(int flag)
{
	if (flag == 1)
		return (int)ws.iCuePointsCreated;
	if (flag == 2)
		return (int)ws.iCuePointsWritten;
	return 0;
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

void MATROSKA::SetNonclusterIndexMode(int mode)
{
	ws.iTargetSeekhead = mode;
}

/* attachment functions */
int MATROSKA::GetAttachmentIndex(__int64 uid)
{
	for (int i=0;i<pActiveSegInfo->attachments.count;i++) {
		ATTACHMENT& att = pActiveSegInfo->attachments.attachments[i];

		if (att.file_uid == uid)
			return i;
	}

	return ATTACHMENTINDEX_INVALID;
}

int save_copy(char* source, char* dest, int max_len)
{
	int source_len = (int)strlen(source);

	int bytes_to_copy = min (max_len - 1, source_len + 1);

	if (dest) 
		strncpy(dest, source, bytes_to_copy);
	else
		return source_len + 1;

	return bytes_to_copy;
}

int save_copy(CStringBuffer* source, char* dest, int max_len)
{
	if (source)
		return save_copy(source->AsString(), dest, max_len);

	if (dest) 
		dest[0] = 0;
	
	return 1;
}

int MATROSKA::GetAttachmentMimeType(int index, char* pDest, int max_len)
{
	if (index < 0 || index > pActiveSegInfo->attachments.count)
		return ATTACHMENTINDEX_INVALID;

	ATTACHMENT& att = pActiveSegInfo->attachments.attachments[index];

	return save_copy(att.file_mime_type, pDest, max_len);
}

int MATROSKA::GetAttachmentFilename(int index, char* pDest, int max_len)
{
	if (index < 0 || index > pActiveSegInfo->attachments.count)
		return ATTACHMENTINDEX_INVALID;

	ATTACHMENT& att = pActiveSegInfo->attachments.attachments[index];

	return save_copy(att.file_name, pDest, max_len);
}

int MATROSKA::GetAttachmentFileDescription(int index, char* pDest, int max_len)
{
	if (index < 0 || index > pActiveSegInfo->attachments.count)
		return ATTACHMENTINDEX_INVALID;

	ATTACHMENT& att = pActiveSegInfo->attachments.attachments[index];

	return save_copy(att.file_description, pDest, max_len);
}

__int64 MATROSKA::GetAttachmentUID(int index)
{
	if (index < 0 || index > pActiveSegInfo->attachments.count)
		return ATTACHMENTINDEX_INVALID;

	ATTACHMENT& att = pActiveSegInfo->attachments.attachments[index];

	return att.file_uid;
}

__int64 MATROSKA::GetAttachmentFileSize(int index)
{
	if (index < 0 || index > pActiveSegInfo->attachments.count)
		return ATTACHMENTINDEX_INVALID;

	ATTACHMENT& att = pActiveSegInfo->attachments.attachments[index];

	return att.file_size;
}

__int64 MATROSKA::GetAttachmentFilePosition(int index)
{
	if (index < 0 || index > pActiveSegInfo->attachments.count)
		return ATTACHMENTINDEX_INVALID;

	ATTACHMENT& att = pActiveSegInfo->attachments.attachments[index];

	return att.file_pos;
}

__int64 MATROSKA::GetAttachmentFlags(int index)
{
	if (index < 0 || index > pActiveSegInfo->attachments.count)
		return ATTACHMENTINDEX_INVALID;

	ATTACHMENT& att = pActiveSegInfo->attachments.attachments[index];

	return att.flags;
}


int MATROSKA::GetAttachmentBinaryData(int index, char* pDest, int max_len)
{
	if (index < 0 || index > pActiveSegInfo->attachments.count)
		return ATTACHMENTINDEX_INVALID;

	ATTACHMENT& att = pActiveSegInfo->attachments.attachments[index];

	__int64 bytes_to_read = 0;
	bytes_to_read = min((_int64)max_len, GetAttachmentFileSize(index));

	if (bytes_to_read > INT_MAX)
		return 0;

	__int64 old_pos = GetSource()->GetPos();
	GetSource()->Seek(GetAttachmentFilePosition(index));
	int r = GetSource()->Read(pDest, (int)bytes_to_read);
	GetSource()->Seek(old_pos);

	return r;
}

int MATROSKA::GetAttachmentCount()
{
	return pActiveSegInfo->attachments.count;
}

bool MATROSKA::VerifyCuePoint(int index) {
	EBMLM_Segment*		seg = (EBMLM_Segment*)e_Segments->pElement[GetActiveSegment()];

	return seg->VerifyCuePoint(index);
}