#include "stdafx.h"
#include "SubTitles.h"
#include "Rifffile.h"
#include "../multimedia_source.h"
#include "..\utf-8.h"
#include "global.h"


SUBTITLESOURCE::SUBTITLESOURCE()
{
	qwBegin=0;
	qwEnd=0;
	lpSSAHeader=NULL;
	lplpSSAStyles=NULL;
	lpSource=NULL;
	iDisplayOrderCount = 0;
	dwNbrOfStyles = 0;

}

int SUBTITLESOURCE::SetRange(__int64 _qwBegin,__int64 _qwEnd)
{
	qwBegin=_qwBegin;
	qwEnd=_qwEnd;

	return true;
}

int SUBTITLESOURCE::GetFormat()
{
	return dwFormat;
}

int SUBTITLESOURCE::Render2AVIChunk(void* lpDest)
{
	return 0;
}

int SUBTITLESOURCE::Render2Text(void* lpDest)
{
	return 0;
}

int SUBTITLESOURCE::doClose()
{
	int i,j;

	if (GetFormat()==SUBFORMAT_SSA)
	{
		for (i=0;i<(int)dwNbrOfStyles;i++)
		{
			for (j=0;j<18;j++)	
			{
				if (lplpSSAStyles[i]->lpcArray[j]) delete lplpSSAStyles[i]->lpcArray[j];
			}
			delete lplpSSAStyles[i];
		}

		for (j=0;j<14;j++)
		{
			if (lpSSAHeader->lpcArray && lpSSAHeader->lpcArray[j]) delete lpSSAHeader->lpcArray[j];
		}
	}
	if (lplpSSAStyles) delete lplpSSAStyles;
	if (lpSSAHeader) delete lpSSAHeader;
	lplpSSAStyles=NULL;
	lpSSAHeader=NULL;

	return 0;
}

int SUBTITLESOURCE::GetSSAStyleCount()
{
	return (int)dwNbrOfStyles;
}

int SUBTITLESOURCE::IsCompatible(SUBTITLESOURCE* s)
{
	if (GetFormat() == s->GetFormat()) {
		return MMS_COMPATIBLE;
	} else {
		return MMSIC_FORMATTAG;
	}
}

int SUBTITLESOURCE::GetType()
{
	return MMT_SUBS;
}

SSA_HEADER* SUBTITLESOURCE::GetSSAHeader()
{
	return lpSSAHeader;
}

void SUBTITLESOURCE::SetFormat(int iFormat)
{
	dwFormat = iFormat;
}

void SUBTITLESOURCE::AddSSAStyle(SSA_STYLE* style)
{
	lplpSSAStyles = (SSA_STYLE**)realloc(lplpSSAStyles,(GetSSAStyleCount()+1) * sizeof(SSA_STYLE*));
	lplpSSAStyles[GetSSAStyleCount()] = new SSA_STYLE;
	
	memcpy(lplpSSAStyles[GetSSAStyleCount()],style,sizeof(SSA_STYLE));;

	style = lplpSSAStyles[GetSSAStyleCount()];

	style->sssStruct.lpcName = new char[64];
	sprintf(style->sssStruct.lpcName,"Style%d",GetSSAStyleCount()+1);	

	dwNbrOfStyles++;
}

SSA_STYLE* SUBTITLESOURCE::FindSSAStyle(SSA_STYLE* style)
{
	SSA_STYLE*	res = NULL;

	for (int i = GetSSAStyleCount()-1;i>=0;i--) {
		if (SSAStylesEqual(&lplpSSAStyles[i]->sssStruct,&style->sssStruct)) res = lplpSSAStyles[i];
	}

	return res;
}

SSA_STYLE* SUBTITLESOURCE::GetSSAStyle(int iIndex)
{
	return lplpSSAStyles[iIndex];
}

SUBTITLESOURCELIST::SUBTITLESOURCELIST()
{
	ZeroMemory(&info,sizeof(info));
}


int SUBTITLESOURCELIST::Append(SUBTITLESOURCE* pNext)
{
	info.subtitles = (SUBTITLESOURCE**)realloc(info.subtitles, (info.iCount+1)*sizeof(SUBTITLESOURCE*));
	info.subtitles[info.iCount] = pNext;
	if (!info.iCount) {
		pNext->SetBias(0);
		char cBuffer[200];
		ZeroMemory(cBuffer,sizeof(cBuffer));
		pNext->GetName(cBuffer);
		SetName(cBuffer);
		ZeroMemory(cBuffer,sizeof(cBuffer));
		pNext->GetLanguageCode(cBuffer);
		SetLanguageCode(cBuffer);
		info.active_source = pNext;
		SetDefault(pNext->IsDefault());
	} else {
		pNext->SetBias(info.subtitles[info.iCount-1]->GetBias(BIAS_UNSCALED)
			+info.subtitles[info.iCount-1]->GetDurationUnscaled(),BIAS_UNSCALED | BIAS_ABSOLUTE);

	}
	info.iCount++;

	// if SSA, update styles
	if (info.active_source->GetFormat() == SUBFORMAT_SSA) {
		for (int i=0;i<pNext->GetSSAStyleCount();i++) {
			SSA_STYLE* style = pNext->GetSSAStyle(i);
			if (!FindSSAStyle(style)) {
				AddSSAStyle(style);
			}
		}
		lpSSAHeader = info.active_source->GetSSAHeader();
	}

	return 1;
}

int SUBTITLESOURCELIST::RenderCodecPrivate(void* lpDest)
{
	if (info.active_source->GetFormat() == SUBFORMAT_VOBSUB) return info.subtitles[0]->RenderCodecPrivate(lpDest);

	return SUBTITLESOURCE::RenderCodecPrivate(lpDest);

}

char* SUBTITLESOURCELIST::GetCodecID()
{
	return info.active_source->GetCodecID();
}

int SUBTITLESOURCELIST::GetFormat()
{
	return info.active_source->GetFormat();
}

int SUBTITLESOURCELIST::IsCompatible(SUBTITLESOURCE* s)
{
	if (!info.iCount) {
		return MMS_COMPATIBLE;
	} else {
		if (GetFormat() != s->GetFormat())
			return MMSIC_FORMATTAG;
		
		if (GetCompressionAlgo() != s->GetCompressionAlgo()) 
			return MMSIC_COMPRESSION;

		return MMS_COMPATIBLE;
	}
}

void SUBTITLESOURCELIST::ReInit()
{
	for (int i=0;i<info.iCount;info.subtitles[i++]->ReInit());
	info.active_source = info.subtitles[0];
	info.iActiveSource = 0;
}

int SUBTITLESOURCELIST::Read(void* lpDest, int* iSize, __int64* lpiTimecode,
							ADVANCEDREAD_INFO* lpAARI)
{
	int i;

	SUBTITLESOURCE* s = info.active_source;
	if (!s->IsEndOfStream()) {
		if (s->Read(lpDest,iSize,lpiTimecode,lpAARI)<=0) {
			return SUBS_ERR;
			if (iSize) *iSize = 0; 
		}
		if (lpiTimecode) {
			SetCurrentTimecode(*lpiTimecode * s->GetTimecodeScale(), TIMECODE_UNSCALED);
			*lpiTimecode = GetCurrentTimecode();
			lpAARI->iDuration = lpAARI->iDuration * s->GetTimecodeScale() / GetTimecodeScale();

			// do style mapping for SSA
			if (s->GetFormat() == SUBFORMAT_SSA) {
				SSA_STYLE*  style = *(SSA_STYLE**)lpDest;
				char*		cText = ((char*)lpDest)+4;
				
				char		cFinal[1024]; cFinal[0] = 0;

				sprintf(cFinal,"%d,,%s,%s",++iDisplayOrderCount,
					(style && (style=FindSSAStyle(style)))?style->sssStruct.lpcName:"Default",cText);
				i = strlen(cFinal);
				if (iSize) *iSize = i; 
				memcpy(lpDest,cFinal,i+1);
			}

			return SUBS_OK;
		}
	} else {
		if (info.iActiveSource < info.iCount - 1) {
			info.active_source = info.subtitles[++info.iActiveSource];
			return Read(lpDest,iSize,lpiTimecode,lpAARI);
		} else {
			return SUBS_ERR;
		}
	}

	return SUBS_ERR;
}

