#ifndef I_BITSTREAM
#define I_BITSTREAM

#include "basestreams.h"

class BITSTREAM 
{
	private:
		STREAM*		source;
		DWORD		dwCurrBitPos;
		void		LoadWord(void);
		WORD		wData;
		int			ReadBit(int iFlag = 0);

	public:
		BITSTREAM(void)	{ source=NULL; dwCurrBitPos=0; }
		virtual ~BITSTREAM() {};
		STREAM*			GetSource() { return source; }
		int		virtual	Open(STREAM* lpStream);
		int		virtual Close(void) { source=NULL; return STREAM_OK; }
		bool	virtual	IsEndOfStream(void) { return (source->IsEndOfStream()&&(dwCurrBitPos==15)); }
		int				Seek(__int64	qwPos);
		int				ReadBits(int n, int iFlag = 0);
		__int64			ReadBits64(int n, int iFlag = 0);
		__int64			GetPos();

};

#endif