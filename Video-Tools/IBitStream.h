#ifndef I_IBITSTREAM
#define I_IBITSTREAM

#include "basestreams.h"
#include <deque>
#include "types.h"

class IBitStream
{
protected:
	STREAM*		source;
protected:
	STREAM virtual*	GetSource() { return source; }
public:
	IBitStream();
	virtual ~IBitStream();

	int		virtual	Open(STREAM* lpStream) = NULL;
	int		virtual Close() = NULL;
	int		virtual Seek(__int64 qwPos) = NULL;
	int		virtual GetBitPos() = NULL;
	__int64 virtual GetPos() = NULL;
	void	virtual SetBitPos(int pos) = NULL;
	void    virtual FlushInputBuffer() = NULL;
	int		virtual ReadBits(int n, int iFlag = 0) = NULL;
	__int64	virtual	ReadBits64(int n, int iFlag = 0) = NULL;
};

#endif