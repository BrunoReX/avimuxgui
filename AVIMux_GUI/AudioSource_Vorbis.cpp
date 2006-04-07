/* !!!  B0RKED  !!!
   ================

Reading of Vorbis headers containing ordered codebooks
is broken. Appearently, the test files I have don't match
with the Vorbis_I_spec.pdf file

Currently, ordered codebooks are skipped, hoping that
there is no data mistaken as a sync header, if there is
non in reality. That means, Codec init packets where the
last codebook is ordered will not be read correctly!



*/

#include "stdafx.h"
#include "audiosource_vorbis.h"
#include "..\cache.h"
#include "math.h"
/*
typedef struct
{

} VORBIS_CODEBOOK;
*/
const int VORBIS_PACKETTYPE_IDHEADER = 0x01;
const int VORBIS_PACKETTYPE_COMMENT = 0x03;
const int VORBIS_PACKETTYPE_SETUPHEADER = 0x05;

const int VORBIS_SETUPHEADER_BROKEN = 0x01;
const int VORBIS_AUDIOFRAME_ERROR   = -0x01;

VORBISFROMOGG::VORBISFROMOGG()
{
	ZeroMemory(&vsh,sizeof(vsh));
	packet = new BITSTREAM();
	packet_cache = new LINEARCACHE;
	binary_packet = new BYTE[1<<18];
	iCurrentPacket = 0;
	for (int i=0;i<3;i++) {
		pConfigFrames[i] = new CBuffer;
	}
	AllowAVIOutput(false); // Vorbis-in-AVI not yet possible!
	SetTimecodeScale(1000);
}

/*
VORBISFROMOGG::VORBISFROMOGG(OGGFILE* _source)
{
	ZeroMemory(&vsh,sizeof(vsh));
	packet  = new BITSTREAM();
	source = _source;
	binary_packet = new BYTE[1<<18];
	packet_cache = new LINEARCACHE;
	iCurrentPacket = 0;
	
	for (int i=0;i<3;i++) {
		pConfigFrames[i] = new CBuffer;
	}
}
*/

PACKETIZER* VORBISFROMOGG::GetSource()
{
	return source;
}

int VORBISFROMOGG::GetNextPacket()
{
	packet_cache->SetSize(GetSource()->ReadPacket(binary_packet, &iSourceTimecode), true);
	packet_cache->SetData(binary_packet);

	packet->Open(packet_cache);
	return 0;
}

int VORBISFROMOGG::Close()
{
	for (int i=0;i<3;i++) {
		DecBufferRefCount(&pConfigFrames[i]);
	}
	delete vsh.pModes;
	packet_cache->Clear();
	delete packet_cache;
	delete binary_packet;
	packet->Close();
	delete packet;
	return 0;
}

int VORBISFROMOGG::Open(PACKETIZER* lpSource)
{
	int j;
	if (lpSource) source = lpSource;
	iAudioData_begin = 0;
	
	do {
		GetNextPacket();
		__int64 i = GetSource()->GetPos();

	} while ((j=ProcessPacket())!=VORBIS_PACKETTYPE_SETUPHEADER && j);

	iPrecedingBlockSize = 0;
	iSourceTimecode = TIMECODE_UNKNOWN;
	return VORBIS_OPEN_OK;
}

static int ilog(unsigned int v){
  int ret=0;
  while(v){
    ret++;
    v>>=1;
  }
  return(ret);
}

