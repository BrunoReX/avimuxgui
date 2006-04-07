#ifndef I_MATROSKAIDS
#define I_MATROSKAIDS

#pragma warning(disable:4305)
#pragma warning(disable:4309)

#include "EBML_IDs.h"

#define uint32 unsigned __int32

const char MID_CRC32 [] =				{ 0xBF };
const char MID_VOID [] =				{ 0xEC };
const char MID_SIGNATURESLOT [] =		{ 0x1B, 0x53, 0x86, 0x67 };
const char MID_SIGNATUREALGO [] =		{ 0x7E, 0x8A };
const char MID_SIGNATUREHASH [] =		{ 0x7E, 0x9A };
const char MID_SIGNATUREPUBLICKEY [] =	{ 0x7E, 0xA5 };
const char MID_SIGNATURE [] =			{ 0x7E, 0xB5 };
const char MID_SIGNATUREELEMENTS [] =	{ 0x7E, 0x5B };
const char MID_SIGNATUREELEMENTLIST [] ={ 0x7E, 0x7B };
const char MID_SIGNEDELEMENT [] =		{ 0x86 };

const char MID_SEGMENT [] =				{ 0x18, 0x53, 0x80, 0x67 };
const char MID_SEEKHEAD [] =			{ 0x11, 0x4D, 0x9B, 0x74 };
const char MID_MS_SEEK [] =				{ 0x4D, 0xBB };
const char MID_MS_SEEKID [] =			{ 0x53, 0xAB };
const char MID_MS_SEEKPOSITION [] =		{ 0x53, 0xAC };

const char MID_SEGMENTINFO [] =			{ 0x15, 0x49, 0xA9, 0x66 };
const char MID_SI_SEGMENTUID [] =		{ 0x73, 0xA4 };
const char MID_SI_SEGMENTFAMILY [] =    { 0x44, 0x44 };
const char MID_SI_SEGMENTFILENAME [] =	{ 0x73, 0x84 };
const char MID_SI_PREVUID [] =			{ 0x3C, 0xB9, 0x23 };
const char MID_SI_PREVFILENAME [] =		{ 0x3C, 0x83, 0xAB };
const char MID_SI_NEXTUID [] =			{ 0x3E, 0xB9, 0x23 };
const char MID_SI_NEXTFILENAME [] =		{ 0x3E, 0x83, 0xBB };
const char MID_SI_TIMECODESCALE [] =	{ 0x2A, 0xD7, 0xB1 };
const char MID_SI_DURATION [] =			{ 0x44, 0x89 };
const char MID_SI_DATEUTC [] =			{ 0x44, 0x61 };
const char MID_SI_TITLE [] =			{ 0x7B, 0xA9 };
const char MID_SI_MUXINGAPP [] =		{ 0x4D, 0x80 };
const char MID_SI_WRITINGAPP [] =		{ 0x57, 0x41 };

const char MID_CLUSTER [] =				{ 0x1F, 0x43, 0xB6, 0x75 };
const char MID_CL_TIMECODE [] =			{ 0xE7 };
const char MID_CL_POSITION [] =			{ 0xA7 };
const char MID_CL_PREVSIZE [] =			{ 0xAB };
const char MID_CL_BLOCKGROUP [] =		{ 0xA0 };
const char MID_CL_BLOCK [] =			{ 0xA1 };
const char MID_CL_BLOCKVIRTUAL [] =		{ 0xA2 };
const char MID_CL_SIMPLEBLOCK [] =		{ 0xA3 };
const char MID_CL_BLOCKADDITIONS [] =	{ 0x75, 0xA1 };
const char MID_CL_BLOCKMORE [] =		{ 0xA6 };
const char MID_CL_BLOCKADDID [] =		{ 0xEE };
const char MID_CL_BLOCKADDITIONAL [] =	{ 0xA5 };
const char MID_CL_BLOCKDURATION [] =	{ 0x9B };
const char MID_CL_REFERENCEPRIORITY []=	{ 0xFA };
const char MID_CL_REFERENCEBLOCK [] =	{ 0xFB };
const char MID_CL_REFERENCEVIRTUAL []=	{ 0xFD };
const char MID_CL_CODECSTATE [] =		{ 0xA4 };
const char MID_CL_TIMESLICE [] =		{ 0xE8 };
const char MID_CL_LACENUMBER [] =		{ 0xCC };
const char MID_CL_FRAMENUMBER [] =		{ 0xCD };
const char MID_CL_BLOCKADDITIONID [] =	{ 0xCB };
const char MID_CL_DELAY [] =			{ 0xCE };
const char MID_CL_DURATION [] =			{ 0xCF };
const char MID_CL_BLOCKSAMPLES [] =     { 0xEF };
const char MID_CL_SILENTTRACKS [] =     { 0x58, 0x54 };
const char MID_CL_SILENTTRACKNUMBER [] ={ 0x58, 0xD7 };