__int64 SUBTITLESOURCELIST::GetNextTimecode()
{
	__int64 j;

	SUBTITLESOURCE* s;
	if (!info.active_source->IsEndOfStream()) {
		s = info.active_source;
			j = s->GetNextTimecode();
			if (j!=TIMECODE_UNKNOWN) {
				return j * s->GetTimecodeScale() / GetTimecodeScale();
			} else {
				return TIMECODE_UNKNOWN;
			}
//		return s->GetNextTimecode() * s->GetTimecodeScale() / GetTimecodeScale();
	} else {
		if (info.iActiveSource < info.iCount - 1) {
			s = info.subtitles[info.iActiveSource+1];
			j = s->GetNextTimecode();
			if (j!=TIMECODE_UNKNOWN) {
				return j * s->GetTimecodeScale() / GetTimecodeScale();
			} else {
				return TIMECODE_UNKNOWN;
			}
		} else {
			s = info.subtitles[info.iActiveSource];
			return (s->GetBias(BIAS_UNSCALED) + s->GetDuration()*s->GetTimecodeScale())/GetTimecodeScale();
		}
	}
}

int SUBTITLESOURCELIST::Seek(__int64 iTime)
{

	if (!iTime) {
		info.active_source = info.subtitles[0];
		info.active_source->Seek(0);
	}

	return 0;
}

int SUBTITLESOURCELIST::Enable(int bEnable)
{
	for (int i=0;i<info.iCount;i++) {
		info.subtitles[i]->Enable(bEnable);
	}
	return 0;
}

int SUBTITLESOURCELIST::GetCompressionAlgo()
{
	return info.subtitles[0]->GetCompressionAlgo();
}

		/////////////////////////////////////////
		// subtitle source from Matroska files //
		/////////////////////////////////////////

SUBTITLESFROMMATROSKA::SUBTITLESFROMMATROSKA()
{
	ZeroMemory(&info,sizeof(info));
	dwNbrOfStyles = 0;
}

SUBTITLESFROMMATROSKA::SUBTITLESFROMMATROSKA(MATROSKA* m, int iStream)
{
	info.m = m;
	info.iStream = iStream;

	if (info.m->GetTrackType(info.iStream)!=MSTRT_SUBT) {
		ZeroMemory(&info,sizeof(info));
	} else {
		info.m->SetActiveTrack(info.iStream);
		cCodecPrivate = (char*)info.m->GetCodecPrivate(info.iStream);
		iCP_length = info.m->GetCodecPrivateSize(info.iStream);

		if (!strcmp(info.m->GetCodecID(),"S_TEXT/UTF8")) {
			info.iFormat = SUBFORMAT_SRT;
		} else 
		if (!strcmp(info.m->GetCodecID(),"S_SSA")) {
			info.iFormat = SUBFORMAT_SSA;
		} else 
		if (!strcmp(info.m->GetCodecID(),"S_TEXT/SSA")) {
			info.iFormat = SUBFORMAT_SSA;
		} else
		if (!strcmp(info.m->GetCodecID(),"S_TEXT/ASS")) {
			info.iFormat = SUBFORMAT_SSA;
		} else
		if (!strcmp(info.m->GetCodecID(),"S_VOBSUB")) {
			info.iFormat = SUBFORMAT_VOBSUB;
		} 

		if (GetFormat() == SUBFORMAT_SSA) {
			ReadSSAHeader();
			ReadSSAStyles();
		}
		SetTimecodeScale(info.m->GetTimecodeScale());
		SetDefault(info.m->IsDefault(info.iStream));
	}
}

int SUBTITLESFROMMATROSKA::RenderCodecPrivate(void* lpDest)
{
	memcpy(lpDest, cCodecPrivate, iCP_length);
	return iCP_length;
}

int SUBTITLESFROMMATROSKA::GetCompressionAlgo()
{
	return info.m->GetTrackCompression(info.iStream, 0);
}

__int64 SUBTITLESFROMMATROSKA::GetUnstretchedDuration()
{
	 return info.m->GetMasterTrackDuration();
}

bool SUBTITLESFROMMATROSKA::IsEndOfStream()
{
	return info.m->IsEndOfStream(info.iStream);
}

char* SUBTITLESFROMMATROSKA::GetCodecID()
{
	return info.m->GetCodecID();
}

int SUBTITLESFROMMATROSKA::Enable(int bEnabled)
{
	return info.m->EnableQueue(info.iStream);
}

int SUBTITLESFROMMATROSKA::Read(void* lpDest, int* iSize, __int64* lpiTimecode,
							ADVANCEDREAD_INFO* lpAARI)
{
	READ_INFO	r;

	// if SRT: read text and return as result
	if (GetFormat() == SUBFORMAT_SRT) {
		info.m->SetActiveTrack(info.iStream);
		if (info.m->Read(&r)==READBL_OK) {
			if (iSize) *iSize = r.pData->GetSize();
			if (lpDest) memcpy(lpDest,r.pData->GetData(),r.pData->GetSize());
			if (lpiTimecode) {
				*lpiTimecode = (r.qwTimecode*info.m->GetTimecodeScale() + GetBias(BIAS_UNSCALED))/GetTimecodeScale();
			}
			if (lpAARI) {
				lpAARI->iDuration = r.qwDuration * info.m->GetTimecodeScale() / GetTimecodeScale();
			}
			DecBufferRefCount(&r.pData);
		} else {
			if (iSize) *iSize = 0;
		}
	}
	// if SRT: read text and return as result
	if (GetFormat() == SUBFORMAT_VOBSUB) {
		info.m->SetActiveTrack(info.iStream);
		if (info.m->Read(&r)==READBL_OK) {
			if (iSize) *iSize = r.pData->GetSize();
			if (lpDest) memcpy(lpDest,r.pData->GetData(),r.pData->GetSize());
			if (lpiTimecode) {
				*lpiTimecode = (r.qwTimecode*info.m->GetTimecodeScale() + GetBias(BIAS_UNSCALED))/GetTimecodeScale();
			}
			if (lpAARI) {
				lpAARI->iDuration = r.qwDuration * info.m->GetTimecodeScale() / GetTimecodeScale();
			}
			DecBufferRefCount(&r.pData);
		} else {
			if (iSize) *iSize = 0;
		}
	}

	// if SSA: split style apart!
	if (GetFormat() == SUBFORMAT_SSA) {
		info.m->SetActiveTrack(info.iStream);
		if (info.m->Read(&r)==READBL_OK) {
			char* lpcDest = (char*)lpDest;
			
			int j = 2; char* c = (char*)r.pData->GetData(); char* cStyle;
			while (*c++ != ',' || --j);
			cStyle = c;
			while (*c++ != ',');
			*(c-1)=0;

			int i = strlen(c);
			if (iSize) *iSize = i;
			if (lpDest) memcpy(lpcDest+4,c,i+1);
			memset(lpcDest,0,4);

			if (lpiTimecode) {
				*lpiTimecode = (r.qwTimecode*info.m->GetTimecodeScale() + GetBias(BIAS_UNSCALED))/GetTimecodeScale();
			}
//			if (lpiTimecode) *lpiTimecode = r.qwTimecode + GetBias();
			if (lpAARI) {
				lpAARI->iDuration = r.qwDuration * info.m->GetTimecodeScale() / GetTimecodeScale();
			}

			SSA_STYLE* style = NULL;
			
			for (i=0;i<GetSSAStyleCount();i++) {
				char* stn = cStyle;
				if (!strcmp(GetSSAStyle(i)->sssStruct.lpcName,cStyle)) {
					style = GetSSAStyle(i);
				}
				if (stn[0]=='*' && !strcmp(stn+1,GetSSAStyle(i)->sssStruct.lpcName)) {
					style = GetSSAStyle(i);
				}
			}
			memcpy(lpDest,&style,sizeof(style));

			DecBufferRefCount(&r.pData);
		} else return SUBS_ERR;
	}

	return SUBS_OK;
}

