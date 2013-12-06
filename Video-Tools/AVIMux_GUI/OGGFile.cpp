#include "stdafx.h"
#include "OGGFile.h"
#include "..\CRC.h"

int PACKETIZER::ReadPacket(BYTE* cDest, __int64* iTimestamp)
{
	return 0;
}

__int64 PACKETIZER::GetUnstretchedDuration(void)
{
	return duration;
}

PACKETIZER::PACKETIZER()
{
	duration = 0;
}

void PACKETIZER::ReInit()
{
	return;
}

void PACKETIZER::SetDuration(__int64 i)
{
	duration = i;
}

int PACKETIZER::GetName(char* lpcName)
{
	if (lpcName)
		*lpcName = 0;

	return 0;
}

int PACKETIZER::GetLanguageCode(char* lpDest)
{
	if (lpDest)
		*lpDest = 0;

	return 0;
}

OGGFILE::OGGFILE()
{
	source = NULL;
	ZeroMemory(&data,sizeof(data));
	current_page_index = 0;
}

OGGFILE::OGGFILE(STREAM* lpStream, DWORD dwAccess)
{
	source = NULL;
	ZeroMemory(&data,sizeof(data));
	Open(lpStream, dwAccess);
	current_page_index = 0;
}

void OGGFILE::ReInit()
{
	GetSource()->InvalidateCache();
	GetSource()->Seek(0);

	int j = data.iOpenMode;
	STREAM* s = GetSource();

	ZeroMemory(&data,sizeof(data));
	Open(s, j);
	current_page_index = 0;
}

STREAM* OGGFILE::GetSource()
{
	return source;
}

STREAM* OGGFILE::GetDest()
{
	return source;
}

int OGGFILE::IsPacketContinuedOnNextPage()
{
	if (IsEndOfCurrentPage() && !current_page->bEndIsEndOfPacket) {
		return true;
	} else {
		return false;
	}
}

int OGGFILE::ReadPacketFromCurrentPage(BYTE* cDest)
{
	int iPacketSize;

	if (IsEndOfCurrentPage()) {
		return OGG_LOAD_PACKET_ERROR;
	}

	iPacketSize = current_page->iPacketSizes[current_page->iCurrentPacket++];
	memcpy(cDest, current_page->bData+current_page->iCurrentPos, iPacketSize);

	current_page->iCurrentPos += iPacketSize;

	return iPacketSize;
}

int OGGFILE::DeletePage(OGG_PAGE* p)
{
	if (p->bData) {
		delete p->bData;
		p->bData = NULL;
	}

	if (p->iPacketSizes) {
		delete p->iPacketSizes;
		p->iPacketSizes = NULL;
	}

	return 0;
}

int OGGFILE::LoadPage(int dest_index)
{
	DWORD	dwOggs, dwFlags = 0, dwCRC = 0, dwSegmentCount = 0;
	int i, iTotalSize = 0;
	int j = 0;
	__int64 iStreamPos = GetSource()->GetPos();
	OGG_PAGE* p = &pages[dest_index];
	DeletePage(p);

	ZeroMemory(&pages[dest_index],sizeof(OGG_PAGE));
	if (GetSource()->Read(&dwOggs, 4)!=4) {
		data.iEndReached = 1;
		return OGG_LOAD_PAGE_ERROR;
	}
	if (dwOggs != 'SggO') {
		return OGG_LOAD_PAGE_ERROR;
	}
	
	GetSource()->Read(&p->iStreamStructureRevision, 1);
	GetSource()->Read(&dwFlags,1);
	p->bContinuedPacket = !!(dwFlags & 1);
	p->bFirstPage = !!(dwFlags & 2);
	p->bLastPage = !!(dwFlags & 4);
	GetSource()->Read(&p->iGranulePos, 8);
	GetSource()->Read(&p->iStreamSerialNumber, 4);
	GetSource()->Read(&p->iPageSequenceNumber, 4);
	GetSource()->Read(&dwCRC, 4);
	GetSource()->Read(&dwSegmentCount, 1);

	p->iPacketSizes = new int[dwSegmentCount];
	ZeroMemory(p->iPacketSizes, dwSegmentCount*sizeof(int));

	for (i=0;i<(int)dwSegmentCount;i++) {
		GetSource()->Read(&j, 1);
		p->iPacketSizes[p->iPacketCount]+=j;
		if (j<255) p->iPacketCount++;
		iTotalSize += j;
	}
	p->iHeaderSize = (int)(GetSource()->GetPos() - iStreamPos);

	if (j==255) p->iPacketCount++;
	p->bEndIsEndOfPacket = !!(j<255);

	p->bData = new BYTE[iTotalSize];
	if (GetSource()->Read(p->bData, iTotalSize) != iTotalSize) {
		return OGG_LOAD_PAGE_ERROR;
	};

	p->iTotalSize = (int)(GetSource()->GetPos() - iStreamPos);

	if (FindStreamSerialNumberIndex(p->iStreamSerialNumber) == -1)
		InsertStreamSerialNumber(p->iStreamSerialNumber);

	return OGG_LOAD_PAGE_OK;
}