int VORBISFROMOGG::ProcessPacket()
{
	int iPacketType = packet->ReadBits(1, 1);
	if (iPacketType==1) {
		iPacketType = 2*packet->ReadBits(7,1) + iPacketType;
		__int64 iVorbis = packet->ReadBits64(48, 1);
		char* cVorbis = (char*)&iVorbis;
	}

	switch (iPacketType) {
		case VORBIS_PACKETTYPE_IDHEADER: 
			ReadIdentificationHeader(); 
			packet_cache->Seek(0);
			pConfigFrames[0]->SetSize((int)packet_cache->GetSize());
			pConfigFrames[0]->SetData(packet_cache->GetData());
			break;
		case VORBIS_PACKETTYPE_COMMENT: 
			packet_cache->Seek(0);
			pConfigFrames[1]->SetSize((int)packet_cache->GetSize());
			pConfigFrames[1]->SetData(packet_cache->GetData());
			break;
		case VORBIS_PACKETTYPE_SETUPHEADER: ReadSetupHeader(); 
			packet_cache->Seek(0);
			pConfigFrames[2]->SetSize((int)packet_cache->GetSize());
			pConfigFrames[2]->SetData(packet_cache->GetData());
			iAudioData_begin = GetSource()->GetPos();
			break;
		default:
			LoadAudioPacket(&last_read[iCurrentPacket^=1]);
			iPacketType = 0;
			break;
	}

	__int64 i = GetSource()->GetPos();
	return iPacketType;
}

int VORBISFROMOGG::RenderSetupHeader(void* pDest)
{
	BYTE* b = (BYTE*)pDest;
	int i = 0;
	int j, k;

	i=1;
	for (j=0;j<2;j++) {
		k=pConfigFrames[j]->GetSize();
		while (k>=255) {
			k-=255;
			b[i++]=255;
		}
		b[i++]=k;
	}
	b[0]=i-1;

	for (j=0;j<3;j++) {
		memcpy(b+i,pConfigFrames[j]->GetData(),pConfigFrames[j]->GetSize());
		i+=pConfigFrames[j]->GetSize();
	}

	return i;
}

bool VORBISFROMOGG::IsEndOfStream()
{
	return GetSource()->IsEndOfStream();
}

int VORBISFROMOGG::LoadAudioPacket(VORBIS_READ_STRUCT* r)
{
	int mode_number = packet->ReadBits(vsh.ilogModeCount, 1);
	int b;

	if (vsh.pModes) {
		r->iSamplecount = idh.blocksize[b = vsh.pModes[mode_number].blockflag];
	} else {
		r->iSamplecount = 0x800;
	}
	r->iSamplecount = iPrecedingBlockSize/4 + r->iSamplecount/4;

	r->iSize = (int)packet_cache->GetSize();
	r->iSamplerate = idh.audio_sample_rate;
	r->pData = new char[r->iSize];
	packet_cache->Seek(0);
	packet_cache->Read(r->pData, r->iSize);

	if (vsh.pModes) {
		iPrecedingBlockSize = idh.blocksize[vsh.pModes[mode_number].blockflag];
	} else {
		iPrecedingBlockSize = 0x800;
	}

	return 0;
}

int VORBISFROMOGG::ReadIdentificationHeader()
{
	idh.vorbis_version = packet->ReadBits(32,1);
	idh.channels = packet->ReadBits(8,1);
	idh.audio_sample_rate = packet->ReadBits(32,1);
	idh.bitrate_maximum = packet->ReadBits(32,1);
	idh.bitrate_nominal = packet->ReadBits(32,1);
	idh.bitrate_minimum = packet->ReadBits(32,1);
	idh.blocksize[0] = 1<<(packet->ReadBits(4,1));
	idh.blocksize[1] = 1<<(packet->ReadBits(4,1));
	idh.framing_flag = packet->ReadBits(1,1);

	return 0;
}

int VORBISFROMOGG::ReadSetupHeader()
{
	ReadCodebooks();
	ReadTimeDomainTransforms();
	ReadFloors();
	ReadResidues();
	ReadMappings();
	ReadModes();
	return 0;
}

int VORBISFROMOGG::ReadCodebooks()
{
	int iCount;
	
	for (vsh.iCodebookCount=iCount=packet->ReadBits(8);iCount>=0;iCount--) {
		ReadCodebook();
	}

	return 0;
}

int lookup1_value(int entries, int dimension)
{
	if (dimension==1) return entries;
	if (dimension==2) return int(sqrt(entries));

	return (int)pow(entries,1./(float)dimension);
}