__int64 SUBTITLESFROMMATROSKA::GetNextTimecode()
{
	__int64 iNTC = info.m->GetNextTimecode(info.iStream);
	if (iNTC!=TIMECODE_UNKNOWN) {
		iNTC=iNTC*info.m->GetTimecodeScale()/GetTimecodeScale();
	}
	return (iNTC!=TIMECODE_UNKNOWN)?iNTC + GetBias():iNTC;
}

int SUBTITLESFROMMATROSKA::GetName(char* lpDest)
{
	info.m->SetActiveTrack(info.iStream);
	char* c=info.m->GetTrackName();
	if (c && strcmp(c,"")) {
		int l;
		memcpy(lpDest,c,l=strlen(c));
		return l;
	} else {
		*lpDest = 0;
		return 0;
	}
}

int SUBTITLESFROMMATROSKA::GetLanguageCode(char* lpDest)
{
	info.m->SetActiveTrack(info.iStream);
	char* c=info.m->GetLanguage();
	if (c && strcmp(c,"")) {
		int l;
		memcpy(lpDest,c,l=strlen(c));
		return l;
	} else {
		*lpDest = 0;
		return 0;
	}
}

int SUBTITLESFROMMATROSKA::ReadLine(char* d)
{
	char c;
	char s[1024];
	int  i=0;
	int  j=0;

	if (iCP_length<0) {
		*d = 0;
		return -1;
	}

	ZeroMemory(d,sizeof(d));
	s[0] = 0;
	while (++j && iCP_length-- && (c = *cCodecPrivate++) && c != 0x0A && j < 1023) {
		if (c!=0x0A && c!=0x0D) s[i++] = c;
	}

	s[i++] = 0;
	strcpy(d,s);

	return (strlen(s));
}

		/////////////////////////////////
		// SUBTITLES from text sources //
		/////////////////////////////////

SUBTITLES::SUBTITLES(void)
{
	return;
}

SUBTITLES::~SUBTITLES(void)
{
}

void SUBTITLESOURCE::SetSource(CTEXTFILE* c)
{
	lpSource = c;
}

int SUBTITLES::Open(CTEXTFILE* source)
{
	DWORD	dwHeader;
	char	cBuffer[2048];
	__int64	qwPos;
	char*	lpcName = NULL;

	if (!source) return false;
	SetSource(source);

	subs=NULL;
	iDisplayOrderCount=0;

	// from AVI
	source->Seek(0);
	source->Read(&dwHeader,4);
	if (dwHeader==MakeFourCC("GAB2"))
	{
		source->Read(cBuffer,3);
		source->Read(&dwHeader,4);

		ZeroMemory(cBuffer,sizeof(cBuffer));
		source->Read(cBuffer,dwHeader);
		if (lpcName) delete lpcName;
		newz(char, 2+dwHeader, lpcName);

		WStr2UTF8(cBuffer,&lpcName);
		SetName(lpcName);

		delete lpcName;

		source->Read(&dwHeader,2);
		source->Read(&dwHeader,4);

		source->SetOffset((int)(source->GetPos()));
		source->ReOpen();
	}
	else
	{
		source->Seek(0);
	}
	qwPos=source->GetPos();
	// SRT format
	
	if (ParseSRT()) 
	{
		curr_sub = subs;
		SetFormat(SUBFORMAT_SRT);
		return true;
	}
	source->Seek(qwPos);
	if (ReadSSAHeader()&&ReadSSAStyles()&&ReadSSAEvents())
	{
		curr_sub = subs;
		SetFormat(SUBFORMAT_SSA);
		return true;
	}
	else
	{
		Close();
		return false;
	}


	return true;
}

CTEXTFILE* SUBTITLESOURCE::GetSource()
{
	return lpSource;
}

int SUBTITLES::doClose()
{
	SUBTITLE_DESCRIPTOR*	lpCurrSub=subs;
	SUBTITLE_DESCRIPTOR*	lpNextSub;

	SetSource(NULL);

	int	i;

	while (lpCurrSub)
	{
		lpNextSub=(SUBTITLE_DESCRIPTOR*)lpCurrSub->lpNext;
		if (lpCurrSub->lpcText)
		{
			delete lpCurrSub->lpcText;
		}

		if (GetFormat()==SUBFORMAT_SSA)
		{
			if (lpCurrSub->lpSSA)
			{
				for (i=0;i<10;i++)
				{
					// don't delete reference to style!
					if (i!=3) if (lpCurrSub->lpSSA->lpcArray[i]) delete lpCurrSub->lpSSA->lpcArray[i];
				}
				delete lpCurrSub->lpSSA;
			}
		}

		delete lpCurrSub;
		lpCurrSub=lpNextSub;
	}
	if (lpcName) delete lpcName;
	return true;
}

