#include "stdafx.h"
#include "Matroska_Clusters.h"
#include "warnings.h"
#include "integers.h"
#include "Matroska_Block.h"
#include <vector>

#ifdef DEBUG_NEW
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

READBLOCK_INFO::READBLOCK_INFO()
{
	iStream = -1;
	reference_types = 0;
	qwDuration = 0;
	iFlags = 0;
	cData = NULL;
	qwTimecode = 0;
	iEnqueueCount = 0;	
}

EBMLM_Cluster::EBMLM_Cluster(STREAM* s,EBMLElement* p) 
{
	SetStream(s);
	SetParent(p); 
	DetermineLength(); 
	SetType(IDVALUE(MID_CLUSTER)); 
	SetMulti(true); 
	SetDataType(EBMLDATATYPE_MASTER); 
	ClusterInfo=NULL;
	e_CurrentBlockGroup = NULL;
	
	return; 
} 

EBMLM_Cluster::~EBMLM_Cluster()
{
	Delete();
}

void EBMLM_Cluster::Delete()
{
	if (ClusterInfo)
		delete ClusterInfo;
	
	ClusterInfo = NULL;
}

__int64 EBMLM_Cluster::GetDuration()
{
	if (ClusterInfo && ClusterInfo->qwDuration) return ClusterInfo->qwDuration;

	{
		EBMLM_Cluster*	e_next = (EBMLM_Cluster*)FindNext((char*)MID_CLUSTER);
		if (!ClusterInfo) {
			RetrieveInfo();
		}
		ClusterInfo->qwDuration=e_next->GetTimecode()-ClusterInfo->qwTimecode;
		if (e_next) DeleteEBML(&e_next);
		return ClusterInfo->qwDuration;
	}
}

char* EBMLM_Cluster::GetTypeString() 
{ 
    return "Cluster"; 
}

// read a block
int EBMLM_Cluster::ReadBlock(READBLOCK_INFO* pDest)
{
	EBMLM_CLBlockGroup*	e_BG	 = NULL; // not allocated
	EBMLM_CLBlock*		e_Block  = NULL; // not allocated either
	CLBLOCKHEADER		hdr;

	EBMLElementVectors	pe_Cont;
	char* cid_Cont[] = {	(char*)MID_CL_BLOCK, (char*)MID_CL_REFERENCEBLOCK,
							(char*)MID_CL_BLOCKDURATION, (char*)MID_CL_BLOCKADDITIONS, NULL,
	};
	int occ_restr[] = { 1, 0, 2, 0, 0 };
	SEARCHMULTIEX sme = { cid_Cont, NULL, occ_restr };

	bool block_group = 0;
	bool simple_block = 0;
	// get blockgroups

	if (!e_CurrentBlockGroup) {

/*		EBMLELEMENTLIST* e_BlockGroups = NULL;
		Search((void**)&e_BlockGroups,(char*)MID_CL_BLOCKGROUP,(char*)MID_CL_BLOCKGROUP);
		e_CurrentBlockGroup = (EBMLM_CLBlockGroup*)(*e_BlockGroups)(0);
		delete e_BlockGroups->pElement;
		delete e_BlockGroups;*/
		Create1stSub((EBMLElement**)&e_CurrentBlockGroup);
	} /*else {*/
		do {
			EBMLElement* e_next = e_CurrentBlockGroup->GetSucc();
			DeleteEBML(&e_CurrentBlockGroup);
			e_CurrentBlockGroup = (EBMLM_CLBlockGroup*)e_next;
		} while (e_CurrentBlockGroup && 
			e_CurrentBlockGroup->GetType() != IDVALUE(MID_CL_BLOCKGROUP) && 
			e_CurrentBlockGroup->GetType() != IDVALUE(MID_CL_SIMPLEBLOCK)
			);
//	}

	// end of cluster?
	if (!e_CurrentBlockGroup) {
		return READBL_ENDOFCLUSTER;
	}

	if (e_CurrentBlockGroup->GetType() == IDVALUE(MID_CL_BLOCKGROUP)) {

		// get current blockgroup
		e_BG = e_CurrentBlockGroup;

		// find elements
		if (e_BG->SearchMulti(pe_Cont, sme) < 0) {
			return READBL_FILEB0RKED;
		}

		EBMLElementVector &e_Blocks			= pe_Cont[0];
		EBMLElementVector &e_Ref			= pe_Cont[1];
		EBMLElementVector &e_Duration		= pe_Cont[2];
		EBMLElementVector &e_BlockAdditions	= pe_Cont[3];

		if (e_Blocks.size() > 1 )
			Note("more than one block in BlockGroup");


		// check references
		for (size_t j=0;j<e_Ref.size();j++) {
			int ref = (int)FSSInt2Int(e_Ref[j]->GetData());
			pDest->references.push_back(ref);
	
			if (ref>0)
				pDest->reference_types |= RBIREF_FORWARD;
		 
			if (ref<0)
				pDest->reference_types |= RBIREF_BACKWARD;
		}
		
		if (e_Duration.size() == 1) {
			pDest->iFlags |= RBIF_DURATION;
			pDest->qwDuration = e_Duration[0]->AsInt();
		}

		// only process 1st block atm
		e_Block = (EBMLM_CLBlock*)e_Blocks[0];

		block_group = 1;
	} else {
		e_Block = (EBMLM_CLBlock*)e_CurrentBlockGroup;

		simple_block = 1;
	}

	// load data
	(pDest->cData = e_Block->Read(&hdr))->IncRefCount();
	pDest->iStream = hdr.iStream;
	//pDest->iFlags = hdr.iFlags;
	if (hdr.iFlags & BLKHDRF_DISCARDABLE) {
		pDest->iFlags |= RBIF_DISCARDABLE;
	}
	if (hdr.iFlags & BLKHDRF_KEYFRAME) {
		pDest->references.clear();
		pDest->iFlags |= RBIF_KEYFRAME;
	}

	if (simple_block) {
		if ((hdr.iFlags & BLKHDRF_KEYFRAME) != BLKHDRF_KEYFRAME) {
			pDest->references.push_back(0);
			pDest->reference_types = RBIREF_BACKWARD;
		}
	}

	
	pDest->qwTimecode = GetTimecode() + (short)hdr.iTimecode;
	pDest->frame_sizes = hdr.frame_sizes;

	iCurrentBlock++;

	DeleteVectors(pe_Cont);

	return READBL_OK;
}
bool EBMLM_Cluster::CheckIDs(char* iID,EBMLElement** p)
{
	DOCOMP(MID_CL_TIMECODE,EBMLM_CLTimeCode)
	DOCOMP(MID_CL_PREVSIZE,EBMLM_CLPrevSize)
	DOCOMP(MID_CL_SIMPLEBLOCK,EBMLM_CLBlock)

	DOCOMP(MID_CL_POSITION,EBMLM_CLPosition)
	DOCOMP(MID_CL_SILENTTRACKS, EBMLM_CLSilentTracks)
	DOCOMPL(MID_CL_BLOCKGROUP,EBMLM_CLBlockGroup)
}