int VORBISFROMOGG::ReadCodebook()
{
	int i;
	packet->GetPos();
	int sync_word = packet->ReadBits(24,1);
	while (sync_word != 0x564342) {
		sync_word = (sync_word >> 1);
		int k = (1<<23) * !!packet->ReadBits(1,1);
		sync_word |= k;
	}
	int codebook_dim = packet->ReadBits(16,1);
	int codebook_entries = packet->ReadBits(24,1);
	int ordered = packet->ReadBits(1,1);
	int length = 0;
	int* lengths = new int[codebook_entries];

	if (!ordered) {
		int sparse = packet->ReadBits(1,1);
		for (i=0;i<codebook_entries;i++) {
			if (sparse) {
				if (packet->ReadBits(1,1)) {
					// entry used
					length = packet->ReadBits(5,1)+1;
				}
			} else {
				length = packet->ReadBits(5,1)+1;
			}
			lengths[i] = length;
		}
	} else {
	/*	int current_entry = 0;
		int current_length = 1+packet->ReadBits(5,1);
		while (current_entry < codebook_entries) {
			int number = packet->ReadBits(ilog(codebook_entries - current_entry));
			current_entry += number;
			current_length++;
		}
//		if (current_entry > codebook_entries) {
//			Sleep(100); // B0rk !!
//		}*/
	}

	if (!ordered) 
	{
	int codebook_lookup_type = packet->ReadBits(4,1);
	int codebook_lookup_values;

	if (codebook_lookup_type) {
		int codebook_minimum_value = packet->ReadBits(32,1);
		int codebook_delta_value = packet->ReadBits(32,1);
		int codebook_value_bits = 1+packet->ReadBits(4,1);
		int codebook_sequence_p = packet->ReadBits(1,1);
		switch (codebook_lookup_type) {
			case 1:
				codebook_lookup_values = lookup1_value(codebook_entries, codebook_dim);
				break;
			case 2:
				codebook_lookup_values = codebook_entries * codebook_dim;
				break;
			default: 
				codebook_lookup_values = codebook_entries * codebook_dim;
				break;
		}
		packet->ReadBits(codebook_lookup_values * codebook_value_bits, 1);
	}
	}
	delete lengths;

	return 0;
}

int VORBISFROMOGG::ReadTimeDomainTransforms()
{
	int i = 1+packet->ReadBits(6,1);
	packet->ReadBits(16*i,1);

	return 0;
}

int VORBISFROMOGG::ReadFloors()
{
	for (int i=packet->ReadBits(6,1);i>=0;i--) {
		ReadFloor();
	}

	return 0;
}

int VORBISFROMOGG::ReadFloor()
{
	int floor1_multiplier;
	int floor1_partitions;
	int* floor1_partition_class_list = NULL;
	int* floor1_class_dimension = NULL;
	int* floor1_class_subclasses = NULL;
	int* floor1_class_masterbooks = NULL;

	int vorbis_floor_type = packet->ReadBits(16,1);

	if (vorbis_floor_type==1) {
		floor1_partitions = packet->ReadBits(5,1);
		int maximum_class = -1;
		floor1_partition_class_list = new int[floor1_partitions];

		// This does not work :o
	/*	for (int j=0;j<floor1_partitions;j++) {
			maximum_class = max(maximum_class, floor1_partition_class_list[j]=packet->ReadBits(4,1));
		}
		*/
		for (int j=0;j<floor1_partitions;j++) {
			floor1_partition_class_list[j]=packet->ReadBits(4,1);
			maximum_class = max(maximum_class, floor1_partition_class_list[j]);
		}

		floor1_class_dimension = new int[maximum_class+1];
		floor1_class_subclasses = new int[maximum_class+1];
		floor1_class_masterbooks = new int[maximum_class+1];

		for (j=0;j<=maximum_class;j++) {
			floor1_class_dimension[j] = 1+packet->ReadBits(3,1);
			if (floor1_class_subclasses[j] = packet->ReadBits(2,1)) {
				floor1_class_masterbooks[j] = packet->ReadBits(8,1);
			}

			for (int k=0;k<(1<<floor1_class_subclasses[j]);k++) {
				packet->ReadBits(8,1);
			}
		}

		floor1_multiplier = 1+packet->ReadBits(2,1);
		int range_bits = packet->ReadBits(4,1);
		int floor_X_list[2] = {
			0, (1 << range_bits) };

		int floor1_values = 2;
		int count = 0;
		for (int i=0;i<floor1_partitions;i++) {
			count=floor1_class_dimension[floor1_partition_class_list[i]];
			for (j=0;j<count;j++) {
				packet->ReadBits(range_bits); floor1_values++;
			}
		}
	}
	if (vorbis_floor_type==0) {
		int floor0_order = packet->ReadBits(8,1);
		int floor0_rate = packet->ReadBits(16,1);
		int floor0_bark_map_size = packet->ReadBits(16,1);
		int floor0_amplitude_bits = packet->ReadBits(6,1);
		int floor0_amplitude_offset = packet->ReadBits(8,1);
		int floor0_number_of_books = 1+packet->ReadBits(4,1);
		packet->ReadBits(8*floor0_number_of_books,1);

	}

	if (floor1_class_dimension) delete floor1_class_dimension;
	if (floor1_class_masterbooks) delete floor1_class_masterbooks;
	if (floor1_class_subclasses) delete floor1_class_subclasses;
	if (floor1_partition_class_list) delete floor1_partition_class_list;

	return 0;
}

