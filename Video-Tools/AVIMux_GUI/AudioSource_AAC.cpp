#include "stdafx.h"

// #include "silence.h"
#include "audiosource_aac.h"
#include "../BitStreamFactory.h"

#ifdef DEBUG_NEW
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

	///////////////////////////////////////////
	// AAC audio source - reading ATDS files //
	///////////////////////////////////////////

#pragma pack(push,1)

const int AAC_PACKTYPE_ADTS = 1;
const int AAC_PACKTYPE_ADIF = 2;

const int AAC_ADTS_ID_MPEG2 = 1;
const int AAC_ADTS_ID_MPEG4 = 0;


const int AAC_SAMPLESPERFRAME = 1024;

AACSOURCE::AACSOURCE()
{
	ZeroMemory(&aacinfo,sizeof(aacinfo));
	bitsource = 0;
}

AACSOURCE::AACSOURCE(STREAM* s)
{
	ZeroMemory(&aacinfo,sizeof(aacinfo));
	bitsource = 0;
	Open(s);
}

int AACSOURCE::Open(STREAM* s)
{

	if (!s) return AS_ERR;
	if (s->GetSize() <= 0)
		return AS_ERR;

	if (CBinaryAudioSource::Open(s)==AS_ERR) return AS_ERR;
	bitsource = CBitStreamFactory::CreateInstance<CBitStream2>();
	bitsource->Open(s);

	// ADTS Header?
	s->SetOffset(0);	 
	s->Seek(0);
	
	void* pBuffer = new char[2<<17];

	if (ReadFrame_ADTS(pBuffer,NULL,NULL)==AS_ERR) {
		bitsource->Close();
		delete bitsource;
		delete [] pBuffer;
		return AS_ERR;
	};

	delete[] pBuffer;
	SetFrameMode(1);

	if (GetFrequency() <= 24000)
		FormatSpecific(MMSSFS_AAC_SETSBR, 1);		

	GetSource()->Seek(0);

	return 1;
}

void AACSOURCE::ReadADTSHeader(ADTSHEADER* h)
{
	bitsource->FlushInputBuffer();
	ZeroMemory(h,sizeof(*h));
	h->syncword = bitsource->ReadBits(12);
	h->ID = bitsource->ReadBits(1);
	h->layer = bitsource->ReadBits(2);
	h->protection_absend = bitsource->ReadBits(1);
	h->profile = bitsource->ReadBits(2);
	h->sampling_fequency_index = bitsource->ReadBits(4);
	h->private_bit = bitsource->ReadBits(1);
	h->channel_configuration = bitsource->ReadBits(3);
	h->original_or_copy = bitsource->ReadBits(1);
	h->home = bitsource->ReadBits(1);
	h->copyright_identification_bit = bitsource->ReadBits(1);
	h->copyright_identification_start = bitsource->ReadBits(1);
	h->frame_length = bitsource->ReadBits(13);
	h->adts_buffer_fullness = bitsource->ReadBits(11);
	h->number_of_data_blocks = bitsource->ReadBits(2)+1;
	if (!h->protection_absend) h->crc16 = bitsource->ReadBits(16);
}

int AACSOURCE::GetSampleRateIndex(int bDouble)
{
	for (int i=0;i<sizeof(aac_sampling_frequencies)/sizeof(int);i++) {
		if ((aac_sampling_frequencies[i] == GetFrequency() && !bDouble) ||
			(aac_sampling_frequencies[i] == 2*GetFrequency() && bDouble)) {
			return i;
		}
	}

	return -1;
}

int AACSOURCE::ReadFrame_ADTS(void* lpDest, DWORD* lpdwMicroSecRead, __int64* lpqwNanoSecRead)
{
	__int64 iPos = GetSource()->GetPos();

	ZeroMemory(&h, sizeof(h));
	ReadADTSHeader(&h);
	int header_size = 7; // size of ADTS header is known

	if (h.syncword != 0xFFF) {
		return AS_ERR;
	}

	SetSampleRateIndex(h.sampling_fequency_index);
	SetFrequency(aac_sampling_frequencies[h.sampling_fequency_index]);
	SetProfile(h.profile);
	SetChannelCount(h.channel_configuration);
	switch (h.ID) {
		case AAC_ADTS_ID_MPEG2: SetMPEGVersion(2); break;
		case AAC_ADTS_ID_MPEG4: SetMPEGVersion(4); break;
	}
	
	GetSource()->Seek(iPos + header_size);
	int iRead = GetSource()->Read(lpDest,(int)(h.frame_length - header_size));

	if (lpdwMicroSecRead) *lpdwMicroSecRead = (int)((__int64)1000000 * h.number_of_data_blocks *
		 AAC_SAMPLESPERFRAME / GetFrequency());
	if (lpqwNanoSecRead) *lpqwNanoSecRead = (__int64)1000000000 * h.number_of_data_blocks * 
		AAC_SAMPLESPERFRAME / GetFrequency();

	return iRead;
}

