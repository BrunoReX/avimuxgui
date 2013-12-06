/*

Class to handle one segment of a matroska file
----------------------------------------------

Important: 
Malicious recursive linking of Seekheads can be used
to make this code hang in an infinite loop. It is 
assumed that no one on earth is silly enough to write
such files



*/




#include "stdafx.h"
#include "stdio.h"
#include "Matroska_Clusters.h"
#include "Matroska_Segment.h"
#include "Matroska_IDs.h"
#include "Warnings.h"
#include "crc.h"
#include "AVIMux_GUI/ecc.h"
#include "UID.h"
#include <algorithm>

#ifdef DEBUG_NEW
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

	//////////////////////
	// Matroska-Segment //
	//////////////////////

const int ISTATE_JUMPSEARCH	= 0x01;
const int ISTATE_LINEARSEARCH = 0x02;



//TRACK_COMPRESSION_DESCRIPTOR::TRACK_COMPRESSION_DESCRIPTOR(TRACK_COMPRESSION_DESCRIPTOR& other)

TRACK_INFO::TRACK_INFO()
{
	// information embedded in file
	iForced = 0;
	iUID = 0;
	qwDefaultDuration = 0;
	iTrackOffset = 0;

//	cName = NULL;
	cCodecID = NULL;
	cCodecName = NULL;
	cCodecPrivate = NULL;
	memset(&tags, 0, sizeof(tags));
	// additional stuff
//	iCompression = 0;
//	iCompressedElements = 0;
//	is_decompressed = 0;

	iLacing = 1;
	cue_points = new TRACK_CUE_POINTS;
	ZeroMemory(cue_points, sizeof(TRACK_CUE_POINTS));

	iDefault = 1;
	iEnabled = 1;
	iDataQueued = 0;
	iType = -1;
	fTimecodeScale = 1;
	iMinCache = 0;
	iMaxCache = 0;
	iNbr = -1;
	iSelected = 1;
	ZeroMemory(&video, sizeof(video));
	video.iDisplayWidth = -1;
	video.iDisplayHeight = -1;
	video.iPixelWidth = -1;
	video.iPixelHeight = -1;
	ZeroMemory(&audio, sizeof(audio));
	audio.iChannels = 1;
	audio.fSamplingFrequency = 8000;
	audio.fOutputSamplingFrequency = -1;
//	compression_private_size = 0;
//	compression_private = NULL;
	
	cLanguage = new CStringBuffer("eng");
	iSparse = 0x00000000;
	iUID = 0;
	iLastBlockEndTimecode = TIMECODE_UNINITIALIZED;

} TRACK_INFO;

TRACK_INFO::~TRACK_INFO()
{
	DecBufferRefCount(&cLanguage);
//	DecBufferRefCount(&cName);
	DecBufferRefCount(&cCodecID);
	DecBufferRefCount(&cCodecPrivate);
	DecBufferRefCount(&cCodecName);
/*	for (size_t j=cue_points->iCount-1; j>=0; j--)
		delete cue_points->point[j];

	delete cue_points->point;
*/	delete cue_points;

	size_t count = track_compression.size();
	if (count) for (size_t i=0;i<count-1;i++) {
		TRACK_COMPRESSION_DESCRIPTOR &tci = track_compression[i];
		if (tci.compression_private_size)
			free(tci.compression_private);
		tci.compression_private = NULL;
	}

//	if (compression_private_size)
//		free(compression_private);
}

CUE_TRACKINFO::CUE_TRACKINFO()
{
	iBlockNbr = 0;
	iRefCount = 0;
	qwClusterPos = 0;
	qwCodecStatePos = 0;
	size = 0;
	
	pReferences = NULL;
}


int CUE_TRACKINFO::GetSize()
{
	if (size)
		return size;

	if (iBlockNbr)
		size += 3 + UIntLen(iBlockNbr);

	size += 2 + UIntLen(qwClusterPos);
	size += 2 + UIntLen(iTrack);

	size += 1 + EBMLUIntLen(size);

	return size;
}

CUE_POINT::CUE_POINT()
{
	qwTimecode = 0;
	size = 0;
}

int CUE_POINT::GetSize()
{
	if (size)
		return size;

	for (size_t i=0; i<tracks.size(); i++)
		size += tracks[i].GetSize();

	size += 2 + UIntLen(qwTimecode);
	size += 1 + EBMLUIntLen(size);

	return size;
	
}

SEGID::SEGID()
{
	memset(cUID, 0, sizeof(cUID));
	cFilename = NULL;
	bValid = false;
}

SEGMENT_INFO::SEGMENT_INFO()
{
	fDuration = 0.f;
	qwBias = 0;					
	iFirstClusterTimecode = 0;	// timecode of first cluster
	iEndReached = 0;			// end-of-segment reached
	iActiveClusterIndex = 0;
	iTimecodeScale = 1000000;			// timecode scale of segment
	iMasterStream = -1;
	iSeekMode = SEEKMODE_IDIOT;

	iBlockCount = NULL;		// number of blocks of Track [i]
	iTotalBlockCount = NULL;	// total number of blocks
	iOtherBlocksThan = NULL;	// blocks of tracks[j!=i] read since the last block of track i

//	cTitle = NULL;
	cMuxingApp = NULL;
	cWritingApp = NULL;
	
	tracks = NULL;
	clusters = NULL;
	pActiveCluster = NULL;
	pGlobElem = NULL;

	chapters = NULL;
	pChapters = NULL;
	cues = NULL;
	pAllTags = NULL;

	attachments.count = 0;
	attachments.uid_recreated = 0;
	
	queue = NULL;
}

GLOBALELEMENTS::GLOBALELEMENTS()
{
	e_SegmentInfo = NULL;
	e_Cues = NULL;
	e_CuePoints = NULL;

	e_Tracks = NULL;
	e_Tracks2 = NULL;
}

GLOBALELEMENTS::~GLOBALELEMENTS()
{
	DeleteElementList(&e_Cues);
	DeleteElementList(&e_Tracks);
//	DeleteElementList(&e_Tracks2);
	DeleteElementList(&e_SegmentInfo);
	DeleteElementList(&e_CuePoints);
	DeleteVector(e_Chapters);
	DeleteVector(e_Seekhead);
	DeleteVector(e_Tags);
	DeleteVector(e_Attachments);
}

ATTACHMENT::ATTACHMENT()
{
	file_description = NULL;
	file_name = NULL;
	file_uid = 0;
	file_mime_type = 0;
	file_pos = -1;
	flags = 0;
}

ATTACHMENT::ATTACHMENT(const ATTACHMENT &copy)
{
	file_uid = copy.file_uid;
	file_size = copy.file_size;
	file_pos = copy.file_pos;
	
	
	file_name = copy.file_name;
	if (file_name)
		file_name->IncRefCount();
	
	file_description = copy.file_description;
	if (file_description)
		file_description->IncRefCount();

	file_mime_type = copy.file_mime_type;
	if (file_mime_type)
		file_mime_type->IncRefCount();
}

ATTACHMENT::~ATTACHMENT()
{
	DecBufferRefCount(&file_description);
	DecBufferRefCount(&file_name);
	DecBufferRefCount(&file_mime_type);
}

bool ATTACHMENT::operator ==(ATTACHMENT& a)
{
	if (file_uid != a.file_uid) return false;

	return true;
}

bool ATTACHMENT::operator <(ATTACHMENT& a)
{
	return (file_uid < a.file_uid);
}

EBMLM_Segment::EBMLM_Segment(STREAM* s,EBMLElement *p)
{
	SetStream(s); 
	SetParent(p); 
	DetermineLength(); 
	SetType(IDVALUE(MID_SEGMENT)); 
	SetMulti(true); 
	SetDataType(EBMLDATATYPE_MASTER); 
	SegmentInfo = new SEGMENT_INFO;

	ZeroMemory(&InfoPrivate, sizeof(InfoPrivate));
	SegmentInfo->pGlobElem = new GLOBALELEMENTS;
	return; 
}

EBMLM_Segment::~EBMLM_Segment()
{
//	delete SegmentInfo->pGlobElem;
//	delete SegmentInfo;
}

bool EBMLM_Segment::CheckIDs(char* iID,EBMLElement** p)
{
	DOCOMP(MID_SEEKHEAD,EBMLM_Seekhead)
	DOCOMP(MID_SEGMENTINFO,EBMLM_SIInfo)
	DOCOMP(MID_CLUSTER,EBMLM_Cluster)
	DOCOMP(MID_TRACKS,EBMLM_Tracks)
	DOCOMP(MID_CUES,EBMLM_Cues)
	DOCOMP(MID_ATTACHMENTS,EBMLM_Attachments)
	DOCOMP(MID_TAGS,EBMLM_Tags)
	DOCOMPL(MID_CHAPTERS,EBMLM_Chapters)
}

int EBMLM_Segment::CheckCRC()
{
	return EBML_CRC_NOT_CHECKED;
}

char* EBMLM_Segment::GetTypeString()
{
	return "Segment";
}

void EBMLM_Segment::AllowObfuscatedSegments(bool bAllow)
{
	bAllowObfuscatedSegments = bAllow;
}

EBMLM_Cluster* EBMLM_Segment::SetActiveCluster(EBMLM_Cluster* pCluster)
{
	if (SegmentInfo->pActiveCluster) {
		DeleteEBML(&SegmentInfo->pActiveCluster);
		return SegmentInfo->pActiveCluster=pCluster;
	} else {
		if (pCluster)
			SegmentInfo->iFirstClusterTimecode = pCluster->GetTimecode();
		
		return SegmentInfo->pActiveCluster=pCluster;
	}
}

EBMLM_Cluster* EBMLM_Segment::GetActiveCluster()
{
	if (SegmentInfo->pActiveCluster) {
		return SegmentInfo->pActiveCluster;
	} else {
		EBMLELEMENTLIST* clusters = NULL;
		Search((void**)&clusters,(char*)MID_CLUSTER,(char*)MID_CLUSTER);
		if (!clusters->iCount) {
			B0rked("no clusters found in segment"); return NULL;
		} 
		EBMLM_Cluster* cluster = (EBMLM_Cluster*)clusters->pElement[0];
		SegmentInfo->pActiveCluster = cluster; 
		delete clusters->pElement;
		delete clusters;
		return cluster;
	}
}

int EBMLM_Segment::GetActiveClusterIndex()
{
	return SegmentInfo->iActiveClusterIndex;
}