int VORBISFROMOGG::ReadResidues()
{
	for (int i=packet->ReadBits(6,1);i>=0;i--) {
		ReadResidue();
	}

	return 0;
}

int VORBISFROMOGG::ReadResidue()
{
	int vorbis_residue_type = packet->ReadBits(16,1);
	int residue_begin = packet->ReadBits(24,1);
	int residue_end = packet->ReadBits(24,1);
	int residue_partition_site = 1+packet->ReadBits(24,1);
	int residue_classification = 1+packet->ReadBits(6,1);
	int residue_classbook = packet->ReadBits(8,1);
	int* residue_cascade = new int[residue_classification];

	for (int i=0;i<residue_classification;i++) {
		int highbits = 0;
		int lowbits = packet->ReadBits(3,1);
		int bitflag = packet->ReadBits(1,1);
		if (bitflag) {
			highbits = packet->ReadBits(5,1);
		}
		residue_cascade[i] = highbits + 8*lowbits;
	}

	for (i=0;i<residue_classification;i++) {
		for (int j=0;j<8;j++) {
			if (residue_cascade[i] & (1<<j)) {
				packet->ReadBits(8,1);
			}
		}
	}

	delete residue_cascade;

	return 0;
}

int VORBISFROMOGG::ReadMappings()
{
	vsh.iMappingCount=packet->ReadBits(6,1);

	for (int i=vsh.iMappingCount;i>=0;i--) {
		ReadMapping();
	}

	return 0;
}

int VORBISFROMOGG::ReadModes()
{
	vsh.iModeCount=packet->ReadBits(6,1);
	vsh.ilogModeCount = ilog(vsh.iModeCount);
	vsh.pModes = new VORBIS_MODE[vsh.iModeCount+1];

	for (int i=0;i<=vsh.iModeCount;i++) {
		ReadMode(&vsh.pModes[i]);
	}

	return 0;
}

int VORBISFROMOGG::GetFormatTag()
{
	return 0;
}

char* VORBISFROMOGG::GetIDString()
{
	return "A_VORBIS";
}

bool VORBISFROMOGG::IsCBR()
{
	return false;
}

int VORBISFROMOGG::ReadMode(VORBIS_MODE *m)
{
	m->blockflag = packet->ReadBits(1,1);
	m->windowtype = packet->ReadBits(16,1);
	m->transformtype = packet->ReadBits(16,1);
	m->mapping = packet->ReadBits(8,1);

	return 0;
}