int SUBTITLES::ParseSRT()
{
	DWORD	dwNumber=1;
	DWORD	dwHour,dwMin,dwSec,dwFrac;
	char	cBuffer[1024];
	char*	lpcBuffer=cBuffer;
	int		iRead;
	int		i;
	int		iFirst;
	
	SUBTITLE_DESCRIPTOR*	lpFirstSub;
	SUBTITLE_DESCRIPTOR*	lpCurrSub;

	lpFirstSub=new SUBTITLE_DESCRIPTOR;
	lpCurrSub=lpFirstSub;
	subs=lpFirstSub;
	lastsub=subs;
	ZeroMemory(subs,sizeof(SUBTITLE_DESCRIPTOR));
	iFirst = true;
	while (ReadLine(cBuffer)>0)
	{
		if (atoi(cBuffer)<(int)dwNumber-1 || !atoi(cBuffer))
		{
			return false;
		}
		iFirst = false;
		dwNumber++;
		ReadLine(cBuffer);
		lpCurrSub->dwNoPos=(lstrlen(cBuffer)<57);
		cBuffer[2]=0; cBuffer[5]=0; cBuffer[8]=0; cBuffer[12]=0;
		cBuffer[19]=0; cBuffer[22]=0; cBuffer[25]=0; cBuffer[29]=0;
		if (!lpCurrSub->dwNoPos)
		{
			cBuffer[37]=0; cBuffer[44]=0; cBuffer[51]=0; cBuffer[58]=0;

			lpCurrSub->dwX1=atoi(&cBuffer[34]);
			lpCurrSub->dwX2=atoi(&cBuffer[41]);
			lpCurrSub->dwY1=atoi(&cBuffer[48]);
			lpCurrSub->dwY2=atoi(&cBuffer[55]);
		}
		else
		{
			lpCurrSub->dwX2=1;
		}
		dwHour=atoi(&cBuffer[0]);
		dwMin=atoi(&cBuffer[3]);
		dwSec=atoi(&cBuffer[6]);
		dwFrac=atoi(&cBuffer[9]);
		lpCurrSub->qwBegin=1000000*(3600000*(__int64)dwHour+60000*(__int64)dwMin+1000*(__int64)dwSec+dwFrac);

		dwHour=atoi(&cBuffer[17]);
		dwMin=atoi(&cBuffer[20]);
		dwSec=atoi(&cBuffer[23]);
		dwFrac=atoi(&cBuffer[26]);
		lpCurrSub->qwEnd=1000000*(3600000*(__int64)dwHour+60000*(__int64)dwMin+1000*(__int64)dwSec+dwFrac);

		lpcBuffer=cBuffer;
		bool	bFirst = true;
		bool	bLineEmpty;
		do
		{
			bLineEmpty = true;
			iRead=ReadLine(lpcBuffer+2*(!bFirst));
			if (iRead) {
				for (int k=iRead-1;k>=0 && bLineEmpty;k--) {
					if (lpcBuffer[k+2]!=0x20) bLineEmpty = false;
				}
			}
			if (!bFirst && iRead>0) {
				*lpcBuffer++ = 0x0D;
				*lpcBuffer++ = 0x0A;
			}
			lpcBuffer+= strlen(lpcBuffer);
			bFirst = false;

		} while (iRead>0 && !bLineEmpty);

		lpCurrSub->lpcText=new char[lstrlen(cBuffer)+1];
		lstrcpy(lpCurrSub->lpcText,cBuffer);
		lpCurrSub->iCharCoding = GetSource()->IsUTF8Out()?CM_UTF8:CM_ANSI;

		lpCurrSub->lpNext=new SUBTITLE_DESCRIPTOR;
		i=sizeof(SUBTITLE_DESCRIPTOR);
		ZeroMemory(lpCurrSub->lpNext,sizeof(SUBTITLE_DESCRIPTOR));
		lastsub=lpCurrSub;
		lpCurrSub=(SUBTITLE_DESCRIPTOR*)lpCurrSub->lpNext;
	}

	SetFormat(SUBFORMAT_SRT);

	return !iFirst;
}

int SUBTITLESOURCE::StoreSSAHeaderInfo(char* lpcName,char* lpcInfo,char** lplpDest)
{
	if (!strnicmp(lpcInfo,lpcName,lstrlen(lpcName)))
	{
		if (*lplpDest) delete (*lplpDest);
		*lplpDest=new char[1+lstrlen(lpcInfo)-lstrlen(lpcName)];
		lstrcpy(*lplpDest,lpcInfo+lstrlen(lpcName));
		return true;
	}
	else
		return false;
}

static char* header_attribs[14] =
{ 
	"Title:",
	"Original Script:",
	"Original Translation:",
	"Original Editing:",
	"Original Timing:",
	"Original Script Checking:",
	"Sync Point:",
	"Script Updated By:",
	"Update Details:",
	"ScriptType:",
	"Collisions:",
	"PlayResY:",
	"PlayDepth:",
	"Timer"
};

static char* style_attribs[18] =
{ 
	"Name",
	"Fontname",
	"Fontsize",
	"PrimaryColour",
	"SecondaryColour",
	"TertiaryColour",
	"BackColour",
	"Bold",
	"Italic",
	"BorderStyle",
	"Outline",
	"Shadow",
	"Alignment",
	"MarginL",
	"MarginR",
	"MarginV",
	"AlphaLevel",
	"Encoding"
};

static char* event_attribs[10] =
{ 
	"Marked",
	"Start",
	"End",
	"Style",
	"Name",
	"MarginL",
	"MarginR",
	"MarginV",
	"Effect",
	"Text"
};

int SUBTITLESOURCE::ReadLine(char* lpBuffer)
{
	return 0;
}

int SUBTITLESOURCE::ReadSSAHeader(void)
{
	char	cBuffer[1024];
	int		i;

	ReadLine(cBuffer);

	i=5;
	while (stricmp(cBuffer+i,"[Script Info]") && i>=0)
	{
		i--;
	}
	if (i==-1) 	return false;


	lpSSAHeader=new SSA_HEADER;
	ZeroMemory(lpSSAHeader,sizeof(SSA_HEADER));

	while ( ReadLine(cBuffer)>-1 && (stricmp(cBuffer,"[V4 Styles]") && stricmp(cBuffer,"[V4+ Styles]")))
	{
		for (i=0;i<14;i++) {
			StoreSSAHeaderInfo(header_attribs[i],cBuffer,&(lpSSAHeader->lpcArray[i]));
		}
	}
	return true;
}

char* RemoveBlanks(char* lpcText)
{
	return lpcText+strspn(lpcText," ");
}

int SUBTITLESOURCE::ReadSSAStyles()
{
	char		cBuffer[1024];
	DWORD*		lpMembers;
	DWORD		dwAttribCount=1;
	DWORD		dwStyleCount;
	int			i,j,iPos;
	SSA_STYLE** styles;

	// retrieve format order
	
	ReadLine(cBuffer);
 	//cBuffer[lstrlen(cBuffer)-2]=0;

	if (strncmp(cBuffer,"Format:",7)) return false;

	for (i=7;i<lstrlen(cBuffer);dwAttribCount+=!!(cBuffer[i++]==','));
	dwAttribCount=min(dwAttribCount,18);
		
	lpMembers=new DWORD[dwAttribCount];
	ZeroMemory(lpMembers,4*dwAttribCount);

	iPos=8;
	for (i=0;i<(int)dwAttribCount;i++)
	{
		for (j=0;j<(int)dwAttribCount;j++)
		{
			if (!strnicmp(RemoveBlanks(cBuffer+iPos),style_attribs[j],lstrlen(style_attribs[j])))
			{
				lpMembers[i]=j;
			}
		}

		iPos+=strcspn(cBuffer+iPos,",")+1;
	}

	// retrieve styles

	styles=new SSA_STYLE*[1024];
	ZeroMemory(styles,4096);
	dwStyleCount=0;
	while ( ReadLine(cBuffer)>-1 && lstrcmp(cBuffer,"[Events]"))
	{
		if (!strncmp(cBuffer,"Style:",6))
		{
			styles[dwStyleCount]=new SSA_STYLE;
			ZeroMemory(styles[dwStyleCount],sizeof(SSA_STYLE));
			iPos=7;
			for (i=0;i<(int)dwAttribCount;i++)
			{
				DWORD dwLen=strcspn(cBuffer+iPos,",");
				styles[dwStyleCount]->lpcArray[lpMembers[i]]=new char[1+dwLen];
				ZeroMemory(styles[dwStyleCount]->lpcArray[lpMembers[i]],1+dwLen);
				strncpy((char*)styles[dwStyleCount]->lpcArray[lpMembers[i]],cBuffer+iPos,dwLen);

				char* c = (char*)styles[dwStyleCount]->lpcArray[lpMembers[i]];
				while (c && c[0] == 0x20)
					strcpy(c, c+1);

				iPos+=strcspn(cBuffer+iPos,",")+1;
			}
			dwStyleCount++;
		}
	}

	lplpSSAStyles=new SSA_STYLE*[dwStyleCount];
	memcpy(lplpSSAStyles,styles,4*dwStyleCount);
	dwNbrOfStyles=dwStyleCount;
	free(styles);
	free(lpMembers);

	return true;
}