void EBMLM_Cluster::RetrieveInfo()
{
	EBMLElementVectors timecode;

	if (!ClusterInfo) {
		ClusterInfo = new CLUSTER_INFO;
		ZeroMemory(ClusterInfo,sizeof(CLUSTER_INFO));
		ClusterInfo->qwPreviousSize = -2;
	}

	char* cids[] = { (char*)MID_CL_TIMECODE, NULL };
	void* targets[] = { &ClusterInfo->qwTimecode, NULL };
	int occ_restr[] = { 3, 0 };
	SEARCHMULTIEX sme = { cids, targets, occ_restr };
	SearchMulti(timecode, sme, (char*)MID_CL_TIMECODE);
	DeleteVectors(timecode);
}

__int64 EBMLM_Cluster::GetPreviousSize()
{
	EBMLELEMENTLIST*	e_PrevSize = NULL;

	if (!ClusterInfo) RetrieveInfo();
	if (ClusterInfo->qwPreviousSize == PREVSIZE_UNKNOWN) return PREVSIZE_UNKNOWN;
	if (ClusterInfo->qwPreviousSize != PREVSIZE_UNINITIALIZED) return ClusterInfo->qwPreviousSize;

	Search((void**)&e_PrevSize, (char*)MID_CL_PREVSIZE, (char*)MID_CL_PREVSIZE);
	if (!e_PrevSize->iCount) {
		ClusterInfo->qwPreviousSize = PREVSIZE_UNKNOWN;
	} else {
		ClusterInfo->qwPreviousSize = (*e_PrevSize)[0];
	}
	DeleteElementList(&e_PrevSize);
	return ClusterInfo->qwPreviousSize;
	
}

__int64 EBMLM_Cluster::GetTimecode()
{
	if (!ClusterInfo)
		RetrieveInfo();

	return ClusterInfo->qwTimecode;
}