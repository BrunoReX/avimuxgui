#ifndef I_AUDIOSOURCE_VORBIS
#define I_AUDIOSOURCE_VORBIS

#include "audiosource_generic.h"
#include "oggfile.h"
#include "..\buffers.h"
#include "audiosource_matroska.h"

const int VORBIS_OPEN_OK	= 0x01;
const int VORBIS_OPEN_ERROR = -0x01;

typedef struct
{
	int		vorbis_version;
	int		channels;
	int		audio_sample_rate;
	int		bitrate_maximum;
	int		bitrate_nominal;
	int		bitrate_minimum;
	int		blocksize[2];
	int		framing_flag;
} VORBIS_IDENTIFICATION_HEADER;

typedef struct
{
	int		blockflag;
	int		windowtype;
	int		transformtype;
	int		mapping;
} VORBIS_MODE;

typedef struct
{
	int				iCodebookCount;
	int				iModeCount;
	int				ilogModeCount;
	int				iMappingCount;
	VORBIS_MODE*	pModes;	
} VORBIS_SETUP_HEADER;

typedef struct
{
	int				iSamplerate;
	int				iSamplecount;
	int				iSize;
	void*			pData;
} VORBIS_READ_STRUCT;

#define VORBISSOURCE VORBISFROMOGG

// reads Vorbis audio data from a source that delivers "packets"
// originally only OGG sources were possible, thus "VORBISFROMOGG"

class VORBISFROMOGG: public AUDIOSOURCE
{
	private:
		BYTE*			binary_packet;

		BITSTREAM*		packet;
		LINEARCACHE*	packet_cache;

		//OGGFILE*	source;
		//OGGFILE*	GetSource();
		PACKETIZER* source;
		PACKETIZER* GetSource();

		VORBIS_IDENTIFICATION_HEADER idh;
		VORBIS_SETUP_HEADER vsh;

		VORBIS_READ_STRUCT last_read[2];
		int			iPrecedingBlockSize;
		__int64		iAudioData_begin;

		CBuffer*	pConfigFrames[3];
		int			iCurrentPacket;
		__int64		iSourceTimecode;
	protected:
		int	virtual	GetNextPacket();
		int			ReadTimeDomainTransforms();
		int			ReadResidue();
		int			ReadResidues();
		int			ReadCodebooks();
		int			ReadCodebook();
		int			ReadFloors();
		int			ReadFloor();
		int			ReadMappings();
		int			ReadMapping();
		int			ReadModes();
		int			ReadMode(VORBIS_MODE* m);
		int			LoadAudioPacket(VORBIS_READ_STRUCT* r);

		int			ReadSetupHeader();
		int			ReadIdentificationHeader();
		int			ProcessPacket();
	public:
		VORBISFROMOGG();
	//	VORBISFROMOGG(OGGFILE* source);
		int				Close();
		__int64 virtual GetExactSize();
		int		virtual GetFrequency();
		int		virtual GetFormatTag();
		char	virtual* GetIDString();
		int		virtual GetGranularity();
		int		virtual GetChannelCount();
		int		virtual GetAvgBytesPerSec();
		__int64 virtual GetUnstretchedDuration(void);
		bool	virtual IsCBR();
		int				Open(PACKETIZER* lpSource = NULL);
		bool	virtual IsEndOfStream();
		int		virtual Seek(__int64 qwTime);
		int		virtual Read(void* lpDest,DWORD dwMicrosecDesired,DWORD* lpdwMicrosecRead,
							__int64* lpqwNanosecRead,__int64* lpiTimeocde = NULL,
							ADVANCEDREAD_INFO* lpAARI = NULL);
		int		virtual RenderSetupHeader(void* pDest);
		__int64 virtual FormatSpecific(__int64 iCode, __int64 iValue = 0);
};

// create input data which can be process by VORBISFROMOGG
// goal: laced Vorbis blocks can be resolved properly

class VORBISPACKETSFROMMATROSKA: public PACKETIZER
{
	private:
		AUDIOSOURCEFROMMATROSKA* source;
		AUDIOSOURCEFROMMATROSKA* GetSource();
		int								iPacketCount;

		int								iLaceSize;
		int								iFramesInLace;
		char*							pData;
		int*							iFrameSizes;
		int								iPos;
		int								iBytePosInLace;

	public:
		VORBISPACKETSFROMMATROSKA();
		int				Close(bool bCloseSource = false);
		int				Open(AUDIOSOURCEFROMMATROSKA* lpSource = NULL);
		int		virtual	ReadPacket(BYTE* cDest, __int64* iTimecode = NULL);
		bool	virtual IsEndOfStream();
		__int64 virtual GetSize();
		__int64 virtual GetUnstretchedDuration(void);
		int		virtual GetAvgBytesPerSec();
};

#endif