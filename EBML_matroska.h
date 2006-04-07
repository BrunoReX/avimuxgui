#ifndef I_EBML_MATROSKA
#define I_EBML_MATROSKA

#include "ebml.h"


#define CHECKIDs	bool virtual CheckIDs(char* iID,EBMLElement** p);
#define GETTYPSTR	char virtual* GetTypeString();

#define DECL_CLASS(a) \
	class a : public EBML_MatroskaElement \
	{ \
		public: \
			a(STREAM* s,EBMLElement* p); \
			char	virtual* GetTypeString(); \
	};


#define DECL_CLASSCID(c) \
		class c : public EBML_MatroskaElement { \
		protected: \
			CHECKIDs; \
		public: \
			c(STREAM* s,EBMLElement* p); \
		    GETTYPSTR; \
	};

class EBML_MatroskaElement : public EBMLElement
{
	protected:
		__int64			virtual AsInt();
		__int64			virtual AsSInt();
		double			virtual AsFloat();
		bool			virtual CheckGlobalIDs(char* iID,EBMLElement** p);
	public:
		EBML_MatroskaElement();
};

class EBML_Matroska : public EBML_MatroskaElement
{
	protected:
		bool			virtual CheckIDs(char* iID,EBMLElement** p);
	public:
		EBML_Matroska();
		EBML_Matroska(STREAM* s,EBMLElement* p);
};




static const char* REF_NAMES[] = { "","forward","backward","bidirectional" };


#define DOCOMP(a,b) \
if (CompIDs(iID,(char*)a)) { \
		*p = (EBMLElement*)new b(GetSource(),this); \
	} else 

#define DOCOMPL(a,b) \
	DOCOMP(a,b) \
	return false; \
	return true;


DECL_CLASSCID(EBMLM_Seekhead);
DECL_CLASSCID(EBMLM_SIInfo);
DECL_CLASS(EBMLM_SISegmentUID);
DECL_CLASS(EBMLM_SISegmentFilename);
DECL_CLASS(EBMLM_SIPrevUID);
DECL_CLASS(EBMLM_SIPrevFilename);
DECL_CLASS(EBMLM_SINextUID);
DECL_CLASS(EBMLM_SINextFilename);
DECL_CLASS(EBMLM_SITimecodeScale);
DECL_CLASS(EBMLM_SIDuration);
DECL_CLASS(EBMLM_SIDateUTC);
DECL_CLASS(EBMLM_SITitle);
DECL_CLASS(EBMLM_SIMuxingApp);
DECL_CLASS(EBMLM_SIWritingApp);

DECL_CLASSCID(EBMLM_Tracks);
DECL_CLASSCID(EBMLM_TRTrackEntry);
DECL_CLASS(EBMLM_TRTrackNumber);
DECL_CLASS(EBMLM_TRTrackUID);
DECL_CLASS(EBMLM_TRTrackType);
DECL_CLASS(EBMLM_TRFlagEnabled);
DECL_CLASS(EBMLM_TRFlagDefault);
DECL_CLASS(EBMLM_TRFlagLacing);
DECL_CLASS(EBMLM_TRMinCache);
DECL_CLASS(EBMLM_TRMaxCache);
DECL_CLASS(EBMLM_TRDefaultDuration);
DECL_CLASS(EBMLM_TRTrackTimeCodeScale);
DECL_CLASS(EBMLM_TRName);
DECL_CLASS(EBMLM_TRLanguage);
DECL_CLASS(EBMLM_TRCodecID);
DECL_CLASS(EBMLM_TRCodecPrivate);
DECL_CLASS(EBMLM_TRCodecName);
DECL_CLASS(EBMLM_TRCodecSettings);
DECL_CLASS(EBMLM_TRCodecInfoURL);
DECL_CLASS(EBMLM_TRCodecDownloadURL);
DECL_CLASS(EBMLM_TRCodecDecodeAll);
DECL_CLASS(EBMLM_TRTrackOverlay);
DECL_CLASS(EBMLM_TRSampleScale);