int AACSOURCE::ReadFrame(MULTIMEDIA_DATA_PACKET** dataPacket)
{
	__int64 pos = GetSource()->GetPos();
	ZeroMemory(&h, sizeof(h));
	ReadADTSHeader(&h);
	int header_size = 7;

	if (h.syncword != 0xFFF) {
		return AS_ERR;
	}

	SetSampleRateIndex(h.sampling_fequency_index);
	SetFrequency(aac_sampling_frequencies[h.sampling_fequency_index]);
	SetProfile(h.profile);
	SetChannelCount(h.channel_configuration);
	switch (h.ID) {
		case AAC_ADTS_ID_MPEG2: SetMPEGVersion(2); break;
		case AAC_ADTS_ID_MPEG4: SetMPEGVersion(4); break;
	}

	char* data = (char*)malloc(h.frame_length - header_size);

	GetSource()->Seek(pos + header_size);
	DWORD dwRead = GetSource()->Read(data, h.frame_length - header_size);

	if (dwRead != h.frame_length - header_size)
		return -1;

	createMultimediaDataPacket(dataPacket);
	(*dataPacket)->cData = data;
	(*dataPacket)->totalDataSize = h.frame_length - header_size;
	(*dataPacket)->duration = GetFrameDuration();
	(*dataPacket)->frameSizes.push_back(h.frame_length);
	(*dataPacket)->timecode = GetCurrentTimecode() * GetTimecodeScale();
	
	if (IsEndOfStream())
		(*dataPacket)->nextTimecode = TIMECODE_UNKNOWN;
	else
		(*dataPacket)->nextTimecode = (*dataPacket)->timecode + GetFrameDuration();

	(*dataPacket)->flags = 0;
	(*dataPacket)->compressionInfo.clear();

	IncCurrentTimecode(GetFrameDuration());

	return (*dataPacket)->totalDataSize;
}

int Pack2ADTS(void* lpSource, void* lpDest, AUDIOSOURCE* a, int iSize)
{
	BYTE* lpbDest = (BYTE*)lpDest;
	BYTE* lpbSource = (BYTE*)lpSource;
	iSize += 7;
	*lpbDest++ = 0xFF;								// syncword 7-0
	*lpbDest++ = 0xF0 |								// syncword 11-8
		(1<<3) * ((a->FormatSpecific(MMSGFS_AAC_MPEGVERSION) == 4)?0:1) |	// MPEG 2 or 4
		1;											// protection absent
	*lpbDest++ = (unsigned char)(
		         (((1<<6) * a->FormatSpecific(MMSGFS_AAC_PROFILE)) |			// profile	
				 ((1<<2) * a->FormatSpecific(MMSGFS_AAC_SAMPLERATEINDEX)) |	// sample_frequency_index
				 (1<<1) * 0	|						// private
				 a->GetChannelCount()>>2));			// bit 2 of channel config
	*lpbDest++ = (1<<6) * (a->GetChannelCount() & 0x03) | // bits 1..0 of channel config
				 (1<<5) * (0) |						// original or copy
				 (1<<4) * (0) |						// home
				 (1<<3) * (0) |						// copyright id bit
				 (1<<2) * (0) |						// copyright id start
				 (iSize >> 11);						// size 12..11
	*lpbDest++ = (iSize >> 3) & 0xFF;				// size 10..3
	*lpbDest++ = ((iSize & 0x07) << 5) |			// size 2..0
				 (0x1F);							// adts_buffer_fulless = 0x7FF
	*lpbDest++ = 0xFC;								// and 1 frame per ADTS block

	memcpy(lpbDest, lpSource, iSize-7);
	return iSize;
}

int AACSOURCE::GetFrequency()
{
	return aacinfo.dwFrequency;
}