static const char* MID_3BYTEIDS [] = { 
	MID_SI_PREVUID, MID_SI_PREVFILENAME, MID_SI_NEXTUID, MID_SI_NEXTFILENAME, MID_SI_TIMECODESCALE,
	};

const char MID_TRACKS [] =				{ 0x16, 0x54, 0xAE, 0x6B };
const char MID_TR_TRACKENTRY [] =		{ 0xAE };
const char MID_TR_TRACKNUMBER [] =		{ 0xD7 };
const char MID_TR_TRACKUID [] =			{ 0x73, 0xC5 };
const char MID_TR_TRACKTYPE [] =		{ 0x83 };
const char MID_TR_FLAGENABLED [] =		{ 0xB9 };
const char MID_TR_FLAGDEFAULT [] =		{ 0x88 };
const char MID_TR_FLAGLACING [] =		{ 0x9C };
const char MID_TR_FLAGFORCED [] =       { 0x55, 0xAA };
const char MID_TR_MINCACHE [] =			{ 0x6D, 0xE7 };
const char MID_TR_MAXCACHE [] =			{ 0x6D, 0xF8 };
const char MID_TR_TRACKOFFSET [] =      { 0x53, 0x7F };
const char MID_TR_DEFAULTDURATION [] =	{ 0x23, 0xE3, 0x83 };
const char MID_TR_TRACKTIMECODESCALE []={ 0x23, 0x31, 0x4F };
const char MID_TR_NAME	[] =			{ 0x53, 0x6E };
const char MID_TR_LANGUAGE [] =			{ 0x22, 0xB5, 0x9C };
const char MID_TR_CODECID [] =			{ 0x86 };
const char MID_TR_CODECPRIVATE [] =		{ 0x63, 0xA2 };
const char MID_TR_CODECNAME [] =		{ 0x25, 0x86, 0x88 };
const char MID_TR_CODECSETTINGS [] =	{ 0x3A, 0x96, 0x97 };
const char MID_TR_CODECINFOURL [] =		{ 0x3B, 0x40, 0x40 };
const char MID_TR_CODECDOWNLOADURL [] =	{ 0x26, 0xB2, 0x40 };
const char MID_TR_CODECDECODEALL [] =	{ 0xAA };
const char MID_TR_TRACKOVERLAY [] =		{ 0x6F, 0xAB };
const char MID_TR_SAMPLESCALE [] =      { 0x67, 0xEF };
const char MID_TR_MAXBLOCKADDITIONID []={ 0x55, 0xEE };
const char MID_TR_VIDEO [] =			{ 0xE0 };
const char MID_TRV_FLAGINTERLACED [] =	{ 0x9A };
const char MID_TRV_STEREOMODE [] =		{ 0x53, 0xB8 };
const char MID_TRV_PIXELWIDTH [] =		{ 0xB0 };
const char MID_TRV_PIXELHEIGHT [] =		{ 0xBA };
const char MID_TRV_PIXELCROPBOTTOM [] = { 0x54, 0xAA };
const char MID_TRV_PIXELCROPTOP []    = { 0x54, 0xBB };
const char MID_TRV_PIXELCROPLEFT []   = { 0x54, 0xCC };
const char MID_TRV_PIXELCROPRIGHT []  = { 0x54, 0xDD };