void EBMLM_Segment::RetrieveSubChapters(EBMLElement* pMaster,CHAPTER_INFO** pInfo)
{
	CHAPTERS*			subchaps;
	EBMLElement*		e_next=NULL, *e_old = NULL;

	*pInfo = new CHAPTER_INFO;
	(*pInfo)->bEnabled = 1;
	(*pInfo)->bHidden = 0;
	(*pInfo)->iTimeend = -1;
	(*pInfo)->iTimestart = 0;

	EBMLElementVectors search;
	char* search_ids[] = { 
		(char*)MID_CH_CHAPTERATOM, (char*)MID_CH_CHAPTERUID, (char*)MID_CH_CHAPTERTIMESTART,
		(char*)MID_CH_CHAPTERTIMEEND, (char*)MID_CH_CHAPTERFLAGENABLED, (char*)MID_CH_CHAPTERFLAGHIDDEN,
		(char*)MID_CH_CHAPTERPHYSICALEQUIV, (char*)MID_CH_CHAPTERSEGMENTUID, (char*)MID_CH_CHAPTERTRACK,
		(char*)MID_CH_CHAPTERDISPLAY, NULL,
	};
	void* dest_ptrs[] = { NULL, &(*pInfo)->iUID, &(*pInfo)->iTimestart, 
		&(*pInfo)->iTimeend, &(*pInfo)->bEnabled, &(*pInfo)->bHidden,
		&(*pInfo)->iPhysicalEquiv, NULL, NULL, NULL, NULL };
	int occurences[] = { 0, 3, 3, 
		2, 2, 2,
		2, 2, 2,
		0, 0 };
	SEARCHMULTIEX sme = { search_ids, dest_ptrs, occurences };
	pMaster->SearchMulti(search, sme);

	if ((*pInfo)->iTimeend != -1) {
		(*pInfo)->iTimeend = search[3][0]->AsInt();
	}

	EBMLElementVector& chapter_atoms = search[0];
	EBMLElementVector& chapter_segment_uid = search[7];
	EBMLElementVector& chapter_track = search[8];
	EBMLElementVector& chapter_display= search[9];

	if (!chapter_atoms.empty()) {
		(*pInfo)->subchapters = new CHAPTERS;
		memset((*pInfo)->subchapters, 0, sizeof(CHAPTERS));
		subchaps = (CHAPTERS*)(*pInfo)->subchapters;
		subchaps->iCount=chapter_atoms.size();
		newz(CHAPTER_INFO*,chapter_atoms.size(),subchaps->chapters);

		for (size_t i=0; i<chapter_atoms.size(); i++)
			RetrieveSubChapters(chapter_atoms[i], &subchaps->chapters[i]);

	}

	if (chapter_segment_uid.size()) {
		(*pInfo)->bSegmentUIDValid = 1;
		memcpy((*pInfo)->cSegmentUID, chapter_segment_uid[0]->AsString(), 16);
	};

	if (!chapter_track.empty()) {
		EBMLElementVector tracks;
		for (size_t i=0;i<chapter_track.size();i++)
			chapter_track[i]->Search(tracks, (char*)MID_CH_CHAPTERTRACKNUMBER);

		(*pInfo)->tracks.iCount = tracks.size();

		if (!tracks.size()) {
			B0rked("ChapterTrack does not contain any ChapterTrackNumbers");
		} else {
			(*pInfo)->tracks.iTracks = (__int64*)calloc(tracks.size(), sizeof(__int64));
			for (size_t i=0;i<(*pInfo)->tracks.iCount;i++) {
				(*pInfo)->tracks.iTracks[i] = tracks[i]->AsInt();
			}
		}
		DeleteVector(tracks);
	}


	if (!chapter_display.empty()) {
		for (size_t i=0;i<chapter_display.size();i++) {
			CHAPTER_DISPLAY_INFO cdi;
			
			char* search_items[] = { 
				(char*)MID_CH_CHAPSTRING, (char*)MID_CH_CHAPLANGUAGE, NULL };
			int cd_occ_restr[] = { 2, 2, 0 };
			SEARCHMULTIEX cd_sme = { search_items, NULL, cd_occ_restr };
			EBMLElementVectors search;
			chapter_display[i]->SearchMulti(search, cd_sme);

			if (search[0].size())
				cdi.cString = new CStringBuffer(search[0][0]->AsString(), CSB_UTF8 | CBN_REF1);

			if (search[1].size())
				cdi.cLanguage = new CStringBuffer(search[1][0]->AsString(), CSB_UTF8 | CBN_REF1);
			else 
				cdi.cLanguage = new CStringBuffer("eng", CSB_UTF8 | CBN_REF1);

			(*pInfo)->display.push_back(cdi);

			DeleteVectors(search);
		}
	} else {
		CHAPTER_DISPLAY_INFO cdi;
//		cdi.cLanguage = new CBuffer("");
		cdi.cString = new CStringBuffer("");
		(*pInfo)->display.push_back(cdi);
	}

	if ((*pInfo)->iTimeend<(*pInfo)->iTimestart) {
		if ((*pInfo)->iTimeend != -1) { 
			B0rked("temporal inversion: chapter ends before it starts");
		} else {
			Note("chapter end has not been written");
		}
	}

	DeleteVectors(search);
}

void EBMLM_Segment::RetrieveChapters(EBMLElementVector& e_Chapters)
{
	using namespace std;
	
	EBMLElement*		e_Chap = NULL;
	int					i, iChapCount = 0;
	EBMLElementVector	edition_entries;

	//EBMLElement** iter = e_Chapters.begin();
	vector<EBMLElement*>::iterator iter = e_Chapters.begin();

	for (; iter != e_Chapters.end(); iter++)
		(*iter)->Search(edition_entries, (char*)MID_CH_EDITIONENTRY);

	if (!edition_entries.empty()) {

		SegmentInfo->chapters = (CHAPTERS*)calloc(1,sizeof(CHAPTERS));
		SegmentInfo->chapters->chapters = (CHAPTER_INFO**)calloc(edition_entries.size(),
			sizeof(CHAPTER_INFO*));

		//EBMLElement** edition = edition_entries.begin();
		vector<EBMLElement*>::iterator edition = edition_entries.begin();

		i = 0;
		for (; edition != edition_entries.end(); edition++) {

			EBMLElementVector chapter_atoms;
			(*edition)->Search(chapter_atoms, (char*)MID_CH_CHAPTERATOM);
			
			if (!chapter_atoms.size()) 
				B0rked("EditionEntry without ChapterAtom");

			SegmentInfo->chapters->chapters[i] = new CHAPTER_INFO;
			SegmentInfo->chapters->chapters[i]->subchapters = (CHAPTERS*)calloc(1,sizeof(CHAPTERS));
				
			CHAPTERS* c = (CHAPTERS*)SegmentInfo->chapters->chapters[i]->subchapters;
			c->chapters = (CHAPTER_INFO**)calloc(chapter_atoms.size(), sizeof(CHAPTER_INFO*));
			c->iCount  = chapter_atoms.size();
				
			c =(CHAPTERS*)SegmentInfo->chapters;
			c->chapters[i]->bEnabled = 1;
			c->chapters[i]->iTimeend = -1;

			char* ep_ids [] = {
				(char*)MID_CH_EDITIONFLAGDEFAULT, (char*)MID_CH_EDITIONFLAGHIDDEN,
				(char*)MID_CH_EDITIONFLAGORDERED, (char*)MID_CH_EDITIONUID, NULL
			};
			void* ep_targets [] = {
				&c->chapters[i]->bDefault, &c->chapters[i]->bHidden,
				&c->chapters[i]->bOrdered, &c->chapters[i]->iUID, NULL 
			};
			int ep_occ_restr [] = {
				2, 2, 2, 2 
			};
			SEARCHMULTIEX ep_sme = { ep_ids, ep_targets, ep_occ_restr };

			EBMLElementVectors	search;
			edition_entries[i]->SearchMulti(search, ep_sme);
			DeleteVectors(search);

			SegmentInfo->chapters->chapters[i]->bEdition = 1;
			for (size_t j=0;j<chapter_atoms.size();j++) {
				RetrieveSubChapters(chapter_atoms[j],
					&((CHAPTERS*)SegmentInfo->chapters->chapters[i]->subchapters)->chapters[j]);
			}

			DeleteVector(chapter_atoms);
			i++;
		}

		SegmentInfo->chapters->iCount = edition_entries.size();
	}

	DeleteVector(edition_entries);
}

void EBMLM_Segment::ParseCuePoints(CUES* cues, __int64 start, __int64 end, int mode)
{
	int k;
	__int64 i;
	CUE_POINT*	cue_point = NULL;
	EBMLELEMENTLIST**	e_CuePoints = &SegmentInfo->pGlobElem->e_CuePoints;

	if (cues) {
		if (mode == PARSECUEPOINTS_TIME) {
			if (end == -1) {
				end = cues->iCuePoints-1;
				start = 0;
				mode = PARSECUEPOINTS_INDEX;
			}
		}

		// go through all cue points
		for (i=start;i<=end;i++) {
			if (cues->pCuePoints[i])
				continue;
			cue_point = cues->pCuePoints[i] = new CUE_POINT;
			EBMLElement* e_CuePoint = (**e_CuePoints)(i);//->pElement[i];

			char* cp_search_ids[] = { (char*)MID_CU_CUETIME, (char*)MID_CU_CUETRACKPOSITIONS, NULL };
			void* cp_targets[] = { &cue_point->qwTimecode, NULL, NULL };
			int cp_occ_restr[] = { 3, 1, 0 };
			SEARCHMULTIEX cp_sme = { cp_search_ids, cp_targets, cp_occ_restr };
			EBMLElementVectors cp_search;
			e_CuePoint->SearchMulti(cp_search, cp_sme);
			
			if (InfoPrivate.bShiftFirstClusterToZero)
				cue_point->qwTimecode -= SegmentInfo->iFirstClusterTimecode;
				
			for (size_t j=0;j<cp_search[1].size();j++) {
				EBMLElement* e_track = cp_search[1][j];
				
				CUE_TRACKINFO	track;
				char* ct_ids [] = { (char*)MID_CU_CUETRACK, (char*)MID_CU_CUECLUSTERPOSITION, 
					(char*)MID_CU_CUEBLOCKNUMBER, NULL };
				void* ct_targets [] = { &track.iTrack, &track.qwClusterPos, &track.iBlockNbr, NULL };
				int ct_occ_restr [] = { 3, 3, 2, 0 };
				SEARCHMULTIEX ct_sme = { ct_ids, ct_targets, ct_occ_restr };
				EBMLElementVectors ct_search;
				e_track->SearchMulti(ct_search, ct_sme);
				DeleteVectors(ct_search);
				track.iTrack = TrackNumber2Index(track.iTrack);
				if (track.iTrack == TN2I_INVALID) {
					B0rked("Cue point points into non-existent track", e_CuePoint->GetStreamPos());
					continue;
				}

				SegmentInfo->tracks->track_info[track.iTrack]->cue_points->iCount++;
				EBMLELEMENTLIST* e_refs = NULL;
				e_track->Search((void**)&e_refs,(char*)MID_CU_CUEREFERENCE);
				track.iRefCount = (int)e_refs->iCount;
				if (e_refs->iCount) {
					track.pReferences = (CUE_REFERENCE**)calloc(track.iRefCount,sizeof(CUE_REFERENCE*));
				}
				for (k=0;k<track.iRefCount;k++) {
					CUE_REFERENCE* ref = track.pReferences[k] = (CUE_REFERENCE*)calloc(1,sizeof(CUE_REFERENCE));
					ref->iRefNumber = 1;
					char* cr_ids[] = { (char*)MID_CU_CUEREFCLUSTER, (char*)MID_CU_CUEREFNUMBER,
						(char*)MID_CU_CUEREFTIME, (char*)MID_CU_CUEREFCODECSTATE, NULL };
					void* cr_targ[] = { &ref->qwRefClusterPos, &ref->iRefNumber,
						&ref->qwRefTime, &ref->qwRefCodecStatePos, NULL };
					int cr_occ_restr[] = { 3, 2, 3, 2, 0 };
					SEARCHMULTIEX cr_sme = { cr_ids, cr_targ, cr_occ_restr };
					EBMLElementVectors cr;
					e_refs->pElement[0]->SearchMulti(cr, cr_sme);
					DeleteVectors(cr);
				}
				cue_point->tracks.push_back(track);

				if (e_refs) DeleteElementList(&e_refs);
			}
			DeleteVectors(cp_search);
		}
	}

}