int VORBISFROMOGG::ReadMapping()
{
	int vorbis_mapping_submaps;
	int flag;
	int vorbis_mapping_coupling_steps;
	int i, ilog_audio_channels;
	int* vorbis_mapping_magnitude = new int[ilog_audio_channels = ilog(idh.channels-1)];
	int* vorbis_mapping_angle = new int[ilog_audio_channels];
	int* vorbis_mapping_mux;
	int reserved;

	int mapping_type = packet->ReadBits(16,1);
	if (mapping_type) {
		return VORBIS_SETUPHEADER_BROKEN;
	}

	flag = packet->ReadBits(1,1);
	if (flag) {
		vorbis_mapping_submaps = 1+packet->ReadBits(4);
	} else {
		vorbis_mapping_submaps = 1;
	}
	vorbis_mapping_mux = new int[idh.channels];

	flag = packet->ReadBits(1,1);
	if (flag) {
		vorbis_mapping_coupling_steps = 1+packet->ReadBits(8,1);
		for (i=0;i<vorbis_mapping_coupling_steps;i++) {
			vorbis_mapping_magnitude[i] = packet->ReadBits(ilog_audio_channels, 1);
			vorbis_mapping_angle[i] = packet->ReadBits(ilog_audio_channels, 1);
		}
	} else {
		vorbis_mapping_coupling_steps = 0;
	}

	reserved = packet->ReadBits(2,1);
	if (vorbis_mapping_submaps>1) for (i=0;i<vorbis_mapping_submaps;i++) {
		vorbis_mapping_mux[i] = packet->ReadBits(4,1);		
	}
	packet->ReadBits(24*vorbis_mapping_submaps, 1);

	delete vorbis_mapping_magnitude;
	delete vorbis_mapping_angle;
	delete vorbis_mapping_mux;
	
	return 0;
}

int VORBISFROMOGG::GetChannelCount()
{
	return idh.channels;
}

int VORBISFROMOGG::GetFrequency()
{
	return idh.audio_sample_rate;
}

int VORBISFROMOGG::GetAvgBytesPerSec()
{
	return GetSource()->GetAvgBytesPerSec();
}

int VORBISFROMOGG::GetGranularity()
{
	return 0;
}

int VORBISFROMOGG::Seek(__int64 qwTime)
{
	if (!qwTime) {
		GetSource()->Seek(iAudioData_begin);
		SetCurrentTimecode(0);
	}

	return 0;
}

int VORBISFROMOGG::Read(void* lpDest,DWORD dwMicrosecDesired,DWORD* lpdwMicrosecRead,
						__int64* lpqwNanosecRead,__int64* lpiTimecode,
						ADVANCEDREAD_INFO* lpAARI)
{
	do {
		GetNextPacket();
	} while (ProcessPacket());

	if (iSourceTimecode != TIMECODE_UNKNOWN) {
		SetCurrentTimecode(iSourceTimecode, TIMECODE_UNSCALED);
		iSourceTimecode = TIMECODE_UNKNOWN;
	}

	VORBIS_READ_STRUCT* r = (VORBIS_READ_STRUCT*)&last_read[iCurrentPacket];
	
	__int64 qwNanoSec = (__int64)1000000000 * r->iSamplecount / r->iSamplerate;
	__int64 qwTimecode = GetCurrentTimecode() * GetTimecodeScale();
	__int64 qwNextTimecode = qwTimecode + qwNanoSec;

	memcpy(lpDest, r->pData, r->iSize);
	delete r->pData;

	if (lpiTimecode) {
		*lpiTimecode = GetCurrentTimecode();
	}

	if (lpAARI) {
		lpAARI->iFramecount = 1;
		lpAARI->iFramesizes = new int[1];
		lpAARI->iFramesizes[0] = r->iSize;
		lpAARI->iNextTimecode = qwNextTimecode / GetTimecodeScale();
	}

	if (lpdwMicrosecRead) *lpdwMicrosecRead = (DWORD)(qwNanoSec/1000);
	if (lpqwNanosecRead) *lpqwNanosecRead = qwNanoSec;

	IncCurrentTimecode(qwNanoSec);

	return r->iSize;
}

__int64 VORBISFROMOGG::GetUnstretchedDuration(void)
{
	return GetSource()->GetUnstretchedDuration() / GetTimecodeScale();
}