const char MID_TRV_DISPLAYWIDTH [] =	{ 0x54, 0xB0 };
const char MID_TRV_DISPLAYHEIGHT [] =	{ 0x54, 0xBA };
const char MID_TRV_DISPLAYUNIT [] =		{ 0x54, 0xB2 };
const char MID_TRV_ASPECTRATIO [] =		{ 0x54, 0xB3 };
const char MID_TRV_COLORSPACE [] =		{ 0x2E, 0xB5, 0x24 };
const char MID_TRV_GAMMAVALUE [] =		{ 0x2F, 0xB5, 0x23 };
const char MID_TR_CONTENTENCODINGS [] = { 0x6D, 0x80 };
const char MID_TRCE_CONTENTENCODING [] ={ 0x62, 0x40 };
const char MID_TRCE_CONTENTENCODINGORDER [] = { 0x50, 0x31 };
const char MID_TRCE_CONTENTENCODINGSCOPE [] = { 0x50, 0x32 };
const char MID_TRCE_CONTENTENCODINGTYPE []  = { 0x50, 0x33 };
const char MID_TRCE_CONTENTCOMPRESSION[]={ 0x50, 0x34 };
const char MID_TRCE_CONTENTCOMPALGO [] ={ 0x42, 0x54 };
const char MID_TRCE_CONTENTCOMPSETTINGS [] ={ 0x42, 0x55 };



const char MID_TR_AUDIO	[] =			{ 0xE1 };
const char MID_TRA_SAMPLINGFREQUENCY []={ 0xB5 };
const char MID_TRA_OUTPUTSAMPLINGFREQUENCY []={ 0x78, 0xB5 };
const char MID_TRA_CHANNELS [] =		{ 0x9F };
const char MID_TRA_CHANNELPOSITIONS [] ={ 0x7D, 0x7B };
const char MID_TRA_BITDEPTH	[] =		{ 0x62, 0x64 };

const char MID_CUES [] =				{ 0x1C, 0x53, 0xBB, 0x6B };
const char MID_CU_CUEPOINT [] =			{ 0xBB };
const char MID_CU_CUETIME [] =			{ 0xB3 };
const char MID_CU_CUETRACKPOSITIONS []=	{ 0xB7 };
const char MID_CU_CUETRACK [] =			{ 0xF7 };
const char MID_CU_CUECLUSTERPOSITION [] =	{ 0xF1 };
const char MID_CU_CUEBLOCKNUMBER [] =	{ 0x53, 0x78 };
const char MID_CU_CUECODECSTATE [] =	{ 0xEA };
const char MID_CU_CUEREFERENCE [] =		{ 0xDB };
const char MID_CU_CUEREFTIME [] =		{ 0x96 };
const char MID_CU_CUEREFCLUSTER [] =	{ 0x97 };
const char MID_CU_CUEREFNUMBER [] =		{ 0x53, 0x5F };
const char MID_CU_CUEREFCODECSTATE [] =	{ 0xEB };

const char MID_ATTACHMENTS [] =			{ 0x19, 0x41, 0xA4, 0x69 };
const char MID_AT_ATTACHEDFILE [] =		{ 0x61, 0xA7 };
const char MID_AT_FILEDESCRIPTION [] =	{ 0x46, 0x7E };
const char MID_AT_FILENAME [] =			{ 0x46, 0x6E };
const char MID_AT_FILEMIMETYPE [] =		{ 0x46, 0x60 };
const char MID_AT_FILEDATA [] =			{ 0x46, 0x5C };
const char MID_AT_FILEUID [] =			{ 0x46, 0xAE };