void EBMLM_Segment::RetrieveCues(EBMLELEMENTLIST** e_Cues, __int64 time_start, __int64 time_end)
{
	EBMLElement*		e_Cue = NULL;
	EBMLELEMENTLIST**	e_CuePoints = &SegmentInfo->pGlobElem->e_CuePoints;
	CUES*				cues;

	if (*e_Cues && (*e_Cues)->iCount) {
		// do cues exist?

		SegmentInfo->iSeekMode = SEEKMODE_NORMAL;
		e_Cue = (*e_Cues)->pElement[0];

		if (!SegmentInfo->cues) {
			e_Cue->Search((void**)e_CuePoints,(char*)MID_CU_CUEPOINT,NULL);
			if (!(*e_CuePoints)->iCount) {
				B0rked("Cues don't contain Cuepoints");
			}
		}

		if (!SegmentInfo->cues) 
			cues = SegmentInfo->cues = (CUES*)calloc(1,sizeof(CUES));
		else
			cues = SegmentInfo->cues;

		cues->iCuePoints = (*e_CuePoints)->iCount;

		if (!cues->pCuePoints) {
			cues->pCuePoints = (CUE_POINT**)calloc(cues->iCuePoints,sizeof(CUE_POINT*));
		}

		if (time_end == -1) 
			ParseCuePoints(cues, 0, cues->iCuePoints, PARSECUEPOINTS_INDEX);
		else
			ParseCuePoints(cues, time_start, time_end, PARSECUEPOINTS_TIME);
	} else {

		if (SegmentInfo->clusters->iCount) {
			Warning("No cues present. Seeking in the file might be slow");
		} else {
			Warning("Neither cues nor index to clusters present. Seeking will most likely be slow");
			SegmentInfo->iSeekMode = SEEKMODE_IDIOT;
		}
	}


}

CUE_POINT* EBMLM_Segment::GetCuePoint(int index)
{
	CUES* cues = GetCues(0, 0);

	if (index >= cues->iCuePoints || index < 0)
		return NULL;

	if (!cues->pCuePoints[index])
		ParseCuePoints(cues, index, index, PARSECUEPOINTS_INDEX);

	return cues->pCuePoints[index];
}

int EBMLM_Segment::RetrieveLastCuepoint(int iTrack, int* index2)
{
	int count = GetCues(0,0)->iCuePoints;
	CUE_POINT* cuepoint;
	if (index2) *index2=-1;

	for (int i=count-1;i>=0;i--) {
		cuepoint = GetCuePoint(i);
		for (size_t j=0;j<cuepoint->tracks.size();j++) {
			if (cuepoint->tracks[j].iTrack == iTrack) {
				if (index2) 
					*index2 = (int)j;
				return i;
			}
		}
	}

	return -1;
}

__int64 EBMLM_Segment::RetrieveLastBlockEndTimecode(int iTrack)
{
	TRACK_INFO*      t = SegmentInfo->tracks->track_info[iTrack];
	int index1,index2;
	__int64 iCurrent = 0;

	if (!GetCues(0, 0)) return (t->iLastBlockEndTimecode = TIMECODE_UNKNOWN);
	if (t->iLastBlockEndTimecode == TIMECODE_UNKNOWN) return TIMECODE_UNKNOWN;
	if (t->iLastBlockEndTimecode != TIMECODE_UNINITIALIZED) return t->iLastBlockEndTimecode;

	index1 = RetrieveLastCuepoint(iTrack, &index2);

	if (index1 == -1 || index2 == -1) return (t->iLastBlockEndTimecode = TIMECODE_UNKNOWN);

	CUE_POINT    * cpt = GetCuePoint(index1);
	CUE_TRACKINFO& cti = cpt->tracks[index2];
	
	Seek(cpt->qwTimecode);

	READBLOCK_INFO  rbi;
	while (ReadBlock(&rbi)!=READBL_ENDOFSEGMENT) {
		if (TrackNumber2Index(rbi.iStream) == iTrack) {
			if (rbi.qwDuration) {
				iCurrent = rbi.qwTimecode + rbi.qwDuration;
			} else {
				if (rbi.frame_sizes.size() < 2) {
					iCurrent = rbi.qwTimecode + t->qwDefaultDuration / SegmentInfo->iTimecodeScale;
				} else {
					iCurrent = rbi.qwTimecode + t->qwDefaultDuration * rbi.frame_sizes.size() / SegmentInfo->iTimecodeScale;
				}
			}
			if (t->iLastBlockEndTimecode < iCurrent) t->iLastBlockEndTimecode = iCurrent;
		}
		DecBufferRefCount(&(rbi.cData));
		rbi.qwDuration = 0;
	}

	if (t->iLastBlockEndTimecode == TIMECODE_UNINITIALIZED)
		t->iLastBlockEndTimecode = TIMECODE_UNKNOWN;
	
	return t->iLastBlockEndTimecode;
}

int EBMLM_Segment::TrackNumber2Index(int iNbr)
{
	SEGMENT_INFO* seg = SegmentInfo;
	TRACKS*		  tra = seg->tracks;
	int			  i;

	if (tra->iTable[iNbr-1] == iNbr) return iNbr-1;

	for (i=0;i<(int)tra->iCount;i++) {
		if (tra->iTable[i] == iNbr) return i;
	}

	return TN2I_INVALID;
}

// Takes O(n^3) time for many doubles!!!
void RemoveDoubles(EBMLELEMENTLIST* e, char* cText)
{
	size_t i,j,k;

	bool bRepaired;
	if (e) do {
		bRepaired = false;
		for (i=0;i<e->iCount && !bRepaired;i++) {
			for (j=i+1;j<e->iCount && !bRepaired;j++) {
				if (e->pElement[i]->GetStreamPos() == e->pElement[j]->GetStreamPos()) {
					bRepaired = true;
					e->pElement[j]->Delete();
					delete e->pElement[j];
					if (cText && strlen(cText))
						Warning(cText);

					for (k=j+1;k<e->iCount;k++) {
						e->pElement[k-1] = e->pElement[k];
					}
					e->iCount--;
				}
			}
		}
	}
	while (bRepaired);
}

void RemoveDoubles(EBMLElementVector& e)
{
	EBMLElementVector result;
	std::sort(e.begin(), e.end());

	size_t size = e.size();

	__int64 last_pos = -1;

	for (size_t i=0;i<e.size();i++) {
		if (last_pos != e[i]->GetStreamPos()) {
			result.push_back(e[i]);
			last_pos = e[i]->GetStreamPos();
		} else {
			e[i]->Delete();
			delete e[i];
		}
	}

	e = result;
}



// Output error about bad seekhead entry to stderr
void BadSeekheadError(int i, int j, char* cID)
{
	char msg[100]; msg[0] = 0;
	sprintf(msg, "Entry %d of Seek head %d (SeekID: %02X %02X %02X %02X) pointing to bad position!"
		         ,j+1, i+1, cID[0] & 0xFF, cID[1] & 0xFF, cID[2] & 0xFF, cID[3] & 0xFF);
	B0rked(msg);
}

void msg() {
	MessageBoxA(NULL, "Ping", "", MB_OK);
}

void EBMLM_Segment::RetrieveAttachments(EBMLElementVector& attachments, ATTACHMENTS& target)
{
	EBMLElementVector attachment;
	ATTACHMENTS temp;

	target.uid_recreated = 0;

	if (attachments.empty())
		return;
	
	std::vector<EBMLElement*>::iterator iter = attachments.begin();
	//EBMLElement** iter = attachments.begin();
	for (; iter != attachments.end(); iter++)
		(*iter)->Search(attachment, (char*)MID_AT_ATTACHEDFILE);
	
	if (attachment.empty())
		return;

	iter = attachment.begin();
	for (; iter != attachment.end(); iter++) {
		ATTACHMENT att;
		
		char* att_uids[] = { (char*)MID_AT_FILENAME, (char*)MID_AT_FILEDESCRIPTION,
			(char*)MID_AT_FILEMIMETYPE, (char*)MID_AT_FILEUID, (char*)MID_AT_FILEDATA,  NULL };
		int att_occ_restr[] = { 3, 2, 
			6, 6, 3, 0 };
		SEARCHMULTIEX att_sme = { att_uids, NULL, att_occ_restr };

		EBMLElementVectors att_search;
		if ((*iter)->SearchMulti(att_search, att_sme, NULL) < 0) {
			att.flags = FATT_B0RKED;
			continue;
		} 

		// filename and filedata are present, otherwise the call above fails
		att.file_name = new CStringBuffer(att_search[0][0]->AsString());
		att.file_pos = att_search[4][0]->GetStreamPos();
		att.flags |= FATT_NAME | FATT_POSITION | FATT_SIZE;
		att.file_size = att_search[4][0]->GetLength();

		// mime type
		if (!att_search[2].empty()) {
			att.file_mime_type = new CStringBuffer(att_search[2][0]->AsString());
			att.flags |= FATT_MIMETYPE;
		}

		if (!att_search[1].empty()) {
			att.file_description = new CStringBuffer(att_search[1][0]->AsString());
			att.flags |= FATT_DESCR;
		}

		if (!att_search[3].empty()) {
			att.file_uid = att_search[3][0]->AsInt();
			att.flags |= FATT_ORIGINALUID;
		} else {
			generate_uid((char*)&att.file_uid, 8);
		}

		temp.attachments.push_back(att);
		DeleteVectors(att_search);

	}

	target = temp;
	target.count = target.attachments.size();

	if (!target.count)
		return;

	std::sort(temp.attachments.begin(), temp.attachments.end());

	/*ATTACHMENT**/
	std::vector<ATTACHMENT>::iterator iter_att = temp.attachments.begin(); iter_att++;
	/*ATTACHMENT**/
	std::vector<ATTACHMENT>::iterator curr_att = temp.attachments.begin();
	
	bool double_uid = false;

	for (; iter_att != temp.attachments.end(); iter_att++) {
		if (*curr_att == *iter_att) {
			char c[64]; c[0]=0;
			char msg[256]; msg[0]=0;
			__int642hex(curr_att->file_uid, c, 8, 1, 1);
			sprintf(msg, "Found non-unique AttachmentUID %s", c);
			iter_att->flags &=~ FATT_ORIGINALUID;
			generate_uid((char*)&iter_att->file_uid, 8);
			B0rked(msg);
			double_uid = true;
		}
	}

	target.uid_recreated = !!double_uid;	
}