__int64 VORBISFROMOGG::FormatSpecific(__int64 iCode, __int64 iValue)
{
	DWORD** p = (DWORD**)&iValue;

	switch (iCode) {
		case MMSGFS_VORBIS_FRAMEDURATIONS:
			return ((__int64)idh.blocksize[0] + (((__int64)idh.blocksize[1])<<24));
			break;
		case MMSGFS_VORBIS_CONFIGPACKETS:
			p[0][0] = pConfigFrames[0]->GetSize();
			p[0][1] = pConfigFrames[1]->GetSize();
			p[0][2] = pConfigFrames[2]->GetSize();
			p[1][0] = (DWORD)pConfigFrames[0]->GetData();
			p[1][1] = (DWORD)pConfigFrames[1]->GetData();
			p[1][2] = (DWORD)pConfigFrames[2]->GetData();
			break;
		case MMSGFS_IS_VORBIS:
			return 1;
			break;
		default:
			return AUDIOSOURCE::FormatSpecific(iCode, iValue);
	}

	return 0;
}

__int64 VORBISFROMOGG::GetExactSize()
{
	return GetSource()->GetSize();
}

VORBISPACKETSFROMMATROSKA::VORBISPACKETSFROMMATROSKA()
{
	iFrameSizes=0;
	source=NULL;
}


__int64 VORBISPACKETSFROMMATROSKA::GetUnstretchedDuration(void)
{
	return duration;
}

int VORBISPACKETSFROMMATROSKA::Open(AUDIOSOURCEFROMMATROSKA* lpSource)
{
	if (lpSource) source = lpSource;
	iPacketCount = 0;
	pData = NULL;
	iFramesInLace = 0;
	iPos = 0x7FFFFFFF;
	iFrameSizes = new int[256];
	ZeroMemory(iFrameSizes, sizeof(int)*256);
	SetDuration(GetSource()->GetUnstretchedDuration() * GetSource()->GetTimecodeScale());
	return 0;
}

int	VORBISPACKETSFROMMATROSKA::Close(bool bCloseSource)
{
	if (iFrameSizes) delete iFrameSizes;
	return 1;
}


AUDIOSOURCEFROMMATROSKA* VORBISPACKETSFROMMATROSKA::GetSource()
{
	return source;
}

int VORBISPACKETSFROMMATROSKA::ReadPacket(BYTE* bDest, __int64* iTimecode)
{
	int j;

	if (iPacketCount<3) {
		DWORD pSizes[3]; DWORD pData[3]; DWORD *ppCfg[] = { &pSizes[0], &pData[0] };
		__int64 iCfg = *(__int64*)(ppCfg);

		GetSource()->FormatSpecific(MMSGFS_VORBIS_CONFIGPACKETS, iCfg);
		memcpy(bDest, (void*)pData[iPacketCount], pSizes[iPacketCount]);
		return pSizes[iPacketCount++];
	} 

	if (iPos >= iFramesInLace) {
		iPos = 0;
		iBytePosInLace = 0;
		ADVANCEDREAD_INFO aari;

		if (!pData) pData = new char[1<<20];
		__int64 _iTimecode;
		iLaceSize = GetSource()->Read(pData, 0, NULL, NULL, &_iTimecode, &aari);
		iFramesInLace = aari.iFramecount;
		if (!iFramesInLace) iFramesInLace = 1;
		if (iTimecode) *iTimecode = _iTimecode * GetSource()->GetTimecodeScale();
		if (aari.iFramesizes) {
			memcpy(iFrameSizes, aari.iFramesizes, 4*aari.iFramecount);
		} else {
			iFrameSizes[0] = iLaceSize;
		}
		if (iTimecode) *iTimecode = _iTimecode * GetSource()->GetTimecodeScale();
	}

	memcpy(bDest, pData+iBytePosInLace, j=iFrameSizes[iPos]);
	
	iBytePosInLace += iFrameSizes[iPos++];
	
	return j;
}

bool VORBISPACKETSFROMMATROSKA::IsEndOfStream()
{
	return GetSource()->IsEndOfStream();
}

__int64 VORBISPACKETSFROMMATROSKA::GetSize()
{
	return GetSource()->GetSize();
}

int  VORBISPACKETSFROMMATROSKA::GetAvgBytesPerSec()
{
	return GetSource()->GetAvgBytesPerSec();
}