DECL_CLASSCID(EBMLM_TRVideo);
DECL_CLASS(EBMLM_TRVFlagInterlaced);
DECL_CLASS(EBMLM_TRVStereoMode);
DECL_CLASS(EBMLM_TRVPixelWidth);
DECL_CLASS(EBMLM_TRVPixelHeight);
DECL_CLASS(EBMLM_TRVDisplayWidth);
DECL_CLASS(EBMLM_TRVDisplayHeight);
DECL_CLASS(EBMLM_TRVDisplayUnit);
DECL_CLASS(EBMLM_TRVAspectRatioType);
DECL_CLASS(EBMLM_TRVColourSpace);
DECL_CLASS(EBMLM_TRVGammaValue);
DECL_CLASSCID(EBMLM_TRAudio);
DECL_CLASS(EBMLM_TRASamplingFrequency);
DECL_CLASS(EBMLM_TRAOutputSamplingFrequency);
DECL_CLASS(EBMLM_TRAChannels);
DECL_CLASS(EBMLM_TRAChannelPositions);
DECL_CLASS(EBMLM_TRABitDepth);
DECL_CLASSCID(EBMLM_TRContentEncodings);
DECL_CLASSCID(EBMLM_TRCEContentEncoding);
DECL_CLASS(EBMLM_TRCEContentEncodingOrder);
DECL_CLASS(EBMLM_TRCEContentEncodingScope);
DECL_CLASS(EBMLM_TRCEContentEncodingType);
DECL_CLASSCID(EBMLM_TRCEContentCompression);
DECL_CLASS(EBMLM_TRCEContentCompAlgo);


DECL_CLASSCID(EBMLM_Cues);
DECL_CLASSCID(EBMLM_CUCuePoint);
DECL_CLASS(EBMLM_CUCueTime);
DECL_CLASSCID(EBMLM_CUCueTrackPositions);
DECL_CLASS(EBMLM_CUCueTrack);
DECL_CLASS(EBMLM_CUCueClusterPosition);
DECL_CLASS(EBMLM_CUCueBlockNumber);
DECL_CLASS(EBMLM_CUCueCodecState);
DECL_CLASSCID(EBMLM_CUCueReference);
DECL_CLASS(EBMLM_CUCueRefTime);
DECL_CLASS(EBMLM_CUCueRefCluster);
DECL_CLASS(EBMLM_CUCueRefNumber);
DECL_CLASS(EBMLM_CUCueRefCodecState);


DECL_CLASSCID(EBMLM_Attachments);
DECL_CLASSCID(EBMLM_ATAttachedFile);
DECL_CLASS(EBMLM_ATFileDescription);
DECL_CLASS(EBMLM_ATFileName);
DECL_CLASS(EBMLM_ATFileMimeType);
DECL_CLASS(EBMLM_ATFileData);
DECL_CLASS(EBMLM_ATFileUID);


DECL_CLASSCID(EBMLM_Chapters);
DECL_CLASSCID(EBMLM_CHEditionEntry);

DECL_CLASSCID(EBMLM_CHChapterAtom);
DECL_CLASS(EBMLM_CHChapterUID);
DECL_CLASS(EBMLM_CHChapterTimeStart);
DECL_CLASS(EBMLM_CHChapterFlagEnabled);
DECL_CLASS(EBMLM_CHChapterFlagHidden);
DECL_CLASS(EBMLM_CHChapterTimeEnd);
DECL_CLASSCID(EBMLM_CHChapterTrack);
DECL_CLASS(EBMLM_CHChapterTrackNumber);
DECL_CLASSCID(EBMLM_CHChapterDisplay);
DECL_CLASS(EBMLM_CHChapString);
DECL_CLASS(EBMLM_CHChapLanguage);
DECL_CLASS(EBMLM_CHChapCountry);

DECL_CLASSCID(EBMLM_Tags);
DECL_CLASSCID(EBMLM_TGTag);
DECL_CLASSCID(EBMLM_TGTarget);
DECL_CLASS(EBMLM_TGBitsPS);
DECL_CLASS(EBMLM_TGFramesPS);
DECL_CLASS(EBMLM_TGTrackUID);
DECL_CLASS(EBMLM_TGChapterUID);
DECL_CLASSCID(EBMLM_TGSimpleTag);
DECL_CLASS(EBMLM_TGTagName);
DECL_CLASS(EBMLM_TGTagString);
DECL_CLASS(EBMLM_TGTagBinary);