void EBMLM_Segment::RetrieveSegmentInfo(EBMLELEMENTLIST* e_SIList)
{
	EBMLElement*	e_SIInfo = NULL;
	EBMLElement*	e_next, *e_old;
	CBuffer*		cb;
	int j;

	RemoveDoubles(e_SIList, "Found 2 seek elements, pointing at the same Segment Info element!");
	if (!e_SIList->iCount) {
		 Warning("Segment Info not found");
	} else {

		char* si_ids[] = { 
			(char*)MID_SI_DURATION, (char*)MID_SI_SEGMENTFAMILY, (char*)MID_SI_SEGMENTUID, 
			(char*)MID_SI_NEXTUID, (char*)MID_SI_PREVUID, (char*)MID_SI_TIMECODESCALE, NULL };
		void* si_targets[] = { &SegmentInfo->fDuration, SegmentInfo->SegmentFamily.cUID,
			SegmentInfo->CurrSeg.cUID, SegmentInfo->NextSeg.cUID, SegmentInfo->PrevSeg.cUID, 
			&SegmentInfo->iTimecodeScale, NULL };
		int si_occ_rests[] = { 2, 2, 6, 2, 2, 2, 0 };
		SEARCHMULTIEX si_sme = { si_ids, si_targets, si_occ_rests };
		
		EBMLElementVectors seginfo;
		e_SIList->pElement[0]->SearchMulti(seginfo, si_sme);
		if (!seginfo[1].empty())
			SegmentInfo->SegmentFamily.bValid = 1;
		if (!seginfo[2].empty())
			SegmentInfo->CurrSeg.bValid = 1;
		if (!seginfo[3].empty())
			SegmentInfo->NextSeg.bValid = 1;
		if (!seginfo[4].empty())
			SegmentInfo->PrevSeg.bValid = 1;
		DeleteVectors(seginfo);

		e_SIInfo = e_SIList->pElement[0];
		e_SIInfo->Create1stSub(&e_next);
		while (e_next) {
			e_old = e_next;
			cb = e_old->GetData();
			switch (e_old->GetType()) {
				case ETM_SI_MUXINGAPP: cb->Refer(&SegmentInfo->cMuxingApp); break;
				case ETM_SI_WRITINGAPP: cb->Refer(&SegmentInfo->cWritingApp); break;
				case ETM_SI_TITLE: 
					//SegmentInfo->cTitle=new CStringBuffer(cb->AsString(), CBN_REF1 | CSB_UTF8);
					SegmentInfo->GetTitleSet()->SetTitle(cb->AsString());
					break;
				case ETM_SI_SEGMENTFILENAME: cb->Refer(&SegmentInfo->CurrSeg.cFilename); break;
				case ETM_SI_PREVFILENAME: cb->Refer(&SegmentInfo->PrevSeg.cFilename); break;
				case ETM_SI_NEXTFILENAME: cb->Refer(&SegmentInfo->NextSeg.cFilename); break;
			}
			e_next = e_next->GetSucc();
			e_old->Delete();
			delete e_old;
		}

		unsigned char* p = new unsigned char[j=(int)e_SIInfo->GetLength()-6];
		e_SIInfo->SeekStream(6);
		e_SIInfo->GetSource()->Read(p, j);
		p[0] = 0x80;
		int icrc = CRC32(p, 4);
//		int icrc2 = CRC32(p, 4, CRC32_POLYNOM);
		delete p;

//		DeleteElementList(&e_SIList);
	}
}

int compare_tcd(TRACK_COMPRESSION_DESCRIPTOR& tc1, TRACK_COMPRESSION_DESCRIPTOR& tc2)
{
	if (tc1.order < tc2.order)
		return -1;
	if (tc1.order == tc2.order)
		return 0;
	return 1;
}

