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
#include "AVIMux_GUI/DynArray.h"
#include "crc.h"
#include "AVIMux_GUI/ecc.h"

	//////////////////////
	// Matroska-Segment //
	//////////////////////

const int ISTATE_JUMPSEARCH	= 0x01;
const int ISTATE_LINEARSEARCH = 0x02;



EBMLM_Segment::EBMLM_Segment(STREAM* s,EBMLElement *p)
{
	SetStream(s); 
	SetParent(p); 
	DetermineLength(); 
	SetType(ETM_SEGMENT); 
	SetMulti(true); 
	SetDataType(EBMLDATATYPE_MASTER); 
	SegmentInfo=NULL;
	newz(SEGMENT_INFO, 1, SegmentInfo);
	SegmentInfo->iMasterStream = -1;
	SegmentInfo->iSeekMode = SEEKMODE_IDIOT;
	ZeroMemory(&InfoPrivate, sizeof(InfoPrivate));
	newz(GLOBALELEMENTS, 1, SegmentInfo->pGlobElem);
	return; 
}

bool EBMLM_Segment::CheckIDs(char* iID,EBMLElement** p)
{
	DOCOMP(MID_MS_SEEKHEAD,EBMLM_Seekhead)
	DOCOMP(MID_SEGMENTINFO,EBMLM_SIInfo)
	DOCOMP(MID_CLUSTER,EBMLM_Cluster)
	DOCOMP(MID_TRACKS,EBMLM_Tracks)
	DOCOMP(MID_CUES,EBMLM_Cues)
	DOCOMP(MID_ATTACHMENTS,EBMLM_Attachments)
	DOCOMP(MID_TAGS,EBMLM_Tags)
	DOCOMPL(MID_CHAPTERS,EBMLM_Chapters)
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
		return SegmentInfo->pActiveCluster=pCluster;
	} else {
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
	EBMLELEMENTLIST*	e_ChapterAtoms = NULL;
	CHAPTERS*			subchaps;
	int					i;
	EBMLElement*		e_next=NULL, *e_old = NULL;
	EBMLELEMENTLIST*	e_Display = NULL;
	EBMLELEMENTLIST*	e_String = NULL;
	EBMLELEMENTLIST*	e_Lng = NULL;


	newz(CHAPTER_INFO, 1, *pInfo);
	pMaster->Search((void**)&e_ChapterAtoms,(char*)MID_CH_CHAPTERATOM,NULL);
	if (e_ChapterAtoms->iCount) {
		newz(CHAPTERS,1,(*pInfo)->subchapters);
		subchaps = (CHAPTERS*)(*pInfo)->subchapters;
		subchaps->iCount=e_ChapterAtoms->iCount;
		newz(CHAPTER_INFO*,e_ChapterAtoms->iCount,subchaps->chapters);
		for (i=0;i<e_ChapterAtoms->iCount;i++) {
			RetrieveSubChapters(e_ChapterAtoms->pElement[i],&subchaps->chapters[i]);
		}
	}

	(*pInfo)->bEnabled = 1;
	(*pInfo)->bHidden = 0;
	(*pInfo)->iTimeend = -1;
	pMaster->Create1stSub(&e_next);
	while (e_next) {
		e_old = e_next;
		switch (e_old->GetType()) {
			case ETM_CH_CHAPTERUID: (*pInfo)->iUID = (int)e_old->AsInt(); break;
			case ETM_CH_CHAPTERTIMESTART: (*pInfo)->iTimestart = e_old->AsInt(); break;
			case ETM_CH_CHAPTERTIMEEND: (*pInfo)->iTimeend = e_old->AsInt(); break;
			case ETM_CH_CHAPTERFLAGENABLED: (*pInfo)->bEnabled = (int)e_old->AsInt(); break;
			case ETM_CH_CHAPTERFLAGHIDDEN: (*pInfo)->bHidden = (int)e_old->AsInt(); break;
			case ETM_CH_CHAPTERTRACK: 
				{
					EBMLELEMENTLIST* e_tracks = NULL;
					e_old->Search((void**)&e_tracks,(char*)MID_CH_CHAPTERTRACKNUMBER);
					(*pInfo)->tracks.iCount = e_tracks->iCount;
					if (!e_tracks->iCount) {
						B0rked("ChapterTrack does not contain any ChapterTrackNumbers");
					} else {
						(*pInfo)->tracks.iTracks = (int*)calloc(e_tracks->iCount,sizeof(int));
						for (i=0;i<(*pInfo)->tracks.iCount;i++) {
							(*pInfo)->tracks.iTracks[i] = (int)e_tracks->pElement[i]->AsInt();
						}
						DeleteElementList(&e_tracks);
					}
				};
				break;
		}
		e_next = e_old->GetSucc();
		DeleteEBML(&e_old);
	}

	// Chapter display elements present?
	pMaster->Search((void**)&e_Display,(char*)MID_CH_CHAPTERDISPLAY,NULL);
	if (e_Display->iCount) {
		(*pInfo)->display.iCount = e_Display->iCount;
		newz(CHAPTER_DISPLAY_INFO*, e_Display->iCount, (*pInfo)->display.cDisp);
		for (i=0;i<e_Display->iCount;i++) {
			newz(CHAPTER_DISPLAY_INFO, 1, (*pInfo)->display.cDisp[i]);
			e_Display->pElement[i]->Search((void**)&e_String,(char*)MID_CH_CHAPSTRING,NULL);
			e_Display->pElement[i]->Search((void**)&e_Lng,(char*)MID_CH_CHAPLANGUAGE,NULL);
			if (e_String->iCount) {
				(*pInfo)->display.cDisp[i]->cString = new CStringBuffer(e_String->pElement[0]->GetData()->AsString(), CSB_UTF8 | CBN_REF1);
			}
			if (e_Lng->iCount) {
				(*pInfo)->display.cDisp[i]->cLanguage = new CStringBuffer(e_Lng->pElement[0]->GetData()->AsString(), CSB_UTF8 | CBN_REF1);
			} else {
				(*pInfo)->display.cDisp[i]->cLanguage = new CStringBuffer("eng");
			}

		}
	} else {
		(*pInfo)->display.iCount = 1;
		newz(CHAPTER_DISPLAY_INFO*, 1, (*pInfo)->display.cDisp);
		newz(CHAPTER_DISPLAY_INFO, 1, (*pInfo)->display.cDisp[0]);
		(*pInfo)->display.cDisp[0]->cString = new CStringBuffer("");
		(*pInfo)->display.cDisp[0]->cLanguage = new CStringBuffer("");
	}

	if ((*pInfo)->iTimeend<(*pInfo)->iTimestart) {
		if ((*pInfo)->iTimeend != -1) { 
			B0rked("temporal inversion: chapter ends earlier than it starts");
		} else {
			Note("chapter end has not been written");
		}
	}
	if (e_ChapterAtoms) DeleteElementList(&e_ChapterAtoms);
	if (e_Display) DeleteElementList(&e_Display);
	if (e_String) DeleteElementList(&e_String);

}