int OGGFILE::GetNumberOfStreams()
{
	return stream_serial_numbers.count;
}

int OGGFILE::SeekIntoCurrentPage(int iOffset)
{
	current_page->iCurrentPos = 0; iOffset -= current_page->iHeaderSize;

	while (current_page->iCurrentPos < iOffset) {
		current_page->iCurrentPos += current_page->iPacketSizes[current_page->iCurrentPacket++];
	}

	return 0;
}

bool OGGFILE::IsEndOfStream()
{
	return !!data.iEndReached;
}

int OGGFILE::IsEndOfCurrentPage()
{
	return (current_page->iCurrentPacket == current_page->iPacketCount);
}

int OGGFILE::GetNextPageIndex()
{
	return current_page_index ^ 1;
}

int OGGFILE::LoadNextPage()
{
	int j;

	if (LoadPage(j=GetNextPageIndex()) == OGG_LOAD_PAGE_OK) {
		current_page_index = j;
		current_page = &pages[j];
		return OGG_LOAD_PAGE_OK;
	} else {
		return OGG_LOAD_PAGE_ERROR;
	}
}

void OGGFILE::InsertStreamSerialNumber(int i)
{
	stream_serial_numbers.count++;
	stream_serial_numbers.serial_nbr = (int*)realloc(stream_serial_numbers.serial_nbr, sizeof(int)*stream_serial_numbers.count);
	stream_serial_numbers.serial_nbr[stream_serial_numbers.count-1] = i;
}

int OGGFILE::FindStreamSerialNumberIndex(int serial)
{
	for (int j=0;j<stream_serial_numbers.count;j++)
		if (stream_serial_numbers.serial_nbr[j] == serial)
			return j;

	return -1;
}

int OGGFILE::Open(STREAM* lpStream, DWORD dwAccess)
{
	pages = new OGG_PAGE[2];
	ZeroMemory(pages,2*sizeof(OGG_PAGE));
	memset(&stream_serial_numbers, 0, sizeof(stream_serial_numbers));
	current_page_index = 0;
	source = lpStream;
	GetSource()->Seek(0);

	if (dwAccess == OGG_OPEN_READ) {
		data.iSize = GetSource()->GetSize();
		if (LoadNextPage() != OGG_LOAD_PAGE_OK) {
			delete pages;
			source = NULL;
			return OGG_OPEN_ERROR;
		}
		data.iLatestGranulePos = current_page->iGranulePos;
		ScanForStreams();
	} else {
		write_page.iCount = 0;
		write_page.pPacket = new void*[256];
		write_page.pPacketSize = new int[256];
		write_page.bContinuedPacket = false;
		write_state.iGranulePos = 0;
		write_state.iPageCount = 0;
		write_state.iStreamSerialNumber = ((rand()%256) << 8) + (rand()%256);
		for (int i=0;i<256;write_page.pPacket[i++]=new char[256]);
		write_state.pPageBuffer[0] = NULL;
		write_state.pPageBuffer[1] = NULL;
		write_state.iPageBuffer_index = NULL;
		write_state.bFirst = true;
		write_state.bLast = false;
		write_page.iCurrSize = 0;
		write_state.iMaxPageSize = 8192;
		write_state.bLocked = 0;
		write_state.iTimestampMode = OGG_TSM_RELATIVE;
	}
	data.iOpenMode = dwAccess;

	return OGG_OPEN_OK;
}

void OGGFILE::ScanForStreams()
{
	int bStop = 0;
	int page_count = 0;

	Seek(0);

	do {
		LoadNextPage();
		page_count++;
		bStop = (page_count >= 20 && page_count >= 3*GetNumberOfStreams());
	} while (!bStop);

	Seek(0);
//	LoadNextPage();
	//SeekIntoCurrentPage(current_page->iHeaderSize);
}


