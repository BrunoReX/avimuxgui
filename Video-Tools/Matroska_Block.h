#ifndef I_MATROSKABLOCK
#define I_MATROSKABLOCK

#include "ebml_matroska.h"
#include <vector>

class CLBLOCKHEADER
{
public:
	CLBLOCKHEADER();
	int			iStream;
	int			iTimecode;
	int			iFlags;
	std::vector<int> frame_sizes;
};

// Cluster Block
class EBMLM_CLBlock : public EBML_MatroskaElement
{
	private:
		CLBLOCKHEADER*	hdr;
	public:
		virtual ~EBMLM_CLBlock();
		void	virtual Delete();
		EBMLM_CLBlock(STREAM* s,EBMLElement* p);
		GETTYPSTR;
		CBuffer	virtual*	Read(CLBLOCKHEADER* pHeader);
};


#endif