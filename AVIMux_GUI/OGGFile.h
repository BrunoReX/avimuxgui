/*

	CLASS to read and write OGG files. Unless crappy libogg,
	it can write pages of user-specified size. Also, it will
	always return packets, not segments

 */


#ifndef I_OGGFILE
#define I_OGGFILE

#include "streams.h"

typedef struct
{
	__int64 iSize;
	int		iEndReached;
	__int64 iLatestGranulePos;
	int		iOpenMode;
} OGG_FILEDATA;

typedef struct
{
	int		iPacketCount;
	int*	iPacketSizes;
	__int64 iGranulePos;
	int		iStreamSerialNumber;
	int		iCurrentPos;
	int		iCurrentPacket;
	int		iStreamStructureRevision;
	int		bEndIsEndOfPacket;
	int		bContinuedPacket;
	int		bFirstPage;
	int		bLastPage;
	int		iPageSequenceNumber;
	int		iTotalSize;
	int		iHeaderSize;
	BYTE*	bData;
} OGG_PAGE;

typedef struct
{
	int		iCount;
	__int64 iGranulePos;
	int		bEndIsEndOfPacket;
	int		bContinuedPacket;
	int		iPageCount;
	void**	pPacket;
	int*    pPacketSize;
	int		iCurrSize;
} OGG_WRITE_PAGE;

typedef struct
{
	__int64	iGranulePos;
	int		iStreamSerialNumber;
	int		iPageCount;
	void*	pPageBuffer[2];
	int		iPageBuffer_index;
	bool	bFirst;
	bool	bLast;
	int		iMaxPageSize;
	bool	bLocked;
	int		iTimestampMode;
} OGG_WRITE_STATE;

const int OGG_OPEN_OK         = 0x00;
const int OGG_OPEN_ERROR      = -0x01;

const int OGG_OPEN_READ       = 0x02;
const int OGG_OPEN_WRITE      = 0x03;

const int OGG_LOAD_PAGE_OK			= 0x04;
const int OGG_LOAD_PAGE_ERROR		= -0x05;

const int OGG_LOAD_PACKET_FINISHED  = 0x06;
const int OGG_LOAD_PACKET_ERROR     = -0x07;
const int OGG_LOAD_SEGMENT_ERROR	= -0x01;

const int OGG_READ_ERROR            = -0x01;

const int OGG_WRITE_SEGMENT_PAGE_FULL = 0x01;
const int OGG_WRITE_SEGMENT_OK      = 0x00;
const int OGG_WRITE_SEGMENT_BADSIZE = -0x01;

const int OGG_TSM_ABSOLUTE = 0x01;
const int OGG_TSM_RELATIVE = 0x02;

class PACKETIZER: public STREAM
{
	private:
	protected:
		__int64		duration;
		void		SetDuration(__int64 i);
		
	public:
		// *iTimestamp MUST stay unmodified if no timestamp is returned
		PACKETIZER();
		int			virtual ReadPacket(BYTE* bDest, __int64* iTimestamp = NULL);
		__int64		virtual GetUnstretchedDuration(void);
};

class OGGFILE: public PACKETIZER
{
	private:
		STREAM*			source;
		OGG_FILEDATA	data;
		OGG_PAGE*		pages;
		int				current_page_index;
		OGG_PAGE*		current_page;
		OGG_WRITE_PAGE  write_page;
		OGG_WRITE_STATE write_state;
	protected:
		int			DeletePage(OGG_PAGE* p);
		int			GetNextPageIndex();
		STREAM*		GetSource();
		STREAM*		GetDest();

		int			FindPage(__int64 iPosition);
		int			IsEndOfCurrentPage();
		int			IsPacketContinuedOnNextPage();
		int			LoadNextPage();
		int			LoadPage(int dest_index);
		int			ReadPacketFromCurrentPage(BYTE* cDest);
		int			SeekIntoCurrentPage(int iOffset);
		int			WriteSegment(void* pData, int iSize);
		int			WritePreparedPageToDisc();
		bool		PageLocked();
	public:
		OGGFILE();
		OGGFILE(STREAM* lpStream, DWORD dwAccess = OGG_OPEN_READ);
		int			Close(bool bCloseSource = false);
		__int64		GetLatestGranulePos();
		__int64		GetPos();
		__int64		GetSize();
		bool		IsEndOfStream();
		int			Open(STREAM* lpStream, DWORD dwAccess);
		int			SetMaxPageSize(int iMaxSize);
		int			ReadPacket(BYTE* cDest, __int64* iTimestamp = NULL);
		int			RenderPage();

		// write one packet to an ogg page
		int			WritePacket(void* pData, int iSize, __int64 iDuration);

		// write several packets on the same ogg page
		int			WritePackets(void* pData, int iCount, int* iSizes, __int64 iDuration);

		// prevent WritePacket from / Allow WritePacket to start a new ogg page
		void		LockPage();
		void		UnlockPage();

		int			GetAvgBytesPerSec();
		int			Seek(__int64 qwFilePos);
		void		SetTimestampMode(int iMode);
};

#endif