__int64 SUBTITLESOURCE::SSATime2NanoSec(char* lpcTime)
{
	char	cBuffer[20];
	DWORD	dwHour,dwMin,dwSec,dwFrac;
	int		iPos,iLen;

	iPos=strcspn(lpcTime,":");
	ZeroMemory(cBuffer,sizeof(cBuffer));
	strncpy(cBuffer,lpcTime,iPos);
	dwHour=atoi(cBuffer);

	iPos+=1;
	iLen=strcspn(lpcTime+iPos,":");
	strncpy(cBuffer,lpcTime+iPos,iLen);
	dwMin=atoi(cBuffer);

	iPos+=iLen+1;
	iLen=strcspn(lpcTime+iPos,".");
	strncpy(cBuffer,lpcTime+iPos,iLen);
	dwSec=atoi(cBuffer);

	iPos+=iLen+1;
	strcpy(cBuffer,lpcTime+iPos);
	dwFrac=10*atoi(cBuffer);

	return 1000000*(3600000*(__int64)dwHour+60000*(__int64)dwMin+1000*(__int64)dwSec+dwFrac);
}

int SUBTITLESOURCE::ReadSSAEvents(void)
{
	char		cBuffer[1024];
	DWORD*		lpMembers;
	DWORD		dwAttribCount=1;
	int			i,j,iPos;
	DWORD		dwLen;

	// retrieve format order
	
	ReadLine(cBuffer);

	if (strncmp(cBuffer,"Format:",7)) return false;

	for (i=7;i<lstrlen(cBuffer);dwAttribCount+=!!(cBuffer[i++]==','));
		
	lpMembers=new DWORD[dwAttribCount];
	ZeroMemory(lpMembers,4*dwAttribCount);

	iPos=8;
	for (i=0;i<(int)dwAttribCount;i++)
	{
		for (j=0;j<(int)dwAttribCount;j++)
		{
			if (!strncmp(RemoveBlanks(cBuffer+iPos),event_attribs[j],lstrlen(event_attribs[j])))
			{
				lpMembers[i]=j;
			}
		}

		iPos+=strcspn(cBuffer+iPos,",")+1;
	}

	// retrieve events

	SUBTITLE_DESCRIPTOR*	lpFirstSub;
	SUBTITLE_DESCRIPTOR*	lpCurrSub;

	lpFirstSub=new SUBTITLE_DESCRIPTOR;
//	ZeroMemory(lpFirstSub,sizeof(lpFirstSub));
	lpCurrSub=lpFirstSub;
	subs=lpFirstSub;
	lastsub=subs;
	ZeroMemory(subs,sizeof(SUBTITLE_DESCRIPTOR));

	while (ReadLine(cBuffer)>-1)
	{
		if (!strncmp("Dialogue: Marked",cBuffer,16))
		{
			iPos=17;
			lpCurrSub->lpSSA=new SSA_SPECIFIC;
			ZeroMemory(lpCurrSub->lpSSA,sizeof(SSA_SPECIFIC));

			for (i=0;i<(int)dwAttribCount;i++)
			{
				if (i<(int)dwAttribCount-1) {
					dwLen=strcspn(cBuffer+iPos,",");
				} else {
					dwLen=lstrlen(cBuffer+iPos);
				}
				lpCurrSub->lpSSA->lpcArray[lpMembers[i]]=new char[1+dwLen];
				ZeroMemory(lpCurrSub->lpSSA->lpcArray[lpMembers[i]],1+dwLen);
				char* cBuffer2 = cBuffer+iPos;
				strncpy((char*)lpCurrSub->lpSSA->lpcArray[lpMembers[i]],cBuffer+iPos,dwLen);
				iPos+=strcspn(cBuffer+iPos,",")+1;
			}
			lpCurrSub->qwBegin=SSATime2NanoSec(lpCurrSub->lpSSA->sesStruct.lpcBegin);
			lpCurrSub->qwEnd=SSATime2NanoSec(lpCurrSub->lpSSA->sesStruct.lpcEnd);
			lpCurrSub->dwX2=1;
			lpCurrSub->iCharCoding = GetSource()->IsUTF8Out()?CM_UTF8:CM_ANSI;
			lpCurrSub->dwNoPos=1;
			for (i=0;i<(int)dwNbrOfStyles;i++)
			{
				if (!lstrcmp(lpCurrSub->lpSSA->sesStruct.lpcStyle,lplpSSAStyles[i]->sssStruct.lpcName))
				{
					free(lpCurrSub->lpSSA->sesStruct.lpcStyle);
					lpCurrSub->lpSSA->sesStruct.lpsssStyle=&lplpSSAStyles[i]->sssStruct;
				}
				if (!lstrcmp(lpCurrSub->lpSSA->sesStruct.lpcStyle+1,lplpSSAStyles[i]->sssStruct.lpcName))
				{
					free(lpCurrSub->lpSSA->sesStruct.lpcStyle);
					lpCurrSub->lpSSA->sesStruct.lpsssStyle=&lplpSSAStyles[i]->sssStruct;
				}
			}

			lpCurrSub->lpNext=new SUBTITLE_DESCRIPTOR;
			i=sizeof(SUBTITLE_DESCRIPTOR);
			ZeroMemory(lpCurrSub->lpNext,i);
			lastsub=lpCurrSub;
			lpCurrSub=(SUBTITLE_DESCRIPTOR*)lpCurrSub->lpNext;	
		}
	}

	dwFormat=SUBFORMAT_SSA;
	delete lpMembers;
	lpMembers=NULL;

	return true;
}

int SUBTITLES::ReadLine(char* lpcBuffer)
{
	return GetSource()->ReadLine(lpcBuffer);
}



SUBTITLE_DESCRIPTOR* SUBTITLES::GetData()
{
	return subs;
}

int SUBTITLES::Render2AVIChunk(void* lpDest)
{
	DWORD*	lpdwDest=(DWORD*)lpDest;
	DWORD*	lpdwLength;

	if (GetFormat()==SUBFORMAT_SRT)
	{
		Render2AVIChunk_Begin((void**)&lpdwDest,&lpdwLength);
		RenderSRT2AVIChunk((void**)&lpdwDest);
		Render2AVIChunk_End(lpdwDest,lpdwLength);
		return (DWORD)lpdwDest-(DWORD)lpDest;
	}
	else
	if (GetFormat()==SUBFORMAT_SSA)
	{
		Render2AVIChunk_Begin((void**)&lpdwDest,&lpdwLength);
		RenderSSA2AVIChunk((void**)&lpdwDest);
		Render2AVIChunk_End(lpdwDest,lpdwLength);
		return (DWORD)lpdwDest-(DWORD)lpDest;
	}
	else
		return false;
}

int SUBTITLES::Render2Text(void* lpDest)
{
	DWORD*	lpdwDest=(DWORD*)lpDest;

	switch (GetFormat())
	{
		case SUBFORMAT_SRT: RenderSRT2AVIChunk((void**)&lpdwDest); break;
		case SUBFORMAT_SSA: RenderSSA2AVIChunk((void**)&lpdwDest); break;
	}

	return (DWORD)lpdwDest-(DWORD)lpDest;
}

