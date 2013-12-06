#ifndef I_AVISTRUCTS
#define I_AVISTRUCTS

// stuff from vfw/mmreg

#ifndef MKFOURCC
#define MKFOURCC( ch0, ch1, ch2, ch3 )                                    \
		( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |	\
		( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif

#pragma pack(push, 1)

#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_
typedef struct tWAVEFORMATEX
{
    WORD    wFormatTag;        /* format type */
    WORD    nChannels;         /* number of channels (i.e. mono, stereo...) */
    DWORD   nSamplesPerSec;    /* sample rate */
    DWORD   nAvgBytesPerSec;   /* for buffer estimation */
    WORD    nBlockAlign;       /* block size of data */
    WORD    wBitsPerSample;    /* Number of bits per sample of mono data */
    WORD    cbSize;            /* The count in bytes of the size of
                                    extra information (after cbSize) */

} WAVEFORMATEX;
#endif

#define AVIF_HASINDEX		0x00000010	// Index at end of file?
#define AVIF_MUSTUSEINDEX	0x00000020
#define AVIF_ISINTERLEAVED	0x00000100
#define AVIF_WASCAPTUREFILE	0x00010000
#define AVIF_COPYRIGHTED	0x00020000

#define AVI_HEADERSIZE  2048                    // size of AVI header list

typedef struct
{
    DWORD		dwMicroSecPerFrame;	// frame display rate (or 0L)
    DWORD		dwMaxBytesPerSec;	// max. transfer rate
    DWORD		dwPaddingGranularity;	// pad to multiples of this
                                                // size; normally 2K.
    DWORD		dwFlags;		// the ever-present flags
    DWORD		dwTotalFrames;		// # frames in file
    DWORD		dwInitialFrames;
    DWORD		dwStreams;
    DWORD		dwSuggestedBufferSize;

    DWORD		dwWidth;
    DWORD		dwHeight;

    DWORD		dwReserved[4];
} MainAVIHeader;


/*
** Stream header
*/

#define AVISF_DISABLED			0x00000001

#define AVISF_VIDEO_PALCHANGES		0x00010000
#define FOURCC DWORD

typedef struct {
    FOURCC		fccType;
    FOURCC		fccHandler;
    DWORD		dwFlags;	/* Contains AVITF_* flags */
    WORD		wPriority;
    WORD		wLanguage;
    DWORD		dwInitialFrames;
    DWORD		dwScale;	
    DWORD		dwRate;	/* dwRate / dwScale == samples/second */
    DWORD		dwStart;
    DWORD		dwLength; /* In units above... */
    DWORD		dwSuggestedBufferSize;
    DWORD		dwQuality;
    DWORD		dwSampleSize;
    RECT		rcFrame;
} AVIStreamHeader;

/* Flags for index */
#define AVIIF_LIST          0x00000001L // chunk is a 'LIST'
#define AVIIF_KEYFRAME      0x00000010L // this frame is a key frame.
#define AVIIF_FIRSTPART     0x00000020L // this frame is the start of a partial frame.
#define AVIIF_LASTPART      0x00000040L // this frame is the end of a partial frame.
#define AVIIF_MIDPART       (AVIIF_LASTPART|AVIIF_FIRSTPART)

#define AVIIF_NOTIME	    0x00000100L // this frame doesn't take any time
#define AVIIF_COMPUSE       0x0FFF0000L // these bits are for compressor use

typedef struct
{
    DWORD		ckid;
    DWORD		dwFlags;
    DWORD		dwChunkOffset;		// Position of chunk
    DWORD		dwChunkLength;		// Length of chunk
} AVIINDEXENTRY;

#define MPEGLAYER3_WFX_EXTRA_BYTES   12

// WAVE_FORMAT_MPEGLAYER3 format sructure
//

typedef struct mpeglayer3waveformat_tag {
  WAVEFORMATEX  wfx;
  WORD          wID;
  DWORD         fdwFlags;
  WORD          nBlockSize;
  WORD          nFramesPerBlock;
  WORD          nCodecDelay;
} MPEGLAYER3WAVEFORMAT;

/*  MPEG-1 audio wave format (audio layer only).   (0x0050)   */
typedef struct mpeg1waveformat_tag {
    WAVEFORMATEX    wfx;
    WORD            fwHeadLayer;
    DWORD           dwHeadBitrate;
    WORD            fwHeadMode;
    WORD            fwHeadModeExt;
    WORD            wHeadEmphasis;
    WORD            fwHeadFlags;
    DWORD           dwPTSLow;
    DWORD           dwPTSHigh;
} MPEG1WAVEFORMAT;
typedef MPEG1WAVEFORMAT                 *PMPEG1WAVEFORMAT;
typedef MPEG1WAVEFORMAT NEAR           *NPMPEG1WAVEFORMAT;
typedef MPEG1WAVEFORMAT FAR            *LPMPEG1WAVEFORMAT;

#define ACM_MPEG_LAYER1             (0x0001)
#define ACM_MPEG_LAYER2             (0x0002)
#define ACM_MPEG_LAYER3             (0x0004)
#define ACM_MPEG_STEREO             (0x0001)
#define ACM_MPEG_JOINTSTEREO        (0x0002)
#define ACM_MPEG_DUALCHANNEL        (0x0004)
#define ACM_MPEG_SINGLECHANNEL      (0x0008)
#define ACM_MPEG_PRIVATEBIT         (0x0001)
#define ACM_MPEG_COPYRIGHT          (0x0002)
#define ACM_MPEG_ORIGINALHOME       (0x0004)
#define ACM_MPEG_PROTECTIONBIT      (0x0008)
#define ACM_MPEG_ID_MPEG1           (0x0010)

#pragma pack(pop)

#endif