const char MID_CHAPTERS [] =			{ 0x10, 0x43, 0xA7, 0x70 };
const char MID_CH_EDITIONENTRY [] =		{ 0x45, 0xB9 };
const char MID_CH_EDITIONUID [] =       { 0x45, 0xBC };
const char MID_CH_EDITIONFLAGHIDDEN [] ={ 0x45, 0xBD };
const char MID_CH_EDITIONFLAGDEFAULT []={ 0x45, 0xDB };
const char MID_CH_EDITIONFLAGORDERED []={ 0x45, 0xDD };


const char MID_CH_CHAPTERMASTER [] =	{ 0xA8 };
const char MID_CH_CHAPTERATOM [] =		{ 0xB6 };
const char MID_CH_CHAPTERUID [] =		{ 0x73, 0xC4 };
const char MID_CH_CHAPTERTIMESTART [] =	{ 0x91 };
const char MID_CH_CHAPTERTIMEEND [] =	{ 0x92 };
const char MID_CH_CHAPTERTRACK [] =		{ 0x8F };
const char MID_CH_CHAPTERTRACKNUMBER []={ 0x89 };
const char MID_CH_CHAPTERDISPLAY [] =	{ 0x80 };
const char MID_CH_CHAPSTRING []	=		{ 0x85 };
const char MID_CH_CHAPLANGUAGE [] =		{ 0x43, 0x7C };
const char MID_CH_CHAPCOUNTRY [] =		{ 0x43, 0x7E };
const char MID_CH_CHAPTERFLAGENABLED []={ 0x45, 0x98 };
const char MID_CH_CHAPTERFLAGHIDDEN [] ={ 0x98 };
const char MID_CH_CHAPTERSEGMENTUID [] ={ 0x6E, 0x67 };
const char MID_CH_CHAPTERPHYSICALEQUIV[]={ 0x63, 0xC3 };
const char MID_CH_CHAPPROCESS [] =      { 0x69, 0x44 };
const char MID_CH_CHAPPROCESSCODECID []={ 0x69, 0x55 };
const char MID_CH_CHAPPROCESSPRIVATE []={ 0x45, 0x0D };
const char MID_CH_CHAPPROCESSCOMMAND []={ 0x69, 0x11 };
const char MID_CH_CHAPPROCESSTIME [] =  { 0x69, 0x22 };
const char MID_CH_CHAPPROCESSDATA [] =  { 0x69, 0x33 };

const char MID_TAGS [] =				{ 0x12, 0x54, 0xC3, 0x67 };
const char MID_TG_TAG [] =              { 0x73, 0x73 };
const char MID_TG_TARGET [] =           { 0x63, 0xC0 };
const char MID_TG_TRACKUID [] =         { 0x63, 0xC5 };
const char MID_TG_TARGETTYPEVALUE [] =  { 0x68, 0xCA };
const char MID_TG_CHAPTERUID [] =       { 0x63, 0xC4 };
const char MID_TG_EDITIONUID [] =       { 0x63, 0xC9 };
const char MID_TG_ATTACHEMENTUID [] =   { 0x63, 0xC6 };

const char MID_TG_BITSPS [] =           { 0x44, 0x85 };
const char MID_TG_FRAMESPS [] =         { 0x44, 0x86 };

const char MID_TG_SIMPLETAG [] =        { 0x67, 0xC8 };
const char MID_TG_TAGNAME [] =          { 0x45, 0xA3 };
const char MID_TG_TAGSTRING [] =        { 0x44, 0x87 };
const char MID_TG_TAGBINARY [] =        { 0x44, 0x85 };
const char MID_TG_TAGLANGUAGE [] =      { 0x44, 0x7A };