int EBMLM_Segment::RetrieveInfo()
{
	EBMLElement*	e_next, *e_old;
	EBMLELEMENTLIST**	e_TrackEntries = NULL;
	EBMLELEMENTLIST**	e_Cues = &SegmentInfo->pGlobElem->e_Cues;

	EBMLElementVector&	e_Tags = SegmentInfo->pGlobElem->e_Tags;
	EBMLElementVector&  e_Attachments = SegmentInfo->pGlobElem->e_Attachments;
	EBMLElementVector&  e_Chapters = SegmentInfo->pGlobElem->e_Chapters;
	EBMLElementVector&	e_Seekhead = SegmentInfo->pGlobElem->e_Seekhead;

	EBMLELEMENTLIST**	e_Tracks = &SegmentInfo->pGlobElem->e_Tracks;
	EBMLELEMENTLIST**	e_Tracks2 = &SegmentInfo->pGlobElem->e_Tracks2;
	EBMLELEMENTLIST**	e_SIList = &SegmentInfo->pGlobElem->e_SegmentInfo;


	int				iChapCount = 0;
	float			f;
	int				k,l;
	size_t			i,j;
	int				iMaxClusters = 0;
	TRACK_INFO*		t;
	char*			cStop;
	int*			table;

	SegmentInfo->iTimecodeScale = 1000000;
	newz(TRACKS, 1, SegmentInfo->tracks);
	SegmentInfo->pActiveCluster = NULL;
	SegmentInfo->iActiveClusterIndex = 0;
	SegmentInfo->iEndReached = 0;
	SegmentInfo->iTotalBlockCount = 0;

	// Headers stop per definition if a Cluster is encountered,
	// unless weird files are allowed
	cStop = (bAllowObfuscatedSegments)?NULL:(char*)MID_CLUSTER;

	// Seekhead Info
	SegmentInfo->clusters = (CLUSTERS*)calloc(1,sizeof(CLUSTERS));

	// Search for Seekheads and Tracks
	char* cid_Seekhead_Tracks[] = { (char*)MID_TRACKS, (char*)MID_CUES };
	void** pe_Sht = NULL;
	SearchMulti(&pe_Sht, cid_Seekhead_Tracks, 2, (char*)MID_CLUSTER);
	*e_Tracks2	= (EBMLELEMENTLIST*)pe_Sht[0];
	*e_Cues     = (EBMLELEMENTLIST*)pe_Sht[1];
	delete pe_Sht;

	// Search for other L1 elements
	EBMLElementVectors L1_elements;
	char* l1_ids[] = {
		(char*)MID_TAGS, (char*)MID_ATTACHMENTS, 
		(char*)MID_CHAPTERS, (char*)MID_SEEKHEAD, NULL 
	};
	SEARCHMULTIEX l1_search = { l1_ids, NULL, NULL };
	SearchMulti(L1_elements, l1_search, (char*)MID_CLUSTER);

	e_Tags = L1_elements[0];
	e_Attachments = L1_elements[1];
	e_Chapters = L1_elements[2];
	e_Seekhead = L1_elements[3];

	i = 0;

	EBMLElement* seekhead;
	for (i=0; i < e_Seekhead.size(); i++) {

		seekhead = e_Seekhead[i];
		EBMLElementVector	e_Seek;
		if ((seekhead)->Search(e_Seek,(char*)MID_MS_SEEK, NULL, 1, NULL) < 0)
			continue;

		SEEKHEAD_INFO seekhead_info;
		seekhead_info.position = (seekhead)->GetStreamPos();
		seekhead_info.size = (seekhead)->GetLength();
		SegmentInfo->seekheads.push_back(seekhead_info);

		/*EBMLElement* */
		std::vector<EBMLElement*>::iterator seek = e_Seek.begin();
		j = 0;
		for (; seek != e_Seek.end(); seek++) {
			char* cid_seek[] = { (char*)MID_MS_SEEKID, (char*)MID_MS_SEEKPOSITION, NULL };
			int occ_restr[] = { 3, 3, 0 }; // SeekID and SeekPosition unique and mandatory
			SEARCHMULTIEX sme = { cid_seek, NULL, occ_restr };

			EBMLElementVectors seek_info;
			int seek_ok = (*seek)->SearchMulti(seek_info, sme);
			EBMLElementVector& e_SeekID			= seek_info[0];
			EBMLElementVector& e_SeekPosition	= seek_info[1];

			if (seek_ok >= 0) {
				SEEKHEAD_ENTRY entry;
				entry.id = seek_info[0][0]->AsInt();
				entry.position = seek_info[1][0]->AsInt();

				// Cluster
				if (CompIDs(e_SeekID[0]->AsString(),(char*)MID_CLUSTER)) {
					if (SegmentInfo->clusters->iCount>=iMaxClusters) {
						SegmentInfo->clusters->cluster_info = (CLUSTER_PROP**)realloc(
							SegmentInfo->clusters->cluster_info,(iMaxClusters+=10)*sizeof(CLUSTER_PROP*));
					}
					CLUSTER_PROP* ci \
						= SegmentInfo->clusters->cluster_info[SegmentInfo->clusters->iCount++] \
						= (CLUSTER_PROP*)calloc(1,sizeof(CLUSTER_PROP));
					ci->iValid |= CLIV_POS;
					ci->qwPosition = e_SeekPosition[0]->AsInt();
				} else 
				// Seekhead
				if (CompIDs(e_SeekID[0]->AsString(),(char*)MID_SEEKHEAD)) {
					entry.id = 0;
					if (InsertElement(e_Seekhead, this, (char*)MID_SEEKHEAD, e_SeekPosition[0])==1){
						Note("Seekhead contains link to another Seekhead");
						seekhead_info.position = e_SeekPosition[0]->AsInt();
						seekhead_info.size = -1;
						SegmentInfo->seekheads.push_back(seekhead_info);
					} else {
						BadSeekheadError(i,j, (char*)MID_SEEKHEAD);
					}
				} else
				// Cues
				if (CompIDs(e_SeekID[0]->AsString(),(char*)MID_CUES)) {
					if (InsertElement((void**)e_Cues,this,(char*)MID_CUES, e_SeekPosition[0])==-1) {
						BadSeekheadError(i,j,(char*)MID_CUES);
						entry.id = 0;
					}
				} else
				// Segment Info
				if (CompIDs(e_SeekID[0]->AsString(),(char*)MID_SEGMENTINFO)) {
					if (InsertElement((void**)&SegmentInfo->pGlobElem->e_SegmentInfo,this,
						(char*)MID_SEGMENTINFO,e_SeekPosition[0])==-1) {
						BadSeekheadError(i,j,(char*)MID_SEGMENTINFO);
						entry.id = 0;
					}
				} else
				// Chapters
				if (CompIDs(e_SeekID[0]->AsString(),(char*)MID_CHAPTERS)) {
					if (InsertElement(e_Chapters, this, (char*)MID_CHAPTERS, e_SeekPosition[0])==-1) {
						BadSeekheadError(i,j,(char*)MID_CHAPTERS);
						entry.id = 0;
					}
				} else
				// Tracks
				if (CompIDs(e_SeekID[0]->AsString(),(char*)MID_TRACKS)) {
					if (InsertElement((void**)e_Tracks, this, (char*)MID_TRACKS, e_SeekPosition[0])==-1) {
						BadSeekheadError(i,j,(char*)MID_TRACKS);
						entry.id = 0;
					}
				} else
				// Tags
				if (CompIDs(e_SeekID[0]->AsString(),(char*)MID_TAGS)) {
					if (InsertElement(e_Tags, this, (char*)MID_TAGS, e_SeekPosition[0])==-1) {
						BadSeekheadError(i,j,(char*)MID_TAGS);
						entry.id = 0;
					}
				} else
				if (CompIDs(e_SeekID[0]->AsString(),(char*)MID_ATTACHMENTS)) {
					if (InsertElement(e_Attachments, this, (char*)MID_ATTACHMENTS, e_SeekPosition[0])==-1) {
						BadSeekheadError(i,j,(char*)MID_ATTACHMENTS);
						entry.id = 0;
					}
				} else
					entry.id = 0;

				if (entry.id)
					SegmentInfo->seekhead_entries.push_back(entry);
						
			}
			DeleteVectors(seek_info);
			j++;
		}
			
		DeleteVector(e_Seek);
	}
	

	// Segment Info present but not indexed by Seekhead? 
	if (!*e_SIList) {
		Search((void**)e_SIList,(char*)MID_SEGMENTINFO,cStop);
		if (!e_Seekhead.empty() && (*e_SIList)->iCount) {
			Warning("Segment Info present but not indexed in existing Seekhead");
		}
	}

	// Segment Info
	RetrieveSegmentInfo(SegmentInfo->pGlobElem->e_SegmentInfo);

	// Chapters
	RemoveDoubles(e_Chapters);//"Found 2 seek elements, pointing at the same Chapters element!");
	RetrieveChapters(SegmentInfo->pGlobElem->e_Chapters);
	SegmentInfo->pChapters = new CChapters(SegmentInfo->chapters);

	if (!*e_Tracks) *e_Tracks = (EBMLELEMENTLIST*)calloc(1,sizeof(EBMLELEMENTLIST));
	// Tracks
	if (!(*e_Tracks2)->iCount) {
		if (!*e_Tracks || !(*e_Tracks)->iCount) {
			FatalError("No track information found");
			return MSRI_FATALERROR;
		} else {
			B0rked("Track information found by following Seekhead, but not by linear parsing.\n  EBML-File structure is broken!");
		}

//		DeleteElementList(e_Tracks2);
	
	} else {
		if (!(*e_Tracks) || !(*e_Tracks)->iCount) {
			*e_Tracks = *e_Tracks2;
			if (!e_Seekhead.empty())
				Warning("Track information present, but not indexed in existing Seekhead");
		}
	}

	// find double links to tracks elements (was special VDM b0rk)
	RemoveDoubles(*e_Tracks, "Found 2 seek elements, pointing at the same Tracks element!");
	e_TrackEntries = (EBMLELEMENTLIST**)calloc((*e_Tracks)->iCount,sizeof(EBMLELEMENTLIST*));

	for (i=0;i<(*e_Tracks)->iCount;i++) {
		int crc = (*e_Tracks)->pElement[i]->CheckCRC();
		(*e_Tracks)->pElement[i]->Search((void**)&(e_TrackEntries[i]),(char*)MID_TR_TRACKENTRY);
		SegmentInfo->tracks->iCount+=e_TrackEntries[i]->iCount;
	}

	if (!SegmentInfo->tracks->iCount) {
		Warning("Found 'Tracks', but no 'TrackEntry'. File does not\nseem to contain any video/audio/subtitle data.");
	//	return MSRI_FATALERROR;
	}

	table = SegmentInfo->tracks->iTable = (int*)calloc(GetTrackCount(),sizeof(int));
	for (i=0;i<256;i++) 
		SegmentInfo->tracks->iIndexTableByType[i] = (int*)calloc(GetTrackCount(),sizeof(int));
	
	SegmentInfo->tracks->track_info = new TRACK_INFO*[GetTrackCount()];//(TRACK_INFO**)calloc(GetTrackCount(),sizeof(TRACK_INFO*));
	
	SegmentInfo->queue = new void*[GetTrackCount()];
	ZeroMemory(SegmentInfo->queue, sizeof(void*)*GetTrackCount());
	k = 0;
	for (i=0;i<(*e_Tracks)->iCount;i++) {
		for (j=0;j<e_TrackEntries[i]->iCount;j++) {
			EBMLELEMENTLIST* e_uid = NULL;
			e_TrackEntries[i]->pElement[j]->Search((void**)&e_uid, (char*)MID_TR_TRACKUID, NULL); //Create1stSub(&e_next);
			__int64 uid;
			if (!e_uid->iCount) {
				B0rked("TrackUID for Track not found");
				uid = k;
			} else {
				uid = e_uid->pElement[0]->AsInt();
				DeleteElementList(&e_uid);
			}
			int index = -1;
			for (int l=0;l<k;l++) {
				t = SegmentInfo->tracks->track_info[l];
				if (t->iUID == uid) {
					index = l;
				}
			}
			if (index == -1) {
				TRACK_INFO* t = new TRACK_INFO;
				SegmentInfo->tracks->track_info[k] = t;
				t->iUID = uid;
				k++;
			}
		}
	}
	int total_track_count = k;
	SegmentInfo->tracks->iCount = total_track_count;
	newz(__int64, k, SegmentInfo->iBlockCount);
	newz(__int64, k, SegmentInfo->iOtherBlocksThan);
	ZeroMemory(SegmentInfo->tracks->iCountByType, sizeof(SegmentInfo->tracks->iCountByType));

	k=0;
	for (i=0;i<(*e_Tracks)->iCount;i++) {
		for (j=0;j<e_TrackEntries[i]->iCount;j++) {
			EBMLElementVector e_uid;
			EBMLElement* e = e_TrackEntries[i]->pElement[j];

			int crc = e->CheckCRC();
			int pcrc = e->GetParent()->CheckCRC();

			__int64 uid = k;
			e_TrackEntries[i]->pElement[j]->Search(e_uid, (char*)MID_TR_TRACKUID, NULL, 3, &uid);

			int index = -1;
			for (int l=0;l<total_track_count;l++) {
				t = SegmentInfo->tracks->track_info[l];
				if (t->iUID == uid) {
					index = l;
				}
			}
			DeleteVector(e_uid);

			t = SegmentInfo->tracks->track_info[index];

			if (crc == EBML_CRC_FAILED || 
				pcrc == EBML_CRC_FAILED && crc != EBML_CRC_OK) {
					continue;
			}

			char* tr_ids[] = {
				(char*)MID_TR_TRACKTYPE, (char*)MID_TR_TRACKNUMBER, (char*)MID_TR_FLAGDEFAULT,
				(char*)MID_TR_FLAGLACING, (char*)MID_TR_FLAGENABLED, (char*)MID_TR_FLAGFORCED,
				(char*)MID_TR_MINCACHE, (char*)MID_TR_MAXCACHE, (char*)MID_TR_DEFAULTDURATION, NULL };
			void* tr_targets[] = {	
				&t->iType, &t->iNbr, &t->iDefault, &t->iLacing, &t->iEnabled, &t->iForced,
				&t->iMinCache, &t->iMaxCache, &t->qwDefaultDuration, NULL };
			int tr_occ_restr[] = { 3, 3, 2, 2, 2, 2, 2, 2, 2, 0 };
			SEARCHMULTIEX tr_sme = { tr_ids, tr_targets, tr_occ_restr };
			EBMLElementVectors tr_search;
			int tr_search_res = e->SearchMulti(tr_search, tr_sme);
			DeleteVectors(tr_search);
			if (tr_search_res < 0)
				continue;
			table[k] = t->iNbr;

			if (t->iType == MSTRT_AUDIO || t->iType == MSTRT_VIDEO) 
				t->iSparse = 0;
			else 
				t->iSparse = 1;

			e->Create1stSub(&e_next);
			while (e_next) {
				e_old = e_next;
				switch (e_old->GetType()) {
					case ETM_TR_NAME: 
						t->GetTitleSet()->SetTitle(e_old->GetData()->AsString()); 
						break; 
					case ETM_TR_LANGUAGE: e_old->GetData()->Refer(&t->cLanguage); break;
					case ETM_TR_CODECID: e_old->GetData()->Refer(&t->cCodecID); break;
					case ETM_TR_CODECNAME: e_old->GetData()->Refer(&t->cCodecName); break;
					case ETM_TR_CODECPRIVATE: e_old->GetData()->Refer(&t->cCodecPrivate); break;
					case ETM_TR_TRACKTIMECODESCALE: 
						f = (float)e_old->AsFloat();
						if (f<0.000000001 || f> 1000000000) {
							Warning("TrackTimecodescale not between 10^-9 and 10^9 !");
						} else t->fTimecodeScale = f;
						break;
					case ETM_TR_VIDEO: 
						{
							char* search_ids[] = {
								(char*)MID_TRV_FLAGINTERLACED, (char*)MID_TRV_COLORSPACE, (char*)MID_TRV_STEREOMODE,
								(char*)MID_TRV_PIXELWIDTH, (char*)MID_TRV_PIXELHEIGHT, (char*)MID_TRV_PIXELCROPLEFT,
								(char*)MID_TRV_PIXELCROPRIGHT, (char*)MID_TRV_PIXELCROPTOP, (char*)MID_TRV_PIXELCROPBOTTOM,
								(char*)MID_TRV_DISPLAYWIDTH, (char*)MID_TRV_DISPLAYHEIGHT, (char*)MID_TRV_DISPLAYUNIT,
								(char*)MID_TRV_GAMMAVALUE, NULL };
							void* targets[] = {
								&t->video.iInterlaced, &t->video.iColorSpace, &t->video.iStereoMode,
								&t->video.iPixelWidth, &t->video.iPixelHeight, &t->video.rPixelCrop.left,
								&t->video.rPixelCrop.right, &t->video.rPixelCrop.top, &t->video.rPixelCrop.bottom,
								&t->video.iDisplayWidth, &t->video.iDisplayHeight, &t->video.iDisplayUnit, &t->video.fGamma, NULL };
							int occ_restr[] = {
								2, 2, 2, 
								3, 3, 2,
								2, 2, 2,
								2, 2, 2,
								2, 0 };
							SEARCHMULTIEX sme = { search_ids, targets, occ_restr };
							EBMLElementVectors search;
							e_old->SearchMulti(search, sme);
							DeleteVectors(search);
						} break;
					case ETM_TR_AUDIO:
						{
							char* search_ids[] = {
								(char*)MID_TRA_SAMPLINGFREQUENCY, (char*)MID_TRA_OUTPUTSAMPLINGFREQUENCY, 
								(char*)MID_TRA_CHANNELS, (char*)MID_TRA_BITDEPTH, NULL };
							void* targets[] = {
								&t->audio.fSamplingFrequency, &t->audio.fOutputSamplingFrequency,
								&t->audio.iChannels, &t->audio.iBitDepth, NULL };
							int occ_restr[] = { 2, 2, 2, 2 };
							SEARCHMULTIEX sme = { search_ids, targets, occ_restr };
							EBMLElementVectors search;
							e_old->SearchMulti(search, sme);
							DeleteVectors(search);

						} break;
					case ETM_TR_CONTENTENCODINGS:
						{
							EBMLELEMENTLIST* e_ContentEncoding = NULL;
							e_old->Search((void**)&e_ContentEncoding, (char*)MID_TRCE_CONTENTENCODING, NULL);
							if (e_ContentEncoding->iCount!=1) {
								DeleteElementList(&e_ContentEncoding);
								break;
							}

							char* cp_ids[] = {	(char*)MID_TRCE_CONTENTENCODINGORDER, 
												(char*)MID_TRCE_CONTENTENCODINGTYPE,
												(char*)MID_TRCE_CONTENTENCODINGSCOPE,
												(char*)MID_TRCE_CONTENTCOMPRESSION, 
												NULL };
							__int64 order = 0;
							__int64 type = 0;
							__int64 scope = 1;
							void* cp_targets[] = { &order, &type, &scope, NULL, NULL };
							int cp_occ_restr[] = { 2, 2, 2, 3, 0 };
							SEARCHMULTIEX cp_sme = { cp_ids, cp_targets, cp_occ_restr };
							EBMLElementVectors cp_search;
							if ((*e_ContentEncoding)(0)->SearchMulti(cp_search, cp_sme) > -1) {
								TRACK_COMPRESSION_DESCRIPTOR tci;
								char* cpr_ids[] = { 
									(char*)MID_TRCE_CONTENTCOMPALGO, 
									(char*)MID_TRCE_CONTENTCOMPSETTINGS, NULL };
								int cpr_occ_restr[] = { 2, 2, 0 };
								SEARCHMULTIEX cpr_sme = { cpr_ids, NULL, cpr_occ_restr };
								EBMLElementVectors cpr_search;

								cp_search[3][0]->SearchMulti(cpr_search, cpr_sme);
							
								if (!cpr_search[0].empty()) {
									if (cpr_search[0][0]->AsInt() == 0) {
										tci.compression = COMPRESSION_ZLIB;
										tci.is_decompressed = 0;
									}
									if (cpr_search[0][0]->AsInt() == 3) {
										tci.compression = COMPRESSION_HDRSTRIPPING;
										tci.is_decompressed = 1;
									}
								}
								if (!cpr_search[1].empty()) {
									tci.compression_private_size = (int)cpr_search[1][0]->GetLength();
									tci.compression_private = malloc(tci.compression_private_size);
									char* c = cpr_search[1][0]->AsString();
									memcpy(tci.compression_private, c, tci.compression_private_size);
								} else {
									tci.compression_private = NULL;
									tci.compression_private_size = 0;
								}
								tci.compressed_elements = (int)scope;
								tci.order = (int)order;
								
								// check if order already exists
								if (t->track_compression.size())
								{
									TRACK_COMPRESSION::iterator iter = t->track_compression.begin();
									bool found = false;
									for (; iter != t->track_compression.end() && !found; iter++)
										found |= iter->order == tci.order;
	//								iter--;

									TRACK_COMPRESSION::reverse_iterator _iter = 
										TRACK_COMPRESSION::reverse_iterator(t->track_compression.rend());
									_iter--;
									// if compression descriptor for that position
									// exists, check for consistency
									if (!found)
										t->track_compression.push_back(tci);
									else {
										if (!(tci == *_iter))
											B0rked("Found inconsistent track compression information", 
												(*e_ContentEncoding)(0)->GetStreamPos());
									}
									
	//								int k = t->track_compression.size();
									DeleteVectors(cpr_search);
								} else
									t->track_compression.push_back(tci);
							}
							DeleteElementList(&e_ContentEncoding);
							DeleteVectors(cp_search);
						} break;
				}
				e_next = e_next->GetSucc();
				DeleteEBML(&e_old);
			}

			if (t->video.iDisplayWidth==-1) t->video.iDisplayWidth = t->video.iPixelWidth;
			if (t->video.iDisplayHeight==-1) t->video.iDisplayHeight = t->video.iPixelHeight;
			k++;
			if (t->iNbr == -1) {
				B0rked("TrackEntry does not contain a track number");
			}

			if (t->iType == -1) {
				FatalError("track type not indicated");
			}

			if (t->iType == MSTRT_VIDEO) {
				if (t->video.iPixelWidth == -1 || t->video.iPixelHeight == -1) {
					FatalError("resolution of video stream not indicated");
				}
			}

			if (t->audio.fOutputSamplingFrequency<0) 
				t->audio.fOutputSamplingFrequency = t->audio.fSamplingFrequency;

			std::sort(t->track_compression.begin(), t->track_compression.end(),
				compare_tcd);
		}
	}

	for (i=0;i<(size_t)total_track_count;i++) {
		int type = SegmentInfo->tracks->track_info[i]->iType;
		if (SegmentInfo->iMasterStream == -1 && type == MSTRT_VIDEO) SegmentInfo->iMasterStream = (int)i; 
		SegmentInfo->tracks->iIndexTableByType[type][SegmentInfo->tracks->iCountByType[type]++] = (int)i;
	}

	// if all streams are sparse: bad

	// Cues
	RemoveDoubles(*e_Cues, /*"Found 2 seek elements, pointing at the same Cues element!"*/"");

	RemoveDoubles(e_Attachments);
	RetrieveAttachments(e_Attachments, SegmentInfo->attachments);

	// tags
	RemoveDoubles(e_Tags);// /*"Found 2 seek elements, pointing at the same Tags element!"*/"");

	if (!e_Tags.empty()) {

		EBMLElementVector e_Tag;
		
		for (std::vector<EBMLElement*>::iterator iter = e_Tags.begin(); iter != e_Tags.end(); iter++)
			(*iter)->Search(e_Tag, (char*)MID_TG_TAG, NULL);

		for (std::vector<EBMLElement*>::iterator _tag = e_Tag.begin(); _tag != e_Tag.end(); _tag++) { 
		
			EBMLElement* tag = *_tag;	
			
			// search for Target and SimpleTags
			EBMLELEMENTLIST** pe_TagCont = NULL;
			char* cTagCont[] = { 
				(char*)MID_TG_TARGET, 
				(char*)MID_TG_SIMPLETAG 
			};
			tag->SearchMulti((void***)&pe_TagCont, cTagCont, 2, NULL);
			EBMLELEMENTLIST* e_Target     = pe_TagCont[0];
			EBMLELEMENTLIST* e_SimpleTags = pe_TagCont[1];
			delete pe_TagCont;
			
			EBMLElementVectors targets;
			char* target_uids[] = {
				(char*)MID_TG_TRACKUID, (char*)MID_TG_EDITIONUID, 
				(char*)MID_TG_CHAPTERUID, NULL };
			SEARCHMULTIEX target_sme = { target_uids, NULL, NULL };

			/* TrackUIDs  : targets[0]
			   EditionUIDs: targets[1]
			   ChapterUIDs: targets[2]
			*/

			//CDynIntArray* aTracks = new CDynIntArray;
			std::vector<int> aTracks;
			if (!e_Target->iCount) {
				Note("Tag does not contain Target and will be considered global for this segment");
				/* fix crash when accessing targets[0], targets[1], ... later 
				   by pushbacking empty vectors */
                targets.push_back(std::vector<EBMLElement*>());
                targets.push_back(std::vector<EBMLElement*>());
                targets.push_back(std::vector<EBMLElement*>());
			} else {
				// find all track IDs
				e_Target->pElement[0]->SearchMulti(targets, target_sme);

				// make array of track indexes from track uids
				for (std::vector<EBMLElement*>::iterator iter = targets[0].begin(); iter != targets[0].end(); iter++) {
					int iIndex = TrackUID2TrackIndex((*iter)->AsInt());
					if (iIndex == TUID2TI_INVALID) {
						B0rked("Couldn't find Track for TrackUID in Target",
							(*iter)->GetStreamPos());
					} else {
						aTracks.push_back(iIndex);
					}
				}
				
				EBMLElement* e_next, *e_old;
				TRACK_INFO* t;
				e_Target->pElement[0]->Create1stSub(&e_next);
				// go through tags and look for bitsps and framesps
				while (e_next) {
					e_old = e_next;
					for (k=0;k<aTracks.size();k++) {
						t = SegmentInfo->tracks->track_info[aTracks[k]];
						switch (e_old->GetType()) {
							case ETM_TG_BITSPS: 
								t->tags.iFlags |= TRACKTAGS_BITSPS;
								t->tags.iBitsPS = e_old->AsInt();
								break;
							case ETM_TG_FRAMESPS:
								t->tags.iFlags |= TRACKTAGS_FRAMESPS;
								t->tags.fFramesPS = (float)e_old->AsFloat();
								break;
						}
					}
					e_next = e_old->GetSucc();
					DeleteEBML(&e_old);
				}
			}

			for (k=0;k<(int)e_SimpleTags->iCount;k++) {
				EBMLElementVectors tags;
				
				char* cTags[] = { 
					(char*)MID_TG_TAGNAME, 
					(char*)MID_TG_TAGSTRING,
					(char*)MID_TG_TAGLANGUAGE, 
					NULL
				};
				int tag_occ_restr[] = { 3, 3, 2, 0 };
				SEARCHMULTIEX tag_sme = { cTags, NULL, tag_occ_restr };
					
				e_SimpleTags->pElement[k]->SearchMulti(tags, tag_sme);

				EBMLElementVector& tag_name = tags[0];
				EBMLElementVector& tag_string = tags[1];
				EBMLElementVector& tag_language = tags[2];

				char* cName		= NULL; if (!tag_name.empty()) 
					cName = tag_name[0]->AsString();
				char* cString	= NULL; if (!tag_string.empty()) 
					cString = tag_string[0]->AsString();
				char* cLanguage	= "und"; if (!tag_language.empty())
					cLanguage= tag_language[0]->AsString();

				// need at least name and string. Binary tags are ignored
				if (cName && cString) {
					// Tag indicating the bitrate of something
					if (!strcmp(cName, "BITSPS") || !strcmp(cName, "BPS")) {
						__int64 iBPS = atoi(cString);
						for (l=0;l<aTracks.size();l++) {
							t = SegmentInfo->tracks->track_info[aTracks[l]];
							t->tags.iFlags |= TRACKTAGS_BITSPS;
							t->tags.iBitsPS = iBPS;
						}
					}
						
					// titles of Editions and Chapters 
					if (!targets[1].empty() || !targets[2].empty()) {
						EBMLElementVector e[] = { targets[2], targets[1] };

						for (int z=0;z<2;z++) for (size_t j=0;j<e[z].size();j++) {
							int iIndex;
							__int64 uid = e[z][j]->AsInt();
							CChapters* pChapter = SegmentInfo->pChapters->FindUID(
								uid, z, &iIndex);
							if (!pChapter) {
								B0rked("Couldn't find Edition/Chapter for EditionUID/ChapterUID in Target",
									e[z][j]->GetStreamPos());
							} else {
								tagAddIndex(pChapter->GetTags(iIndex), tagAdd(
									&SegmentInfo->pAllTags, cName, cString, cLanguage));

								int c = pChapter->FindChapterDisplayLanguageIndex(iIndex, cLanguage);
								if (c<0) c = pChapter->GetChapterDisplayCount(iIndex);
								if (!strcmp(cName, "TITLE")) {
									pChapter->SetChapterText(iIndex, cString, c);
									pChapter->SetChapterLng(iIndex, cLanguage, c);
								}
								if (!strcmp(cName, "SegmentUID") && !z) {
									// SegmentUID only makes sense for Chapters, not for Editions
									char b[17]; memset(b, 0, sizeof(b));
									if (hex2int128(cString, b)) {
										SINGLE_CHAPTER_DATA scd; memset(&scd, 0, sizeof(scd));
										pChapter->GetChapter(iIndex, &scd);
										// if SegmentUID already present, compare to existing value
										if (scd.bSegmentUIDValid) {
											if (memcmp(scd.cSegmentUID, b, 16))
												B0rked("Value of ChapterSegmentUID differs from SegmentUID tag",
													tag->GetStreamPos());
										} else
											pChapter->SetSegmentUID(iIndex, 1, b);	
									} else
										B0rked("Value of SegmentUID does not seem to be a SegmentUID",
											tag_string[0]->GetStreamPos());
								}
								c = pChapter->FindChapterDisplayLanguageIndex(iIndex, cLanguage);
							}
						}
					}

					// generic tags without any meaning
					if (!e_Target->iCount || 
							targets[0].empty() && 
							targets[1].empty() && 
							targets[2].empty()
						) {
						// global Tag
						tagAddIndex(SegmentInfo->pGlobalTags, 	
							tagAdd(&SegmentInfo->pAllTags, cName, cString, cLanguage));

						if (!strcmp(cName, "TITLE")) {
							SegmentInfo->GetTitleSet()->AddTitle(cString, cLanguage);
						}
					} 

					if (!targets[0].empty()) {
						int index = tagAdd(&SegmentInfo->pAllTags, cName, cString, cLanguage);
						for (size_t i=0;i<targets[0].size();i++) {
							int track_index = TrackUID2TrackIndex(targets[0][i]->AsInt());
							if (track_index == TUID2TI_INVALID) 
								B0rked("No track found for specified TrackUID in Target", targets[0][i]->GetStreamPos());
							else {
								tagAddIndex(SegmentInfo->tracks->track_info[track_index]->pTags, index);
								if (!strcmp(cName, "TITLE")) {
									SegmentInfo->tracks->track_info[track_index]->GetTitleSet()->AddTitle(cString, cLanguage);
								}
							}
						}
					}
				}
				DeleteVectors(tags);
			}

			DeleteVectors(targets);
			DeleteElementList(&e_Target);
			DeleteElementList(&e_SimpleTags);

		//	aTracks->DeleteAll();
		//	delete aTracks;
		}
		
		DeleteVector(e_Tag);
	}

/*	if (*e_Seekhead) {
		for (i=0;i<(*e_Seekhead)->iCount;i++) {
			DeleteElementList(&e_Seek[i]);
		}
		delete e_Seek;
	}
*/
	if (e_TrackEntries) {
		for (i=0;(int)i<(*e_Tracks)->iCount;i++) {
			DeleteElementList(&e_TrackEntries[i]);
		}
		delete e_TrackEntries;
	}


//	LocateCluster_MetaSeek(0);
//	Seek(0);

	InfoPrivate.bInfoRetrieved = true;

	return MSRI_OK;
}