__int64 SUBTITLES::GetFeature(__int64 iFeature)
{
	switch (iFeature) {
		case FEATURE_SUB_EXTRACT2TEXT: return 1; break;
		default: return SUBTITLESOURCE::GetFeature(iFeature);
	}

	return 0;
}

int SUBTITLES::Render2AVIChunk_Begin(void** lpDest,DWORD** lpdwLength)
{
	union
	{
		DWORD*	lpdwDest;
		WORD*	lpwDest;
		BYTE*	lpbDest;
		char*	lpcDest;
	};
	int	i;

	lpdwDest=*((DWORD**)lpDest);

	*lpdwDest++=MakeFourCC("GAB2");
	*lpbDest++=0;
	*lpwDest++=2;
	i = UTF82WStr(lpcName,(char*)(lpdwDest+1));
	*lpdwDest++=i;//;2*lstrlen(lpcName)+2;
 
	lpbDest+=i;

//	*lpwDest++=0;
	*lpwDest++=4;
	*lpdwLength=lpdwDest;
	*lpdwDest++=0;

	*lpbDest++ = 0xEF;
	*lpbDest++ = 0xBB;
	*lpbDest++ = 0xBF;

	(*lpDest)=lpdwDest;

	return 1;
}

int SUBTITLES::Render2AVIChunk_End(void* lpDest, DWORD* lpdwLength)
{
	DWORD*	lpdwDest;

	lpdwDest=(DWORD*)lpDest;
	*lpdwLength=(DWORD)(lpdwDest)-(DWORD)lpdwLength-4;

	return 1;
}

int SUBTITLES::Read(void* lpDest, int* iSize, __int64* lpiTimecode, ADVANCEDREAD_INFO* lpAARI)
{
	if (!curr_sub) return SUBS_ERR;

	__int64 iBegin = (curr_sub->qwBegin - GetBias(BIAS_UNSCALED)) / 1000000;
	__int64 iEnd = (curr_sub->qwEnd - GetBias(BIAS_UNSCALED)) / 1000000;
	int		i;
	char*	lpcDest = (char*)lpDest;

	if (!curr_sub) return SUBS_ERR;
	
	switch (GetFormat()) {
		case SUBFORMAT_SRT:
			if (curr_sub) {
				lstrcpy(lpcDest,curr_sub->lpcText);
				lpcDest+=lstrlen(lpcDest);
			
				i = (curr_sub->lpcText)?strlen(curr_sub->lpcText):0; //RenderSRTLine_time(curr_sub,&lpcDest,iBegin,iEnd);
				if (iSize) *iSize = i;
				if (lpiTimecode) *lpiTimecode = iBegin;
				if (lpAARI && *iSize) {
					lpAARI->iDuration = iEnd-iBegin;
					curr_sub = (SUBTITLE_DESCRIPTOR*)curr_sub->lpNext;
					if (curr_sub) {
						lpAARI->iNextTimecode = (curr_sub->qwBegin - GetBias(BIAS_UNSCALED)) / 1000000;
					} else {
						lpAARI->iNextTimecode = iBegin;
					}
				}
			}
			break;
		case SUBFORMAT_SSA:
			*iSize = RenderSSAEvent4MKV(curr_sub,(char**)&lpDest,iBegin,iEnd);
			if (lpiTimecode) *lpiTimecode = iBegin;
			if (lpAARI && *iSize) {
				lpAARI->iDuration = iEnd-iBegin;
				curr_sub = (SUBTITLE_DESCRIPTOR*)curr_sub->lpNext;
				if (curr_sub) {
					lpAARI->iNextTimecode = (curr_sub->qwBegin - GetBias(BIAS_UNSCALED)) / 1000000;
				} else {
					lpAARI->iNextTimecode = iBegin;
				}
			}

			break;
	}

	return SUBS_OK;
}

__int64 SUBTITLES::GetNextTimecode()
{
	return (curr_sub)?(curr_sub->qwBegin - GetBias(BIAS_UNSCALED)) / GetTimecodeScale():0;
}

int SUBTITLES::RenderSRTLine_time(SUBTITLE_DESCRIPTOR* subtitle, char** lpcDest, __int64 qwMSBegin, __int64 qwMSEnd)
{
	char*	lpcFormat = new char[100];
	ZeroMemory(lpcFormat,100);
	int		iSize;

	if (subtitle->dwNoPos) {
		lpcFormat="%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d";
	} else {
		lpcFormat="%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d  X1:%03d X2:%03d Y1:%03d Y2:%03d";
	}

	wsprintf(*lpcDest,lpcFormat,
		(DWORD)(qwMSBegin/3600000),
		(DWORD)(qwMSBegin/60000)%60,
		(DWORD)(qwMSBegin/1000)%60,
		(DWORD)(qwMSBegin)%1000,
		(DWORD)(qwMSEnd/3600000),
		(DWORD)(qwMSEnd/60000)%60,
		(DWORD)(qwMSEnd/1000)%60,
		(DWORD)(qwMSEnd)%1000,
		subtitle->dwX1,
		subtitle->dwX2,
		subtitle->dwY1,
		subtitle->dwY2);
		*lpcDest+=iSize=lstrlen(*lpcDest);

	return iSize;
}

int SUBTITLES::RenderSRT2AVIChunk(void** lpDest)
{
	union
	{
		DWORD*	lpdwDest;
		WORD*	lpwDest;
		BYTE*	lpbDest;
		char*	lpcDest;
	};
	int	iNbr;
	__int64 qwBias = GetBias(BIAS_UNSCALED);

	SUBTITLE_DESCRIPTOR*	lpCurrSub=subs;
	__int64	qwMSBegin,qwMSEnd;

	lpdwDest=*(DWORD**)lpDest;
	lpwDest=*(WORD**)lpDest;


	iNbr=1;
	while (lpCurrSub)
	{
		qwMSBegin=0;
		qwMSEnd=0;
		if ((lpCurrSub->qwBegin-qwBias>=qwBegin)&&(lpCurrSub->qwEnd-qwBias<=qwEnd)&&(lpCurrSub->dwX2))
		{
			qwMSBegin=(lpCurrSub->qwBegin-qwBias)/1000000;
			qwMSEnd=(lpCurrSub->qwEnd-qwBias)/1000000;
		}
		else
		if ((lpCurrSub->qwBegin-qwBias>=qwBegin)&&(lpCurrSub->qwBegin-qwBias<=qwEnd)&&(lpCurrSub->dwX2))
		{
			qwMSBegin=(lpCurrSub->qwBegin-qwBias)/1000000;
			qwMSEnd=(qwEnd)/1000000;
		}
		else
		if ((lpCurrSub->qwEnd-qwBias>=qwBegin)&&(lpCurrSub->qwEnd-qwBias<=qwEnd)&&(lpCurrSub->dwX2))
		{
			qwMSBegin=(qwBegin)/1000000;
			qwMSEnd=(lpCurrSub->qwEnd-qwBias)/1000000;
		}

		if (qwMSBegin||qwMSEnd)
		{
			wsprintf(lpcDest,"%d%c%c",iNbr++,13,10);
			lpcDest+=lstrlen(lpcDest);

			RenderSRTLine_time(lpCurrSub,&lpcDest,qwMSBegin,qwMSEnd);

			*lpcDest++=13;
			*lpcDest++=10;

			lstrcpy(lpcDest,lpCurrSub->lpcText);
			lpcDest+=lstrlen(lpcDest);
			wsprintf(lpcDest,"%c%c%c%c",13,10,13,10);
			lpcDest+=4;
		}
		lpCurrSub=(SUBTITLE_DESCRIPTOR*)lpCurrSub->lpNext;
	}

	(*lpDest)=lpdwDest;

	return (1);
}