void OGGFILE::GetCurrentPageHeader(OGG_PAGE* dest)
{
	if (!dest)
		return;

	memcpy(dest, current_page, sizeof(*dest));
	dest->bData = NULL;
}

int OGGFILE::SetMaxPageSize(int iMaxSize)
{
	if (iMaxSize < 0) return 0;
	if (iMaxSize > 65025) iMaxSize = 65025;

	write_state.iMaxPageSize = iMaxSize;

	return 0;
}

int OGGFILE::Seek(__int64 qwFilePos)
{
	data.iEndReached = 0;
	if (qwFilePos) {
		FindPage(qwFilePos);
	} else GetSource()->Seek(0);
	__int64 i = GetSource()->GetPos();
	if (LoadNextPage() == OGG_LOAD_PAGE_OK) {
		SeekIntoCurrentPage((int)(qwFilePos - i));
	}
	return OGG_LOAD_PAGE_OK;
}

int OGGFILE::FindPage(__int64 iPosition)
{
	int i = 0;
	STREAM* s = GetSource();
	s->Seek(iPosition);
	__int64 iSearchWidth = 1<<17;
	__int64 iSearchStart;
	char* pBuffer;
	DWORD oggs = 0x5367674F;

	if (iPosition < iSearchWidth) {
		s->Seek(0);
		iSearchWidth = iPosition;
		iSearchStart = 0;
	} else {
		iSearchStart = iPosition - iSearchWidth;
	}
	pBuffer = new char[(int)(iSearchWidth+4)];
	s->Seek(iPosition - iSearchWidth);
	s->Read(pBuffer, (int)(iSearchWidth--+4));


	while (iSearchWidth>=0) {
		DWORD d = *((DWORD*)(pBuffer+iSearchWidth));
		if (d == oggs) {
			s->Seek(iSearchStart+iSearchWidth);
			iSearchWidth = 0;
		}
		iSearchWidth--;
									
	}

	delete pBuffer;

	return 0;
}

__int64 OGGFILE::GetSize()
{
	return data.iSize;
}

int OGGFILE::ReadPacket(BYTE* cDest, __int64* iTimestamp)
{
	int iRead = 0;
	int iLoadPageResult = 1;
	int	iLoadPacketResult = 0;
//	if (iTimestamp) *iTimestamp = -1;

	if (IsEndOfStream()) {
		return OGG_READ_ERROR;
	}

	do {
		iLoadPacketResult = ReadPacketFromCurrentPage(cDest+iRead);
		if (iLoadPacketResult == OGG_LOAD_PACKET_ERROR) {
			return OGG_READ_ERROR;
		}
		iRead += iLoadPacketResult;
	} while (IsPacketContinuedOnNextPage() && (iLoadPageResult = (LoadNextPage() == OGG_LOAD_PAGE_OK)));

	data.iLatestGranulePos = current_page->iGranulePos;

	if (IsEndOfCurrentPage()) {
		LoadNextPage();
	}

	if (iLoadPageResult == 1) {
		return iRead;
	} else {
		return OGG_READ_ERROR;
	}
}

__int64 OGGFILE::GetPos()
{
	return GetSource()->GetPos() + current_page->iCurrentPos + 
		current_page->iHeaderSize - current_page->iTotalSize;
}

__int64 OGGFILE::GetLatestGranulePos()
{
	__int64 res = data.iLatestGranulePos;
	data.iLatestGranulePos = -1;

	return res;
}

int OGGFILE::GetAvgBytesPerSec()
{
	return GetSource()->GetAvgBytesPerSec();
}

int OGGFILE::Close(bool bCloseSource)
{
	DeletePage(&pages[0]);
	DeletePage(&pages[1]);

	if (data.iOpenMode == OGG_OPEN_WRITE) {
		write_state.bLast = true;
		RenderPage();
		WritePreparedPageToDisc();
		WritePreparedPageToDisc();
		delete write_page.pPacketSize;
		for (int i=0;i<256;delete write_page.pPacket[i++]);
		delete write_page.pPacket;
	}

	if (bCloseSource) {
		GetSource()->Close();
		delete GetSource();
	}

	source = NULL;
	data.iOpenMode = -1;
	return 0;
//	ZeroMemory(&data,sizeof(data));
}