// Tags
/*class EBMLM_Tags : public EBML_MatroskaElement
{
	public:
		EBMLM_Tags(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};
*/
// Void
class EBMLM_Void : public EBML_MatroskaElement
{
	public:
		EBMLM_Void(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// CRC32
class EBMLM_CRC32 : public EBML_MatroskaElement
{
	public:
		EBMLM_CRC32(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// Seek
class EBMLM_Seek : public EBML_MatroskaElement
{
	public:
		bool			virtual CheckIDs(char* iID,EBMLElement** p);
	public:
		EBMLM_Seek(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// SeekID
class EBMLM_SeekID : public EBML_MatroskaElement
{
	public:
		EBMLM_SeekID(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// Seek
class EBMLM_SeekPosition : public EBML_MatroskaElement
{
	public:
		EBMLM_SeekPosition(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// Cluster Timecode
class EBMLM_CLTimeCode : public EBML_MatroskaElement
{
	public:
		EBMLM_CLTimeCode(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// Cluster Position
class EBMLM_CLPosition : public EBML_MatroskaElement
{
	public:
		EBMLM_CLPosition(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// Cluster Prevsize
class EBMLM_CLPrevSize : public EBML_MatroskaElement
{
	public:
		EBMLM_CLPrevSize(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// Cluster BlockGroup
class EBMLM_CLBlockGroup : public EBML_MatroskaElement
{
	private:
		CHECKIDs;
	public:
		EBMLM_CLBlockGroup(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};





// Cluster BlockVirtual
class EBMLM_CLBlockVirtual : public EBML_MatroskaElement
{
	public:
		EBMLM_CLBlockVirtual(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

DECL_CLASS(EBMLM_CLBlockSamples)

// Cluster BlockAdditions
class EBMLM_CLBlockAdditions : public EBML_MatroskaElement
{
	private:
		CHECKIDs;
	public:
		EBMLM_CLBlockAdditions(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// Cluster BlockMore
class EBMLM_CLBlockMore : public EBML_MatroskaElement
{
	private:
		CHECKIDs
	public:
		EBMLM_CLBlockMore(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// Cluster BlockAddID
class EBMLM_CLBlockAddID : public EBML_MatroskaElement
{
	public:
		EBMLM_CLBlockAddID(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// Cluster AdditionalBlock
class EBMLM_CLAdditionalBlock : public EBML_MatroskaElement
{
	public:
		EBMLM_CLAdditionalBlock(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// Cluster BlockDuration
class EBMLM_CLBlockDuration : public EBML_MatroskaElement
{
	public:
		EBMLM_CLBlockDuration(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// Cluster ReferencePriority
class EBMLM_CLReferencePriority : public EBML_MatroskaElement
{
	public:
		EBMLM_CLReferencePriority(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// Cluster ReferenceBlock
class EBMLM_CLReferenceBlock : public EBML_MatroskaElement
{
	public:
		EBMLM_CLReferenceBlock(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// Cluster ReferenceVirtual
class EBMLM_CLReferenceVirtual : public EBML_MatroskaElement
{
	public:
		EBMLM_CLReferenceVirtual(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// Cluster CodecState
class EBMLM_CLCodecState : public EBML_MatroskaElement
{
	public:
		EBMLM_CLCodecState(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// Cluster TimeSlice
class EBMLM_CLTimeSlice : public EBML_MatroskaElement
{	private:
		CHECKIDs;
	public:
		EBMLM_CLTimeSlice(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// Cluster TS LaceNumber
class EBMLM_CLTSLaceNumber : public EBML_MatroskaElement
{
	public:
		EBMLM_CLTSLaceNumber(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// Cluster TS FrameNumber
class EBMLM_CLTSFrameNumber : public EBML_MatroskaElement
{
	public:
		EBMLM_CLTSFrameNumber(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// Cluster TS BlockAdditionID
class EBMLM_CLTSBlockAdditionID : public EBML_MatroskaElement
{
	public:
		EBMLM_CLTSBlockAdditionID(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// Cluster TS Delay
class EBMLM_CLTSDelay : public EBML_MatroskaElement
{
	public:
		EBMLM_CLTSDelay(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

// Cluster TS Duration
class EBMLM_CLTSDuration : public EBML_MatroskaElement
{
	public:
		EBMLM_CLTSDuration(STREAM* s,EBMLElement* p);
		GETTYPSTR;
};

#endif
