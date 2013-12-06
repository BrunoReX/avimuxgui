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
DECL_CLASS(EBMLM_SISegmentFamily);
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
DECL_CLASS(EBMLM_TRFlagForced);
DECL_CLASS(EBMLM_TRTrackOffset);
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
DECL_CLASS(EBMLM_TRMaxBlockAdditionID);

DECL_CLASSCID(EBMLM_TRVideo);
DECL_CLASS(EBMLM_TRVFlagInterlaced);
DECL_CLASS(EBMLM_TRVStereoMode);
DECL_CLASS(EBMLM_TRVPixelWidth);
DECL_CLASS(EBMLM_TRVPixelHeight);
DECL_CLASS(EBMLM_TRVPixelCropLeft);
DECL_CLASS(EBMLM_TRVPixelCropRight);
DECL_CLASS(EBMLM_TRVPixelCropTop);
DECL_CLASS(EBMLM_TRVPixelCropBottom);
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
DECL_CLASS(EBMLM_TRCEContentCompressionSettings);
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
DECL_CLASS(EBMLM_CHEditionUID);
DECL_CLASS(EBMLM_CHEditionFlagDefault);
DECL_CLASS(EBMLM_CHEditionFlagHidden);
DECL_CLASS(EBMLM_CHEditionFlagOrdered);

DECL_CLASSCID(EBMLM_CHChapProcess);
DECL_CLASSCID(EBMLM_CHChapProcessCommand);
DECL_CLASS(EBMLM_CHChapProcessCodecID);
DECL_CLASS(EBMLM_CHChapProcessPrivate);
DECL_CLASS(EBMLM_CHChapProcessTime);
DECL_CLASS(EBMLM_CHChapProcessData);

DECL_CLASSCID(EBMLM_CHChapterAtom);
DECL_CLASS(EBMLM_CHChapterUID);
DECL_CLASS(EBMLM_CHChapterTimeStart);
DECL_CLASS(EBMLM_CHChapterFlagEnabled);
DECL_CLASS(EBMLM_CHChapterFlagHidden);
DECL_CLASS(EBMLM_CHChapterTimeEnd);
DECL_CLASS(EBMLM_CHChapterPhysicalEquiv);
DECL_CLASS(EBMLM_CHChapterSegmentUID);
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
DECL_CLASS(EBMLM_TGEditionUID);
DECL_CLASS(EBMLM_TGAttachementUID);
DECL_CLASSCID(EBMLM_TGSimpleTag);
DECL_CLASS(EBMLM_TGTagName);
DECL_CLASS(EBMLM_TGTagString);
DECL_CLASS(EBMLM_TGTagBinary);
DECL_CLASS(EBMLM_TGTargetTypeValue);
DECL_CLASS(EBMLM_TGTagLanguage);


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
DECL_CLASSCID(EBMLM_Seek);
DECL_CLASS(EBMLM_SeekID);
DECL_CLASS(EBMLM_SeekPosition);

DECL_CLASS(EBMLM_CLTimeCode);
DECL_CLASS(EBMLM_CLPosition);
DECL_CLASS(EBMLM_CLPrevSize);
DECL_CLASSCID(EBMLM_CLBlockGroup);

// no separate class for simple block!
//DECL_CLASS(EBMLM_CLSimpleBlock);


DECL_CLASS(EBMLM_CLBlockVirtual);
DECL_CLASSCID(EBMLM_CLBlockAdditions);
DECL_CLASSCID(EBMLM_CLBlockMore);
DECL_CLASS(EBMLM_CLBlockAddID);
DECL_CLASS(EBMLM_CLBlockAdditional);
DECL_CLASS(EBMLM_CLBlockDuration);
DECL_CLASS(EBMLM_CLReferencePriority);
DECL_CLASS(EBMLM_CLReferenceBlock);
DECL_CLASSCID(EBMLM_CLSilentTracks);
DECL_CLASS(EBMLM_CLSilentTrackNumber);

DECL_CLASS(EBMLM_CLBlockSamples)


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
