#include "stdafx.h"
#include "matroska_block.h"
#include "matroska_IDs.h"
#include "integers.h"

#ifdef DEBUG_NEW
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

EBMLM_CLBlock::EBMLM_CLBlock(STREAM* s,EBMLElement* p) 
{
	SetStream(s); 
	SetParent(p); 
	DetermineLength(); 
	
	if (GetParent()->GetType() == IDVALUE(MID_CL_BLOCKGROUP))
		SetType(IDVALUE(MID_CL_BLOCK)); 
	if (GetParent()->GetType() == IDVALUE(MID_CLUSTER))
		SetType(IDVALUE(MID_CL_SIMPLEBLOCK)); 

	SetMulti(true); 
	SetDataType(EBMLDATATYPE_BIN); 
	hdr=NULL;
	
	return; 
} 

CLBLOCKHEADER::CLBLOCKHEADER()
{
	iStream = -1;
	iTimecode = INT_MIN;
	iFlags = 0;
}

char* EBMLM_CLBlock::GetTypeString() \
{
	if (GetParent()->GetType() == IDVALUE(MID_CL_BLOCKGROUP))
	    return "Block"; 
	if (GetParent()->GetType() == IDVALUE(MID_CLUSTER))
	    return "SimpleBlock"; 

	return "b0rked";
}


#define WSWAPB(x) x=((x >> 8) | (x << 8)) & 0xFFFF

EBMLM_CLBlock::~EBMLM_CLBlock()
{
}

void EBMLM_CLBlock::Delete()
{
	if (hdr) {
		delete hdr;
		hdr = NULL;
	}

	DecBufferRefCount(&buffer);
}

CBuffer* EBMLM_CLBlock::Read(CLBLOCKHEADER *pHeader)
{
	int i,j,k,l,hdrl,iRawSize;
	int iFrameSizes[256];
	unsigned char* cFrame;
	unsigned char* c;
	int iFrameCount = 0;
	memset(iFrameSizes, 0, sizeof(iFrameSizes));

	if (!buffer) {

		int length = (int)GetLength();

		cFrame = (unsigned char*)malloc(length); //GetLength());
		c = cFrame;
		GetSource()->Seek(GetStreamPos());
		GetSource()->Read(cFrame,length);//(int)GetLength());

		hdrl=0;
		hdr = new CLBLOCKHEADER;
		
		hdr->iStream = *c++;

		hdr->iTimecode = *c++;
		hdr->iTimecode *= 256;
		hdr->iTimecode += *c++;
		hdr->iTimecode &= 0xFFFF;
		hdr->iFlags = *c++;
		hdrl+=4;
		if (hdr->iFlags & BLKHDRF_LACINGNEW) {

			if (((hdr->iFlags & BLKHDRF_LACINGNEW) < BLKHDRF_LACINGCONST)) {
				iFrameCount = *c++;

				hdrl++;

				iFrameCount++;
				l=iFrameCount;

				j=0;

				for (i=0;i<l-1;i++) {
					k=255;
					while (k==255) {
						k = *c++;
						hdrl++;
						iFrameSizes[j]+=k;
						iFrameSizes[l-1]+=k;
					}
					j++;
				}
				iFrameSizes[l-1] = (int)(GetLength()-iFrameSizes[l-1]-hdrl);
			} else {
				iFrameCount = 1+*c++;
				hdrl++;
				l=iFrameCount;
				j=0; __int64 f = 0;
				if ((hdr->iFlags & BLKHDRF_LACINGNEW) != BLKHDRF_LACINGCONST) {
					for (i=0;i<l-1;i++) {
						__int64 j = 0;
						int k;
						if (i) {
							c+=k=VSSInt2Int((char*)c,&j);
							hdrl+=k;
							j+=f;
							f=j;
						} else {
							k=VSUInt2Int((char*)c,&j);
							c+=k=VSUInt2Int((char*)c,&j);
							hdrl+=k;
							f=j;
						}
						
						iFrameSizes[i] = (int)j;
						iFrameSizes[l-1]+=(int)j;
					}
					iFrameSizes[l-1] = (int)(GetLength()-iFrameSizes[l-1]-hdrl);
				} else {
					__int64 ifs = (GetLength() - hdrl) / /*hdr->iFrameCountInLace*/iFrameCount;
					for (i=0;i<l;i++) {
						iFrameSizes[i] = (int)ifs;
					}
				}
			}

		}
		hdr->iStream&=~0x80;

		buffer = new CBuffer;

		/* the following should never happen */
		if (length - hdrl < 0) {
#ifdef _T
			MessageBox(0, _T("block header length seems to be larger than block"), 
				_T("Matroska Block Reader:"), MB_OK | MB_ICONERROR);
#endif 
			buffer->SetSize(0);
		} else 
		{
			buffer->SetSize(iRawSize=(int)(length/*GetLength()*/-hdrl));
	
			if (iRawSize) memcpy(buffer->GetData(),c,iRawSize);
		}
		free(cFrame);
	}

	buffer->IncRefCount();
	*pHeader = *hdr;

	for (i=0;i<iFrameCount;i++) {
		(*pHeader).frame_sizes.push_back(iFrameSizes[i]);
	}

	return buffer;
}