int OGGFILE::WriteSegment(void* pData, int iSize)
{
	if (iSize < 0 || iSize > 255) {
		return OGG_WRITE_SEGMENT_BADSIZE;
	}

	if (write_page.iCount == 256) {
		return OGG_WRITE_SEGMENT_PAGE_FULL;
	}

	if ((write_page.iCurrSize > write_state.iMaxPageSize) && !PageLocked() && iSize) {
		return OGG_WRITE_SEGMENT_PAGE_FULL;
	}

	memcpy(write_page.pPacket[write_page.iCount], pData, iSize);
	write_page.pPacketSize[write_page.iCount++] = iSize;

	write_page.iCurrSize += iSize;
	
	return OGG_WRITE_SEGMENT_OK;
}

int OGGFILE::WritePacket(void* pData, int iSize, __int64 iDuration)
{
	int i;
	char* pbData = (char*)pData;

	if (write_state.iTimestampMode == OGG_TSM_RELATIVE) {
		write_state.iGranulePos += iDuration;
	} else {
		write_state.iGranulePos = iDuration;
	}
	write_page.iGranulePos = write_state.iGranulePos;

	while (iSize) {
		if (WriteSegment(pbData, i=min(iSize, 255)) == OGG_WRITE_SEGMENT_PAGE_FULL) {
			RenderPage();
		} else {
			write_page.bEndIsEndOfPacket = !(iSize-=i);
			if (i==255 && !iSize) WriteSegment(pbData, 0);
			pbData+=i;
		}
	}

	return iSize;
}

int OGGFILE::WritePackets(void* pData, int iCount, int* iSizes, __int64 iDuration)
{
	BYTE* pbData = (BYTE*)pData;
	int	  res = 0;

	if (iCount + write_page.iCount >= 255) {
		RenderPage();
	}

	LockPage();
	for (int i=0;i<iCount;i++) {
		if (i==iCount-1) {
			UnlockPage();
		}
		WritePacket(pbData, iSizes[i], (!!(i==iCount-1))*iDuration);
		pbData+=iSizes[i];
		res+=iSizes[i];
	}

	return res;
}

int OGGFILE::WritePreparedPageToDisc()
{
	write_state.iPageBuffer_index^=1;

	if (write_state.pPageBuffer[write_state.iPageBuffer_index]) {
		DWORD* pdwSize = (DWORD*)write_state.pPageBuffer[write_state.iPageBuffer_index];
		GetDest()->Write(pdwSize+1, *pdwSize);
		delete[] pdwSize;

	}
	return 0;
}

int OGGFILE::RenderPage()
{
	int i, j;

	WritePreparedPageToDisc();

	BYTE* pPage_begin = new BYTE[65540];
	write_state.pPageBuffer[write_state.iPageBuffer_index] = pPage_begin;
	
	union {
		BYTE* pbPage;
		DWORD* pdwPage;
		__int64* pi64Page;
	};

	pbPage = pPage_begin;
	DWORD* pdwCRC = NULL;
	DWORD* pdwSize = pdwPage;
	pbPage+=4;
	*pbPage++='O';
	*pbPage++='g';
	*pbPage++='g';
	*pbPage++='S';
	*pbPage++=0;
	*pbPage++=1*write_page.bContinuedPacket + 2*write_state.bFirst + 4*write_state.bLast;
	*pi64Page++=write_page.iGranulePos;
	*pdwPage++=write_state.iStreamSerialNumber;
	*pdwPage++=write_state.iPageCount;
	pdwCRC = pdwPage;
	*pdwPage++ = 0;
	*pbPage++ = write_page.iCount;
	for (i=0;i<write_page.iCount;i++) {
		j = write_page.pPacketSize[i];
		*pbPage++ = j;
	}

	for (i=0;i<write_page.iCount;i++) {
		memcpy(pbPage, write_page.pPacket[i], write_page.pPacketSize[i]);
		pbPage+=write_page.pPacketSize[i];
	}
	*pdwSize = pbPage - pPage_begin - 4;
	CRC32_ogg((unsigned char*)(pdwSize+1), *pdwSize);

	write_state.bFirst = false;
	write_page.iPageCount = write_state.iPageCount++;
	write_page.iCount = 0;
	write_page.iCurrSize = 0;
	write_page.bContinuedPacket = !write_page.bEndIsEndOfPacket;

	return 1;
}

void OGGFILE::LockPage()
{
	write_state.bLocked=1;
}

void OGGFILE::UnlockPage()
{
	write_state.bLocked=0;
}

bool OGGFILE::PageLocked()
{
	return write_state.bLocked;
}

void OGGFILE::SetTimestampMode(int iMode)
{
	write_state.iTimestampMode = iMode;
}