int AACSOURCE::GetFormatTag()
{
	return AAC_WFORMATTAG;
}

int AACSOURCE::GetChannelCount()
{
	return aacinfo.dwChannels;
}

__int64 AACSOURCE::GetFrameDuration()
{
	return (__int64)1000000000 * AAC_SAMPLESPERFRAME / GetFrequency();
}

int AACSOURCE::doClose()
{
	return CBinaryAudioSource::doClose();
}

int AACSOURCE::doRead(void* lpDest, DWORD dwMicroSecDesired, DWORD *lpdwMicroSecRead, __int64* lpqwNanoSecRead)
{
	// always read one single frame
	return ReadFrame_ADTS(lpDest, lpdwMicroSecRead, lpqwNanoSecRead);
}

int AACSOURCE::GetProfile()
{
	return aacinfo.dwProfile;
}

void AACSOURCE::GetProfileString(char* buf, int buf_len)
{
	char* AAC_prof_name = NULL;

	if (buf_len < 8)
		return;

	if ((int)FormatSpecific(MMSGFS_AAC_ISSBR)) 
		AAC_prof_name = "HE";
	else switch (FormatSpecific(MMSGFS_AAC_PROFILE)) {
		case AAC_ADTS_PROFILE_LC: AAC_prof_name = "LC"; break;
		case AAC_ADTS_PROFILE_LTP: AAC_prof_name = "LTP"; break;
		case AAC_ADTS_PROFILE_MAIN: AAC_prof_name = "MAIN"; break;
		case AAC_ADTS_PROFILE_SSR: AAC_prof_name = "SSR"; break;
		default: AAC_prof_name = "unknown";
	}

	buf[0]=0;
	_snprintf_s(buf, buf_len, buf_len, "%s-AAC", AAC_prof_name);
	buf[buf_len - 1] = 0;
}

int AACSOURCE::GetMPEGVersion()
{
	return aacinfo.dwMPEGVersion;
}

__int64 AACSOURCE::FormatSpecific(__int64 iCode, __int64 iValue)
{
	switch (iCode) {
		case MMSGFS_AAC_PROFILE: return GetProfile();
		case MMSGFS_AAC_MPEGVERSION: return GetMPEGVersion();
		case MMSGFS_AAC_ISSBR: return aacinfo.iIsSBR;
		case MMSSFS_AAC_SETSBR: aacinfo.iIsSBR = !!iValue; return 1; break;
		case MMSGFS_AAC_SAMPLERATEINDEX: return GetSampleRateIndex(0); break;
		case MMSGFS_AAC_DOUBLESAMPLERATEINDEX: return GetSampleRateIndex(1); break;
		case MMSGFS_IS_AAC: return 1; break;

	}

	return 0;
}

typedef struct {
	AACSOURCE* s;
	HANDLE hSem;
	DWORD *pStatus;
} AAC_CFR_DATA;

int Check_AAC_CFR(void* pData) 
{
	AAC_CFR_DATA* p = (AAC_CFR_DATA*)pData;
	p->s->Seek(0);
	void* pBuffer = new char[2<<17];
	__int64 iFileSize = p->s->GetSize();
	int i_duration = -1;
	int i_curr_duration = -1;
	int iRes = 1;
	__int64 iPos = 0;

	while (!p->s->IsEndOfStream()) {
		iPos += p->s->Read(pBuffer, 0, (DWORD*)&i_curr_duration, NULL);		
		if (i_duration == -1) {
			i_duration = i_curr_duration;
		} else {
			if (i_duration != i_curr_duration) {
				iRes = 0;
			}
		}
		(*p->pStatus) = (DWORD)(1000 * iPos / iFileSize);
	}

	delete[] pBuffer;
	p->s->SetCFRFlag(iRes);
	p->s->Seek(0);
	ReleaseSemaphore(p->hSem, 1, NULL);

	delete pData;
	return 1;
}

int AACSOURCE::SetCFRFlag(int bFlag)
{
	aacinfo.iCFR = bFlag;
	return 1;
}

int AACSOURCE::IsCFR()
{
	return aacinfo.iCFR;
}