__int64 EBMLM_Segment::GetTrackDuration(int iTrack)
{
	return RetrieveLastBlockEndTimecode(iTrack);
}

__int64 EBMLM_Segment::GetMasterTrackDuration()
{
	if (SegmentInfo->iMasterStream == -1) {
		return (__int64)SegmentInfo->fDuration;
	} else {
		__int64 d = GetTrackDuration(SegmentInfo->iMasterStream);
		if (d != TIMECODE_UNKNOWN) return d;
		return (__int64)SegmentInfo->fDuration;
	}
}

int EBMLM_Segment::TrackUID2TrackIndex(__int64 iUID)
{
	for (int i=0;i<(int)SegmentInfo->tracks->iCount;i++) {
		if (iUID == SegmentInfo->tracks->track_info[i]->iUID) {
			return i;
		}
	}

	return TUID2TI_INVALID;
}

int EBMLM_Segment::ReadBlock(READBLOCK_INFO* pInfo)
{
	EBMLM_Cluster*	c;

	c=GetActiveCluster();
	if (c && c->ReadBlock(pInfo)==READBL_ENDOFCLUSTER) {
		if (NextCluster(&c)==NEXTCL_OK) {
			return ReadBlock(pInfo);
		} else {
			return READBL_ENDOFSEGMENT;
		}
	}
	
	if (!c) 
		return READBL_ENDOFSEGMENT;

	if (InfoPrivate.bShiftFirstClusterToZero) {
		pInfo->qwTimecode -= SegmentInfo->iFirstClusterTimecode;
	}

	

	return READBL_OK;
}

