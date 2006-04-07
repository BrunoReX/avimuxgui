#ifndef I_AUDIOSOURCE_GENERIC
#define I_AUDIOSOURCE_GENERIC

const int MMSGFS_MPEG_LAYERVERSION	= 0x0000000000000001;
const int MMSGFS_MPEG_VERSION		= 0x0000000000000020;
const int MMSGFS_AAC_PROFILE		= 0x0000000000000010;
const int MMSGFS_AAC_MPEGVERSION	= 0x0000000000000011;
const int MMSGFS_AAC_ISSBR			= 0x0000000000000012;
const int MMSSFS_AAC_SETSBR			= 0x0000000000000013;
const int MMSGFS_IS_AAC				= 0x0000000000000014;
const int MMSGFS_IS_MPEG			= 0x000000000000001C;
const int MMSGFS_AAC_SAMPLERATEINDEX = 0x000000000015;
const int MMSGFS_AAC_DOUBLESAMPLERATEINDEX = 0x000000000016;
const int MMSGFS_VORBIS_FRAMEDURATIONS	= 0x0000000000000017;
const int MMSGFS_VORBIS_CONFIGPACKETS	= 0x0000000000000018;
const int MMSGFS_IS_VORBIS			= 0x0000000000000019;
const int MMSGFS_IS_AC3				= 0x000000000000001A;
const int MMSGFS_IS_DTS				= 0x000000000000001B;

typedef int (_stdcall *RESYNCCALLBACK)(__int64,DWORD,DWORD);

#include "../multimedia_source.h"

const int AUDIOTYPE_CBR			= 0x01;
const int AUDIOTYPE_VBR			= 0x00;
const int AUDIOTYPE_PLAINCBR	= 0x03;
const int AUDIOTYPE_MP3VBR		= 0x04;
const int AUDIOTYPE_MP3CBR		= 0x05;
const int AUDIOTYPE_AC3			= 0x09;
const int AUDIOTYPE_DTS			= 0x11;
const int AUDIOTYPE_PCM			= 0x13;
const int AUDIOTYPE_DIVX		= 0x15;
const int AUDIOTYPE_AAC			= 0x17;
const int AUDIOTYPE_VORBIS		= 0x19;

const int AS_OK					= 0x01;
const int AS_ERR				= -0x01;

const int ASOPEN_ERROR          = 0x00;
const int ASOPEN_OK             = 0x01;

const int SCANFORCBR_ALL		 = 0xFFFFFFFF;
const int FRAMEMODE_ON			 = -0x01;
const int FRAMEMODE_OFF			 = -0x02;
const int FRAMEMODE_SINGLEFRAMES = 0x01;

// abstract, generic, basic audio source class
class AUDIOSOURCE: public MULTIMEDIASOURCE
{
	private:
		int				iFrameMode;
		bool			bSeamless;

	protected:
		bool	virtual	IsSeamless();		
		int		virtual	doClose();
		__int64	virtual	GetExactSize();
	public:
		AUDIOSOURCE();
		~AUDIOSOURCE();
		int		virtual GetAvgBytesPerSec();
		int		virtual GetBitDepth();
		int		virtual GetChannelCount();
		int		virtual GetFormatTag(void);
		__int64 virtual GetFeature(int iFeature);
		void	virtual *GetFormat();
		int		virtual GetFrameMode(void);
		int		virtual GetFrequency();
		int		virtual GetGranularity();
		int		virtual GetOutputFrequency();
		__int64	virtual GetUnstretchedDuration();
		char	virtual *GetCodecID();
		int		virtual GetOffset();
		int		virtual GetType();
		bool	virtual IsCBR();
		int		virtual IsCompatible(AUDIOSOURCE* a);
		int		virtual JoinSeamless(bool bSeamless);
		int		virtual Read(void* lpDest,DWORD dwMicrosecDesired,DWORD* lpdwMicrosecRead,
							__int64* lpqwNanosecRead,__int64* lpiTimeocde = NULL,
							ADVANCEDREAD_INFO* lpAARI = NULL);
		int		virtual Seek(__int64 iPos);
		int		virtual	SetFrameMode(DWORD dwMode);
		void	virtual AssumeCBR(void) {};
		void	virtual AssumeVBR(void) {};
};



#endif