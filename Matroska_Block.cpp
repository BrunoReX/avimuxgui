#include "stdafx.h"
#include "matroska_block.h"
#include "matroska_IDs.h"
#include "integers.h"

EBMLM_CLBlock::EBMLM_CLBlock(STREAM* s,EBMLElement* p) 
{
	SetStream(s); 
	SetParent(p); 
	DetermineLength(); 
	SetType(ETM_CLBLOCK); 
	SetMulti(true); 
	SetDataType(EBMLDATATYPE_BIN); 
	hdr=NULL;
	
	return; 
} 

char* EBMLM_CLBlock::GetTypeString() \
{
    return "Block"; 
}


#define WSWAPB(x) x=((x >> 8) | (x << 8)) & 0xFFFF

EBMLM_CLBlock::~EBMLM_CLBlock()
{
}

void EBMLM_CLBlock::Delete()
{
	if (hdr) {
		DecBufferRefCount(&hdr->cFrameSizes);
		delete hdr;
	}
	DecBufferRefCount(&buffer);
}

CBuffer* EBMLM_CLBlock::Read(CLBLOCKHEADER *pHeader)
{
	int i,j,k,l,hdrl,iRawSize;
	int* iFrameSizes;
	unsigned char* cFrame;
	unsigned char* c;

	if (!buffer) {

		cFrame = new unsigned char[(int)GetLength()];
		c = cFrame;
		GetSource()->Seek(GetStreamPos());
		GetSource()->Read(cFrame,(int)GetLength());

		hdrl=0;
		hdr = new CLBLOCKHEADER;
		ZeroMemory(hdr,sizeof(*hdr));
		hdr->iStream = *c++;
		hdr->iTimecode = *c++;
		hdr->iTimecode *= 256;
		hdr->iTimecode += *c++;
		hdr->iTimecode &= 0xFFFF;
		hdr->iFlags = *c++;
		hdrl+=4;
		if (hdr->iFlags & BLKHDRF_LACINGNEW) {

			if (((hdr->iFlags & BLKHDRF_LACINGNEW) < BLKHDRF_LACINGCONST)) {
				hdr->iFrameCountInLace = *c++;
				hdrl++;
				hdr->cFrameSizes = new CBuffer;
				hdr->cFrameSizes->SetSize((l=++hdr->iFrameCountInLace)*sizeof(int));
				iFrameSizes = (int*)hdr->cFrameSizes->GetData();;
				ZeroMemory(iFrameSizes,hdr->iFrameCountInLace * sizeof(int));
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
				hdr->iFrameCountInLace = 1+*c++;
				hdrl++;
				hdr->cFrameSizes = new CBuffer;
				hdr->cFrameSizes->SetSize((l=hdr->iFrameCountInLace)*sizeof(int));
				iFrameSizes = (int*)hdr->cFrameSizes->GetData();;
				ZeroMemory(iFrameSizes,hdr->iFrameCountInLace * sizeof(int));
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
					__int64 ifs = (GetLength() - hdrl) / hdr->iFrameCountInLace;
					for (i=0;i<l;i++) {
						iFrameSizes[i] = (int)ifs;
					}
				}
			}

		}
		hdr->iStream&=~0x80;

		buffer = new CBuffer;
		buffer->SetSize(iRawSize=(int)(GetLength()-hdrl));
	
		if (iRawSize) memcpy(buffer->GetData(),c,iRawSize);
		delete cFrame;
//		if (hdr->iFlags == BLKHDRF_LACINGNEW
	}

	buffer->IncRefCount();
	if (hdr->cFrameSizes ) {
		hdr->cFrameSizes->IncRefCount();
	}
	*pHeader = *hdr;
//	delete hdr;

	return buffer;
}