int EBMLM_Segment::NextCluster(EBMLM_Cluster **p)
{
	EBMLM_Cluster*	c = GetActiveCluster();
	EBMLM_Cluster*  n = NULL;

	if (n = (EBMLM_Cluster*)c->FindNext((char*)MID_CLUSTER)) {
		SetActiveCluster(n);
//		c->Delete();
//		delete c;

		*p = n;
	} else {
		*p = NULL;
		return NEXTCL_ENDOFSEGMENT;
	}

	return NEXTCL_OK;
}

void EBMLM_Segment::Delete()
{
	int i,j;
	TRACK_INFO* t;
	CUE_POINT*  cp;
	CLUSTER_PROP* ci;
	EBMLELEMENTLIST** e_CuePoints = &SegmentInfo->pGlobElem->e_CuePoints;
	EBMLELEMENTLIST** e_Cues      = &SegmentInfo->pGlobElem->e_Cues;


	if (!SegmentInfo)
		return;
	
	if (SegmentInfo->iBlockCount) 
		delete SegmentInfo->iBlockCount;
	
	if (SegmentInfo->iOtherBlocksThan) 
		delete SegmentInfo->iOtherBlocksThan;
	
	if (SegmentInfo->queue) 
		delete SegmentInfo->queue;

	DeleteTagList(&SegmentInfo->pAllTags);

	if (SegmentInfo->cues) {
		for (i=0;i<SegmentInfo->cues->iCuePoints;i++) {
			if (cp = SegmentInfo->cues->pCuePoints[i]) {
				for (size_t k=0;k<cp->tracks.size();k++) {
					if (cp->tracks[k].pReferences) {
						for (j=0;j<cp->tracks[k].iRefCount;j++) {
							delete cp->tracks[k].pReferences[j];
						}
						delete cp->tracks[k].pReferences;
					}
				}
				delete cp;
			}
		}
		delete SegmentInfo->cues->pCuePoints;
		delete SegmentInfo->cues;
	}

	// delete cluster seek points
	if (SegmentInfo->clusters) {
		for (i=0;i<SegmentInfo->clusters->iCount;i++) {
			ci = SegmentInfo->clusters->cluster_info[i];
			if (ci->pCluster) {
				ci->pCluster->Delete();
				delete ci->pCluster;
			}
			delete ci;
		}
		delete SegmentInfo->clusters->cluster_info;
		delete SegmentInfo->clusters;
	}

	// delete chapters
	if (SegmentInfo->pChapters) {
		SegmentInfo->pChapters->Delete();
		delete SegmentInfo->pChapters;
	}

	if (SegmentInfo->tracks) {
		for (i=0;i<(int)SegmentInfo->tracks->iCount;i++) {
			t = SegmentInfo->tracks->track_info[i];
			DecBufferRefCount(&t->audio.cChannelPositions);
			delete SegmentInfo->tracks->track_info[i];
		}
	}

	if (SegmentInfo->tracks) {
		if (SegmentInfo->tracks->track_info) 
			delete [] SegmentInfo->tracks->track_info;
		
		if (SegmentInfo->tracks->iTable) 
			delete SegmentInfo->tracks->iTable;
		
		for (i=0;i<256;i++) 
			if (SegmentInfo->tracks->iIndexTableByType[i]) 
				delete SegmentInfo->tracks->iIndexTableByType[i];

		delete SegmentInfo->tracks;
	}
	DecBufferRefCount(&SegmentInfo->cMuxingApp);
	//DecBufferRefCount(&SegmentInfo->cTitle);
	DecBufferRefCount(&SegmentInfo->cWritingApp);
	DecBufferRefCount(&SegmentInfo->CurrSeg.cFilename);
	DecBufferRefCount(&SegmentInfo->PrevSeg.cFilename);
	DecBufferRefCount(&SegmentInfo->NextSeg.cFilename);

	SetActiveCluster(NULL);

	delete SegmentInfo->pGlobElem;
	delete SegmentInfo;
}

EBMLM_Cluster* EBMLM_Segment::LocateCluster_MetaSeek(__int64 qwTime)
{
	CLUSTER_PROP** clusters = SegmentInfo->clusters->cluster_info;
	CLUSTER_PROP*  curr_clu;
	EBMLM_Cluster* cluster;
	if (!SegmentInfo->clusters->iCount) return 0;
	int		iState;
	int		bStop = 0;
	int		iEstimation = (int)(qwTime * SegmentInfo->clusters->iCount / SegmentInfo->fDuration);
	int		iNewEst;
	curr_clu = clusters[iEstimation];

	iState = ISTATE_JUMPSEARCH;

	while (1 && !bStop) {
	// time of cluster already determined?
		if (curr_clu->iValid & CLIV_TIME) {
			// cluster object existing?
			__int64	qwDuration = curr_clu->qwDuration;
			
			if (iState==ISTATE_JUMPSEARCH)
			{
				cluster = curr_clu->pCluster;
				if (qwTime>=curr_clu->qwTimecode && qwTime<=curr_clu->qwTimecode+qwDuration) {
					SegmentInfo->iActiveClusterIndex = iEstimation;
					SetActiveCluster(curr_clu->pCluster);
					//SegmentInfo->pActiveCluster = curr_clu->pCluster;
					curr_clu->pCluster = NULL;
					return SegmentInfo->pActiveCluster;
				} 
				else if (qwTime<curr_clu->qwTimecode) {
					iNewEst = (int)(iEstimation* qwTime / curr_clu->qwTimecode);
					if (iNewEst == iEstimation) bStop = 1;
					iEstimation = iNewEst;
					//if (curr_clu->pCluster) 
					//	DeleteEBML(&curr_clu->pCluster);
					curr_clu = clusters[iEstimation];
				}
				else {
					iNewEst = (int)(iEstimation* qwTime / curr_clu->qwTimecode);
					if (abs(iNewEst-iEstimation)<3) {
						iState = ISTATE_LINEARSEARCH;
						cluster = curr_clu->pCluster;
						curr_clu->pCluster = NULL;
						if (!cluster) {
							SeekStream(curr_clu->qwPosition);
							Create((EBMLElement**)&cluster);
						}


					} else {
						if (curr_clu->pCluster) DeleteEBML(&curr_clu->pCluster);
						iEstimation = iNewEst;
						curr_clu = clusters[iEstimation];
					}
				}
			} else	{
				__int64 qwDuration = cluster->GetDuration();
				if (GetShiftedClusterTimecode(cluster)+cluster->GetDuration() < qwTime) {
					return SetActiveCluster(cluster);
					//return SegmentInfo->pActiveCluster = cluster;
				} else {
					EBMLM_Cluster*	c = cluster;
					cluster = (EBMLM_Cluster*)cluster->FindNext((char*)MID_CLUSTER);
					DeleteEBML(&c);
				}
			}

		} else {
			if (curr_clu->iValid && CLIV_POS) {
				if (!curr_clu->pCluster) {
					SeekStream(curr_clu->qwPosition);
					Create((EBMLElement**)&curr_clu->pCluster);
					SetActiveCluster(curr_clu->pCluster);
					curr_clu->qwTimecode = GetShiftedClusterTimecode(curr_clu->pCluster);//->GetTimecode();
					curr_clu->qwDuration = curr_clu->pCluster->GetDuration();
					curr_clu->iValid |= CLIV_TIME;
				}
			}
		}
	}

	return NULL;
}

