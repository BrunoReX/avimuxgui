#ifndef I_MATROSKACLUSTERS
#define I_MATROSKACLUSTERS

#include "ebml.h"
#include "ebml_matroska.h"
#include "matroska_IDs.h"
#include "buffers.h"

const int PREVSIZE_UNINITIALIZED = -0x02;
const int PREVSIZE_UNKNOWN       = -0x01;

typedef struct
{
	__int64		qwTimecode;
	__int64		qwDuration;
	__int64		qwPreviousSize;
} CLUSTER_INFO;

typedef struct
{
	int				iStream;
	int				iReferences;
	int				iReferencedFrames[2];
	__int64			qwDuration;
	int				iFrameCountInLace;
	CBuffer*		cFrameSizes;
	int				iFlags;
	CBuffer*		cData;
	__int64			qwTimecode;
	__int64			iEnqueueCount;
} READBLOCK_INFO;

class EBMLM_Cluster : public EBML_MatroskaElement 
{
	private:
		CLUSTER_INFO*		ClusterInfo;
		int					iCurrentBlock;
	//	EBMLELEMENTLIST*	e_BlockGroups;
		EBMLM_CLBlockGroup*	e_CurrentBlockGroup;
		EBMLM_CRC32*		e_CRC32;
	protected: 
		CHECKIDs; 
		void			RetrieveInfo();
	public:
		EBMLM_Cluster(STREAM* s,EBMLElement* p);
		__int64			GetTimecode();
		__int64			GetDuration();
		__int64			GetPreviousSize();
		int				ReadBlock(READBLOCK_INFO* pDest);
	    GETTYPSTR; 
		void	virtual Delete();
};

typedef struct
{
	__int64			qwPosition;
	__int64			qwTimecode;
	__int64			qwDuration;
	EBMLM_Cluster*	pCluster;
	int				iValid;
} CLUSTER_PROP;

typedef struct
{
	int				iCount;
	CLUSTER_PROP**	cluster_info;
} CLUSTERS;


#endif