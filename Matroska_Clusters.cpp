#include "stdafx.h"
#include "Matroska_Clusters.h"
#include "warnings.h"
#include "integers.h"
#include "Matroska_Block.h"

EBMLM_Cluster::EBMLM_Cluster(STREAM* s,EBMLElement* p) 
{
	SetStream(s);
	SetParent(p); 
	DetermineLength(); 
	SetType(ETM_CLUSTER); 
	SetMulti(true); 
	SetDataType(EBMLDATATYPE_MASTER); 
	ClusterInfo=NULL;
	e_CurrentBlockGroup = NULL;
	
	return; 
} 

void EBMLM_Cluster::Delete()
{
	delete ClusterInfo;
	ClusterInfo = NULL;
}

__int64 EBMLM_Cluster::GetDuration()
{
	if (ClusterInfo && ClusterInfo->qwDuration) return ClusterInfo->qwDuration;

/*	EBMLELEMENTLIST* e_Duration = NULL;
	Search((void**)&e_Duration,(char*)MID_CL_DURATION,NULL);
	if (e_Duration->iCount) {
		int iDuration = (int)(ClusterInfo->qwDuration=e_Duration->pElement[0]->AsInt());
		DeleteElementList(&e_Duration);
		return iDuration;
	} else*/ {
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
	EBMLELEMENTLIST*	e_Blocks = NULL;
	EBMLM_CLBlockGroup*	e_BG	 = NULL; // not allocated
	EBMLM_CLBlock*		e_Block  = NULL; // not allocated either
	EBMLELEMENTLIST*	e_Ref    = NULL;
	EBMLELEMENTLIST*	e_duration = NULL;
	CLBLOCKHEADER		hdr;

	EBMLELEMENTLIST**	pe_Cont  = NULL;
	char* cid_Cont[] = {	(char*)MID_CL_BLOCK, 
							(char*)MID_CL_REFERENCEBLOCK,
							(char*)MID_CL_BLOCKDURATION 
	};

	ZeroMemory(pDest,sizeof(READBLOCK_INFO));
	// get blockgroups

	if (!e_CurrentBlockGroup) {
		EBMLELEMENTLIST* e_BlockGroups = NULL;
		Search((void**)&e_BlockGroups,(char*)MID_CL_BLOCKGROUP,(char*)MID_CL_BLOCKGROUP);
		e_CurrentBlockGroup = (EBMLM_CLBlockGroup*)e_BlockGroups->pElement[0];
		delete e_BlockGroups->pElement;
		delete e_BlockGroups;
	} else {
		do {
			EBMLElement* e_next = e_CurrentBlockGroup->GetSucc();
			DeleteEBML(&e_CurrentBlockGroup);
			e_CurrentBlockGroup = (EBMLM_CLBlockGroup*)e_next;
		} while (e_CurrentBlockGroup && e_CurrentBlockGroup->GetType() != ETM_CLBLOCKGROUP);
	}

	// end of cluster?
	if (!e_CurrentBlockGroup) {
		return READBL_ENDOFCLUSTER;
	}

	// get current blockgroup
	e_BG = e_CurrentBlockGroup;
	// find elements
						
	e_BG->SearchMulti((void***)&pe_Cont, cid_Cont, 3);
	e_Blocks	= pe_Cont[0];
	e_Ref		= pe_Cont[1];
	e_duration	= pe_Cont[2];

	// any blocks found in blockgroup?
	if (!e_Blocks->iCount) {
		B0rked("no Block found in Blockgroup");
		return READBL_FILEB0RKED;
	} else if (e_Blocks->iCount!=1) {
		Note("more than one block in BlockGroup");
	}

	// only process 1st block atm
	e_Block = (EBMLM_CLBlock*)e_Blocks->pElement[0];

	// check references
	if (e_Ref->iCount>2) {
		Obfuscated("block refers to more than 2 other blocks");
	} else if (e_Ref->iCount==0) {
		pDest->iReferences = 0;
	} else if (e_Ref->iCount==1) {
		pDest->iReferencedFrames[0] = (int)FSSInt2Int(e_Ref->pElement[0]->GetData());
		if (pDest->iReferencedFrames[0]<0) {
			pDest->iReferences |= RBIREF_BACKWARD;
		} else {
			if (!pDest->iReferencedFrames[0]) {
				Obfuscated("frame only refers to itself");
			} else pDest->iReferences |= RBIREF_FORWARD;
		}
	} else if (e_Ref->iCount==2) {
		if ((pDest->iReferencedFrames[0]=(int)FSSInt2Int(e_Ref->pElement[0]->GetData()))*
			(pDest->iReferencedFrames[1]=(int)FSSInt2Int(e_Ref->pElement[1]->GetData()))<0) {
			pDest->iReferences |= RBIREF_BIDIRECTIONAL;
		} else Obfuscated("Frame refers to 2 other frames but is not bidirectional");
	}

	// duration indicated?
	if (e_duration->iCount>1) {
		B0rked("more than one 'duration' element encountered in 'BlockGroup'");
	}

	// load data
	(pDest->cData = e_Block->Read(&hdr))->IncRefCount();
	pDest->iStream = hdr.iStream;
	pDest->iFlags = hdr.iFlags;

	if (e_duration->iCount==1) {
		pDest->iFlags |= RBIDUR_INDICATED;
		pDest->qwDuration = e_duration->pElement[0]->AsInt();
	}
	
	pDest->qwTimecode = GetTimecode() + (short)hdr.iTimecode;
	pDest->iFrameCountInLace = hdr.iFrameCountInLace;
	if (hdr.cFrameSizes) hdr.cFrameSizes->Refer(&pDest->cFrameSizes);
	iCurrentBlock++;
	
	DeleteElementLists(&pe_Cont, 3);

	return READBL_OK;
}

bool EBMLM_Cluster::CheckIDs(char* iID,EBMLElement** p)
{
	DOCOMP(MID_CL_TIMECODE,EBMLM_CLTimeCode)
	DOCOMP(MID_CL_PREVSIZE,EBMLM_CLPrevSize)
	DOCOMP(MID_CL_POSITION,EBMLM_CLPosition)
	DOCOMPL(MID_CL_BLOCKGROUP,EBMLM_CLBlockGroup)
}

void EBMLM_Cluster::RetrieveInfo()
{
	EBMLELEMENTLIST*	e_Timecode = NULL;

	if (!ClusterInfo) {
		ClusterInfo = new CLUSTER_INFO;
		ZeroMemory(ClusterInfo,sizeof(*ClusterInfo));
		ClusterInfo->qwPreviousSize = -2;
	}

	Search((void**)&e_Timecode,(char*)MID_CL_TIMECODE,(char*)MID_CL_TIMECODE); // stop after timecode has been found
	if (e_Timecode->iCount!=1) {
		B0rked("not exactly one Timecode in Cluster");
	}

	ClusterInfo->qwTimecode = e_Timecode->pElement[0]->AsInt();
	DeleteElementList(&e_Timecode);
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
		ClusterInfo->qwPreviousSize = e_PrevSize->pElement[0]->AsInt();
	}
	DeleteElementList(&e_PrevSize);
	return ClusterInfo->qwPreviousSize;
	
}

__int64 EBMLM_Cluster::GetTimecode()
{
	if (!ClusterInfo) {
		RetrieveInfo();
	}
	return ClusterInfo->qwTimecode;
}