int SUBTITLESOURCE::RenderSSAScriptInfo(char** lpcDest) 
{
	char	cBuffer[1024];
	char*	lpcBegin = *lpcDest;

	wsprintf(cBuffer,"[Script Info]");
	strcpy(*lpcDest,cBuffer);
	*lpcDest+=strlen(*lpcDest);
	sprintf(*lpcDest,"%c%c",13,10);
	*lpcDest+=2;

	for (int i=0;i<14;i++)
	{
		if (lpSSAHeader->lpcArray[i])
		{
			sprintf(cBuffer,"%s%s%c%c",header_attribs[i],lpSSAHeader->lpcArray[i],13,10);
			strcpy(*lpcDest,cBuffer);
			*lpcDest+=strlen(*lpcDest);
		}
	}
	sprintf(*lpcDest,"%c%c",13,10);
	*lpcDest+=2;

	return (*lpcDest - lpcBegin);
}

int SUBTITLESOURCE::RenderSSAStyles(char** lpcDest)
{
	char	cBuffer[1024];
	char*	lpcBegin = *lpcDest;

	sprintf(*lpcDest,"[V4 Styles]");
	*lpcDest+=lstrlen(*lpcDest);
	sprintf(*lpcDest,"%c%c",13,10);
	*lpcDest+=2;

	sprintf(*lpcDest,"Format:");
	*lpcDest+=strlen(*lpcDest);
	for (int i=0;i<18;i++)
	{
		*((*lpcDest)++)=32;
		wsprintf(*lpcDest,style_attribs[i]);
		*lpcDest+=strlen(*lpcDest);
		if (i<17) *(*lpcDest)++=',';
	}
	sprintf(*lpcDest,"%c%c",13,10);
	*lpcDest+=2;

	for (int j=0;j<(int)dwNbrOfStyles;j++)
	{
		ZeroMemory(cBuffer,sizeof(cBuffer));
		wsprintf(*lpcDest,"Style: ");
		*lpcDest+=strlen(*lpcDest);

		for (i=0;i<18;i++)
		{
			sprintf(*lpcDest,"%s",lplpSSAStyles[j]->lpcArray[i]);
			*lpcDest+=lstrlen(*lpcDest);
			if (i<17) {
				*((*lpcDest)++)=',';
				*((*lpcDest)++)=' ';
			}
		}
		sprintf(*lpcDest,"%c%c",13,10);
		*lpcDest+=2;
	}
	sprintf(*lpcDest,"%c%c",13,10);
	*lpcDest+=2;

	return (*lpcDest - lpcBegin);
}

int SUBTITLESOURCE::RenderSSAHeaderAfterStyles(char** lpcDest)
{
	char*	lpcBegin = *lpcDest;

	sprintf(*lpcDest, "[Events]%c%cFormat: Marked, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text",
		13, 10);
	*lpcDest += strlen(*lpcDest);

	return (*lpcDest - lpcBegin);
}

int SUBTITLESOURCE::RenderCodecPrivate(void* lpDest)
{
	char* lpcDest = (char*)lpDest;
	int iSize = 0;

	if (GetFormat() == SUBFORMAT_SRT) return 0;

	if (GetFormat() == SUBFORMAT_SSA)  {
		iSize += RenderSSAScriptInfo(&lpcDest);
		iSize += RenderSSAStyles(&lpcDest);
		iSize += RenderSSAHeaderAfterStyles(&lpcDest);
		return iSize;
	}

	return 0;
}

int SUBTITLES::RenderSSAEvent(SUBTITLE_DESCRIPTOR* lpCurrSub,char** lpcDest, __int64 qwMSBegin, __int64 qwMSEnd)
{
	char cBuffer[1024];
	cBuffer[0] = 0;
	char*	lpcBegin = *lpcDest;

	wsprintf(cBuffer,"Dialogue: Marked=%s,%d:%02d:%02d.%02d,%d:%02d:%02d.%02d,%s,%s,%s,%s,%s,%s,%s",
		lpCurrSub->lpSSA->sesStruct.lpcMarked,
		(DWORD)(qwMSBegin/3600000),
		(DWORD)(qwMSBegin/60000)%60,
		(DWORD)(qwMSBegin/1000)%60,
		((DWORD)(qwMSBegin)%1000)/10,
		(DWORD)(qwMSEnd/3600000),
		(DWORD)(qwMSEnd/60000)%60,
		(DWORD)(qwMSEnd/1000)%60,
		((DWORD)(qwMSEnd)%1000)/10,
		lpCurrSub->lpSSA->sesStruct.lpsssStyle->lpcName,
		lpCurrSub->lpSSA->sesStruct.lpcName,
		lpCurrSub->lpSSA->sesStruct.lpcMarginL,
		lpCurrSub->lpSSA->sesStruct.lpcMarginR,
		lpCurrSub->lpSSA->sesStruct.lpcMarginV,
		lpCurrSub->lpSSA->sesStruct.lpcEffect,
		lpCurrSub->lpSSA->sesStruct.lpcText		
		);
	lstrcpy(*lpcDest,cBuffer);
	*lpcDest+=lstrlen(cBuffer);

	return *lpcDest - lpcBegin;
}

int SUBTITLES::RenderSSAEvent4MKV(SUBTITLE_DESCRIPTOR* lpCurrSub,char** lpcDest, __int64 qwMSBegin, __int64 qwMSEnd)
{
	char cBuffer[1024];
	cBuffer[0] = 0;
	char*	lpcBegin = *lpcDest;

	if (!lpCurrSub || !lpCurrSub->lpSSA || !lpCurrSub->lpSSA->sesStruct.lpcText) return 0;

	wsprintf(cBuffer,"%d,,%s,%s,%s,%s,%s,%s,%s",
		iDisplayOrderCount++,
		lpCurrSub->lpSSA->sesStruct.lpsssStyle->lpcName,
		lpCurrSub->lpSSA->sesStruct.lpcName,
		lpCurrSub->lpSSA->sesStruct.lpcMarginL,
		lpCurrSub->lpSSA->sesStruct.lpcMarginR,
		lpCurrSub->lpSSA->sesStruct.lpcMarginV,
		lpCurrSub->lpSSA->sesStruct.lpcEffect,
		lpCurrSub->lpSSA->sesStruct.lpcText		
		);
	lstrcpy(*lpcDest,cBuffer);
	*lpcDest+=lstrlen(cBuffer);

	return *lpcDest - lpcBegin;
}