#define IDVALUE1(a) (uint32)(a[0])
#define IDVALUE2(a) ((uint32)(a[1])<<8) + (uint32)(a[0]) 
#define IDVALUE3(a) ((uint32)(a[2])<<16) + ((uint32)(a[1])<<8) + (uint32)(a[0]) 
#define IDVALUE4(a) ((uint32)(a[3]<<24)) + ((uint32)(a[2])<<16) + ((uint32)(a[1])<<8) + (uint32)(a[0]) 
#define IDVALUEb(a,b) ((b==1)?IDVALUE1(a):((b==2)?IDVALUE2(a):((b==3)?IDVALUE3(a):((b==4)?IDVALUE4(a):-1))))
#define IDVALUE(a) (IDVALUEb(a,VSizeInt_Len[unsigned char(a[0])]))

const static char* MID_4BYTEIDS [] = {
	EID_EBML, MID_SEGMENT, MID_SEGMENTINFO, MID_CLUSTER, MID_TRACKS, MID_CUES,
	MID_ATTACHMENTS, MID_CHAPTERS, MID_TAGS
};

const static EID_DESCRIPTOR MID_4BYTEDESCR [] = {
	{ "EBML", (char*)EID_EBML },
	{ "Segment", (char*)MID_SEGMENT },
	{ "SegmentInfo", (char*)MID_SEGMENTINFO },
	{ "Seekhead", (char*)MID_SEEKHEAD },
	{ "Cluster", (char*)MID_CLUSTER },
	{ "Tracks", (char*)MID_TRACKS },
	{ "Cues", (char*)MID_CUES },
	{ "Attachments", (char*)MID_ATTACHMENTS },
	{ "Chapters", (char*)MID_CHAPTERS },
	{ "Tags", (char*)MID_TAGS }
};

const static int MID_4BYTEDESCR_COUNT = sizeof(MID_4BYTEDESCR)/sizeof(MID_4BYTEDESCR[0]);

const int ETM_FILE					= 0x7F;

const int ETM_TR_TRACKTIMECODESCALE	= 0x4F3123;
const int ETM_TR_NAME				= 0xE653;
const int ETM_TR_LANGUAGE			= 0x9CB522;
const int ETM_TR_CODECID			= 0x86;
const int ETM_TR_CODECPRIVATE		= 0xA263;
const int ETM_TR_CODECNAME			= 0x888625;
const int ETM_TR_CODECSETTINGS		= 0x97963A;
const int ETM_TR_CODECINFOURL		= 0x40403B;
const int ETM_TR_CODECDOWNLOADURL	= 0x40B226;
const int ETM_TR_CODECDECODEALL		= 0xAA;
const int ETM_TR_TRACKOVERLAY		= 0xAB6F;
const int ETM_TR_SAMPLESCALE        = 0xEF67;

const int ETM_TR_VIDEO				= 0xE0;
const int ETM_TR_AUDIO				= 0xE1;

const int ETM_TR_CONTENTENCODINGS   = 0x6D80;

const int ETM_SI_SEGMENTUID			= 0xA473;
const int ETM_SI_SEGMENTFILENAME	= 0x8473;
const int ETM_SI_PREVUID			= 0x23B93C;
const int ETM_SI_PREVFILENAME		= 0xAB833C;
const int ETM_SI_NEXTUID			= 0x23B93E;
const int ETM_SI_NEXTFILENAME		= 0xBB833E;
const int ETM_SI_TIMECODESCALE		= 0xB1D72A;
const int ETM_SI_DURATION			= 0x8944;
const int ETM_SI_DATEUTC			= 0x6144;
const int ETM_SI_TITLE				= 0xA97B;
const int ETM_SI_MUXINGAPP			= 0x804D;
const int ETM_SI_WRITINGAPP			= 0x4157;

const int ETM_CH_CHAPTERATOM		= 0xB6;

