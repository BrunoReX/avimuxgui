#ifndef I_MATROSKABLOCK
#define I_MATROSKABLOCK

#include "ebml_matroska.h"

typedef struct 
{
	int			iStream;
	int			iTimecode;
	int			iFlags;
	int			iFrameCountInLace;
	CBuffer*	cFrameSizes;
} CLBLOCKHEADER;

// Cluster Block
class EBMLM_CLBlock : public EBML_MatroskaElement
{
	private:
		CLBLOCKHEADER*	hdr;
	public:
		~EBMLM_CLBlock();
		void	virtual Delete();
		EBMLM_CLBlock(STREAM* s,EBMLElement* p);
		GETTYPSTR;
		CBuffer	virtual*	Read(CLBLOCKHEADER* pHeader);
};


#endif