void EBMLM_Segment::RetrieveChapters(EBMLELEMENTLIST** e_Chapters)
{
	EBMLElement*		e_Chap = NULL;
	EBMLELEMENTLIST*	e_EditionEntries = NULL;
	EBMLELEMENTLIST**	e_ChapterAtoms = NULL;
	int					i, j, k, iChapCount = 0;

	if (*e_Chapters) {
		if ((*e_Chapters)->iCount)	{
			SegmentInfo->chapters = (CHAPTERS*)calloc(1,sizeof(CHAPTERS));
			e_Chap = (*e_Chapters)->pElement[0];
			e_Chap->Search((void**)&e_EditionEntries,(char*)MID_CH_EDITIONENTRY,NULL);
			if (!e_EditionEntries->iCount) {
				B0rked("Chapters without EditionEntries");
			}

			e_ChapterAtoms = (EBMLELEMENTLIST**)calloc(e_EditionEntries->iCount,sizeof(EBMLELEMENTLIST*));
			for (i=0;i<e_EditionEntries->iCount;i++) {
				e_EditionEntries->pElement[i]->Search((void**)&e_ChapterAtoms[i],(char*)MID_CH_CHAPTERATOM,NULL);
				iChapCount+=e_ChapterAtoms[i]->iCount;
				if (!e_ChapterAtoms[i]->iCount) {
					B0rked("EditionEntry without ChapterAtom");
				}
			}

			SegmentInfo->chapters->chapters = (CHAPTER_INFO**)calloc(iChapCount,sizeof(CHAPTER_INFO*));
			SegmentInfo->chapters->iCount = iChapCount;
			k=0;
			for (i=0;i<e_EditionEntries->iCount;i++) {
				for (j=0;j<e_ChapterAtoms[i]->iCount;j++) {
					RetrieveSubChapters(e_ChapterAtoms[i]->pElement[j],&SegmentInfo->chapters->chapters[k++]);
				}
			}
		}
		DeleteElementList(e_Chapters);
		DeleteElementList(e_ChapterAtoms);
	}
}

void EBMLM_Segment::RetrieveCues(EBMLELEMENTLIST** e_Cues)
{
	EBMLElement*		e_Cue = NULL;
	EBMLELEMENTLIST*	e_CuePoints = NULL;
	EBMLELEMENTLIST*	e_CueTime = NULL;
	EBMLELEMENTLIST*	e_CueTrackPositions = NULL;
	EBMLElement*		e_next, *e_old;
	int					i, j, k;

	if (*e_Cues && (*e_Cues)->iCount) {
		// do cues exist?
		SegmentInfo->iSeekMode = SEEKMODE_NORMAL;
		e_Cue = (*e_Cues)->pElement[0];
		e_Cue->Search((void**)&e_CuePoints,(char*)MID_CU_CUEPOINT,NULL);
		if (!e_CuePoints->iCount) {
			B0rked("Cues don't contain Cuepoints");
		}
		CUES*		cues = SegmentInfo->cues = (CUES*)calloc(1,sizeof(CUES));
		CUE_POINT*	cue_point = NULL;
		cues->iCuePoints = e_CuePoints->iCount;
		cues->pCuePoints = (CUE_POINT**)calloc(cues->iCuePoints,sizeof(CUE_POINT*));

		// go through all cue points
		for (i=0;i<cues->iCuePoints;i++) {
			cue_point = cues->pCuePoints[i] = (CUE_POINT*)calloc(1,sizeof(CUE_POINT));
			EBMLElement* e_CuePoint = e_CuePoints->pElement[i];
			e_CuePoint->Search((void**)&e_CueTime,(char*)MID_CU_CUETIME,NULL);
			if (e_CueTime->iCount!=1) {
				B0rked("not exactly 1 CueTime in CuePoint");
			} else {
				cue_point->qwTimecode = e_CueTime->pElement[0]->AsInt();
				e_CuePoint->Search((void**)&e_CueTrackPositions,(char*)MID_CU_CUETRACKPOSITIONS);
				if (!e_CueTrackPositions->iCount) {
					B0rked("CuePoint without CueTrackPositions");
				} else {
					cue_point->iTracks = e_CueTrackPositions->iCount;
					cue_point->pTracks = (CUE_TRACKINFO**)calloc(cue_point->iTracks,sizeof(CUE_TRACKINFO*));
					
					for (j=0;j<cue_point->iTracks;j++) {
						EBMLElement* e_track = e_CueTrackPositions->pElement[j];
						e_track->Create1stSub(&e_next);
						CUE_TRACKINFO*	track = cue_point->pTracks[j] = (CUE_TRACKINFO*)calloc(1,sizeof(CUE_TRACKINFO));
						while (e_next) {
							e_old = e_next;
							switch (e_old->GetType()) {
								case ETM_CU_CUETRACK: track->iTrack = TrackNumber2Index((int)e_old->AsInt()); break;
								case ETM_CU_CUECLUSTERPOSITION: track->qwClusterPos = e_old->AsInt(); break;
								case ETM_CU_CUEBLOCKNUMBER: track->iBlockNbr = (int)e_old->AsInt(); break;
							}
							e_next = e_next->GetSucc();
							DeleteEBML(&e_old);
						}
						SegmentInfo->tracks->track_info[track->iTrack]->cue_points->iCount++;
						EBMLELEMENTLIST* e_refs = NULL;
						e_track->Search((void**)&e_refs,(char*)MID_CU_CUEREFERENCE);
						track->iRefCount = e_refs->iCount;
						if (e_refs->iCount) {
							track->pReferences = (CUE_REFERENCE**)calloc(track->iRefCount,sizeof(CUE_REFERENCE*));
						}
						for (k=0;k<track->iRefCount;k++) {
							CUE_REFERENCE* ref = track->pReferences[k] = (CUE_REFERENCE*)calloc(1,sizeof(CUE_REFERENCE));
							e_refs->pElement[0]->Create1stSub(&e_next);
							while (e_next) {
								e_old = e_next;
								switch (e_old->GetType()) {
									case ETM_CU_CUEREFCLUSTER: ref->qwRefClusterPos = e_old->AsInt(); break;
									case ETM_CU_CUEREFNUMBER: ref->iRefNumber = (int)e_old->AsInt(); break;
									case ETM_CU_CUEREFTIME: ref->qwRefTime = e_old->AsInt(); break;
									case ETM_CU_CUEREFCODECSTATE: ref->qwRefCodecStatePos = e_old->AsInt(); break;
								}
								e_next = e_next->GetSucc();
								DeleteEBML(&e_old);
							}
						}
						if (e_refs) DeleteElementList(&e_refs);
					}
				}
			}
		}
		
	} else {
		if (SegmentInfo->clusters->iCount) {
			Warning("No cues present. Seeking in the file might be slow");
		} else {
			Warning("Neither cues nor index to clusters present. Seeking will most likely be slow");
			SegmentInfo->iSeekMode = SEEKMODE_IDIOT;
		}
	}

	if (e_CuePoints) DeleteElementList(&e_CuePoints);
	if (e_CueTrackPositions) DeleteElementList(&e_CueTrackPositions);
	if (e_CueTime) DeleteElementList(&e_CueTime);
	if (*e_Cues) DeleteElementList(e_Cues);
}