const int ETM_CH_CHAPTERTRACKNUMBER = 0x89;
const int ETM_CH_CHAPTERDISPLAY		= 0x80;
const int ETM_CH_CHAPSTRING			= 0x85;
const int ETM_CH_CHAPLANGUAGE		= 0x7C43;
const int ETM_CH_CHAPCOUNTRY		= 0x7E43;
const int ETM_CH_CHAPTERSEGMENTUID  = IDVALUE(MID_CH_CHAPTERSEGMENTUID);

const int ETM_TG_TAG				= IDVALUE(MID_TG_TAG);//0x17A;
const int ETM_TG_TARGET				= IDVALUE(MID_TG_TARGET);//0x17B;
const int ETM_TG_TRACKUID			= IDVALUE(MID_TG_TRACKUID);//0x17C;
const int ETM_TG_CHAPTERUID			= IDVALUE(MID_TG_CHAPTERUID);//0x17D;
const int ETM_TG_BITSPS				= 0x181;

const int ETM_TG_FRAMESPS			= 0x185;
const int ETM_TG_SIMPLETAG          = IDVALUE(MID_TG_SIMPLETAG);//0xC867;
const int ETM_TG_TAGNAME            = IDVALUE(MID_TG_TAGNAME);//0xA348;
const int ETM_TG_TAGSTRING          = IDVALUE(MID_TG_TAGSTRING);//0x8744;
const int ETM_TG_TAGBINARY          = IDVALUE(MID_TG_TAGBINARY);//0x8544;

const int MSTRT_VIDEO = 1;
const int MSTRT_AUDIO = 2;
const int MSTRT_SUBT  = 0x11;

const int MTR_TRACKTYPE_VIDEO = 1;
const int MTR_TRACKTYPE_AUDIO = 2;
const int MTR_TRACKTYPE_SUBTITLE  = 0x11;


const int MDISPU_PIXEL = 0;
const int MDISPU_CM    = 1;
const int MDISPU_INCH  = 2;

const int RBIREF_FORWARD       = 0x01;
const int RBIREF_BACKWARD      = 0x02;

const int RBIF_KEYFRAME			= 0x04;

const int RBIREF_BIDIRECTIONAL = 0x03;
const int RBIREF_MASK		   = 0x03;

const int RBIF_DURATION		   = 0x0100;
const int RBIF_DISCARDABLE     = 0x0001;


const int READBL_OK				= 0x01;
const int READBL_ENDOFCLUSTER	= 0x02;
const int READBL_ENDOFSEGMENT	= 0x04;
const int READBL_ENDOFTRACK     = 0x40;
const int READBL_FILEB0RKED		= 0x08;
const int READBL_SPARSEQUEUEEMPTY = 0x10;
const int READBL_STREAMNOWSPARSE = 0x20;

const int NEXTCL_OK				= 0x01;
const int NEXTCL_ENDOFSEGMENT	= 0x02;

const int MRF_CURRENT			= 0x00; // read from active stream
const int MRF_SPARSE			= 0x01; // stream to read from is sparse
const int MRF_NEXT				= 0x02; // read temporally next block
const int MRF_FILEORDER         = 0x04; // read blocks in the same order as they came from the file


// reading blocks
const int BLKHDRF_LACING		= 0x02;
const int BLKHDRF_LACINGNEW		= 0x06; // mask for new lacing
const int BLKHDRF_LACINGMASK	= 0x06; // mask for new lacing
const int BLKHDRF_LACINGEBML	= 0x06; // mask for new lacing
const int BLKHDRF_LACINGCONST	= 0x04;

//const int BLKHDRF_ENDOFTRACK	= 0x01; // un-defined
const int BLKHDRF_DISCARDABLE	= 0x01;
const int BLKHDRF_KEYFRAME		= 0x80;
const int BLKHDRF_INVISIBLE		= 0x08;

const int RIF_DURATION			= RBIF_DURATION;
const int RIF_DISCARDABLE		= 0x01;


//const int RIF_LACING			= 0x02;

const int MSRI_OK				= 0x01;
const int MSRI_FATALERROR       = -0x01;



#endif