int AACSOURCE::PerformCFRCheck(char** pSemaphore, DWORD* pStatus)
{
	GetSource()->Seek(0);

	char* pS = new char[20];
	*pSemaphore = pS;
	pS[19]=0;
	for (int i=0;i<19;i++) {
		pS[i] = 'a'+rand()%26;
	}

	AAC_CFR_DATA* p = new AAC_CFR_DATA;
	p->hSem = CreateSemaphore(NULL, 0, 1, pS);
	p->s = this;
	p->pStatus = pStatus;

	DWORD dwID;
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE )Check_AAC_CFR, p, NULL, &dwID);
//	AfxBeginThread((AFX_THREADPROC)Check_AAC_CFR, p);

	return 0;
}

void AACSOURCE::SetChannelCount(int i)
{
	aacinfo.dwChannels = i;
}

void AACSOURCE::SetFrequency(int i)
{
	aacinfo.dwFrequency = i;
}

void AACSOURCE::SetProfile(int i)
{
	aacinfo.dwProfile = i;
}

void AACSOURCE::SetSampleRateIndex(int i)
{
	aacinfo.dwSampleRateIndex = i;
} 

void AACSOURCE::SetMPEGVersion(int i)
{
	aacinfo.dwMPEGVersion = i;
}

int AACSOURCE::GetOutputFrequency()
{
	return (FormatSpecific(MMSGFS_AAC_ISSBR)?2*GetFrequency():GetFrequency());
}

		////////////////////////////
		// read AAC back from AVI //
		////////////////////////////

#ifdef f_AVIFILE_I

AACFROMAVI::AACFROMAVI()
{
	avifile = NULL;
	stream = NULL;
}

AACFROMAVI::AACFROMAVI(AVIFILEEX* s, int j)
{
	Open(s,j);
}

int AACFROMAVI::Open(AVIFILEEX* s, int j)
{
	avifile = s;
	stream = j;

	if (avifile->GetFormatTag(j) != AAC_WFORMATTAG) {
		avifile = NULL;
		stream = -1;
		return AS_ERR;
	}

	WAVEFORMATEX* lpwfe = (WAVEFORMATEX*)avifile->GetStreamFormat(stream);
	if (lpwfe->cbSize == 2) {
		FormatSpecific(MMSSFS_AAC_SETSBR, 0);
	} else 
	if (lpwfe->cbSize == 5) {
		FormatSpecific(MMSSFS_AAC_SETSBR, 1);
	}

	SetFrequency(lpwfe->nSamplesPerSec);
	lpwfe++;
	BYTE* b = (BYTE*)lpwfe;
	SetProfile((*b++>>3)-1);
	SetMPEGVersion(4);
	SetSampleRateIndex((int)FormatSpecific(MMSGFS_AAC_SAMPLERATEINDEX));
	SetChannelCount(*b++>>3 & 0x7);
	SetDefault(!(avifile->GetStreamHeader(stream)->dwFlags & AVISF_DISABLED));

	return 1;
}

int AACFROMAVI::doRead(void* lpDest, DWORD dwMicroSecDesired, DWORD *lpdwMicroSecRead, __int64* lpqwNanoSecRead)
{
	// always read one single frame
	int iSize = avifile->GetChunkSize(stream, CN_CURRENT_CHUNK);
	int iRead = avifile->LoadAudioData(stream, iSize, lpDest);
	WAVEFORMATEX* lpwfe = (WAVEFORMATEX*)avifile->GetStreamFormat(stream);
	AVIStreamHeader* ash = avifile->GetStreamHeader(stream);
	
	if (lpdwMicroSecRead) *lpdwMicroSecRead = (int)((__int64)1000000 *
		ash->dwScale / GetFrequency());
	if (lpqwNanoSecRead) *lpqwNanoSecRead = (__int64)1000000000 * 
		ash->dwScale / GetFrequency();
	
	return iRead;
}

int AACFROMAVI::Seek(__int64 qwPos)
{
	if (!qwPos) 
		SetCurrentTimecode(0);
	return avifile->SeekByteStream(stream, qwPos);
}

void AACFROMAVI::ReInit()
{
	if (!avifile)
		return;

	Seek(0);
}

__int64 AACFROMAVI::GetExactSize()
{
	return avifile->GetStreamSize(stream);
}

int AACFROMAVI::GetAvgBytesPerSec()
{
	return avifile->GetAvgBytesPerSec(stream);
}

bool AACFROMAVI::IsEndOfStream()
{
	return avifile->IsEndOfStream(stream);
}

#endif

#pragma pack(pop)