CUES* EBMLM_Segment::GetCues(__int64 start_time, __int64 end_time)
{
	if (!SegmentInfo->cues) {
		RetrieveCues(&SegmentInfo->pGlobElem->e_Cues, start_time, end_time);
	}

	ParseCuePoints(SegmentInfo->cues, start_time, end_time, PARSECUEPOINTS_TIME);

	return SegmentInfo->cues;
}

bool EBMLM_Segment::VerifyCuePoint(int index)
{
	CUES* cues = GetCues();

	CUE_POINT* p = GetCuePoint(index);

	if (!p)
		return false;

	std::vector<CUE_TRACKINFO>::iterator cti = p->tracks.begin();

	__int64 pos = GetStreamPos();

	for (; cti != p->tracks.end(); cti++) {
		SeekStream(cti->qwClusterPos);

		EBMLM_Cluster* cluster = NULL;
		Create((EBMLElement**)&cluster);

		char pt[16]; pt[0]=0;
		sprintf(pt, "%d", index);

		char trk[16]; trk[0]=0;
		sprintf(trk, "%d", cti->iTrack);

		char pos[64]; pos[0]=0;
		QW2Str(cti->qwClusterPos, pos, 1);

		if (cluster->GetType() != IDVALUE(MID_CLUSTER)) {
			char msg[2048]; msg[0]=0;
			sprintf(msg, "Cue point #%d, track #%d pointing to fake Cluster at %s.\n  Found %s instead",
				index, SegmentInfo->tracks->iTable[cti->iTrack], pos, cluster->GetTypeString());
			B0rked(msg);
		}

		cluster->Delete();
		delete cluster;
	}

	SeekStream(pos);

	return true;
}

EBMLM_Cluster* EBMLM_Segment::LocateCluster_Cues(__int64 qwTime)
{
	CUES* cues = GetCues(0,0);

	// interpolation search on cues
	int	iMin = 0;
	int iMax = cues->iCuePoints-1;
	int iMid = 0;
	int iPrevMid = -1;
	bool bUseFirstCluster = false;

	while (iMin != iMax && GetCuePoint(iMin)->qwTimecode != GetCuePoint(iMax)->qwTimecode) {

		double min_time = (double)GetCuePoint(iMin)->qwTimecode;
		double max_time = (double)GetCuePoint(iMax)->qwTimecode;
		double des_time = (double)qwTime;

		double ratio = (des_time - min_time) / (max_time - min_time);
		double offset = ratio * ((double)iMax - (double)iMin);

		iMid = min(cues->iCuePoints-1, max(0, iMin + (int)(offset + .5)));
		
		if (iMid == iPrevMid) {
			if (iMid == iMin)
				iMid++;
			else 
				iMid--;
		}

		iPrevMid = iMid;

		if (GetCuePoint(iMid)->qwTimecode > qwTime) {
		// need earlier cue point
			iMax = iMid;
		} else 
		// behind last cue point
		if (GetCuePoint(iMax)->qwTimecode < qwTime) {
			iMin = iMax;
		} else 
		if (iMid == iMax || GetCuePoint(iMid+1)->qwTimecode < qwTime) {
		// somewhere behind next cue point
			iMin = iMid;
		} else {
		// found!
			iMin = iMid;
			iMax = iMid;
		}
	}

	if (GetCuePoint(iMid)->qwTimecode > qwTime && iMid < 1) {
		bUseFirstCluster = true;
	}

	EBMLM_Cluster* e_Cluster = NULL;
	if (!cues->pCuePoints[iMid]->tracks.empty() && !bUseFirstCluster) {
		SeekStream(cues->pCuePoints[iMid]->tracks[0].qwClusterPos);
		Create((EBMLElement**)&e_Cluster);
		if (e_Cluster->GetType() != IDVALUE(MID_CLUSTER)) {
			if (!iMid)
				iMid++;
			else 
				iMid--;
				
			SeekStream(cues->pCuePoints[iMid]->tracks[0].qwClusterPos);
			Create((EBMLElement**)&e_Cluster);
		}
	} else {
		EBMLELEMENTLIST* e = NULL;
		Search((void**)&e, (char*)MID_CLUSTER, (char*)MID_CLUSTER);
		e_Cluster = (EBMLM_Cluster*)e->pElement[0];
		delete e->pElement;
		delete e;
	}

	if (e_Cluster->GetType() != IDVALUE(MID_CLUSTER)) {
		char pt[16]; pt[0]=0;
		char pos[32]; pos[0]=0;
		char msg[2048]; msg[0]=0;
		sprintf(pt, "%d", iMid);
		QW2Str(cues->pCuePoints[iMid]->tracks[0].qwClusterPos, pos, 1);

		sprintf(msg, "When you see this message, either a 2 Cue elements in a row are broken, or the Matroska reader stumbled again over a bug in the Cache class. Please contact me, and please keep the source file in question! The next thing AVI-Mux GUI will do is most likely crash. Try disabling \"unbuffered read\" in the input settings.\n\nThe cue point in question is #%s and points to %s.",
			pt, pos);

		MessageBoxA(0, msg , "Warning", MB_OK);
		GetSource()->InvalidateCache();
		SeekStream(cues->pCuePoints[iMid]->tracks[0].qwClusterPos);
		Create((EBMLElement**)&e_Cluster);
	}

	return SetActiveCluster(e_Cluster);
}


SEGMENT_INFO* EBMLM_Segment::GetSegmentInfo()
{
	if (!SegmentInfo || !InfoPrivate.bInfoRetrieved) {
		if (RetrieveInfo() == MSRI_FATALERROR) {
			return NULL;
		}
	}

	return SegmentInfo;
}

int EBMLM_Segment::GetTrackCount()
{
	return SegmentInfo->tracks->iCount;
}

void EBMLM_Segment::Seek(__int64 iTimecode)
{
	CUES* cues = GetCues(0, 0);

	if (SegmentInfo->iSeekMode == SEEKMODE_NORMAL) {
		if (cues && cues->iCuePoints) {
			LocateCluster_Cues(iTimecode);
		} else {
			LocateCluster_MetaSeek(iTimecode);
		}
	} else {
		if (!GetActiveCluster()) return;
		if (GetActiveCluster()->GetTimecode() > iTimecode) {
			__int64 iPosition = GetActiveCluster()->GetStreamPos();
			__int64 iPrevSize = GetActiveCluster()->GetPreviousSize();
			if (1) {
			//if (iPrevSize == PREVSIZE_UNKNOWN) {
				// file is total shit
				SeekStream(0);
				EBMLELEMENTLIST* e_Clusters = NULL;
				Search((void**)&e_Clusters, (char*)MID_CLUSTER, (char*)MID_CLUSTER);
				if (!e_Clusters->iCount) {
					// first cluster is also gone
					FatalError("First Cluster not found when trying to seek in a crappy file");
					return;
				} else {
					SetActiveCluster((EBMLM_Cluster*)e_Clusters->pElement[0]);
					delete e_Clusters->pElement;
					delete e_Clusters;
				}
			} else {
				// file is only almost total shit
			}
		}
	}
}

void EBMLM_Segment::EnableShiftFirstClusterTimecode2Zero(bool bEnable)
{
	int j;
	GetCues(0, 0);
	Seek(-40000);
	if (!InfoPrivate.bShiftFirstClusterToZero && bEnable && SegmentInfo->iFirstClusterTimecode) {
		if (SegmentInfo->cues) for (j=0;j<SegmentInfo->cues->iCuePoints;j++) {
			if (SegmentInfo->cues->pCuePoints[j])
				SegmentInfo->cues->pCuePoints[j]->qwTimecode -= SegmentInfo->iFirstClusterTimecode;
		}
	} else
	if (InfoPrivate.bShiftFirstClusterToZero && !bEnable && SegmentInfo->iFirstClusterTimecode) {
		if (SegmentInfo->cues) for (j=0;j<SegmentInfo->cues->iCuePoints;j++) {
			if (SegmentInfo->cues->pCuePoints[j])
				SegmentInfo->cues->pCuePoints[j]->qwTimecode += SegmentInfo->iFirstClusterTimecode;
		}
	}

	InfoPrivate.bShiftFirstClusterToZero = !!bEnable;
}

__int64 EBMLM_Segment::GetShiftedClusterTimecode(EBMLM_Cluster* pCluster)
{
	if (!InfoPrivate.bShiftFirstClusterToZero) {
		return pCluster->GetTimecode();
	} else {
		return pCluster->GetTimecode() - SegmentInfo->iFirstClusterTimecode;
	}
}

/*__int64 EBMLM_Segment::GetTrackDuration(int iTrack) 
{
	return RetrieveLastBlockEndTimecode(iTrack);
}
*/
int tagAdd(TAG_LIST** pTags, char* cName, char* cText, char* cLng)
{
	if (!*pTags) {
		newz(TAG_LIST, 1, *pTags);
		(*pTags)->pTags = new TAG*;
		(*pTags)->iCount = 0;
	} else {
		(*pTags)->pTags = (TAG**)realloc((*pTags)->pTags, sizeof(TAG*) * ((*pTags)->iCount+1));
	}

	TAG** t = &(*pTags)->pTags[(*pTags)->iCount++];

	*t = new TAG;

	(*t)->iRef = 1;
	(*t)->iType = TAGTYPE_STRING;
	(*t)->cName = new char[1+strlen(cName)];
	strcpy((*t)->cName, cName);
	(*t)->cData = new char[1+strlen(cText)];
	strcpy((*t)->cData, cText);
	(*t)->cLanguage = new char[1+strlen(cLng)];
	strcpy((*t)->cLanguage, cLng);

	return (*pTags)->iCount-1;
}

int tagAddIndex(TAG_INDEX_LIST &tags, int iIndex)
{
	tags.push_back(iIndex);

	return (int)tags.size()-1;

/*	if (!*pTagIndexList) {
		newz(TAG_INDEX_LIST, 1, *pTagIndexList);
		(*pTagIndexList)->iCount = 0;
		(*pTagIndexList)->pIndex = new int;
	} else {
		(*pTagIndexList)->pIndex = (int*)realloc((*pTagIndexList)->pIndex, sizeof(int) * ((*pTagIndexList)->iCount+1));
	}

	int* pIndex = &(*pTagIndexList)->pIndex[(*pTagIndexList)->iCount];

	*pIndex = iIndex;

	return (*pTagIndexList)->iCount++;*/
}