int EBMLM_Segment::RetrieveLastCuepoint(int iTrack, int* index2)
{
	int count = GetCues()->iCuePoints;
	CUE_POINT* cuepoint;
	if (index2) *index2=-1;

	for (int i=count-1;i>=0;i--) {
		cuepoint = GetCues()->pCuePoints[i];
		for (int j=0;j<cuepoint->iTracks;j++) {
			if (cuepoint->pTracks[j]->iTrack == iTrack) {
				if (index2) *index2 = j;
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

	if (!GetCues()) return (t->iLastBlockEndTimecode = TIMECODE_UNKNOWN);
	if (t->iLastBlockEndTimecode == TIMECODE_UNKNOWN) return TIMECODE_UNKNOWN;
	if (t->iLastBlockEndTimecode != TIMECODE_UNINITIALIZED) return t->iLastBlockEndTimecode;

	index1 = RetrieveLastCuepoint(iTrack, &index2);

	if (index1 == -1 || index2 == -1) return (t->iLastBlockEndTimecode = TIMECODE_UNKNOWN);

	CUE_POINT    * cpt = GetCues()->pCuePoints[index1];
	CUE_TRACKINFO* cti = cpt->pTracks[index2];
	
	Seek(cpt->qwTimecode);

	READBLOCK_INFO  rbi;
	while (ReadBlock(&rbi)!=READBL_ENDOFSEGMENT) {
		if (TrackNumber2Index(rbi.iStream) == iTrack) {
			if (rbi.qwDuration) {
				iCurrent = rbi.qwTimecode + rbi.qwDuration;
				//t->iLastBlockEndTimecode = rbi.qwTimecode + rbi.qwDuration;
			} else {
				if (rbi.iFrameCountInLace<2) {
					iCurrent = rbi.qwTimecode + t->qwDefaultDuration / SegmentInfo->iTimecodeScale;
				} else {
					iCurrent = rbi.qwTimecode + t->qwDefaultDuration * rbi.iFrameCountInLace / SegmentInfo->iTimecodeScale;
				}
			}
			if (t->iLastBlockEndTimecode < iCurrent) t->iLastBlockEndTimecode = iCurrent;
		}
		DecBufferRefCount(&(rbi.cData));
		if (rbi.cFrameSizes) DecBufferRefCount(&rbi.cFrameSizes);
	}

	if (t->iLastBlockEndTimecode == TIMECODE_UNINITIALIZED) {
		t->iLastBlockEndTimecode = TIMECODE_UNKNOWN;
	}
	return t->iLastBlockEndTimecode;
}

int EBMLM_Segment::TrackNumber2Index(int iNbr)
{
	SEGMENT_INFO* seg = SegmentInfo;
	TRACKS*		  tra = seg->tracks;
	int			  i;

	if (tra->iTable[iNbr-1] == iNbr) return iNbr-1;

	for (i=0;i<tra->iCount;i++) {
		if (tra->iTable[i] == iNbr) return i;
	}

	return TN2I_INVALID;
}

// Takes O(n^3) time for many doubles!!!
void RemoveDoubles(EBMLELEMENTLIST* e, char* cB0rkText)
{
	int i,j,k;

	bool bRepaired;
	if (e) do {
		bRepaired = false;
		for (i=0;i<e->iCount && !bRepaired;i++) {
			for (j=i+1;j<e->iCount && !bRepaired;j++) {
				if (e->pElement[i]->GetStreamPos() == e->pElement[j]->GetStreamPos()) {
					bRepaired = true;
					Warning(cB0rkText);
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

// pointer to list and element description
int EBMLElement::InsertElement(void** _e, EBMLElement* seg, char* ID, EBMLElement* pos)
{
	EBMLELEMENTLIST** e = (EBMLELEMENTLIST**)_e;
	char* created_id;
	created_id = new char[5];
	ZeroMemory(created_id,sizeof(created_id));

	if (!*e) {
		newz(EBMLELEMENTLIST, 1, *e);
	}
	seg->SeekStream(pos->AsInt());
	(*e)->pElement = (EBMLElement**)realloc((*e)->pElement,
		((*e)->iCount+1)*sizeof(EBMLElement));

	seg->Create(&(*e)->pElement[(*e)->iCount++], (char**)&created_id);

	if (pos->CompIDs(ID, (char*)created_id)) {
		delete created_id;
		return 1;
	} else {
		delete created_id;
		(*e)->iCount--;
		return -1;
	}
}

// Output error about b0rked seekhead entry to stderr
void BadSeekheadError(int i, int j, char* cID)
{
	char msg[100]; msg[0] = 0;
	sprintf(msg, "Entry %d of Seek head %d (SeekID: %02X %02X %02X %02X) pointing to bad position!"
		         ,j+1, i+1, cID[0] & 0xFF, cID[1] & 0xFF, cID[2] & 0xFF, cID[3] & 0xFF);
	B0rked(msg);
}

void msg() {
	MessageBox(NULL, "Ping", "", MB_OK);
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
		e_SIInfo = e_SIList->pElement[0];
		e_SIInfo->Create1stSub(&e_next);
		while (e_next) {
			e_old = e_next;
			cb = e_old->GetData();
			switch (e_old->GetType()) {
				case ETM_SI_DURATION: SegmentInfo->fDuration = e_old->AsFloat(); break;
				case ETM_SI_MUXINGAPP: cb->Refer(&SegmentInfo->cMuxingApp); break;
				case ETM_SI_WRITINGAPP: cb->Refer(&SegmentInfo->cWritingApp); break;
				case ETM_SI_TIMECODESCALE: SegmentInfo->iTimecodeScale = (int)e_old->AsInt(); break;
				case ETM_SI_TITLE: 
					SegmentInfo->cTitle=new CStringBuffer(cb->AsString(), CBN_REF1 | CSB_UTF8);
					break;
				case ETM_SI_SEGMENTUID: cb->Refer(&SegmentInfo->CurrSeg.cUID); break;
				case ETM_SI_SEGMENTFILENAME: cb->Refer(&SegmentInfo->CurrSeg.cFilename); break;
				case ETM_SI_PREVUID: cb->Refer(&SegmentInfo->PrevSeg.cUID); break;
				case ETM_SI_PREVFILENAME: cb->Refer(&SegmentInfo->PrevSeg.cFilename); break;
				case ETM_SI_NEXTUID: cb->Refer(&SegmentInfo->NextSeg.cUID); break;
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

		DeleteElementList(&e_SIList);
	}
}

int EBMLM_Segment::RetrieveInfo()
{
	EBMLElement*	e_next, *e_old;
	EBMLELEMENTLIST**	e_TrackEntries = NULL;
	
	EBMLELEMENTLIST**	e_Seekhead = &SegmentInfo->pGlobElem->e_Seekhead;
	EBMLELEMENTLIST**	e_Seek = NULL;
	EBMLELEMENTLIST*	e_SeekID = NULL;
	EBMLELEMENTLIST*	e_SeekPosition = NULL; 
	EBMLELEMENTLIST**	e_Cues = &SegmentInfo->pGlobElem->e_Cues;
	EBMLELEMENTLIST**	e_Chapters = &SegmentInfo->pGlobElem->e_Chapters;
	EBMLELEMENTLIST**	e_Tags = &SegmentInfo->pGlobElem->e_Tags;
	EBMLELEMENTLIST**	e_Tag = NULL;
	EBMLELEMENTLIST**	e_Tracks = &SegmentInfo->pGlobElem->e_Tracks;
	EBMLELEMENTLIST**	e_Tracks2 = &SegmentInfo->pGlobElem->e_Tracks2;
	EBMLELEMENTLIST**	e_SIList = &SegmentInfo->pGlobElem->e_SegmentInfo;


	int					iChapCount = 0;
	float			f;
	int				i,j,k,l;
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
	// unless obfuscated files are allowed
	cStop = (bAllowObfuscatedSegments)?NULL:(char*)MID_CLUSTER;

	// Seekhead Info
	SegmentInfo->clusters = (CLUSTERS*)calloc(1,sizeof(CLUSTERS));

	// Search Seekheads and Tracks
	char* cid_Seekhead_Tracks[] = { (char*)MID_MS_SEEKHEAD, (char*)MID_TRACKS };
	void** pe_Sht = NULL;
	SearchMulti(&pe_Sht, cid_Seekhead_Tracks, 2, (char*)MID_CLUSTER);
	*e_Seekhead	= (EBMLELEMENTLIST*)pe_Sht[0];
	*e_Tracks2	= (EBMLELEMENTLIST*)pe_Sht[1];
	delete pe_Sht;

	if ((*e_Seekhead)->iCount>1 && !bAllowObfuscatedSegments) {
		Note("found more than one Seekhead");
	}
	// Seekhead found
	if ((*e_Seekhead)->iCount) {
		e_Seek = (EBMLELEMENTLIST**)calloc((*e_Seekhead)->iCount,sizeof(EBMLELEMENTLIST*));

		for (i=0;i<(*e_Seekhead)->iCount;i++) {
			(*e_Seekhead)->pElement[i]->Search((void**)&e_Seek[i],(char*)MID_MS_SEEK,NULL);
			// Seek found?
			if (!e_Seek[i]->iCount) {
				B0rked("Seekhead without Seek");
			} else {
				for (j=0;j<e_Seek[i]->iCount;j++) {
					
					char* cidSeek[] = { (char*)MID_MS_SEEKID, (char*)MID_MS_SEEKPOSITION };
					EBMLELEMENTLIST** pe_Seek = NULL;

					e_Seek[i]->pElement[j]->SearchMulti((void***)&pe_Seek, cidSeek, 2);
					e_SeekID = pe_Seek[0];
					e_SeekPosition = pe_Seek[1];

					// exactly one SeekID?
					if (e_SeekID->iCount != 1) {
						B0rked("invalid number of SeekIDs in Seek");
					} else {
						// exactly one SeekPosition?
						if (e_SeekPosition->iCount != 1) {
							B0rked("invalid number of SeekPosition in Seek");
						} else {
							// Cluster
							if (CompIDs(e_SeekID->pElement[0]->GetData()->AsString(),(char*)MID_CLUSTER)) {
								if (SegmentInfo->clusters->iCount>=iMaxClusters) {
									SegmentInfo->clusters->cluster_info = (CLUSTER_PROP**)realloc(
										SegmentInfo->clusters->cluster_info,(iMaxClusters+=10)*sizeof(CLUSTER_PROP*));
								}
								CLUSTER_PROP* ci \
									= SegmentInfo->clusters->cluster_info[SegmentInfo->clusters->iCount++] \
									= (CLUSTER_PROP*)calloc(1,sizeof(CLUSTER_PROP));
								ci->iValid |= CLIV_POS;
								ci->qwPosition = e_SeekPosition->pElement[0]->AsInt();
							} else 
							// Seekhead
							if (CompIDs(e_SeekID->pElement[0]->GetData()->AsString(),(char*)MID_MS_SEEKHEAD)) {
								if (InsertElement((void**)e_Seekhead, this, (char*)MID_MS_SEEKHEAD, e_SeekPosition->pElement[0])==1){
									e_Seek = (EBMLELEMENTLIST**)realloc(e_Seek,(*e_Seekhead)->iCount*sizeof(EBMLELEMENTLIST*));
									e_Seek[(*e_Seekhead)->iCount-1] = NULL;
									Note("Seekhead contains link to another Seekhead");
								} else {
									BadSeekheadError(i,j, (char*)MID_MS_SEEKHEAD);
								}

							} else
							// Cues
							if (CompIDs(e_SeekID->pElement[0]->GetData()->AsString(),(char*)MID_CUES)) {
								if (InsertElement((void**)e_Cues,this,(char*)MID_CUES, e_SeekPosition->pElement[0])==-1) {
									BadSeekheadError(i,j,(char*)MID_CUES);
								}
							} else
							// Segment Info
							if (CompIDs(e_SeekID->pElement[0]->GetData()->AsString(),(char*)MID_SEGMENTINFO)) {
								if (InsertElement((void**)&SegmentInfo->pGlobElem->e_SegmentInfo,this,
									(char*)MID_SEGMENTINFO,e_SeekPosition->pElement[0])==-1) {
									BadSeekheadError(i,j,(char*)MID_SEGMENTINFO);
								}
							} else
							// Chapters
							if (CompIDs(e_SeekID->pElement[0]->GetData()->AsString(),(char*)MID_CHAPTERS)) {
								if (InsertElement((void**)e_Chapters, this, (char*)MID_CHAPTERS, e_SeekPosition->pElement[0])==-1) {
									BadSeekheadError(i,j,(char*)MID_CHAPTERS);
								}
							} else
							// Tracks
							if (CompIDs(e_SeekID->pElement[0]->GetData()->AsString(),(char*)MID_TRACKS)) {
								if (InsertElement((void**)e_Tracks, this, (char*)MID_TRACKS, e_SeekPosition->pElement[0])==-1) {
									BadSeekheadError(i,j,(char*)MID_TRACKS);
								}
							} else
							// Tags
							if (CompIDs(e_SeekID->pElement[0]->GetData()->AsString(),(char*)MID_TAGS)) {
								if (InsertElement((void**)e_Tags, this, (char*)MID_TAGS, e_SeekPosition->pElement[0])==-1) {
									BadSeekheadError(i,j,(char*)MID_TAGS);
								}
							}
						}
					}
					DeleteElementLists(&pe_Seek,2);
				}
			}
		}
	}

	// Segment Info present but not indexed by Seekhead? 
	if (!*e_SIList) {
		Search((void**)e_SIList,(char*)MID_SEGMENTINFO,cStop);
		if ((*e_Seekhead)->iCount && (*e_SIList)->iCount) {
			Warning("Segment Info present but not indexed in existing Seekhead");
		}
	}

	// Segment Info
	RetrieveSegmentInfo(SegmentInfo->pGlobElem->e_SegmentInfo);

	// Chapters
	RemoveDoubles(*e_Chapters, "Found 2 seek elements, pointing at the same Chapters element!");
	RetrieveChapters(&SegmentInfo->pGlobElem->e_Chapters);
	SegmentInfo->cChapters = new CChapters(SegmentInfo->chapters);

	// Tracks
	if (!(*e_Tracks2)->iCount) {
		if (!(*e_Tracks)->iCount) {
			FatalError("No track information found");
			return MSRI_FATALERROR;
		} else {
			B0rked("Track information found by following Seekhead, but not by linear parsing.\n  EBML-File structure is broken!");
		}

		DeleteElementList(e_Tracks2);
	
	} else {
		if (!(*e_Tracks) || !(*e_Tracks)->iCount) {
			*e_Tracks = *e_Tracks2;
			Warning("Track information present, but not indexed in existing Seekhead");
		}
	}


	// find double links to tracks elements (was special VDM b0rk)
	RemoveDoubles(*e_Tracks, "Found 2 seek elements, pointing at the same Tracks element!");
	e_TrackEntries = (EBMLELEMENTLIST**)calloc((*e_Tracks)->iCount,sizeof(EBMLELEMENTLIST*));

	for (i=0;i<(*e_Tracks)->iCount;i++) {
		(*e_Tracks)->pElement[i]->Search((void**)&(e_TrackEntries[i]),(char*)MID_TR_TRACKENTRY);
		SegmentInfo->tracks->iCount+=e_TrackEntries[i]->iCount;
	}
	if (!SegmentInfo->tracks->iCount) {
		FatalError("Found 'Tracks', but no 'TrackEntry'");
		return MSRI_FATALERROR;
	}

	table = SegmentInfo->tracks->iTable = (int*)calloc(GetTrackCount(),sizeof(int));
	for (i=0;i<256;i++) SegmentInfo->tracks->iIndexTableByType[i] = (int*)calloc(GetTrackCount(),sizeof(int));
	SegmentInfo->tracks->track_info = (TRACK_INFO**)calloc(GetTrackCount(),sizeof(TRACK_INFO*));
	SegmentInfo->queue = new void*[GetTrackCount()];
	ZeroMemory(SegmentInfo->queue, sizeof(void*)*GetTrackCount());
	k = 0;
	for (i=0;i<(*e_Tracks)->iCount;i++) {
		for (j=0;j<e_TrackEntries[i]->iCount;j++) {
			EBMLELEMENTLIST* e_uid = NULL;
			e_TrackEntries[i]->pElement[j]->Search((void**)&e_uid, (char*)MID_TR_TRACKUID, NULL); //Create1stSub(&e_next);
			int uid;
			if (!e_uid->iCount) {
				B0rked("TrackUID for Track not found");
				uid = k;
			//	return MSRI_FATALERROR;
			} else {
				uid = (int)e_uid->pElement[0]->AsInt();
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
				newz(TRACK_INFO,1,SegmentInfo->tracks->track_info[k]);
				//SegmentInfo->tracks->track_info[k]=(TRACK_INFO*)calloc(1,sizeof(TRACK_INFO));
				t = SegmentInfo->tracks->track_info[k];
				t->iLacing = 1;
				t->cue_points = new TRACK_CUE_POINTS;
				ZeroMemory(t->cue_points, sizeof(TRACK_CUE_POINTS));
				t->iDefault = 1;
				t->iEnabled = 1;
				t->iDataQueued = 0;
				t->iType = -1;
				t->fTimecodeScale = 1;
				t->iMinCache = 0;
				t->iMaxCache = 0;
				t->iNbr = -1;
				t->iSelected = 1;
				t->video.iDisplayWidth = -1;
				t->video.iDisplayHeight = -1;
				t->video.iPixelWidth = -1;
				t->video.iPixelHeight = -1;
				t->audio.iChannels = 1;
				t->audio.fSamplingFrequency = 8000;
				t->audio.fOutputSamplingFrequency = -1;
				t->cLanguage = new CStringBuffer("eng");
				t->iSparse = 0x00000000;
				t->iUID = uid;
				t->iLastBlockEndTimecode = TIMECODE_UNINITIALIZED;
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
			EBMLELEMENTLIST* e_uid = NULL;
			e_TrackEntries[i]->pElement[j]->Search((void**)&e_uid, (char*)MID_TR_TRACKUID, NULL);
			int uid;
			if (e_uid->iCount) {
				uid = (int)e_uid->pElement[0]->AsInt();
				DeleteElementList(&e_uid);
			} else {
				uid = k;
			}
			int index = -1;
			for (int l=0;l<total_track_count;l++) {
				t = SegmentInfo->tracks->track_info[l];
				if (t->iUID == uid) {
					index = l;
				}
			}
			t = SegmentInfo->tracks->track_info[index];
			e_TrackEntries[i]->pElement[j]->Create1stSub(&e_next);
			while (e_next) {
				e_old = e_next;
				switch (e_old->GetType()) {
					case ETM_TR_TRACKTYPE: 
						t->iType = (int)e_old->AsInt();
						if (t->iType == MSTRT_AUDIO || t->iType == MSTRT_VIDEO) {
							t->iSparse = 0;
						} else t->iSparse = 1;
						
						break;

					case ETM_TR_TRACKNUMBER: table[k] = (int)e_old->AsInt(); t->iNbr = table[k]; break;
					case ETM_TR_FLAGDEFAULT: t->iDefault = (int)e_old->AsInt(); break;
					case ETM_TR_FLAGLACING: t->iLacing = (int)e_old->AsInt(); break;
					case ETM_TR_FLAGENABLED: t->iEnabled = (int)e_old->AsInt(); break;
					case ETM_TR_NAME: e_old->GetData()->Refer(&t->cName); break;
					case ETM_TR_MINCACHE: t->iMinCache = (int)e_old->AsInt(); break;
					case ETM_TR_MAXCACHE: t->iMaxCache = (int)e_old->AsInt(); break;
					case ETM_TR_DEFAULTDURATION: t->qwDefaultDuration = (int)e_old->AsInt(); break;
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
							EBMLElement* e_Video = e_old;
							EBMLElement* e_VOld = NULL, *e_VNext = NULL;
							e_Video->Create1stSub(&e_VNext);
							while (e_VNext) {
								e_VOld = e_VNext;
								switch (e_VOld->GetType()) {
									case ETM_TRV_FLAGINTERLACED: t->video.iInterlaced = (int)e_VOld->AsInt(); break;
									case ETM_TRV_COLOURSPACE: t->video.iColorSpace = (int)e_old->AsInt(); break;
									case ETM_TRV_STEREOMODE: t->video.iStereoMode = (int)e_VOld->AsInt(); break;
									case ETM_TRV_PIXELWIDTH: t->video.iPixelWidth = (int)e_VOld->AsInt(); break;
									case ETM_TRV_PIXELHEIGHT: t->video.iPixelHeight = (int)e_VOld->AsInt(); break;
									case ETM_TRV_DISPLAYWIDTH: t->video.iDisplayWidth = (int)e_VOld->AsInt(); break;
									case ETM_TRV_DISPLAYHEIGHT: t->video.iDisplayHeight = (int)e_VOld->AsInt(); break;
									case ETM_TRV_DISPLAYUNIT: t->video.iDisplayUnit = (int)e_VOld->AsInt(); break;
									case ETM_TRV_GAMMAVALUE: t->video.fGamma = (float)e_VOld->AsFloat(); break;
								}
								e_VNext = e_VNext->GetSucc();
								DeleteEBML(&e_VOld);
							} 
						} break;
					case ETM_TR_AUDIO:
						{
							EBMLElement* e_Audio = e_old;
							EBMLElement* e_AOld = NULL, *e_ANext = NULL;
							e_Audio->Create1stSub(&e_ANext);
							while (e_ANext) {
								e_AOld = e_ANext;
								switch (e_ANext->GetType()) {
									case ETM_TRA_SAMPLINGFREQUENCY: t->audio.fSamplingFrequency = (float)e_AOld->AsFloat(); break;
									case ETM_TRA_OUTPUTSAMPLINGFREQUENCY: t->audio.fOutputSamplingFrequency = (float)e_AOld->AsFloat(); break;
									case ETM_TRA_CHANNELS: t->audio.iChannels = (int)e_AOld->AsInt(); break;
									case ETM_TRA_CHANNELPOSITIONS: e_AOld->GetData()->Refer(&t->audio.cChannelPositions); break;
									case ETM_TRA_BITDEPTH: t->audio.iBitDepth = (int)e_AOld->AsInt(); break;
								}
								
								e_ANext = e_ANext->GetSucc();
								DeleteEBML(&e_AOld);
							}
						} break;
					case ETM_TR_CONTENTENCODINGS:
						{
							EBMLELEMENTLIST* e_ContentEncoding = NULL;
							e_old->Search((void**)&e_ContentEncoding, (char*)MID_TRCE_CONTENTENCODING, NULL);
							if (e_ContentEncoding->iCount!=1) break;

							char* cid_Cont[] = {	(char*)MID_TRCE_CONTENTENCODINGORDER, 
													(char*)MID_TRCE_CONTENTENCODINGTYPE,
													(char*)MID_TRCE_CONTENTENCODINGSCOPE,
													(char*)MID_TRCE_CONTENTCOMPRESSION	};
							EBMLELEMENTLIST** e_CECont = NULL;
							e_ContentEncoding->pElement[0]->SearchMulti((void***)&e_CECont, cid_Cont, 4, NULL);
							EBMLELEMENTLIST* e_CEOrder = e_CECont[0];
							EBMLELEMENTLIST* e_CEType  = e_CECont[1];
							EBMLELEMENTLIST* e_CEScope = e_CECont[2];
							EBMLELEMENTLIST* e_CECompr = e_CECont[3];

							if (e_CEOrder->iCount == 1 && e_CEScope->iCount == 1 && e_CEType->iCount == 1
								&& e_CECompr->iCount == 1 && e_CEOrder->pElement[0]->AsInt() == 0 &&
								e_CEScope->pElement[0]->AsInt() == 1 && e_CEType->pElement[0]->AsInt() == 0)
							{
								EBMLELEMENTLIST* e_ComprAlgo = NULL;
								e_CECompr->pElement[0]->Search((void**)&e_ComprAlgo, (char*)MID_TRCE_CONTENTCOMPALGO, NULL);
								if (e_ComprAlgo->iCount == 1) {
									switch (e_ComprAlgo->pElement[0]->AsInt()) {
										case 0: t->iCompression = COMP_ZLIB;
									}
								}
								DeleteElementList(&e_ComprAlgo);
							}
							
							DeleteElementLists(&e_CECont, 4);
							DeleteElementList(&e_ContentEncoding);
							
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

			if (t->audio.fOutputSamplingFrequency<0) t->audio.fOutputSamplingFrequency = t->audio.fSamplingFrequency;
		}
	}

	for (i=0;i<GetTrackCount();i++) {
		int type = SegmentInfo->tracks->track_info[i]->iType;
		if (SegmentInfo->iMasterStream == -1 && type == MSTRT_VIDEO) SegmentInfo->iMasterStream = i; 
		SegmentInfo->tracks->iIndexTableByType[type][SegmentInfo->tracks->iCountByType[type]++] = i;
//		SegmentInfo->tracks->iCountByType[type]++;
	}

	// if all streams are sparse: bad

	// Cues
	RemoveDoubles(*e_Cues, "Found 2 seek elements, pointing at the same Cues element!");

	// tags
	RemoveDoubles(*e_Tags, "Found 2 seek elements, pointing at the same Tags element!");
	if (*e_Tags) {
		e_Tag = new EBMLELEMENTLIST*[(*e_Tags)->iCount];
		ZeroMemory(e_Tag,(*e_Tags)->iCount*sizeof(EBMLELEMENTLIST*));
		for (i=0;i<(*e_Tags)->iCount;i++) {
			// search for Tag
			(*e_Tags)->pElement[i]->Search((void**)&e_Tag[i],(char*)MID_TG_TAG,NULL);
			for (j=0;j<e_Tag[i]->iCount;j++) {
				
				// search for Target and SimpleTags
				EBMLELEMENTLIST** pe_TagCont = NULL;
				char* cTagCont[] = { (char*)MID_TG_TARGET, (char*)MID_TG_SIMPLETAG };
				e_Tag[i]->pElement[j]->SearchMulti((void***)&pe_TagCont, cTagCont, 2, NULL);
				EBMLELEMENTLIST* e_Target     = pe_TagCont[0];
				EBMLELEMENTLIST* e_SimpleTags = pe_TagCont[1];
				EBMLELEMENTLIST* e_TrackUID   = NULL;

				CDynIntArray* aTracks = new CDynIntArray;
				if (!e_Target->iCount) {
					B0rked("Tag does not contain Target");
				} else {
					// find all track IDs
					e_Target->pElement[0]->Search((void**)&e_TrackUID,(char*)MID_TG_TRACKUID,NULL);
					// make array of track indexes from track uids
					for (k=0;k<e_TrackUID->iCount;k++) {
						int iIndex = UID2TrackIndex((int)e_TrackUID->pElement[k]->AsInt());
						if (iIndex == U2TN_INVALID) {
							B0rked("invalid TrackUID in Target");
						} else {
							aTracks->Insert(iIndex);
						}
					}
					
					EBMLElement* e_next, *e_old;
					TRACK_INFO* t;
					e_Target->pElement[0]->Create1stSub(&e_next);
					// go through tags and look for bitsps and framesps
					while (e_next) {
						e_old = e_next;
						for (k=0;k<aTracks->GetCount();k++) {
							t = SegmentInfo->tracks->track_info[aTracks->At(k)];
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
					}
				}

				for (k=0;k<e_SimpleTags->iCount;k++) {
					// search for TagName and TagString
					EBMLELEMENTLIST** pe_Tags = NULL;
					char* cTags[] = { (char*)MID_TG_TAGNAME, (char*)MID_TG_TAGSTRING };
					e_SimpleTags->pElement[k]->SearchMulti((void***)&pe_Tags, cTags, 2, NULL);
					EBMLELEMENTLIST* e_TagName = pe_Tags[0];
					EBMLELEMENTLIST* e_TagString = pe_Tags[1];
					if (e_TagName->iCount && e_TagString->iCount) {
						if (!strcmp(e_TagName->pElement[0]->GetData()->AsString(), "BITSPS")
							|| !strcmp(e_TagName->pElement[0]->GetData()->AsString(), "BPS")) {
							__int64 iBPS = atoi(e_TagString->pElement[0]->GetData()->AsString());
							for (l=0;l<aTracks->GetCount();l++) {
								t = SegmentInfo->tracks->track_info[aTracks->At(l)];
								t->tags.iFlags |= TRACKTAGS_BITSPS;
								t->tags.iBitsPS = iBPS;
							}
						} else {
							// generic tags without any meaning
							if (!e_Target->iCount || !e_TrackUID || !e_TrackUID->iCount) {
								// global Tag
								tagAddIndex(&SegmentInfo->pGlobalTags, 	tagAdd(&SegmentInfo->pAllTags, 
									e_TagName->pElement[0]->GetData()->AsString(),
									e_TagString->pElement[0]->GetData()->AsString()));
							}
						}
					}
					DeleteElementList(&e_TagName);
					DeleteElementList(&e_TagString);
					delete pe_Tags;
				}

				DeleteElementList(&e_Target);
				DeleteElementList(&e_SimpleTags);
				if (e_TrackUID) DeleteElementList(&e_TrackUID);
				delete pe_TagCont;
				aTracks->DeleteAll();
				delete aTracks;
			}
		}
	}


	if (e_TrackEntries) {
		for (i=0;i<(*e_Tracks)->iCount;i++) {
			DeleteElementList(&e_TrackEntries[i]);
		}
		delete e_TrackEntries;
	}

	if (*e_Seekhead) {
		for (i=0;i<(*e_Seekhead)->iCount;i++) {
			DeleteElementList(&e_Seek[i]);
		}
		delete e_Seek;
	}

	DeleteElementList(e_Seekhead);

//	LocateCluster_MetaSeek(0);
//	Seek(0);

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

int EBMLM_Segment::UID2TrackIndex(int iUID)
{
	for (int i=0;i<SegmentInfo->tracks->iCount;i++) {
		if (iUID == SegmentInfo->tracks->track_info[i]->iUID) {
			return i;
		}
	}

	return U2TN_INVALID;
}

int EBMLM_Segment::ReadBlock(READBLOCK_INFO* pInfo)
{
	EBMLM_Cluster*	c;

	c=GetActiveCluster();
	if (c && c->ReadBlock(pInfo)==READBL_ENDOFCLUSTER) {
		if (NextCluster(&c)==NEXTCL_OK) {
			return ReadBlock(pInfo);
		} else return READBL_ENDOFSEGMENT;
	}
	if (!c) return READBL_ENDOFSEGMENT;

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
		c->Delete();
		delete c;

		*p = n;
	} else {
		*p = NULL;
		return NEXTCL_ENDOFSEGMENT;
	}

	return NEXTCL_OK;
}

void EBMLM_Segment::Delete()
{
	int i,j,k;
	TRACK_INFO* t;
	CUE_POINT*  cp;
	CLUSTER_PROP* ci;
	
	if (!SegmentInfo) return;
	// delete all cue info
	if (SegmentInfo->iBlockCount) delete SegmentInfo->iBlockCount;
	if (SegmentInfo->iOtherBlocksThan) delete SegmentInfo->iOtherBlocksThan;
	if (SegmentInfo->queue) delete SegmentInfo->queue;
	if (SegmentInfo->cues) {
		for (i=0;i<SegmentInfo->cues->iCuePoints;i++) {
			cp = SegmentInfo->cues->pCuePoints[i];
			for (k=0;k<cp->iTracks;k++) {
				if (cp->pTracks[k]->pReferences) {
					for (j=0;j<cp->pTracks[k]->iRefCount;j++) {
						delete cp->pTracks[k]->pReferences[j];
					}
					delete cp->pTracks[k]->pReferences;
				}
				delete cp->pTracks[k];
			}
			delete cp->pTracks;
			delete cp;
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
	if (SegmentInfo->cChapters) {
		SegmentInfo->cChapters->Delete();
		delete SegmentInfo->cChapters;
	}

	if (SegmentInfo->tracks) {
		for (i=0;i<SegmentInfo->tracks->iCount;i++) {
			t = SegmentInfo->tracks->track_info[i];
			DecBufferRefCount(&t->cLanguage);
			DecBufferRefCount(&t->cName);
			DecBufferRefCount(&t->audio.cChannelPositions);
			DecBufferRefCount(&t->cCodecID);
			DecBufferRefCount(&t->cCodecName);
			DecBufferRefCount(&t->cCodecPrivate);
		
			delete SegmentInfo->tracks->track_info[i];
		}
	}

	if (SegmentInfo->tracks) {
		if (SegmentInfo->tracks->track_info) delete SegmentInfo->tracks->track_info;
		if (SegmentInfo->tracks->iTable) delete SegmentInfo->tracks->iTable;
		for (i=0;i<256;i++) if (SegmentInfo->tracks->iIndexTableByType[i]) delete SegmentInfo->tracks->iIndexTableByType[i];

		delete SegmentInfo->tracks;
	}
	DecBufferRefCount(&SegmentInfo->cMuxingApp);
	DecBufferRefCount(&SegmentInfo->cTitle);
	DecBufferRefCount(&SegmentInfo->cWritingApp);
	DecBufferRefCount(&SegmentInfo->CurrSeg.cUID);
	DecBufferRefCount(&SegmentInfo->CurrSeg.cFilename);
	DecBufferRefCount(&SegmentInfo->PrevSeg.cUID);
	DecBufferRefCount(&SegmentInfo->PrevSeg.cFilename);
	DecBufferRefCount(&SegmentInfo->NextSeg.cUID);
	DecBufferRefCount(&SegmentInfo->NextSeg.cFilename);
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
					if (curr_clu->pCluster) DeleteEBML(&curr_clu->pCluster);
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

CUES* EBMLM_Segment::GetCues()
{
	if (!SegmentInfo->cues) {
		RetrieveCues(&SegmentInfo->pGlobElem->e_Cues);
	}
	return SegmentInfo->cues;
}

EBMLM_Cluster* EBMLM_Segment::LocateCluster_Cues(__int64 qwTime)
{
	CUES* cues = GetCues();
	// binary search on cues
	int	iMin = 0;
	int iMax = cues->iCuePoints-1;
	int iMid = 0;

	while (iMin != iMax) {
		iMid = (iMin+iMax)>>1;

		if (cues->pCuePoints[iMid]->qwTimecode > qwTime) {
		// need earlier cue point
			iMax = iMid;
		} else 
		// behind last cue point
		if (cues->pCuePoints[iMax]->qwTimecode < qwTime) {
			iMin = iMax;
		} else 
		if (cues->pCuePoints[iMid+1]->qwTimecode < qwTime) {
		// somewhere behind next cue point
			iMin = iMid;
		} else {
		// found!
			iMin = iMid;
			iMax = iMid;
		}
	}

	EBMLM_Cluster* e_Cluster = NULL;
	SeekStream(cues->pCuePoints[iMid]->pTracks[0]->qwClusterPos);
	Create((EBMLElement**)&e_Cluster);

	return SetActiveCluster(e_Cluster);
	//return SegmentInfo->pActiveCluster = e_Cluster;

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
	CUES* cues = GetCues();

	if (SegmentInfo->iSeekMode == SEEKMODE_NORMAL) {
		if (cues && cues->iCuePoints) {
			LocateCluster_Cues(iTimecode);
		} else {
			LocateCluster_MetaSeek(iTimecode);
		}
	} else {
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
	GetCues();
	Seek(-40000);
	if (!InfoPrivate.bShiftFirstClusterToZero & bEnable) {
		if (SegmentInfo->cues) for (j=0;j<SegmentInfo->cues->iCuePoints;j++) {
			SegmentInfo->cues->pCuePoints[j]->qwTimecode -= SegmentInfo->iFirstClusterTimecode;
		}
	} else
	if (InfoPrivate.bShiftFirstClusterToZero & !bEnable) {
		if (SegmentInfo->cues) for (j=0;j<SegmentInfo->cues->iCuePoints;j++) {
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
int tagAdd(TAG_LIST** pTags, char* cName, char* cText)
{
	if (!*pTags) {
		newz(TAG_LIST, 1, *pTags);
		(*pTags)->tags = new TAG*;
		(*pTags)->iCount = 0;
	} else {
		(*pTags)->tags = (TAG**)realloc((*pTags)->tags, sizeof(TAG*) * (*pTags)->iCount);
	}

	TAG** t = &(*pTags)->tags[(*pTags)->iCount++];

	*t = new TAG;

	(*t)->iRef = 1;
	(*t)->iType = TAGTYPE_STRING;
	(*t)->cName = new char[1+strlen(cName)];
	strcpy((*t)->cName, cName);
	(*t)->cData = new char[1+strlen(cText)];
	strcpy((*t)->cData, cText);

	return (*pTags)->iCount-1;
}

int tagAddIndex(TAG_INDEX_LIST** pTagIndexList, int iIndex)
{
	if (!*pTagIndexList) {
		newz(TAG_INDEX_LIST, 1, *pTagIndexList);
		(*pTagIndexList)->iCount = 0;
		(*pTagIndexList)->pIndex = new int;
	} else {
		(*pTagIndexList)->pIndex = (int*)realloc((*pTagIndexList)->pIndex, sizeof(int) * (*pTagIndexList)->iCount);
	}

	int* pIndex = &(*pTagIndexList)->pIndex[(*pTagIndexList)->iCount++];

	*pIndex = iIndex;

	return 0;
}