int SUBTITLES::RenderSSA2AVIChunk(void** lpDest)
{
	union
	{
		DWORD*	lpdwDest;
		WORD*	lpwDest;
		BYTE*	lpbDest;
		char*	lpcDest;
	};
	int i;
	
	SUBTITLE_DESCRIPTOR*	lpCurrSub=subs;
	__int64	qwMSBegin,qwMSEnd;

	char	cBuffer[1024];
	__int64 qwBias = GetBias(BIAS_UNSCALED);

	lpdwDest=*(DWORD**)lpDest;
	lpwDest=*(WORD**)lpDest;

	// render headers

	RenderSSAScriptInfo(&lpcDest);

	// render styles

	RenderSSAStyles(&lpcDest);

	// render subtitles

	wsprintf(cBuffer,"[Events]");
	lstrcpy(lpcDest,cBuffer);
	lpcDest+=lstrlen(cBuffer);
	wsprintf(lpcDest,"%c%c",13,10);
	lpcDest+=2;

	wsprintf(lpcDest,"Format:");
	lpcDest+=lstrlen("Format:");
	for (i=0;i<10;i++)
	{
		*lpcDest++=32;
		wsprintf(lpcDest,event_attribs[i]);
		lpcDest+=lstrlen(lpcDest);
		if (i<9) *lpcDest++=',';
	}
	wsprintf(lpcDest,"%c%c",13,10);
	lpcDest+=2;

	while (lpCurrSub)
	{
		qwMSBegin=0;
		qwMSEnd=0;
		if ((lpCurrSub->qwBegin-qwBias>=qwBegin)&&(lpCurrSub->qwEnd-qwBias<=qwEnd)&&(lpCurrSub->dwX2))
		{
			qwMSBegin=(lpCurrSub->qwBegin-qwBias)/1000000;
			qwMSEnd=(lpCurrSub->qwEnd-qwBias)/1000000;
		}
		else
		if ((lpCurrSub->qwBegin-qwBias>=qwBegin)&&(lpCurrSub->qwBegin-qwBias<=qwEnd)&&(lpCurrSub->dwX2))
		{
			qwMSBegin=(lpCurrSub->qwBegin-qwBias)/1000000;
			qwMSEnd=(qwEnd)/1000000;
		}
		else
		if ((lpCurrSub->qwEnd-qwBias>=qwBegin)&&(lpCurrSub->qwEnd-qwBias<=qwEnd)&&(lpCurrSub->dwX2))
		{
			qwMSBegin=(qwBegin)/1000000;
			qwMSEnd=(lpCurrSub->qwEnd-qwBias)/1000000;
		}

		if (qwMSBegin||qwMSEnd)
		{
			ZeroMemory(cBuffer,sizeof(cBuffer));

			RenderSSAEvent(lpCurrSub,&lpcDest,qwMSBegin,qwMSEnd);
			
			wsprintf(lpcDest,"%c%c",13,10);
			lpcDest+=2;
		}
		lpCurrSub=(SUBTITLE_DESCRIPTOR*)lpCurrSub->lpNext;
	}


	(*lpDest)=lpdwDest;

	return (1);
}

int SUBTITLESOURCE::SSAStylesEqual(SSA_STYLE_STRUCT* lpSSA1,SSA_STYLE_STRUCT* lpSSA2)
{
	for (int i=1;i<18;i++)
	{
		if (
			(((SSA_STYLE*)lpSSA1)->lpcArray[i] == 0) ^
			(((SSA_STYLE*)lpSSA2)->lpcArray[i] == 0)
			
			)
			return 0;

		if (((SSA_STYLE*)lpSSA1)->lpcArray[i] != NULL &&
			((SSA_STYLE*)lpSSA2)->lpcArray[i] != NULL)
		{
			if (strcmp(((SSA_STYLE*)lpSSA1)->lpcArray[i],((SSA_STYLE*)lpSSA2)->lpcArray[i])) return 0;
		}
	}

	return 1;
}

int SUBTITLES::Merge(SUBTITLES* lpSubsToMerge,__int64 qwBias)
{
	SUBTITLE_DESCRIPTOR*	lpTemp;
	SUBTITLE_DESCRIPTOR*	lpNext;

	SSA_STYLE**				styles;
	DWORD					dwStyleCount=0;
	int						i;
	bool					bStyleExists;
	char*					lpcText1,*lpcText2;
	bool					bIdenticalStyles;

	if (GetFormat()!=lpSubsToMerge->GetFormat()) return false;

	lpTemp=lastsub;
	lastsub->lpNext=lpSubsToMerge->GetData();
	lpNext=lpSubsToMerge->GetData();

	if (lpNext->dwX2)
	{
		switch (GetFormat())
		{
			case SUBFORMAT_SRT:
				lpcText1=lastsub->lpcText;
				lpcText2=lpNext->lpcText;
				bIdenticalStyles=true;
				break;
			case SUBFORMAT_SSA:
				if (lastsub && lastsub->lpSSA) {
					lpcText1=lastsub->lpSSA->sesStruct.lpcText;
				} else {
					lpcText1="";
				}
				lpcText2=lpNext->lpSSA->sesStruct.lpcText;
				bIdenticalStyles=lastsub && lastsub->lpSSA && !!SSAStylesEqual(lpNext->lpSSA->sesStruct.lpsssStyle,lastsub->lpSSA->sesStruct.lpsssStyle);
				break;
		}
		if (bIdenticalStyles&&(!lstrcmp(lastsub->lpcText,lpNext->lpcText)))
		{
			if (lastsub->qwEnd==lpNext->qwBegin+qwBias)
			{
				lastsub->qwEnd=lpNext->qwEnd+qwBias;
				lastsub->lpNext=lpNext->lpNext;
			}
		}
	}

	lastsub=(SUBTITLE_DESCRIPTOR*)lastsub->lpNext;

	while (lastsub->dwX2)
	{
		lastsub->qwBegin+=qwBias;
		lastsub->qwEnd+=qwBias;
		lpTemp=lastsub;
		lastsub=(SUBTITLE_DESCRIPTOR*)lastsub->lpNext;
	}
	lastsub=lpTemp;

	if ((GetFormat()==SUBFORMAT_SSA)&&(subs))
	{
		styles=new SSA_STYLE*[1024];
		ZeroMemory(styles,4096);

		lpTemp=subs;
		if (!lpTemp->dwX2) lpTemp = (SUBTITLE_DESCRIPTOR*)lpTemp->lpNext;
		while (lpTemp->dwX2)
		{
			bStyleExists=false;
			for (i=0;i<(int)dwStyleCount;i++)
			{
				if (SSAStylesEqual(&styles[i]->sssStruct,lpTemp->lpSSA->sesStruct.lpsssStyle))
				{
					bStyleExists=true;
					lpTemp->lpSSA->sesStruct.lpsssStyle=(SSA_STYLE_STRUCT*)styles[i];

				}
			}
			if (!bStyleExists)
			{
				styles[dwStyleCount]=new SSA_STYLE;
				ZeroMemory(styles[dwStyleCount],sizeof(SSA_STYLE));
				styles[dwStyleCount]->sssStruct=*lpTemp->lpSSA->sesStruct.lpsssStyle;
				lpTemp->lpSSA->sesStruct.lpsssStyle=(SSA_STYLE_STRUCT*)styles[dwStyleCount++];
			}
			lpTemp=(SUBTITLE_DESCRIPTOR*)lpTemp->lpNext;
		}

		for (i=0;i<(int)dwStyleCount;i++)
		{
			free(styles[i]->sssStruct.lpcName);
			styles[i]->sssStruct.lpcName=new char[20];
			wsprintf(styles[i]->sssStruct.lpcName,"style%d",i+1);
		}

		free(lplpSSAStyles);
		lplpSSAStyles=new SSA_STYLE*[dwStyleCount];
		memcpy(lplpSSAStyles,styles,4*dwStyleCount);
		dwNbrOfStyles=dwStyleCount;
	}


	return 1;
}

int SUBTITLES::Seek(__int64 iTime)
{
	SUBTITLE_DESCRIPTOR*	lpTemp;
	iTime*=GetTimecodeScale();
	lpTemp = subs;

	while (lpTemp && lpTemp->qwEnd < iTime - GetBias(BIAS_UNSCALED)) {
		lpTemp = (SUBTITLE_DESCRIPTOR*)lpTemp->lpNext;
	}

	curr_sub = lpTemp;

	return 0;
}
