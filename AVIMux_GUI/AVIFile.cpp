/* Disclaimer:

   Recently someone tried to understand the code in this file, and asked
   me one question after another, why I did certain things the way I did
   them. I came to the conclusion that this file sucks terribly and is NOT
   suited to be used for "educational purpose", it rather demonstrates how
   obscurely things can be done and what happens if you first code and then
   think about what you should have done instead of what you have done.

   If you try to understand this file, try it on your own risk.

*/

#include "stdafx.h"
#include "debug.h"
#include "math.h"
#include "..\utf-8.h"
#include "AVIFile.h"

				///////////////////////////
				//  AVIFILEEX - Methoden //
				///////////////////////////

enum { FORMAT_UNKNOWN, FORMAT_PAL_SQUARE, FORMAT_PAL_CCIR_601, FORMAT_NTSC_SQUARE, FORMAT_NTSC_CCIR_601 } VIDEO_FORMAT;
enum { STANDARD_UNKNOWN, STANDARD_PAL, STANDARD_NTSC, STANDARD_SECAM } VIDEO_STANDARD;

#define even(x) x+x%2

AVIFILEEX::AVIFILEEX(void)
{
	bOpened=false;
	hDebugFile=NULL;
	SetSource(NULL);
	dest=NULL;
	bDebug=false;
	qwNSPF=0;
	bCreateLegacyIndexForODML=false;
	lpRSIP=NULL;
	frametypes.dwDelta=0;
	frametypes.dwDropped=0;
	frametypes.dwKey=0;
	dwMaxAllowedChunkSize=0;
	cWritingApp = 0;
	cFileTitle = NULL;
	ZeroMemory(&cbfs,sizeof(cbfs));
	ZeroMemory(&vprp,sizeof(vprp));
	bLowOverheadMode = false;
	iStreamOfLastChunk = -2;
	bMoveHDRLAround = false;
	dwAccess = -1;
	video_stream_index = -1;
	siStreams = 0;
}

AVIFILEEX::~AVIFILEEX(void)
{
}

STREAMINFO::STREAMINFO()
{
	lpHeader = NULL;
	lpFormat = NULL;
	lpOutputFormat = NULL;
	lpIndx = NULL;
	dwChunkCount = NULL;
	qwStreamLength = NULL;
	dwProcessMode = NULL;
	dwPos = NULL;		
	bCompressed = NULL;	
	bDefault = NULL;
	lpBufIn = NULL;
	lpBufOut = NULL;
	dwOffset = NULL;
	lpcName = NULL;
}

STREAMINFO::~STREAMINFO()
{
}

void AVIFILEEX::EnableLowOverheadMode(bool bEnabled)
{
	if (!IsWriteODML()) return;
	if (IsLegacyEnabled()) return;
	bLowOverheadMode = bEnabled;
}

bool AVIFILEEX::IsLowOverheadMode()
{
	return bLowOverheadMode;
}

DWORD AVIFILEEX::Open(STREAM* lpStream, DWORD Access, DWORD dwAVIType)
{
	char*	Buffer;
	DWORD	dwRead,dwWritten;
	DWORD	i=0;
	union
	{
		LISTHEADER		lhListHdr;
		CHUNKHEADER		chChunkHdr;
	};

	ZeroMemory(&cbfs,sizeof(cbfs));

//	siStreams = NULL;

	lpExtAVIHeader = NULL;
	lpMainAVIHeader = NULL;
	void*	lpTemp;
	lpIdx1 = NULL;
	fmFieldMode = FM_NONE;
	dwFrameCachePos = 0;
	bCreateLegacyIndexForODML=false;
	WAVEFORMATEX*  lpwfe,*lpwfeIn;
	DWORD	dwRebuildIndex=0;
	dwRealFramesInRIFF=0;
	if (cFileTitle) delete cFileTitle;
	cFileTitle = NULL;

	if (Access & FA_READ)
	{	
		bDummyMode = !!(Access & FA_DUMMY);
		ZeroMemory(&abs_pos,sizeof(abs_pos));
		Buffer=new char[4096];
		ZeroMemory(Buffer,sizeof(char));
		SetSource(lpStream);
		lpStream->Seek(0);
		if (bDebug)
		{
			ZeroMemory(Buffer,sizeof(char));
			lstrcpy(Buffer, FileName);
			lstrcat(Buffer, " - Debug-Messages.txt");
			hDebugFile=CreateFile(Buffer,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
			if (hDebugFile==INVALID_HANDLE_VALUE)
			{
				wsprintf(Buffer, "Datei %s konnte nicht geˆffnet werden",
					Buffer);
				MessageBox(0,Buffer,"Fehler",MB_OK);
				SetDebugState(DS_DEACTIVATE);
			}
		}
		DebugMsg("Datei geˆffnet (read-only)");

		if (dwAVIType==AT_AUTODETECT) 
		{
			atType=AT_STANDARD;
		}
		else
		{
			return AFE_INVALIDPARAM;
		}
		if (!ProcessHeader()) 
		{
			DebugMsg ("hdrl fehlerhaft !");
			delete Buffer;
			return false;
		}
		
		if (bDummyMode) {
			delete Buffer;
			return AFE_OK;
		}

		if (!CheckIndxCount())
	//		DebugMsg ("Anzahl an indx-Chunks ist ung¸ltig! Datei wird als Standard-AVI behandelt");
			atType=AT_STANDARD;
		
		/*else
		{
			DebugMsg("Anzahl an indx-Chunks ist g¸ltig");
		}*/
	
		if (!LocateData(MakeFourCC("movi"),NULL,NULL,&lhListHdr,1000000,DT_LIST))
		{
			DebugMsg ("movi nicht gefunden !");
			delete Buffer;
			return OA_INVALIDFILE;
		}
		DebugMsg ("movi gefunden");
		qwMoviPos=GetSource()->GetPos();
		qwFirstDataChunkPos=qwMoviPos;
		DWORD d; GetSource()->Read(&d, 4);
		if (d == MakeFourCC("LIST")) {
			qwFirstDataChunkPos+=12;
		}
		GetSource()->Seek(qwMoviPos);

		GetSource()->Seek(GetSource()->GetPos()+even(lhListHdr.dwLength)-4);
		bidx1present=true;

		if (!LocateData(MakeFourCC("idx1"),NULL,NULL,&chChunkHdr,1000000,DT_CHUNK))
		{
			DebugMsg ("idx1 nicht gefunden");
			bidx1present=false;
			if (atType!=AT_OPENDML)
			{
				return OA_NOINDEX;
			}
		}
		else
		{
			DebugMsg ("idx1 gefunden");
		}
		
		for (i=0;i<lpMainAVIHeader->dwStreams;i++)
		{
			siStreams[i].dwPos=0;
			siStreams[i].dwChunkCount=0;
			siStreams[i].qwStreamLength=0;
			siStreams[i].bCompressed=true;
		}
				
		if (atType==AT_OPENDML)
		{
			DebugMsg ("idx1 wird ignoriert.");
			if (cbfs.lpsri)
			{
				dwRebuildIndex=(cbfs.lpsri)(&cbfs.lpriscb,cbfs.lpsriud);
			}
			else
			{
				dwRebuildIndex=0;
			}
			if (!dwRebuildIndex)
			{
				for (i=0;i<lpMainAVIHeader->dwStreams;i++)
				{
					lpTemp=lpRSIP;
					if (!ProcessExtIndex((_aviindex_chunk*)siStreams[i].lpIndx,i,&(lpRSIP[i])))
					{
						wsprintf(Buffer,"  Indx von Stream %d fehlerhaft !",i); 
						DebugMsg("  Datei wird als Standard-AVI behandelt. ");
						DebugMsg("  --> Indx wird ignoriert; verwende Idx1");
						atType=AT_STANDARD;
						i=lpMainAVIHeader->dwStreams+1;
						
						for (DWORD j=0;j<lpMainAVIHeader->dwStreams;j++)
						{
							siStreams[j].dwPos=0;
							siStreams[j].dwChunkCount=0;
							siStreams[j].qwStreamLength=0;
						}
					}
					else
					{
						wsprintf(Buffer,"  Indx von Stream %d i.O.",i); 
					};
					lpRSIP=(READSUPERINDEXPROTOCOL*)lpTemp;
					DebugMsg(Buffer);
				}
			}
			else
			{
				// Rebuild Index
			}
		}
		if (atType==AT_STANDARD)
		{
			if (lpRSIP) free (lpRSIP);
			lpRSIP=NULL;
			lpIdx1=(AVIINDEXENTRY*)new char[chChunkHdr.dwLength];
			dwRead=GetSource()->Read(lpIdx1,chChunkHdr.dwLength);
			ProcessIdx1(lpIdx1,chChunkHdr.dwLength / sizeof(AVIINDEXENTRY));
			free(lpIdx1);
			lpIdx1=NULL;
		}
	
		switch (atType)
		{
			case AT_STANDARD: DebugMsg ("Standard-AVI festgestellt"); break;
			case AT_OPENDML: DebugMsg ("OpenDML-AVI festgestellt"); break;
		}
		for (i=0;i<lpMainAVIHeader->dwStreams;i++)
		{
			siStreams[i].dwPos=0;
			siStreams[i].lpOutputFormat=NULL;
			siStreams[i].dwOffset=0;
			ZeroMemory(Buffer,sizeof(Buffer));
			wsprintf(Buffer,"  Stream %d: %7.0d Chunks, %10.0d kBytes",i,siStreams[i].dwChunkCount,(DWORD)(siStreams[i].qwStreamLength/1024));
			DebugMsg(Buffer);
			if (IsVideoStream(i)) video_stream_index = i;
		}
		for (i=1;i<lpMainAVIHeader->dwStreams;i++)
		{
			if (IsAudioStream(i))
			{
				siStreams[i].lpOutputFormat=new WAVEFORMATEX;
				lpwfe=((WAVEFORMATEX*)siStreams[i].lpOutputFormat);
				lpwfeIn=((WAVEFORMATEX*)siStreams[i].lpFormat);
				memcpy(lpwfe,siStreams[i].lpFormat,sizeof(WAVEFORMATEX));
				siStreams[i].bCompressed=(lpwfeIn->wFormatTag==1)?false:true;
				lpwfe->wFormatTag=1;
				lpwfe->cbSize=0;
				lpwfe->wBitsPerSample=16;
				lpwfe->nBlockAlign=lpwfe->nChannels*(lpwfe->wBitsPerSample/8);
				lpwfe->nAvgBytesPerSec=lpwfe->nSamplesPerSec*lpwfe->nBlockAlign;
				wsprintf(Buffer,"Audiostr. %d: ID: 0x%x, %d Samples/s, %d Kan‰le, %d Bit, %d Bytes/s, Blockalign: %d Bytes, %d komp. Bytes/s",
					i,lpwfeIn->wFormatTag,lpwfeIn->nSamplesPerSec,lpwfeIn->nChannels,lpwfe->wBitsPerSample,lpwfe->nAvgBytesPerSec,lpwfeIn->nBlockAlign,
					lpwfeIn->nAvgBytesPerSec);
				DebugMsg(Buffer);
			}
		}
/*		if (!(hic=ICOpen(MakeFourCC("VIDC"),siStreams[0].lpHeader->fccHandler,ICMODE_FASTDECOMPRESS)))
		{
			if (!(hic=ICLocate(MakeFourCC("VIDC"),0,(BITMAPINFOHEADER*)siStreams[0].lpFormat,NULL,ICMODE_FASTDECOMPRESS)))
			{
				DebugMsg ("Videohandler nicht gefunden !");
//				return OA_INVALIDVIDEOHANDLER;
			}
		}
*///		hic=NULL;
	//	DebugMsg ("Videohandler geˆffnet");
		strfVideo=(BITMAPINFOHEADER*)siStreams[0].lpFormat;
		ZeroMemory(&bmi24bpp,sizeof(bmi24bpp));
		bmi24bpp.biSize=sizeof(BITMAPINFOHEADER);
		bmi24bpp.biWidth=strfVideo->biWidth;
		bmi24bpp.biHeight=strfVideo->biHeight;
		bmi24bpp.biPlanes=1;
		bmi24bpp.biBitCount=24;
		bmi24bpp.biCompression=BI_RGB;
		bmi24bpp.biSizeImage=0;
		bmi24bpp.biXPelsPerMeter=0;
		bmi24bpp.biYPelsPerMeter=0;
		bmi24bpp.biClrUsed=0;
		bmi24bpp.biClrImportant=0;
		siStreams[0].lpOutputFormat=new BITMAPINFOHEADER;
		memcpy(siStreams[0].lpOutputFormat,&bmi24bpp,sizeof(BITMAPINFOHEADER));
		for (i=0;i<FrameCacheSize;i++)
		{
			FrameCache[i]=new FRAME;
			((FRAMEINFO*)(FrameCache[i]->GetUserData(sizeof(FRAMEINFO))))->dwAccCount=0;
			((FRAMEINFO*)(FrameCache[i]->GetUserData(sizeof(FRAMEINFO))))->iFrameNbr=-1;
			FrameCache[i]->SetWidth(strfVideo->biWidth);
			FrameCache[i]->SetHeight(strfVideo->biHeight);
			FrameCache[i]->Realloc();
		}
		free (Buffer);
	}
	else
	if (Access==FA_WRITE)
	{
		if (atType==AT_AUTODETECT) return AFE_INVALIDPARAM;

		atType=dwAVIType;
		dwHeaderSpace=(atType==AT_STANDARD)?4096:65524;
		Buffer=new char[dwHeaderSpace+12];
		ZeroMemory(Buffer,dwHeaderSpace+12);
		dest=lpStream;

		if (bDebug)
		{
			ZeroMemory(Buffer,sizeof(char));
			lstrcpy(Buffer, FileName);
			lstrcat(Buffer, " - Debug-Messages.txt");
			hDebugFile=CreateFile(Buffer,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
			if (!hDebugFile)
			{
				wsprintf(Buffer, "Datei %s konnte nicht geˆffnet werden",
					Buffer);
				MessageBox(0,Buffer,"Fehler",MB_OK);
				SetDebugState(DS_DEACTIVATE);
			}
		}

		lpMainAVIHeader=new MainAVIHeader;
		ZeroMemory(lpMainAVIHeader,sizeof(MainAVIHeader));
		lpExtAVIHeader=new ODMLExtendedAVIHeader;
		ZeroMemory(lpExtAVIHeader,sizeof(ODMLExtendedAVIHeader));

		ZeroMemory(Buffer,dwHeaderSpace+12);
		dwWritten=dest->Write(Buffer,dwHeaderSpace+12);
		dwMoviPos=dwHeaderSpace;
		qwFilePos=dwHeaderSpace+12;
		dwCacheSize=1*1048576;
		dwChunkCount=0;
		dwRecCount=0;
		dwMoviSize=0;
		qwRIFFStart=0;

		FirstChunk=new CHUNK;
		FirstList=new LIST;
		LastChunk=FirstChunk;
		LastList=FirstList;

		bRECListOpen=false;

		if (GetAVIType()==AT_STANDARD)
		{
			Index=new INDEX;
			LastIndex=Index;
		}
		else
		{
			Index=new STANDARDINDEX;
			LastIndex=Index;
		}

//		SuperIndex wird erst beim Schlieﬂen der RIFF-AVI begonnen!
		SetPadding(2);
		free(Buffer);
		SetMaxRIFFAVISize(1000*(1<<20));
		SetMaxRIFFAVIXSize(1000*(1<<20));
		dwFramesPerIndex=1<<30;
		dwFramesInCurrIndex=0;
		SupIndex=NULL;
		LastSupIndex=NULL;
		Index->SetRange(0,(__int64)1<<60);
		dwStdIndex_RIFFAVIXOverhead=0;
	}
	else
		return AFE_INVALIDPARAM;

	bOpened=true;
	dwAccess=Access;
	return AFE_OK;
}

void AVIFILEEX::SetTitle(char* cTitle)
{
	if (cFileTitle) delete cFileTitle;
	cFileTitle = new char[1+strlen(cTitle)];
	strcpy(cFileTitle, cTitle);
}

int AVIFILEEX::GetVideoStreamNumber()
{
	return video_stream_index;
}

void AVIFILEEX::SetWritingAppName(char* cName)
{
	if (cWritingApp) {
		delete cWritingApp;
	}

	cWritingApp = new char[1+strlen(cName)];
	strcpy(cWritingApp, cName);
}

char* AVIFILEEX::GetWritingAppName()
{
	return (cWritingApp?cWritingApp:"");
}

void AVIFILEEX::FillMP3VBR(AVIStreamHeader* lphdr,MPEGLAYER3WAVEFORMAT* lpmp3,STREAMINFO* siStr)
{
	if (lpmp3->wfx.wFormatTag == 0x55) {
		lpmp3->wfx.cbSize=12;
		lpmp3->fdwFlags=2;
		lpmp3->nFramesPerBlock=1;
		lpmp3->nCodecDelay=0;
		lpmp3->nBlockSize=(WORD)QW_div(siStr->qwStreamLength,siStr->dwChunkCount,
			"AVIFILEEX::FillMP3VBR::siStr-dwChunkCount");//(WORD)(siStr->qwStreamLength/siStr->dwChunkCount);
	} else {
	}
	
	lphdr->dwLength=siStr->dwChunkCount;

	lphdr->fccHandler=0;
	lphdr->dwFlags=0;
	lphdr->wPriority=0;
	lphdr->wLanguage=0;
	lphdr->dwInitialFrames=0;
	lphdr->dwStart=0;
	lphdr->dwSuggestedBufferSize=0;
	lphdr->dwQuality=0xffffffff;
	lphdr->dwSampleSize=0;
}

void AVIFILEEX::FillMP3CBR(AVIStreamHeader* lphdr,MPEGLAYER3WAVEFORMAT* lpmp3,STREAMINFO* siStr)
{
	if (lpmp3->wfx.wFormatTag == 0x0055) {
		lpmp3->wfx.cbSize=12;
		lpmp3->fdwFlags=2;
		lpmp3->nFramesPerBlock=1;
	}
	
	lphdr->dwLength=(DWORD)(siStr->qwStreamLength);
	lphdr->dwRate=lpmp3->wfx.nAvgBytesPerSec;
	lphdr->dwScale=1;
	lphdr->fccHandler=0;
	lphdr->dwFlags=0;
	lphdr->wPriority=0;
	lphdr->wLanguage=0;
	lphdr->dwInitialFrames=1;
	lphdr->dwStart=0;
	lphdr->dwSuggestedBufferSize=0;
	lphdr->dwQuality=0xffffffff;
	lphdr->dwSampleSize=1;
}

void AVIFILEEX::FillTXTSHeaders(AVIStreamHeader* lphdr)
{
	lphdr->dwFlags=0;
	lphdr->dwLength=1;
	lphdr->dwQuality=0;
	lphdr->dwInitialFrames=0;
	lphdr->dwRate=1000;
	lphdr->dwSampleSize=0;
	lphdr->dwScale=0;
	lphdr->dwStart=0;
	lphdr->dwSuggestedBufferSize=0x80000;
	lphdr->fccHandler=0;
	lphdr->fccType=MakeFourCC("txts");
	ZeroMemory(&lphdr->rcFrame,sizeof(lphdr->rcFrame));
	lphdr->wLanguage=0;
	lphdr->wPriority=0;
}

__int64 ggT(__int64 x, __int64 y) 
{
	if (!(x%y)) {
		return y;
	}
	return ggT(y, x%y);
}

bool AVIFILEEX::Close(bool bCloseSource)
{
	DWORD		i;
	void*		Buffer;
	DWORD		dwIdx1Size,dwWritten;
	CHUNKHEADER ch;
	LISTHEADER	lh;
	double		dPlaytime;
	WAVEFORMATEX* lpwfe;
	MPEGLAYER3WAVEFORMAT* lpmp3;
	AVIStreamHeader *lphdr;
	DWORD		dwSize;


	if (dwAccess==FA_READ)
	{
		if (bCloseSource) GetSource()->Close();
		for (i=0;i<FrameCacheSize;i++)
		{
			if (FrameCache[i]) delete FrameCache[i];
			FrameCache[i]=NULL;
		}
		if (siStreams)
		{
			for (DWORD i=0;i<lpMainAVIHeader->dwStreams;i++)
			{
				if (siStreams[i].lpFormat) free (siStreams[i].lpFormat);
				if (siStreams[i].lpHeader) free (siStreams[i].lpHeader);
				if (siStreams[i].lpIndx) free (siStreams[i].lpIndx);
				if (siStreams[i].lpOutputFormat) free (siStreams[i].lpOutputFormat);
				if (siStreams[i].lpcName) free (siStreams[i].lpcName);
				if (lpRSIP) if (lpRSIP[i].rsipEntries) free (lpRSIP[i].rsipEntries);
			}
			delete[] siStreams;
			siStreams=NULL;
			if (lpRSIP)
			{
				free (lpRSIP);
				lpRSIP=NULL;
			}
		}
		if (lpMainAVIHeader) free(lpMainAVIHeader); lpMainAVIHeader=NULL;
		if (lpExtAVIHeader) free (lpExtAVIHeader); lpExtAVIHeader=NULL;
		if (lpIdx1) free(lpIdx1); lpIdx1=NULL;
	//	DebugMsg("Datei geschlossen");
		SetSource(NULL);
		if (hDebugFile)
		{
			CloseHandle(hDebugFile);
		}
//		if (hic) ICClose(hic);
	}
	if (dwAccess==FA_WRITE)
	{
		
		int _ggt = (int)ggT((DWORD)(GetNanoSecPerFrame()/100),10000000);
		siStreams[0].lpHeader->dwScale=(DWORD)(GetNanoSecPerFrame()/100)/_ggt;
		siStreams[0].lpHeader->dwRate=10000000/_ggt;

		if (GetAVIType()==AT_STANDARD)
		{
		// Index schreiben
			dwIdx1Size=Index->GetSize();//16*(dwChunkCount+dwRecCount);
			Buffer=new char[dwIdx1Size];
			Index->Store(Buffer);
			dwMoviSize=(DWORD)(qwFilePos-dwMoviPos-8);

			ch.dwFourCC=MakeFourCC("idx1");
			ch.dwLength=dwIdx1Size;
			dwWritten=dest->Write(&ch,sizeof(ch));
			dwWritten=dest->Write(Buffer,dwIdx1Size);
			free (Buffer);
		// Header bauen und schreiben
			lh.dwListID=MakeFourCC("RIFF");
			lh.dwFourCC=MakeFourCC("AVI ");
			lh.dwLength=(DWORD)(qwFilePos+dwIdx1Size);
			dest->Seek(0);
			dwWritten=dest->Write(&lh,12);
			
			ch.dwFourCC = MakeFourCC("JUNK");
			ch.dwLength = 0;
			if (bMoveHDRLAround) {
				dest->Write(&ch, 8);
			//	dwMoviSize -= 8;
			}

			lpMainAVIHeader->dwFlags=AVIF_HASINDEX;

			lpMainAVIHeader->dwTotalFrames=siStreams[0].dwChunkCount;
			lpMainAVIHeader->dwWidth=((BITMAPINFOHEADER*)(siStreams[0].lpFormat))->biWidth;
			lpMainAVIHeader->dwHeight=((BITMAPINFOHEADER*)(siStreams[0].lpFormat))->biHeight;
			lpMainAVIHeader->dwPaddingGranularity=0;
			lpMainAVIHeader->dwSuggestedBufferSize=0;//0x40000;
			siStreams[0].lpHeader->dwLength=siStreams[0].dwChunkCount;
			FirstList->SetFourCC(MakeFourCC("hdrl"));
			FirstList->SetData(FirstChunk);
			FirstChunk->SetSize(sizeof(MainAVIHeader));
			FirstChunk->SetFourCC(MakeFourCC("avih"));
			FirstChunk->SetData(lpMainAVIHeader);
			FirstChunk->lpNext=new LIST;
			dPlaytime=((double)(siStreams[0].dwChunkCount)*(double)(GetMicroSecPerFrame())/1000000);
			LastList=(LIST*)FirstChunk->lpNext;
			for (i=0;i<lpMainAVIHeader->dwStreams;i++)
			{
				lpwfe=NULL;
				lphdr=siStreams[i].lpHeader;
				if (IsAudioStream(i))
				{
					lpwfe=((WAVEFORMATEX*)siStreams[i].lpFormat);
					if (!(lpwfe->nAvgBytesPerSec))
					{
						lpwfe->nAvgBytesPerSec=(DWORD)round(d_div((double)siStreams[i].qwStreamLength,
							dPlaytime,"AVIFILEEX::Close()::dPlaytime"));
					}
					if (lpwfe->wFormatTag==0x55 || lpwfe->wFormatTag==0x50)
					{
						lpmp3=((MPEGLAYER3WAVEFORMAT*)siStreams[i].lpFormat);
						if ((lpmp3->wID==1)&&(IsMP3SampleCount(lpmp3->wfx.nBlockAlign)))
						{
							// VBR MP3
							FillMP3VBR(lphdr,lpmp3,&(siStreams[i]));
							
						}
						else
						{
							// CBR MP3
							FillMP3CBR(lphdr,lpmp3,&(siStreams[i]));
						}
					}
					else
					{
							lphdr=siStreams[i].lpHeader;
							lphdr->dwLength=(DWORD)QW_div(siStreams[i].qwStreamLength,
								lphdr->dwScale,"AVIFILEEX::Close()::lphdr->dwScale");//(DWORD)(siStreams[i].qwStreamLength/lphdr->dwScale);
					}
					if (!lphdr->dwSuggestedBufferSize) lphdr->dwSuggestedBufferSize=2*dwLargestChunks[i];
					if (lpwfe->wFormatTag==0x2000) // AC3
					{
						lpwfe->cbSize=0;
						lphdr->dwInitialFrames=1;
					}
					else
					if (lpwfe->wFormatTag==0x2001) // DTS
					{
						lpwfe->cbSize=0;
						lphdr->dwInitialFrames=1;
					}
					else
					if (lpwfe->wFormatTag==0x01)
					{
						lpwfe->cbSize=0;
						lpwfe->nAvgBytesPerSec=lpwfe->nSamplesPerSec*lpwfe->wBitsPerSample/8*lpwfe->nChannels;
					}
					else
					if (lpwfe->wFormatTag==0xFF)
					{
						lphdr->dwLength = siStreams[i].dwChunkCount;
					}

				}
				else
				if (IsTextStream(i))
				{
					lphdr=siStreams[i].lpHeader;
					FillTXTSHeaders(lphdr);
					lphdr->dwSuggestedBufferSize=dwLargestChunks[i];
					lphdr->dwScale=(int)(siStreams[i].qwStreamLength/1000);
				}
				else
				if (IsVideoStream(i))
				{
					lphdr=siStreams[i].lpHeader;
					lphdr->dwSuggestedBufferSize=2*dwLargestChunks[i];
					siStreams[i].bDefault = 1;
				}
					

				LastList->SetFourCC(MakeFourCC("strl"));
				LastChunk=new CHUNK;
				LastList->SetData(LastChunk);
			
				LastChunk->SetFourCC(MakeFourCC("strh"));
				if (lpwfe) 
				{
					if (lpwfe->wFormatTag==0x2000) 
					{
						LastChunk->SetSize(0x38);
					}
					else
					if (lpwfe->wFormatTag==0x2001) 
					{
						LastChunk->SetSize(0x38);
					}
					else
					{
						LastChunk->SetSize(sizeof(AVIStreamHeader));
					}
				}
				else
				{
					LastChunk->SetSize(sizeof(AVIStreamHeader));
				}
				siStreams[i].lpHeader->dwFlags &=~ AVISF_DISABLED;
				siStreams[i].lpHeader->dwFlags |= (!siStreams[i].bDefault) * AVISF_DISABLED;
				LastChunk->SetSize(0x38);
				LastChunk->SetData(siStreams[i].lpHeader);

				LastChunk->lpNext=new CHUNK;
				LastChunk=(CHUNK*)LastChunk->lpNext;
				LastChunk->SetFourCC(MakeFourCC("strf"));
				LastChunk->SetSize(strfSize(i,siStreams[i].lpFormat));
				LastChunk->SetData(siStreams[i].lpFormat);

				if (siStreams[i].lpcName)
				{
					LastChunk->lpNext=new CHUNK;
					LastChunk=(CHUNK*)LastChunk->lpNext;
					LastChunk->SetFourCC(MakeFourCC("strn"));
					LastChunk->SetSize(1+lstrlen(siStreams[i].lpcName));
					LastChunk->SetData(siStreams[i].lpcName);
				}

				if (i+1<lpMainAVIHeader->dwStreams)
				{
					LastList->lpNext=new LIST;
					LastList=(LIST*)LastList->lpNext;
				}
				//lpMainAVIHeader->dwSuggestedBufferSize+=lphdr->dwSuggestedBufferSize;
			}

			FirstChunk->SetData(lpMainAVIHeader);

			LastList->lpNext=new CHUNK;
			LastChunk=(CHUNK*)LastList->lpNext;
			LastChunk->SetFourCC(MakeFourCC("JUNK"));
			LastChunk->SetSize(0);
			i=FirstChunk->GetSize(LE_CHAIN);
			LastChunk->SetSize(dwHeaderSpace-12-12-i-((bMoveHDRLAround)?8:0));
			Buffer=new char[FirstList->GetSize(LE_CHAIN)];
			FirstList->Store(Buffer,LE_CHAIN);
			dwWritten=dest->Write(Buffer,FirstList->GetSize(LE_CHAIN));
			lh.dwListID=MakeFourCC("LIST");
			lh.dwFourCC=MakeFourCC("movi");
			lh.dwLength=dwMoviSize;
			dwWritten=dest->Write(&lh,12);
			free (Buffer);
			Index->Delete();
			delete Index;
			if (hDebugFile)
			{
				CloseHandle(hDebugFile);
			}
		}
		else
		{
		// OpenDML-Datei schlieﬂen
			if (qwRIFFStart) 
			{
				EndRIFFAVIX();
			}
			else
			{
				EndRIFFAVI();
			}
			dest->Seek(12);
			lpExtAVIHeader->dwTotalFrames=siStreams[0].dwChunkCount;
			lpMainAVIHeader->dwFlags=AVIF_HASINDEX | AVIF_ISINTERLEAVED;
			if (IsLowOverheadMode()) {
				lpMainAVIHeader->dwFlags |= AVIF_MUSTUSEINDEX;
			}

			lpMainAVIHeader->dwWidth=((BITMAPINFOHEADER*)(siStreams[0].lpFormat))->biWidth;
			lpMainAVIHeader->dwHeight=((BITMAPINFOHEADER*)(siStreams[0].lpFormat))->biHeight;
			lpMainAVIHeader->dwPaddingGranularity=0;
			lpMainAVIHeader->dwSuggestedBufferSize=0;//0x40000;
			siStreams[0].lpHeader->dwLength=siStreams[0].dwChunkCount;
			
			ch.dwFourCC = MakeFourCC("JUNK");
			ch.dwLength = 0;
			if (bMoveHDRLAround)
				dest->Write(&ch, 8);

			FirstList->SetFourCC(MakeFourCC("hdrl"));
			FirstList->SetData(FirstChunk);
			/*FirstChunk->SetSize(sizeof(MainAVIHeader));
			FirstChunk->SetFourCC(MakeFourCC("avih"));
			FirstChunk->SetData(lpMainAVIHeader);
			FirstChunk->lpNext=new LIST;*/
			FirstChunk->Set(MakeFourCC("avih"), sizeof(MainAVIHeader), lpMainAVIHeader, new LIST);
			dPlaytime=((double)(siStreams[0].dwChunkCount)*(double)(lpMainAVIHeader->dwMicroSecPerFrame)/1000000);
			LastList=(LIST*)FirstChunk->lpNext;
			for (i=0;i<lpMainAVIHeader->dwStreams;i++)
			{
				lpwfe=NULL;
				lphdr=siStreams[i].lpHeader;
				if (IsAudioStream(i))
				{
					lpwfe=((WAVEFORMATEX*)siStreams[i].lpFormat);
					if (!(lpwfe->nAvgBytesPerSec)) lpwfe->nAvgBytesPerSec=(DWORD)round(d_div((double)siStreams[i].qwStreamLength,dPlaytime,
						"AVIFILEEX::Close()::dPlayTime"));

					if (lpwfe->wFormatTag==0x55 || lpwfe->wFormatTag == 0x50)
					{
						lpmp3=((MPEGLAYER3WAVEFORMAT*)siStreams[i].lpFormat);
						if ((lpmp3->wID==1)&&(IsMP3SampleCount(lpmp3->wfx.nBlockAlign)))
						{
							// VBR MP3
							FillMP3VBR(lphdr,lpmp3,&(siStreams[i]));
						}
						else
						{
							// CBR MP3
							FillMP3CBR(lphdr,lpmp3,&(siStreams[i]));
						}
					}
					else
					{
							lphdr=siStreams[i].lpHeader;
							lphdr->dwLength=(DWORD)QW_div(siStreams[i].qwStreamLength,lphdr->dwScale,
								"AVIFILEEX::Close()::lphdr->dwScale");
					}
					if (!lphdr->dwSuggestedBufferSize) lphdr->dwSuggestedBufferSize=2*dwLargestChunks[i];
					if (lpwfe->wFormatTag==0x2000) // AC3
					{
						lpwfe->cbSize=0;
					}
					else
					if (lpwfe->wFormatTag==0x2001) // DTS
					{
						lpwfe->cbSize=0;
					}
					else
					if (lpwfe->wFormatTag==0x01)
					{
						lpwfe->cbSize=0;
						lpwfe->nAvgBytesPerSec=lpwfe->nSamplesPerSec*lpwfe->wBitsPerSample/8*lpwfe->nChannels;
					}
					else
					if (lpwfe->wFormatTag==0xFF) // AAC
					{
						lphdr->dwLength = siStreams[i].dwChunkCount;
					}

				}
				else
				if (IsTextStream(i))
				{
					lphdr=siStreams[i].lpHeader;
					FillTXTSHeaders(lphdr);
					lphdr->dwSuggestedBufferSize=dwLargestChunks[i];
					lphdr->dwScale=(int)(siStreams[i].qwStreamLength/1000);
				}
				else
				if (IsVideoStream(i))
				{
					lphdr=siStreams[i].lpHeader;
					lphdr->dwSuggestedBufferSize=2*dwLargestChunks[i];
					siStreams[i].bDefault = 1;
				}
				LastList->SetFourCC(MakeFourCC("strl"));
				LastChunk=new CHUNK;
				LastList->SetData(LastChunk);
			
				LastChunk->SetFourCC(MakeFourCC("strh"));
				if (lpwfe) 
				{
					if (lpwfe->wFormatTag==0x2000) 
					{
						LastChunk->SetSize(0x38);
					}
					else
					if (lpwfe->wFormatTag==0x2001) 
					{
						LastChunk->SetSize(0x38);
					}
					else
					{
						LastChunk->SetSize(sizeof(AVIStreamHeader));
					}
				}
				else
				{
					LastChunk->SetSize(sizeof(AVIStreamHeader));
				}
				siStreams[i].lpHeader->dwFlags &=~ AVISF_DISABLED;
				siStreams[i].lpHeader->dwFlags |= (!siStreams[i].bDefault) * AVISF_DISABLED;
				LastChunk->SetSize(0x38);
				LastChunk->SetData(siStreams[i].lpHeader);

				LastChunk->lpNext=new CHUNK;
				LastChunk=(CHUNK*)LastChunk->lpNext;
				/*LastChunk->SetFourCC(MakeFourCC("strf"));
				LastChunk->SetSize(strfSize(i,siStreams[i].lpFormat));
				LastChunk->SetData(siStreams[i].lpFormat);*/
				LastChunk->Set(MakeFourCC("strf"), strfSize(i,siStreams[i].lpFormat),
					siStreams[i].lpFormat, new CHUNK);

				if (siStreams[i].lpcName)
				{
					//LastChunk->lpNext=new CHUNK;
					LastChunk=(CHUNK*)LastChunk->lpNext;
					/*LastChunk->SetFourCC(MakeFourCC("strn"));
					LastChunk->SetSize(1+lstrlen(siStreams[i].lpcName));
					LastChunk->SetData(siStreams[i].lpcName);*/
					LastChunk->Set(MakeFourCC("strn"), 1+lstrlen(siStreams[i].lpcName),
						siStreams[i].lpcName, new CHUNK);
				}

				//LastChunk->lpNext=new CHUNK;
				LastChunk=(CHUNK*)(LastChunk->lpNext);
				LastChunk->SetFourCC(MakeFourCC("indx"));

				SupIndex->SelectStream(i);
				SupIndex->SetStreamHeader(siStreams[i].lpHeader);
				SupIndex->SetStreamFormat(siStreams[i].lpFormat);
				dwSize=SupIndex->GetSize();
				Buffer=new char[dwSize];
				SupIndex->Store(Buffer);
				LastChunk->SetSize(dwSize);
				LastChunk->SetData(Buffer);
				free (Buffer);

				if (i+1<lpMainAVIHeader->dwStreams)
				{
					LastList->lpNext=new LIST;
					LastList=(LIST*)LastList->lpNext;
				}
				//lpMainAVIHeader->dwSuggestedBufferSize+=lphdr->dwSuggestedBufferSize;
			}
			FirstChunk->SetData(lpMainAVIHeader);

			LastList->lpNext=new LIST;
			LastList=(LIST*)LastList->lpNext;
			LastList->SetFourCC(MakeFourCC("odml"));
			LastChunk=new CHUNK;
			LastChunk->SetFourCC(MakeFourCC("dmlh"));
			LastChunk->SetSize(248);
			LastChunk->SetData(lpExtAVIHeader);
			LastList->SetData(LastChunk);

			int j;
			if (cWritingApp || cFileTitle) {
				FirstList->lpNext=new LIST;
				LIST* LastList=(LIST*)FirstList->lpNext;
				LastList->SetFourCC(MakeFourCC("INFO"));
				LastChunk = new CHUNK;
				LastList->SetData(LastChunk);
				
				if (cWritingApp) {
					LastChunk->SetFourCC(MakeFourCC("ISFT"));
					LastChunk->SetSize(strlen(cWritingApp));
					LastChunk->SetData(cWritingApp);
					LastChunk->lpNext = new CHUNK;
					LastChunk = (CHUNK*)LastChunk->lpNext;
				}

				if (cFileTitle) {
					LastChunk->SetFourCC(MakeFourCC("INAM"));
					LastChunk->SetSize(strlen(cFileTitle));
					LastChunk->SetData(cFileTitle);
					LastChunk->lpNext = new CHUNK;
					LastChunk = (CHUNK*)LastChunk->lpNext;
				}

				LastChunk->SetFourCC(MakeFourCC("JUNK"));
				LastChunk->SetSize(0);
				
				j = LastList->GetSize(LE_CHAIN);
			}

			LastList->lpNext=new CHUNK;
			LastChunk=(CHUNK*)LastList->lpNext;
			LastChunk->SetFourCC(MakeFourCC("JUNK"));
			LastChunk->SetSize(0);
			i=FirstChunk->GetSize(LE_CHAIN);
			LastChunk->SetSize(dwHeaderSpace-12-12-i-j-((bMoveHDRLAround)?8:0));
			
			Buffer=new char[FirstList->GetSize(LE_CHAIN)];
			FirstList->Store(Buffer,LE_CHAIN);
			dwWritten=dest->Write(Buffer,FirstList->GetSize(LE_CHAIN));
			Index->Delete();
			SupIndex->Delete();
			delete SupIndex;
			delete Index;
			free(Buffer);
			if (hDebugFile)
			{
				CloseHandle(hDebugFile);
			}
		}
//		hic=0;

		for (i=0;i<lpMainAVIHeader->dwStreams;i++)
		{
			free(siStreams[i].lpHeader);
			free(siStreams[i].lpFormat);
			if (siStreams[i].lpcName) free(siStreams[i].lpcName);
		}
		delete[] siStreams;
		siStreams=NULL;
		delete lpMainAVIHeader;
		lpMainAVIHeader=NULL;
		delete lpExtAVIHeader;
		lpExtAVIHeader=NULL;
		FirstList->FreeData(LE_CHAIN);
		delete FirstList;
		FirstList=NULL;
		delete dwLargestChunks;
		dwLargestChunks=NULL;

	}

	return true;
}

bool AVIFILEEX::IsList(LISTHEADER *lplhListHdr,char* lpFourCC)
{
	return ((lplhListHdr->dwListID!=MakeFourCC("LIST"))||(lplhListHdr->dwFourCC!=MakeFourCC(lpFourCC)))?false:true;
}

void AVIFILEEX::SetMaxAllowedChunkSize(DWORD dwSize)
{
	dwMaxAllowedChunkSize=dwSize;
}

bool AVIFILEEX::CheckRIFF_AVI(void)
{
	LISTHEADER		lhListHdr;
	DWORD			dwRead;

	dwRead=GetSource()->Read(&lhListHdr,12);
	if (lhListHdr.dwListID!=MakeFourCC("RIFF")) 
	{
		DebugMsg("Keine RIFF-Datei !");
		return false;
	}
	if (lhListHdr.dwFourCC!=MakeFourCC("AVI "))
	{
		DebugMsg("Keine AVI-Datei !");
		return false;
	}
	DebugMsg("RIFF-AVI verifiziert");
	dwRIFFSize=lhListHdr.dwLength;
	return true;
}

bool AVIFILEEX::ProcessHDRL(char* lpBuffer,DWORD dwLength)
{
	union
	{
		LISTHEADER  lhListHdr;
		CHUNKHEADER chChunkHdr;
	};
	char	Buffer[200];

	if (!LocateData(MakeFourCC("avih"),&lpBuffer,NULL,&chChunkHdr,dwLength,DT_CHUNK))
	{
		DebugMsg("MainAVIHeader nicht gefunden");
		return false;
	}
	abs_pos.dwAVIH=(DWORD)(lpBuffer)-(dwHDRLBufferStart)+abs_pos.dwHDRL;
	abs_pos.dwMicroSecPerFrame=abs_pos.dwAVIH;
	abs_pos.dwFlags=abs_pos.dwAVIH+12;
	abs_pos.dwTotalFrames=abs_pos.dwMicroSecPerFrame+16;
	GetAVIH(lpBuffer,&chChunkHdr);
	DebugMsg ("MainAVIHeader gefunden");
	lpBuffer+=even(chChunkHdr.dwLength);

	ZeroMemory(Buffer,sizeof(Buffer));
	wsprintf(Buffer,"Streams: %d  µspf: %d  Frames in RIFF-AVI: %d",GetNbrOfStreams(),
		GetMicroSecPerFrame(),lpMainAVIHeader->dwTotalFrames);
	DebugMsg(Buffer);

	siStreams=new STREAMINFO[lpMainAVIHeader->dwStreams];
	lpRSIP=(READSUPERINDEXPROTOCOL*)malloc(sizeof(READSUPERINDEXPROTOCOL)*(lpMainAVIHeader->dwStreams));
	ZeroMemory(lpRSIP,sizeof(READSUPERINDEXPROTOCOL)*(lpMainAVIHeader->dwStreams));
	for (DWORD i=0;i<lpMainAVIHeader->dwStreams;i++)
	{
		ZeroMemory(Buffer,sizeof(Buffer));
		if (!LocateData(MakeFourCC("strl"),&lpBuffer,NULL,&lhListHdr,dwLength,DT_LIST))
		{
			wsprintf(Buffer,"  strl-List Nbr. %d nicht gefunden !",i);
			DebugMsg(Buffer);
			return false;
		}
		else
		{
			siStreams[i].dwProcessMode=PM_PROCESS;
			wsprintf(Buffer,"  strl-List Nbr. %d gefunden",i);
			DebugMsg(Buffer);
			if (!ProcessSTRL(lpBuffer,lhListHdr.dwLength-4,i))
			{
				DebugMsg("  strl fehlerhaft !");
				return false;
			}
			lpBuffer+=even(lhListHdr.dwLength)-4;
			DebugMsg("  strl i.O.");
		}
	}

	if (!LocateData(MakeFourCC("odml"),&lpBuffer,NULL,&lhListHdr,dwLength,DT_LIST))
	{
		DebugMsg ("odml nicht gefunden");
	}
	else
	{
		DebugMsg ("odml gefunden");
		if (!ProcessODML(lpBuffer,dwLength))
		{
			DebugMsg ("odml fehlerhaft; wird ignoriert. Datei wird als Standard-AVI behandelt");
			atType=AT_STANDARD;
		}
		else
		{
			DebugMsg ("odml i.O.");
		}
	}

	return true;
}

bool AVIFILEEX::GetAVIH(char* lpBuffer,CHUNKHEADER* lpchChunkHdr)
{
	lpMainAVIHeader=(MainAVIHeader*)malloc(lpchChunkHdr->dwLength);
    *lpMainAVIHeader=*(MainAVIHeader*)lpBuffer;

	return true;
}

int AVIFILEEX::SetStreamName(DWORD dwStream,char* lpcName)
{
	if (dwStream>=GetNbrOfStreams())
	{
		return AFE_INVALIDPARAM;
	}

	if (siStreams[dwStream].lpcName) delete siStreams[dwStream].lpcName;
	siStreams[dwStream].lpcName=NULL;

	if (lstrlen(lpcName))
	{
		siStreams[dwStream].lpcName=(char*)malloc(1+lstrlen(lpcName));
		UTF82Str(lpcName,siStreams[dwStream].lpcName);
//		lstrcpy(,);
	}
	return true;
}

int AVIFILEEX::GetStreamName(DWORD dwStream,char* lpcDest)
{
	if (dwStream>=GetNbrOfStreams()||(!lpcDest))
	{
		return AFE_INVALIDPARAM;
	}
	
	if (siStreams[dwStream].lpcName)
	{
		Str2UTF8(siStreams[dwStream].lpcName,lpcDest);
	}
	else
		*lpcDest=0;

	return true;
}

bool AVIFILEEX::ProcessSTRL(char* lpBuffer,DWORD dwLength,DWORD dwStreamNbr)
{
	CHUNKHEADER		chChunkHdr;
	DWORD			dwPos=0;
	DWORD			dwOldPos;
	char*			lpcOld;

	siStreams[dwStreamNbr].lpIndx=NULL;
	siStreams[dwStreamNbr].lpHeader=NULL;
	siStreams[dwStreamNbr].lpFormat=NULL;
//	siStreams[dwStreamNbr].ciChunks=NULL;

	// Stream header	
	
	if (!LocateData(MakeFourCC("strh"),&lpBuffer,&dwPos,&chChunkHdr,dwLength,DT_CHUNK))
	{
		DebugMsg ("    strh nicht gefunden !");
		return false;
	}
	DebugMsg("    strh gefunden");

	siStreams[dwStreamNbr].lpHeader=(AVIStreamHeader*)malloc(chChunkHdr.dwLength);
	memcpy(siStreams[dwStreamNbr].lpHeader,lpBuffer,chChunkHdr.dwLength);
	if (IsVideoStream(dwStreamNbr))
	{
		abs_pos.dwScaleSTRH0=20+DWORD(lpBuffer-dwHDRLBufferStart)+abs_pos.dwHDRL;
		abs_pos.dwRateSTRH0=24+DWORD(lpBuffer-dwHDRLBufferStart)+abs_pos.dwHDRL;
	}


	lpBuffer+=even(chChunkHdr.dwLength);
	dwPos+=even(chChunkHdr.dwLength);//+chChunkHdr.dwLength%2;

	// Stream format

	if (!LocateData(MakeFourCC("strf"),&lpBuffer,&dwPos,&chChunkHdr,dwLength-dwPos,DT_CHUNK))
	{
		DebugMsg("    strf nicht gefunden !");
		return false;
	}
	DebugMsg("    strf gefunden");

	siStreams[dwStreamNbr].lpFormat=malloc(chChunkHdr.dwLength);
	memcpy(siStreams[dwStreamNbr].lpFormat,lpBuffer,chChunkHdr.dwLength);
	lpBuffer+=even(chChunkHdr.dwLength); dwPos+=even(chChunkHdr.dwLength);

	// stream name

	lpcOld=lpBuffer;
	dwOldPos=dwPos;
	if (dwPos+8<dwLength)
	{
		if (LocateData(MakeFourCC("strn"),&lpBuffer,&dwPos,&chChunkHdr,dwLength-dwPos,DT_CHUNK))
		{
			lpBuffer[chChunkHdr.dwLength-1]=0;
			char name[256]; name[0] = 0;
			Str2UTF8(lpBuffer, name);
			SetStreamName(dwStreamNbr,name);
			lpBuffer+=even(chChunkHdr.dwLength);
			dwPos+=even(chChunkHdr.dwLength);
		}
		else
		{
			lpBuffer=lpcOld;
			dwPos=dwOldPos;
		}
	}


	if (dwPos+8<dwLength)
	{
		// kann indx-Chunk vorhanden sein ?
		if (!LocateData(MakeFourCC("indx"),&lpBuffer,&dwPos,&chChunkHdr,dwLength-dwPos,DT_CHUNK))
		{
			DebugMsg("    indx nicht gefunden");
		}
		else
		{
			lpBuffer-=8;
			DebugMsg("    indx gefunden");
			siStreams[dwStreamNbr].lpIndx=malloc(chChunkHdr.dwLength+8);
			memcpy(siStreams[dwStreamNbr].lpIndx,lpBuffer,chChunkHdr.dwLength+8);
			if (lpRSIP) lpRSIP[dwStreamNbr].qwFilePos=(DWORD)(lpBuffer-dwHDRLBufferStart)+abs_pos.dwHDRL;
			lpBuffer+=8;
			lpBuffer+=even(chChunkHdr.dwLength); dwPos+=even(chChunkHdr.dwLength);
			atType=AT_OPENDML;
		}
	}
	else
	{
		DebugMsg("    indx nicht gefunden");
	}


	return true;
}


bool AVIFILEEX::ProcessHeader(void)
{
	union
	{
		LISTHEADER	lhListHdr;
		CHUNKHEADER chChunkHdr;
	};
	char*	lpBuffer;
	DWORD	dwRead;
	DWORD	dwTotalSize;

	GetSource()->Seek(0);
	
	if (!CheckRIFF_AVI()) return false;

	LocateData(MakeFourCC("hdrl"), NULL, NULL, &lhListHdr, 16384, DT_LIST);

//	dwRead=GetSource()->Read(&lhListHdr,12);
	dwTotalSize = lhListHdr.dwLength;
	if ( !IsList(&lhListHdr,"hdrl"))
	{
		DebugMsg("hdrl-List nicht gefunden !");
		return false;
	}
	DebugMsg("hdrl-List gefunden ");

	if (bDummyMode) return true;

	abs_pos.dwHDRL=GetSource()->GetPos();//24;
	lpBuffer=(char*)malloc(lhListHdr.dwLength);
	dwHDRLBufferStart=(DWORD)lpBuffer;
	dwRead=GetSource()->Read(lpBuffer,lhListHdr.dwLength-4);

	if (!ProcessHDRL(lpBuffer,lhListHdr.dwLength-4))
	{
		DebugMsg("hdrl-List fehlerhaft !");
		return false;
	}
	DebugMsg("hdrl-List i.O.");

	int p = (int)GetSource()->GetPos();
	if (LocateData(MakeFourCC("INFO"), NULL, NULL, &lhListHdr, 16384, DT_LIST)) {
		ProcessINFO(lhListHdr.dwLength);
	}
	GetSource()->Seek(p);
	p = (int)GetSource()->GetPos();

	free(lpBuffer);
	return true;
}

bool AVIFILEEX::ProcessINFO(DWORD dwLength)
{
	CHUNKHEADER chChunkHdr; int i;
	__int64 pos = GetSource()->GetPos();

	if (dwLength>8) {
		__int64 q = GetSource()->GetPos();
		if (LocateData(MakeFourCC("ISFT"), NULL, NULL, &chChunkHdr, dwLength, DT_CHUNK)) {
			if (cWritingApp) delete cWritingApp;
			cWritingApp = new char[i=1+chChunkHdr.dwLength];
			ZeroMemory(cWritingApp, i);
			GetSource()->Read(cWritingApp, chChunkHdr.dwLength);
		}
		GetSource()->Seek(q);
		if (LocateData(MakeFourCC("INAM"), NULL, NULL, &chChunkHdr, dwLength, DT_CHUNK)) {
			if (cFileTitle) delete cFileTitle;
			cFileTitle = new char[i=1+chChunkHdr.dwLength];
			ZeroMemory(cFileTitle, i);
			GetSource()->Read(cFileTitle, chChunkHdr.dwLength);
		}

	}

	GetSource()->Seek(pos);
	return true;
}

char* AVIFILEEX::GetTitle()
{
	return (cFileTitle?cFileTitle:"");
}

bool AVIFILEEX::ProcessODML(char* lpBuffer,DWORD dwLength)
{
	CHUNKHEADER		chChunkHdr;

	if (!LocateData(MakeFourCC("dmlh"),&lpBuffer,NULL,&chChunkHdr,dwLength,DT_CHUNK))
	{
		DebugMsg("  dmlh nicht gefunden !");
		return false;
	}
	DebugMsg("  dmlh gefunden");
	lpExtAVIHeader=(ODMLExtendedAVIHeader*)malloc(chChunkHdr.dwLength);
	memcpy(lpExtAVIHeader,lpBuffer,chChunkHdr.dwLength);
	return true;
}

void AVIFILEEX::TryToRepairLargeChunks(bool bTry)
{
	bTryToRepairLargeChunks=bTry;
}

bool AVIFILEEX::ProcessIdx1(AVIINDEXENTRY* lpBuffer,DWORD dwCount)
{
	DWORD		i=0;
	char		Buffer[200];
	DWORD		dwS=0;
	DWORD		dwChunkID;
	STREAMINFO* lpsiStr;

	DWORD		dwAddVal=0;
	__int64	qwSourcePos=GetSource()->GetPos();

	for (i=0;i<dwCount;i++)
	{
		dwS=GetStreamNbrFromFourCC(lpBuffer[i].ckid);
		if (dwS!=0xffffffff) siStreams[dwS].dwChunkCount++;
	}
	ZeroMemory(Buffer,sizeof(Buffer));

	if (lpBuffer[0].dwChunkOffset==4) {
		dwAddVal=(DWORD)qwMoviPos-lpBuffer[0].dwChunkOffset;
	} else {
		dwAddVal=(DWORD)qwFirstDataChunkPos-lpBuffer[0].dwChunkOffset;
	}

	for (i=0;i<dwCount;i++)
	{
		dwS=GetStreamNbrFromFourCC(lpBuffer[i].ckid);
		if (dwS!=0xffffffff)
		{
			CHUNKINFO	chunk;

			lpsiStr=&(siStreams[dwS]);
			lpsiStr->dwPos++;

			chunk.dwLength = lpBuffer[i].dwChunkLength;

			if ((lpBuffer[i].dwChunkLength>=dwMaxAllowedChunkSize)&&(dwMaxAllowedChunkSize))
			{
				if (bTryToRepairLargeChunks)
				{
					GetSource()->Seek(lpBuffer[i].dwChunkOffset+dwAddVal);
					GetSource()->Read(&dwChunkID,4);
					if (GetStreamNbrFromFourCC(dwChunkID)==(int)dwS)
						GetSource()->Read(&chunk.dwLength, 4);
					
				}
				if ((lpBuffer[i].dwChunkLength>=dwMaxAllowedChunkSize)||(!dwMaxAllowedChunkSize))
					chunk.dwLength = 0;
				
			}
			chunk.qwPosition = lpBuffer[i].dwChunkOffset+dwAddVal;
			chunk.dwOffsetFieldB = 0;

			if (!chunk.dwLength) {
				chunk.ftFrameType = FT_DROPPEDFRAME;
			} else {
				if (lpBuffer[i].dwFlags&AVIIF_KEYFRAME) 
				{
					chunk.ftFrameType = FT_KEYFRAME;
				}
				else
					chunk.ftFrameType = FT_DELTAFRAME;
			}

			chunk.qwStreamPos = lpsiStr->qwStreamLength;
			lpsiStr->qwStreamLength += chunk.dwLength;
			if (IsVideoStream(dwS))
			{
				switch (chunk.ftFrameType)
				{
					case FT_KEYFRAME: frametypes.dwKey++; break;
					case FT_DELTAFRAME: frametypes.dwDelta++; break;
					case FT_DROPPEDFRAME: frametypes.dwDropped++; break;
				}
			}
			lpsiStr->chunks.push_back(chunk);
		}
	}
	GetSource()->Seek(qwSourcePos);
	dwRealFramesInRIFF=siStreams[0].dwChunkCount;
	return true;
}

void AVIFILEEX::SetDebugState(DWORD dwDebugState)
{
	bDebug= ((dwDebugState==DS_ACTIVATE)?true:false);
}

void AVIFILEEX::SetProcessMode(DWORD dwNbr,DWORD dwProcessMode)
{
	if (dwNbr=SPM_SETALL)
	{
		for (DWORD i=0;i<lpMainAVIHeader->dwStreams;siStreams[i++].dwProcessMode=dwProcessMode);
	}
	else
	{
		siStreams[dwNbr].dwProcessMode=dwProcessMode;
	}
	if (dwAccess==FA_READ)
	{
		for (int i=0;i<FrameCacheSize;i++)
		{
			FRAMEINFO* fiFrame=(FRAMEINFO*)FrameCache[i]->GetUserData(8);
			fiFrame->dwAccCount=0;
			fiFrame->iFrameNbr=-1;
		}
	}
}

DWORD AVIFILEEX::GetProcessMode(DWORD dwNbr)
{
	return (siStreams[dwNbr].dwProcessMode);
}

AVITYPE AVIFILEEX::GetAVIType(void)
{
	return atType;
}

DWORD AVIFILEEX::GetNbrOfFrames(DWORD dwType)
{
	DWORD	dwRes=0;

	if (dwType&FT_KEYFRAME) dwRes+=frametypes.dwKey;
	if (dwType&FT_DELTAFRAME) dwRes+=frametypes.dwDelta;
	if (dwType&FT_DROPPEDFRAME) dwRes+=frametypes.dwDropped;
	return dwRes;
}

bool AVIFILEEX::DebugMsg(char* lpMsg)
{
	DWORD  dwLength = lstrlen(lpMsg);
	DWORD  dwWritten;
	WORD   wLineFeed = 13+(10<<8);

	if (bDebug)
	{
		WriteFile(hDebugFile,lpMsg,dwLength,&dwWritten,NULL);
	    WriteFile(hDebugFile,&wLineFeed,2,&dwWritten,NULL);
	}
	return true;
}

int AVIFILEEX::GetStreamNbrFromFourCC(DWORD dwFourCC)
{
	int		Digits[4];
	
	if (dwFourCC == 0) return -1;

	for (int i=0;i<4;i++)
	{
		Digits[i]=(dwFourCC)&(0xFF)-48;
		dwFourCC>>=8;
	}

	if ((Digits[0]/10==0)&&(Digits[1]/10==0))
	{
		return (Digits[1]+10*Digits[0]);
	}
	else
	if ((Digits[2]/10==0)&&(Digits[3]/10==0))
	{
		return (Digits[3]+10*Digits[2]);
	}
	else
		return -1;


	return 0;
}

bool AVIFILEEX::CheckIndxCount(void)
{
	DWORD		dwIndxCount=0;

	for (DWORD i=0;i<lpMainAVIHeader->dwStreams;dwIndxCount+=(siStreams[i++].lpIndx)?1:0);

	return ((dwIndxCount==0)||(dwIndxCount==lpMainAVIHeader->dwStreams));
}

bool AVIFILEEX::ProcessBaseIndx(_aviindex_chunk* lpIndx, DWORD dwProcessMode,void* lpData)
{
	AVISTDINDEX*	lpStdIndx=(AVISTDINDEX*)lpIndx;
	AVIFIELDINDEX*	lpFldIndx=(AVIFIELDINDEX*)lpIndx;
	AVISUPERINDEX*	lpSupIndx=(AVISUPERINDEX*)lpIndx;
	void*			lpSubIndex;
	DWORD			i=0;
	_int64			qwFilePos;
	DWORD			dwRead;
	DWORD			dwChunkID;
	DWORD			dwS=0;
	STREAMINFO*		lpsiStr;

	CHUNKHEADER		chChunkHdr;
	lpRSIP=(READSUPERINDEXPROTOCOL*)lpData;
	READSUPERINDEXPROTOCOL*	lpTemp;
	DWORD*			lpdwEntries=(DWORD*)lpData;
	WAVEFORMATEX*	lpwfe;
	DWORD			dwBaseOffset=(DWORD)(lpIndx-dwHDRLBufferStart);
	CHUNKINFO		chunk;

	
	qwFilePos=GetSource()->GetPos();

	if (lpIndx) if (lpIndx->bIndexType==AVI_INDEX_OF_INDEXES)
	{
		if (dwProcessMode==PI_PROCESSINDEX)
		{
			lpRSIP->dwEntries=lpIndx->nEntriesInUse;
			lpRSIP->rsipEntries=(READSUPERINDEXPROTOCOLENTRY*)malloc(lpRSIP->dwEntries*sizeof(READSUPERINDEXPROTOCOLENTRY));
			ZeroMemory(lpRSIP->rsipEntries,lpRSIP->dwEntries*sizeof(READSUPERINDEXPROTOCOLENTRY));
		}
	}

	if (lpIndx) for (i=0;i<lpIndx->nEntriesInUse;i++)
	{
		switch (lpIndx->bIndexType)
		{
			case AVI_INDEX_OF_INDEXES:

				GetSource()->Seek(lpSupIndx->aIndex[i].qwOffset);
				dwRead=GetSource()->Read(&chChunkHdr,8);
				ASSERT (dwRead == 8);
				if (chChunkHdr.dwLength)
				{
					lpSubIndex=malloc(chChunkHdr.dwLength+8);
					ASSERT(lpSubIndex);
					GetSource()->Seek(lpSupIndx->aIndex[i].qwOffset);
					dwRead=GetSource()->Read(lpSubIndex,chChunkHdr.dwLength+8);
					if (lpRSIP)
					{
						lpRSIP->rsipEntries[i].dwDurationValue=lpSupIndx->aIndex[i].dwDuration;
						lpRSIP->rsipEntries[i].qwFilePos=lpRSIP->qwFilePos+44+16*i;
					}					

					lpTemp=lpRSIP;
					ProcessBaseIndx((_aviindex_chunk*)lpSubIndex,dwProcessMode,(lpRSIP)?&(lpRSIP->rsipEntries[i].dwRealDuration):NULL);
					lpRSIP=lpTemp;
					free(lpSubIndex);
				}
				break;
			case AVI_INDEX_OF_CHUNKS:
				dwS=GetStreamNbrFromFourCC(lpStdIndx->dwChunkId);
				if (dwS!=0xffffffff)
				{
					lpsiStr=&(siStreams[dwS]);
					if (dwProcessMode==PI_COUNTCHUNKS)
					{
						lpsiStr->dwChunkCount++;					
					}
					else
					{
						lpsiStr->dwPos++;
						lpsiStr->dwChunkCount++;					
						{
							if (lpIndx->bIndexSubType!=AVI_INDEX_2FIELD) {
								chunk.dwLength = lpStdIndx->aIndex[i].dwSize;
								chunk.dwOffsetFieldB = 0;
								chunk.qwPosition = lpStdIndx->qwBaseOffset+lpStdIndx->aIndex[i].dwOffset-8;
							} else {
								chunk.dwLength = lpFldIndx->aIndex[i].dwSize;
								chunk.dwOffsetFieldB = lpFldIndx->aIndex[i].dwOffsetField2-lpFldIndx->aIndex[i].dwOffset;
								chunk.qwPosition = lpFldIndx->aIndex[i].dwOffset+lpFldIndx->qwBaseOffset-8;
							}
							if ((chunk.qwPosition<(__int64)dwRIFFSize)&&(IsVideoStream(dwS))) dwRealFramesInRIFF++;
							if (((chunk.dwLength & 0x7FFFFFFF)>dwMaxAllowedChunkSize)&&(dwMaxAllowedChunkSize))
							{
								if (bTryToRepairLargeChunks)
								{
									GetSource()->Seek(chunk.qwPosition);
									GetSource()->Read(&dwChunkID,4);
									if (GetStreamNbrFromFourCC(dwChunkID)==(int)dwS)
									{
										GetSource()->Read(&chunk.dwLength, 4);
										if (IsVideoStream(dwS))
											chunk.dwLength |= (1<<31);
									}
								}
								if (((chunk.dwLength & 0x7FFFFFFF)>dwMaxAllowedChunkSize)&&(dwMaxAllowedChunkSize))
									chunk.dwLength = 0;
								
							}
							chunk.ftFrameType = (chunk.dwLength&(1<<31))?(((chunk.dwLength)&0x7FFFFFFF)?FT_DELTAFRAME:FT_DROPPEDFRAME):(chunk.dwLength)?FT_KEYFRAME:FT_DROPPEDFRAME;
							chunk.dwLength &= 0x7FFFFFFF;
							chunk.qwStreamPos = lpsiStr->qwStreamLength;
							lpsiStr->qwStreamLength += chunk.dwLength;

							if (IsVideoStream(dwS))
							{
								if (lpdwEntries) *lpdwEntries=*lpdwEntries+1;
								switch (chunk.ftFrameType)
								{
									case FT_KEYFRAME: frametypes.dwKey++; break;
									case FT_DELTAFRAME: frametypes.dwDelta++; break;
									case FT_DROPPEDFRAME: frametypes.dwDropped++; break;
								}
							}
							else
							if (IsAudioStream(dwS))
							{
								lpwfe=(WAVEFORMATEX*)siStreams[dwS].lpFormat;
								if ((IsMP3SampleCount(lpwfe->nBlockAlign))&&(lpwfe->wFormatTag==0x55) || (lpwfe->wFormatTag==0xFF))
								{	// MP3 VBR
									if (lpdwEntries) *lpdwEntries=*lpdwEntries+1;
								}
								else
								{
									if (lpdwEntries) *lpdwEntries=*lpdwEntries+(DWORD)QW_div((chunk.dwLength),siStreams[dwS].lpHeader->dwScale,
										"AVIFILEEX::ProcessBadeIndex::siStreams[dwS].lpHeader->dwScale");//(lpciCh->dwLength)/siStreams[dwS].lpHeader->dwScale;
								}
							}
							else
							if (IsTextStream(dwS))
							{
								*lpdwEntries=1;
							}
						}
						lpsiStr->chunks.push_back(chunk);
					}
				}
				break;
			case AVI_INDEX_IS_DATA: break; // time-code etc. nicht unterst¸tzt !
			default:
				return false;
		}
		
	}
	GetSource()->Seek(qwFilePos);
	return true;
}

bool AVIFILEEX::ProcessExtIndex(_aviindex_chunk* lpIndx,DWORD dwStreamNbr,READSUPERINDEXPROTOCOL* lpRSIP)
{
	return ProcessBaseIndx(lpIndx,PI_PROCESSINDEX,lpRSIP);
}

READSUPERINDEXPROTOCOL*	AVIFILEEX::GetLoadSuperIndexProtocol(void)
{
	if ((dwAccess!=FA_READ)||(GetAVIType()!=AT_OPENDML)) return NULL;
	return lpRSIP;
}

void AVIFILEEX::SetFieldMode(DWORD fmNewMode)
{
	fmFieldMode = fmNewMode;
}

void AVIFILEEX::GetFramesInFirstRIFF(DWORD* lpdwHeaderValue,DWORD* lpdwTrueValue)
{
	if (lpdwHeaderValue) *lpdwHeaderValue=lpMainAVIHeader->dwTotalFrames;
	if (lpdwTrueValue) *lpdwTrueValue=dwRealFramesInRIFF;
	return;
}

DWORD AVIFILEEX::GetFrameCount(void)
{
	DWORD dwNbr;

	dwNbr=siStreams[video_stream_index].dwChunkCount;

	return ((fmFieldMode==FM_NONE)?dwNbr:(fmFieldMode&(FM_DISCARD_FIRST|FM_DISCARD_SECOND))?dwNbr:2*dwNbr);
}

DWORD AVIFILEEX::GetMicroSecPerFrame(void)
{
	return (DWORD)round((double)GetNanoSecPerFrame()/1000);
}

__int64 AVIFILEEX::GetNanoSecPerFrame(void)
{
	__int64	qwOrgNSPF;

	if (qwNSPF)
	{
		qwOrgNSPF=qwNSPF;
	}
	else
	if (siStreams)
	{
		qwOrgNSPF=round(1000000000/d_div((double)siStreams[0].lpHeader->dwRate,(double)siStreams[0].lpHeader->dwScale,
			"AVIFILEEX::GetNanoSecPerFrame::siStreams[0].lpHeader->dwScale"));//((double)siStreams[0].lpHeader->dwRate/(double)siStreams[0].lpHeader->dwScale));
	}
	else
	{		
		qwOrgNSPF=lpMainAVIHeader->dwMicroSecPerFrame*1000;
	}

	return ((fmFieldMode==FM_NONE)?qwOrgNSPF:(fmFieldMode&(FM_DISCARD_FIRST|FM_DISCARD_SECOND))?qwOrgNSPF:qwOrgNSPF/2);
}

int AVIFILEEX::GetAudioStreamCount()
{
	int i = 0;
	int j = 0;
	while (j<(int)GetNbrOfStreams()) if (IsAudioStream(j++)) i++;

	return i++;
}

/*bool AVIFILEEX::DecompressBeginVideo(BITMAPINFOHEADER* lpFormat)
{
	bool	bResult;
	BITMAPINFOHEADER* lpbiOut;
	int		i;

	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;

	if (lpFormat)
	{
		memcpy(siStreams[0].lpOutputFormat,lpFormat,sizeof(BITMAPINFOHEADER));
		for (i=0;i<FrameCacheSize;i++)
		{
 			FrameCache[i]->SetBitDepth(lpFormat->biBitCount);
			if (lpFormat->biCompression==BI_RGB)
			{
				FrameCache[i]->SetColorSpace(CS_RGB);
			}
			if (lpFormat->biCompression==MakeFourCC("YUY2"))
			{
				FrameCache[i]->SetColorSpace(CS_PACKEDYUV);
			}
			FrameCache[i]->Realloc();
		}
	}
	else
	{
		memcpy(siStreams[0].lpOutputFormat,&bmi24bpp,sizeof(BITMAPINFOHEADER));
		for (i=0;i<FrameCacheSize;i++)
		{
 			FrameCache[i]->SetBitDepth(24);
			FrameCache[i]->SetColorSpace(CS_RGB);
			FrameCache[i]->Realloc();
		}
	}

	lpbiOut=(BITMAPINFOHEADER*)siStreams[0].lpOutputFormat;

	DebugMsg("DecompressBegin");
	bResult=SUCCEEDED(ICDecompressBegin(hic,siStreams[0].lpFormat,lpbiOut));
	switch (bResult)
	{
		case true: DebugMsg("  erfolgreich"); break;
		case false:	DebugMsg ("  fehlgeschlagen"); break;
	}
	return bResult;
}

bool AVIFILEEX::DecompressEndVideo(void)
{
	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;

	DebugMsg("DecompressEnd");
	return (SUCCEEDED(ICDecompressEnd(hic)));
}
*/
// L‰dt Videochunk OHNE TranslateChunkNumber auszurufen!
int AVIFILEEX::LoadVideoChunk(DWORD dwChunkNbr,DWORD* lpdwSize)
{
	void*		lpBuffer;
	BITMAPINFOHEADER 
				*lpbiIn,*lpbiOut;
	DWORD		dwLength;
	int			iResult;
	DWORD		dwRead;
//	DWORD		dwFlags;
	DWORD		FrameType;
//	FRAMEINFO*	fiFrame;
	FRAME*		fDest;
	bool		bResult;
//	char		Msg[1000];

	DebugMsg ("LoadVideoChunk: ");
	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;

	if (dwChunkNbr>=siStreams[0].dwChunkCount)
	{
		return AFE_ENDOFSTREAM;
	}

//	GetSource()->Seek(siStreams[0].ciChunks[dwChunkNbr].qwPosition+8);
	STREAMINFO* str = &siStreams[video_stream_index];
	CHUNKINFO chunk = str->chunks[dwChunkNbr];
	//dwLength=siStreams[0].ciChunks[dwChunkNbr].dwLength;
	GetSource()->Seek(chunk.qwPosition + 8);
	dwLength = chunk.dwLength;

	if (lpdwSize) 
		*lpdwSize=dwLength;

	lpBuffer=(dwLength)?malloc(dwLength*2):malloc(1024);
	if (dwLength) {
		dwRead=GetSource()->Read(lpBuffer,dwLength);
		if (!dwRead)
			return AFE_CANTREADFROMSOURCE;
		
	}

	/*ZeroMemory(&Msg,sizeof(Msg));
	wsprintf(Msg,"  Frame %d (%d Bytes) wurde gelesen @ %d kByte",dwChunkNbr,dwLength,
		(DWORD)(siStreams[0].ciChunks[dwChunkNbr].qwPosition/1024));
	DebugMsg(Msg); */

	//switch (FrameType=siStreams[0].ciChunks[dwChunkNbr].ftFrameType)
	switch (FrameType = chunk.ftFrameType) {
		case FT_DROPPEDFRAME: 
			DebugMsg ("  -> enth‰lt keine Daten"); 
			break;
		case FT_DELTAFRAME:
			DebugMsg ("  -> ist Deltaframe");
			break;
		case FT_KEYFRAME:
			DebugMsg ("  -> ist Keyframe");
			break;
	}

	lpbiIn=(BITMAPINFOHEADER*)siStreams[0].lpFormat;
	lpbiOut=(BITMAPINFOHEADER*)siStreams[0].lpOutputFormat;
    
	if (siStreams[0].dwProcessMode==PM_PROCESS)
	{
/*		if ((FrameType)!=FT_DROPPEDFRAME)
		{
			fDest=FrameCache[iResult=dwFrameCachePos++];
			dwFrameCachePos%=FrameCacheSize;
			dwFlags=(FrameType==FT_DELTAFRAME)?ICDECOMPRESS_NOTKEYFRAME:0;
			if (fDest->IsExternalBuffer()) fDest->UseInternalBuffer();
			if (!(fDest->GetBuffer(0))) DebugMsg("  Outputbuffer ung¸ltig");
			fiFrame=((FRAMEINFO*)fDest->GetUserData(sizeof(FRAMEINFO)));
			fiFrame->dwAccCount=0;
			DebugMsg ("  versuche zu dekomprimieren...");
			bResult=(!FAILED(ICDecompress(hic,dwFlags,lpbiIn,lpBuffer,lpbiOut,fDest->GetBuffer(0))));
			switch (bResult)
			{
				// Wenn das hier nicht mehr ausgef¸hrt wurde, gabs einen
				// Ausnahmefehler bei ICDecompress....
				case true: 
					DebugMsg("  Dekompression erfolgreich");	
					fiFrame->iFrameNbr=dwChunkNbr;
					fiFrame->dwAccCount++;
					break;
				case false:DebugMsg("  Dekompression fehlgeschlagen"); break;
			}
		}
		else
		{
			iResult=(dwFrameCachePos-1);
			iResult+=(iResult<0)?FrameCacheSize:0;
			bResult=true;
		}*/
	}
	else
	{
		fDest=FrameCache[iResult=dwFrameCachePos++];
		dwFrameCachePos%=FrameCacheSize;
		fDest->UseExternalBuffer(malloc(dwLength),dwLength);
		memcpy(fDest->GetBuffer(0),lpBuffer,dwLength);
		bResult=true;
	}
	free(lpBuffer);
	return ((bResult)?iResult:(-1));
}

int AVIFILEEX::TranslateChunkNumber(DWORD dwStreamNbr,DWORD dwChunkNbr)
{
	int	iRes;

	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;
	
	switch (dwChunkNbr)
	{
		case CN_CURRENT_CHUNK:
			return TranslateChunkNumber(dwStreamNbr,siStreams[dwStreamNbr].dwPos);
			break;
		case CN_NEXT_CHUNK:
			return TranslateChunkNumber(dwStreamNbr,siStreams[dwStreamNbr].dwPos+1);
			break;
		case CN_PREV_CHUNK:
			return TranslateChunkNumber(dwStreamNbr,siStreams[dwStreamNbr].dwPos-1);
			break;
		default:
			if (dwStreamNbr)
			{
				return dwChunkNbr;
			}
			else
			{
					return dwChunkNbr;
			}
			break;
	}

	return iRes;
}


// ruft TranslateChunkNumber auf
int AVIFILEEX::GetVideoChunk(DWORD dwChunkNbr,void* lpDest,DWORD* lpdwSize)
{
	int    dwIndex=0;
	char   Buffer[100];
	DWORD  dwStream = GetVideoStreamNumber();

	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;

	if (dwChunkNbr==CN_NEXT_CHUNK)
	{
		DebugMsg("n‰chste Frame...");
		return GetVideoChunk(siStreams[dwStream].dwPos++,lpDest,lpdwSize);
	}

	wsprintf(Buffer,"GetVideoChunk: dwChunkNbr = %d",dwChunkNbr);
	DebugMsg(Buffer);

	dwIndex=LoadVideoChunk(TranslateChunkNumber(dwStream,dwChunkNbr),lpdwSize);
	if (dwIndex==AFE_CANTREADFROMSOURCE)
	{
		return AFE_CANTREADFROMSOURCE;
	}
	if (lpDest)
	{
		FrameCache[dwIndex]->GetBuffer(0);
		memcpy(lpDest,FrameCache[dwIndex]->GetBuffer(0),FrameCache[dwIndex]->GetBufferSize());
	}
	
	return AFE_OK;
}

int AVIFILEEX::GetAudioChunk(DWORD dwStreamNbr,DWORD dwChunkNbr,void* lpDest)
{
	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;

	STREAMINFO*		siStr=&(siStreams[dwStreamNbr]);
	if (dwChunkNbr==CN_NEXT_CHUNK)
	{
		return GetAudioChunk (dwStreamNbr,siStr->dwPos++,lpDest);
	}
	else
	{
		siStr->dwPos=dwChunkNbr;
	}
	siStr->dwOffset=0;
 
//	return LoadAudioData(dwStreamNbr,siStr->ciChunks[dwChunkNbr].dwLength,lpDest); 
	return LoadAudioData(dwStreamNbr,siStr->chunks[dwChunkNbr].dwLength,lpDest); 
}

DWORD AVIFILEEX::FindKeyFrame(DWORD dwFrameNbr)
{
	DWORD	dwResult;

	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;

	dwResult=dwFrameNbr;
	while (!IsKeyFrame(dwResult) && dwResult)
	{
		dwResult--;
	}

	return dwResult;
}

// ruft TranslateChunkNumber auf
bool AVIFILEEX::IsKeyFrame(DWORD dwChunkNbr)
{
	DWORD	dwFrame=TranslateChunkNumber(0,dwChunkNbr);

	if (dwFrame>=siStreams[0].dwChunkCount) {
		return true;
	} else {
//		return (siStreams[0].ciChunks[dwFrame].ftFrameType==FT_KEYFRAME)?true:false;
		return (siStreams[0].chunks[dwFrame].ftFrameType==FT_KEYFRAME)?true:false;
	}
}

int AVIFILEEX::SeekVideoStream(DWORD dwFrameNbr)
{
	DWORD	dwLastKF;

	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;

	dwLastKF=FindKeyFrame(dwFrameNbr);
    siStreams[GetVideoStreamNumber()].dwPos=dwLastKF;

    for (DWORD i=dwLastKF;i++<dwFrameNbr;GetVideoChunk(CN_NEXT_CHUNK,NULL));
	
	return AFE_OK;
}

bool AVIFILEEX::IsAudioStream(DWORD dwStreamNbr)
{
	if (dwStreamNbr<lpMainAVIHeader->dwStreams)
	{
		if (siStreams[dwStreamNbr].lpHeader->fccType==MakeFourCC("auds"))
		{
			return true;		
		}
	}
	return false;
}

bool AVIFILEEX::IsVideoStream(DWORD dwStreamNbr)
{
	if (dwStreamNbr<lpMainAVIHeader->dwStreams)
	{
		if (siStreams[dwStreamNbr].lpHeader->fccType==MakeFourCC("vids"))
		{
			return true;		
		}
	}
	return false;
}

bool AVIFILEEX::IsTextStream(DWORD dwStreamNbr)
{
	if (dwStreamNbr<lpMainAVIHeader->dwStreams)
	{
		if (siStreams[dwStreamNbr].lpHeader->fccType==MakeFourCC("txts"))
		{
			return true;		
		}
	}
	return false;
}

bool AVIFILEEX::IsDefault(DWORD dwStreamNbr)
{
	return !(siStreams[dwStreamNbr].lpHeader->dwFlags & AVISF_DISABLED);
}

DWORD AVIFILEEX::GetKindOfStream(DWORD dwStreamNbr)
{
	if (IsVideoStream(dwStreamNbr))
	{
		return AVI_VIDEOSTREAM;
	}
	else
	if (IsAudioStream(dwStreamNbr))
	{ 
		return AVI_AUDIOSTREAM;
	}
	else
	if (IsTextStream(dwStreamNbr))
	{
		return AVI_TEXTSTREAM;
	}
	else return 0;
}
/*
DWORD AVIFILEEX::DecompressBeginAudio(DWORD dwStreamNbr,DWORD dwMilliSec)
{
	WAVEFORMATEX*	lpwfeIn=(WAVEFORMATEX*)siStreams[dwStreamNbr].lpFormat;
	WAVEFORMATEX*	lpwfeOut=(WAVEFORMATEX*)siStreams[dwStreamNbr].lpOutputFormat;
	STREAMINFO*		siStr=&(siStreams[dwStreamNbr]);
	DWORD			dwInLength,dwOutLength;

	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;

	if (!IsAudioStream(dwStreamNbr)) return AFE_INVALIDPARAM;

	if ((siStr->bCompressed)&&(siStr->dwProcessMode==PM_PROCESS))
	{
		if (FAILED(acmStreamOpen(&siStreams[dwStreamNbr].has,NULL,lpwfeIn,lpwfeOut,NULL,NULL,NULL,
			0))) return -1;
		siStr->lpBufIn=malloc(dwInLength=lpwfeIn->nAvgBytesPerSec*dwMilliSec/1000);
		siStr->lpBufOut=malloc(dwOutLength=lpwfeOut->nAvgBytesPerSec*dwMilliSec/1000);
		ZeroMemory(&(siStr->hash),sizeof(siStr->hash));
		siStr->hash.cbStruct=sizeof(siStr->hash);
		siStr->hash.fdwStatus=0;
		siStr->hash.dwUser=0;
		siStr->hash.dwSrcUser=dwInLength;
		siStr->hash.dwDstUser=dwOutLength;
		siStr->hash.pbSrc=(BYTE*)siStr->lpBufIn;
		siStr->hash.cbSrcLength=dwInLength;
		siStr->hash.pbDst=(BYTE*)siStr->lpBufOut;
		siStr->hash.cbDstLength=dwOutLength;
		if (FAILED(acmStreamPrepareHeader(siStr->has,&(siStr->hash),0))) return -1;
	}
	
	return 1;
}

DWORD AVIFILEEX::DecompressEndAudio(DWORD dwStreamNbr)
{
	STREAMINFO*		siStr=&(siStreams[dwStreamNbr]);
	
	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;

	if ((siStr->bCompressed)&&(siStr->dwProcessMode==PM_PROCESS))
	{
		if (!IsAudioStream(dwStreamNbr)) return AFE_INVALIDPARAM;
		siStr->hash.cbSrcLength=siStr->hash.dwSrcUser;
		siStr->hash.cbDstLength=siStr->hash.dwDstUser;
		acmStreamUnprepareHeader(siStr->has,&(siStr->hash),0);
		acmStreamClose(siStreams[dwStreamNbr].has,0);
	}

	return 1;
}
*/
int AVIFILEEX::SeekByteStream(DWORD dwStreamNbr,_int64 qwPos)
{
	STREAMINFO* siStr;
//	CHUNKINFO*	ciCh;
	CHUNKINFO	chunk;
	int			dwLo,dwHi,dwMiddle;
	DWORD		dwChunkNbr=0;

	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;
	siStr=&(siStreams[dwStreamNbr]);
	if (siStr->qwStreamLength<qwPos) return AFE_ENDOFSTREAM;
/*	if (siStr->qwStreamLength<qwPos+1000)
	{
//		Beep(100,100);
	}
*/
	dwLo=0;
	dwHi=siStr->dwChunkCount-1;
	

// binary search

	while (abs(dwHi-dwLo)>1)
	{
		dwMiddle=(dwLo+dwHi)/2;
//		if ((siStr->ciChunks[dwLo].qwStreamPos<=qwPos)&&(siStr->ciChunks[dwMiddle].qwStreamPos>qwPos))
		if ((siStr->chunks[dwLo].qwStreamPos<=qwPos)&&(siStr->chunks[dwMiddle].qwStreamPos>qwPos)) {
			dwHi=dwMiddle;
		} else {
			dwLo=dwMiddle;
		}
	}
	dwChunkNbr=dwLo;
	//ciCh=&(siStr->ciChunks[dwChunkNbr]);
	chunk = siStr->chunks[dwChunkNbr];

	siStr->dwPos=dwChunkNbr;
//	siStr->dwOffset=(DWORD)(qwPos-ciCh->qwStreamPos);
	siStr->dwOffset=(DWORD)(qwPos-chunk.qwStreamPos);

//	while ((siStr->dwOffset>=ciCh->dwLength)&&(siStr->dwPos+1<siStr->dwChunkCount))
	while ((siStr->dwOffset>=chunk.dwLength)&&(siStr->dwPos+1<siStr->dwChunkCount))
	{
//		siStr->dwOffset-=ciCh->dwLength;
		siStr->dwOffset -= chunk.dwLength;
		siStr->dwPos++;
//		ciCh=&(siStr->ciChunks[siStr->dwPos]);
		chunk = siStr->chunks[siStr->dwPos]; 
	}

	return AFE_OK;
}

int AVIFILEEX::GetChannels(DWORD dwStreamNbr)
{
	WAVEFORMATEX*	strfAudio;
	STREAMINFO* siStr;

	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;

	if ((!IsAudioStream(dwStreamNbr))) return AFE_INVALIDPARAM;
	siStr=&(siStreams[dwStreamNbr]);
	strfAudio=(WAVEFORMATEX*)siStr->lpFormat;

	return strfAudio->nChannels;
}


int AVIFILEEX::SeekAudioStream(DWORD dwStreamNbr,_int64 qwPos,DWORD dwFlags)
{
	DWORD		dwChunkNbr=0;
	STREAMINFO* siStr;
	DWORD		dwBytesPerSec;
	WAVEFORMATEX*	strfAudio;

	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;

	if ((!IsAudioStream(dwStreamNbr))) return AFE_INVALIDPARAM;
	siStr=&(siStreams[dwStreamNbr]);
	strfAudio=(WAVEFORMATEX*)siStr->lpFormat;
	
	if (dwFlags==SAS_BYTES)
	{
		return SeekByteStream(dwStreamNbr,qwPos);
	}
	if (dwFlags==SAS_MILLISEC)
	{
		dwBytesPerSec=((WAVEFORMATEX*)(siStr->lpFormat))->nAvgBytesPerSec;

		return SeekAudioStream(dwStreamNbr,QW_div(qwPos*dwBytesPerSec/1000,strfAudio->nBlockAlign,
			"AVIFILEEX::SeekAudioStream::strfAudio->nBlockAlign")/*(qwPos*dwBytesPerSec/1000)/(strfAudio->nBlockAlign)*/*strfAudio->nBlockAlign,SAS_BYTES);
	}
	if (dwFlags==SAS_VIDEOFRAMES)
	{
		return SeekAudioStream(dwStreamNbr,qwPos*GetMicroSecPerFrame()/1000,SAS_MILLISEC);
	}
	if (dwFlags==SAS_VIDEOPOS)
	{
		return SeekAudioStream(dwStreamNbr,siStreams[0].dwPos,SAS_VIDEOFRAMES);
	}

	return AFE_OK;
}

_int64 AVIFILEEX::GetByteStreamPos(DWORD dwStreamNbr)
{
	if ((!IsAudioStream(dwStreamNbr))&&(!IsTextStream(dwStreamNbr))) return AFE_INVALIDPARAM;

	STREAMINFO*		siStr=&(siStreams[dwStreamNbr]);

//	return (siStr->ciChunks[siStr->dwPos].qwStreamPos+siStr->dwOffset);
	return (siStr->chunks[siStr->dwPos].qwStreamPos + siStr->dwOffset);
}

int AVIFILEEX::LoadPartialChunk(DWORD dwStreamNbr,DWORD dwLength,void* lpDest)
{
	DWORD	dwRead;

	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;

	STREAMINFO*	siStr=&(siStreams[dwStreamNbr]);;
	if (siStr->dwPos>=siStr->dwChunkCount) return 0;
	CHUNKINFO	chunk = siStr->chunks[siStr->dwPos];

	DWORD		l1=chunk.dwLength-siStr->dwOffset; 
	DWORD		dwBytesToRead=(l1<dwLength)?l1:dwLength;

	GetSource()->Seek(chunk.qwPosition+8+siStr->dwOffset);
	dwRead=GetSource()->Read(lpDest,dwBytesToRead);

	return dwRead;
}

DWORD AVIFILEEX::LoadAudioData(DWORD dwStreamNbr,DWORD dwSize,void* lpDest)
{
	BYTE*	lpbDest=(BYTE*)(lpDest);
	DWORD	dwDestPos=0;
	DWORD	dwLastRead=1;

	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;

	if (!lpDest) return AFE_INVALIDPARAM;
	while ((dwDestPos<dwSize)&&(dwLastRead))
	{
		if (dwLastRead=LoadPartialChunk(dwStreamNbr,dwSize-dwDestPos,&(lpbDest[dwDestPos])))
		{
			dwDestPos+=dwLastRead;
			SeekByteStream(dwStreamNbr,GetByteStreamPos(dwStreamNbr)+dwLastRead);
		}
	}
	
	return dwDestPos;
}

DWORD AVIFILEEX::GetAudioData(DWORD dwStreamNbr,DWORD dwMilliSec,void* lpDest)
{
	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;

	if (!IsAudioStream(dwStreamNbr)) return AFE_INVALIDPARAM;
	
	STREAMINFO* siStr=&(siStreams[dwStreamNbr]);
	DWORD	dwSize = ((WAVEFORMATEX*)siStr->lpFormat)->nAvgBytesPerSec*dwMilliSec/1000;
	DWORD	dwRead, dwBlockSize;
//	_int64  qwOldPos;
	char	Buffer[200];

	dwBlockSize=((WAVEFORMATEX*)siStr->lpFormat)->nBlockAlign;
	dwSize/=dwBlockSize;
	if (!dwSize) dwSize++;
	dwSize*=dwBlockSize;

	if ((siStreams[dwStreamNbr].dwProcessMode==PM_DIRECTSTREAMCOPY)||(!siStr->bCompressed))
	{
		dwRead=LoadAudioData(dwStreamNbr,dwSize,lpDest);
		wsprintf(Buffer,"Stream %d: %d Bytes kopiert",dwStreamNbr,dwRead);
		DebugMsg(Buffer);
		return dwRead;
	}
	else
	{
/*		qwOldPos=GetByteStreamPos(dwStreamNbr);
		dwRead=LoadAudioData(dwStreamNbr,dwSize,siStr->hash.pbSrc);
		if (dwRead>0)
		{
			siStr->hash.cbSrcLength=dwRead;
			if (FAILED(acmStreamConvert(siStr->has,&(siStr->hash),ACM_STREAMCONVERTF_BLOCKALIGN))) return 0;
			SeekAudioStream(dwStreamNbr,qwOldPos+siStr->hash.cbSrcLengthUsed,SAS_BYTES);
			wsprintf(Buffer,"Stream %d: %d Bytes geladen",dwStreamNbr,siStr->hash.cbSrcLengthUsed);
			DebugMsg(Buffer);
			memcpy(lpDest,siStr->hash.pbDst,dwUsed=siStr->hash.cbDstLengthUsed);
			return dwUsed;
		}*/
	}
	return 0;
}

bool AVIFILEEX::IsEndOfStream(DWORD dwStreamNbr)
{
	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;

	if (dwStreamNbr>=lpMainAVIHeader->dwStreams) return false;

	if (IsVideoStream(dwStreamNbr))
	{
		return (siStreams[dwStreamNbr].dwPos>=siStreams[dwStreamNbr].dwChunkCount);
	}
	if (IsAudioStream(dwStreamNbr))
	{
		if (siStreams[dwStreamNbr].dwPos>=siStreams[dwStreamNbr].dwChunkCount) return true;
		return (GetByteStreamPos(dwStreamNbr)>=siStreams[dwStreamNbr].qwStreamLength);
	}
	return false;
}

int AVIFILEEX::GetVideoResolution(int *lpWidth,int* lpHeight)
{
	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;
	
	BITMAPINFOHEADER*	lpStr=(BITMAPINFOHEADER*)(siStreams[0].lpFormat);

	if (lpWidth) *lpWidth=(int)lpStr->biWidth;
	if (lpHeight) *lpHeight=(int)lpStr->biHeight;

	return AFE_OK;
}

_int64 AVIFILEEX::GetFilePosOfChunk(DWORD dwStreamNbr,DWORD dwChunkNbr)
{
	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;
	if (dwStreamNbr>=lpMainAVIHeader->dwStreams) return AFE_INVALIDPARAM;
	if (dwChunkNbr>=siStreams[dwStreamNbr].dwChunkCount) return AFE_INVALIDPARAM;

//	return (siStreams[dwStreamNbr].ciChunks[dwChunkNbr].qwPosition);
	return (siStreams[dwStreamNbr].chunks[dwChunkNbr].qwPosition);
}

_int64 AVIFILEEX::GetFileSize(void)
{
	_int64	qwSize=0;

	if (dwAccess==FA_READ)
	{
		qwSize=GetSource()->GetSize();
		return (qwSize);
	}
	if (dwAccess==FA_WRITE)
	{
		return qwFilePos;
	}
	return 0;
}

int AVIFILEEX::VBR_FrameCountInChunk(DWORD dwStream, DWORD dwChunk)
{
	int  nsize = VBR_MaxFrameSize(dwStream);
//	int  chsize = siStreams[dwStream].ciChunks[dwChunk].dwLength;
	int  chsize = siStreams[dwStream].chunks[dwChunk].dwLength;

	return (chsize / nsize) + (!!(chsize%nsize));
}

int AVIFILEEX::VBR_FrameCountTillChunk(DWORD dwStream, DWORD dwChunk)
{
	int res = 0;
	for (DWORD j=0;j<dwChunk;res+=VBR_FrameCountInChunk(dwStream, j++));
	return res;
}

int AVIFILEEX::VBR_MaxFrameSize(DWORD dwStream)
{
	return ((WAVEFORMATEX*)siStreams[dwStream].lpFormat)->nBlockAlign;
}

int AVIFILEEX::VBR_FrameCountTillPos(DWORD dwStream, __int64 iPos)
{
	int currchunk = 0;
	int res = 0;
	int mfs = VBR_MaxFrameSize(dwStream);

	if (iPos > 0) do {
		
//		int nsize = siStreams[dwStream].ciChunks[currchunk].dwLength;
		int nsize = siStreams[dwStream].chunks[currchunk].dwLength;
		if (nsize<iPos) {
			res+=VBR_FrameCountInChunk(dwStream, currchunk);
		} else {
			res+=(int)((iPos/mfs)+(!!(iPos%mfs)));
		}

		iPos -= nsize;
	} while (iPos>0);

	return res;
}

DWORD AVIFILEEX::GetNbrOfChunks(DWORD dwStreamNbr)
{
	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;
	if (dwStreamNbr>=lpMainAVIHeader->dwStreams) return AFE_INVALIDPARAM;

	return (siStreams[dwStreamNbr].dwChunkCount);
}

_int64 AVIFILEEX::GetStreamSize(DWORD dwStreamNbr)
{
	if (dwStreamNbr>=GetNbrOfStreams()) return AFE_INVALIDPARAM;

	return (siStreams[dwStreamNbr].qwStreamLength);
}

DWORD AVIFILEEX::GetNbrOfStreams(void)
{
	return lpMainAVIHeader->dwStreams;
}

_int64 AVIFILEEX::GetStreamPosOfChunk(DWORD dwStreamNbr,DWORD dwChunkNbr)
{
	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;


//	return siStreams[dwStreamNbr].ciChunks[dwChunkNbr].qwStreamPos;
	return siStreams[dwStreamNbr].chunks[dwChunkNbr].qwStreamPos;
}

DWORD AVIFILEEX::GetChunkSize(DWORD dwStreamNbr,DWORD dwChunkNbr)
{
	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;

	if (dwChunkNbr==CN_CURRENT_CHUNK)
	{
		return GetChunkSize(dwStreamNbr,siStreams[dwStreamNbr].dwPos);
	}
	if (dwChunkNbr==CN_PREV_CHUNK)
	{
		return GetChunkSize(dwStreamNbr,siStreams[dwStreamNbr].dwPos-1);
	}

//	return siStreams[dwStreamNbr].ciChunks[dwChunkNbr].dwLength;
	return siStreams[dwStreamNbr].chunks[dwChunkNbr].dwLength;
}

AVIStreamHeader* AVIFILEEX::GetStreamHeader(DWORD dwStreamNbr)
{
	if (dwStreamNbr>=GetNbrOfStreams()) return NULL;
	return (siStreams[dwStreamNbr].lpHeader);
}

void* AVIFILEEX::GetStreamFormat(DWORD dwStreamNbr)
{
	if (dwStreamNbr>=GetNbrOfStreams()) return NULL;
	return (void*)(siStreams[dwStreamNbr].lpFormat);
}

void* AVIFILEEX::GetStreamOutputFormat(DWORD dwStreamNbr)
{
	if (dwStreamNbr>=GetNbrOfStreams()) return NULL;
	return (siStreams[dwStreamNbr].lpOutputFormat);
}

DWORD AVIFILEEX::GetCurrChunk(DWORD dwStreamNbr)
{
	if (dwStreamNbr>=GetNbrOfStreams()) return 0;
	return (siStreams[dwStreamNbr].dwPos);
}

bool AVIFILEEX::IsCBR(DWORD dwStreamNbr)
{
	if (dwAccess!=FA_READ) return false;
	if (dwStreamNbr>=GetNbrOfStreams()) return false;

	if (dwStreamNbr==0)
	{
		return false;
	}
	else
	{
		WAVEFORMATEX*	lpwfe=(WAVEFORMATEX*)siStreams[dwStreamNbr].lpFormat;
		if (lpwfe->wFormatTag!=0x55 && lpwfe->wFormatTag!=0xFF) 
		{
			return true;
		}
		else
		{
			if (!(lpwfe->nBlockAlign % 576) && lpwfe->wFormatTag == 0x55 ||
				lpwfe->nBlockAlign == 1024 && lpwfe->wFormatTag == 0xFF)
			{
				return false;
			}
			else
			{
				return true;
			}
		}
	}
}

DWORD AVIFILEEX::GetFormatTag(DWORD dwStreamNbr)
{
	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;
	if (dwStreamNbr>=GetNbrOfStreams()) return AFE_INVALIDPARAM;
	
	if (dwStreamNbr)
	{
		WAVEFORMATEX*	lpwfe=(WAVEFORMATEX*)siStreams[dwStreamNbr].lpFormat;
		return lpwfe->wFormatTag;
	}
	else
	{
		return siStreams[0].lpHeader->fccHandler;
	}
}

DWORD AVIFILEEX::GetAvgBytesPerSec(DWORD dwStreamNbr)
{
	if (dwAccess!=FA_READ) return AFE_INVALIDCALL;
	if (dwStreamNbr>=GetNbrOfStreams()) return AFE_INVALIDPARAM;
	if (IsVideoStream(dwStreamNbr)) return AFE_INVALIDPARAM;
	return ((WAVEFORMATEX*)GetStreamFormat(dwStreamNbr))->nAvgBytesPerSec;
}

int AVIFILEEX::GetStreamFrequency(DWORD dwStreamNbr)
{
	if (dwAccess!=FA_READ) return false;
	if (dwStreamNbr>=GetNbrOfStreams()) return false;

	if (dwStreamNbr==0)
	{
		return 0;
	}
	WAVEFORMATEX*	lpwfe=(WAVEFORMATEX*)siStreams[dwStreamNbr].lpFormat;
	
	return lpwfe->nSamplesPerSec;
}

int AVIFILEEX::GetStreamGranularity(DWORD dwStreamNbr)
{
	if (dwAccess!=FA_READ) return false;
	if (dwStreamNbr>=GetNbrOfStreams()) return false;

	if (dwStreamNbr==0)
	{
		return 0;
	}
	else
	{
		WAVEFORMATEX*	lpwfe=(WAVEFORMATEX*)siStreams[dwStreamNbr].lpFormat;
		if (lpwfe->wFormatTag==0x55)
		{
			return (lpwfe->nBlockAlign!=1)?0:1;
		}
		else
		if (lpwfe->wFormatTag==0xFF)
		{
			return 0;
		}
		else
		if (lpwfe->wFormatTag==0x2000)
		{
			return (DWORD)((__int64)(lpwfe->nAvgBytesPerSec)*GetMicroSecPerFrame()/1000000);
		}
		else
		if (lpwfe->wFormatTag==0x2001)
		{
			return (DWORD)((__int64)(lpwfe->nAvgBytesPerSec)*GetMicroSecPerFrame()/1000000);
		}
		else
		{
			return lpwfe->nBlockAlign;
		}
	}
}


//////// Schreibroutinen //////////

bool AVIFILEEX::IsWriteODML()
{
	if (dwAccess!=FA_WRITE) return false;
	if (GetAVIType()!=AT_OPENDML) return false;
	return true;
}

int AVIFILEEX::SetNumberOfStreams(DWORD dwNbr)
{
	if (dwAccess!=FA_WRITE) return AFE_INVALIDCALL;
	if (dwNbr==0) return AFE_INVALIDPARAM;

	lpMainAVIHeader->dwStreams=dwNbr;

	siStreams= new STREAMINFO[dwNbr];
//	ZeroMemory(siStreams,dwNbr*sizeof(STREAMINFO));
	dwLargestChunks=(DWORD*)malloc(4*dwNbr);
	ZeroMemory(dwLargestChunks,4*dwNbr);
	SetProcessMode(SPM_SETALL,PM_DIRECTSTREAMCOPY);
	return AFE_OK;
}

int AVIFILEEX::SetStreamDefault(DWORD dwStreamNbr, bool bDefault)
{
	siStreams[dwStreamNbr].bDefault = bDefault;
	siStreams[dwStreamNbr].lpHeader->dwFlags &=~ AVISF_DISABLED;

	siStreams[dwStreamNbr].lpHeader->dwFlags |= AVISF_DISABLED * !bDefault;
	return AFE_OK;
}

int AVIFILEEX::SetMicroSecPerFrame(DWORD dwMSPF)
{
	if (dwAccess!=FA_WRITE) return AFE_INVALIDCALL;
	if (dwMSPF==0) return AFE_INVALIDPARAM;

	lpMainAVIHeader->dwMicroSecPerFrame=dwMSPF;
	qwNSPF=(__int64)dwMSPF*1000;
	return AFE_OK;
}

int AVIFILEEX::SetNanoSecPerFrame(__int64 _qwNSPF)
{
	if (dwAccess!=FA_WRITE) return AFE_INVALIDCALL;
	if (!_qwNSPF) return AFE_INVALIDPARAM;

	lpMainAVIHeader->dwMicroSecPerFrame=(DWORD)round((double)_qwNSPF/1000);
	qwNSPF=_qwNSPF;

	return AFE_OK;
}

int AVIFILEEX::SetStreamHeader(DWORD dwStreamNbr,AVIStreamHeader* strh)
{
	if (dwAccess!=FA_WRITE) return AFE_INVALIDCALL;
	if (dwStreamNbr>=GetNbrOfStreams()) return AFE_INVALIDPARAM;

	STREAMINFO* siStr=&(siStreams[dwStreamNbr]);
	if (siStr->lpHeader)
	{
		delete siStr->lpHeader;
		siStr->lpHeader=NULL;
	}
	siStr->lpHeader=(AVIStreamHeader*)malloc(sizeof(AVIStreamHeader));
	ZeroMemory(siStr->lpHeader,sizeof(AVIStreamHeader));
	memcpy(siStr->lpHeader,strh,sizeof(AVIStreamHeader));
	return AFE_OK;
}

int AVIFILEEX::strfSize(DWORD dwStreamNbr,void* strf)
{
	BITMAPINFOHEADER*	lpbmihdr=(BITMAPINFOHEADER*)strf;
	WAVEFORMATEX*		lpwavfex=(WAVEFORMATEX*)strf;

	if (strf)
	{
		return (IsVideoStream(dwStreamNbr))?lpbmihdr->biSize:(lpwavfex->wFormatTag==1)?18:sizeof(WAVEFORMATEX)+lpwavfex->cbSize;
	}
	else
	{
		return 0;
	}
}

int AVIFILEEX::SetStreamFormat(DWORD dwStreamNbr,void* strf)
{
	BITMAPINFOHEADER*	lpbmihdr=(BITMAPINFOHEADER*)strf;
	WAVEFORMATEX*		lpwfe=(WAVEFORMATEX*)strf;
	DWORD				dwSize=0;

	if (dwAccess!=FA_WRITE) return AFE_INVALIDCALL;
	if (dwStreamNbr>=GetNbrOfStreams()) return AFE_INVALIDPARAM;
	STREAMINFO* siStr=&(siStreams[dwStreamNbr]);
	dwSize=strfSize(dwStreamNbr,strf);
	if (siStr->lpFormat)
	{
		delete siStr->lpFormat;
		siStr->lpFormat=NULL;
	}
	if (dwSize)
	{
		siStr->lpFormat=malloc(dwSize);
		memcpy(siStr->lpFormat,strf,dwSize);
		lpwfe=(WAVEFORMATEX*)(siStr->lpFormat);
		if (dwStreamNbr>0) lpwfe->nAvgBytesPerSec=0;
	}
	else
	{
		siStr->lpFormat=NULL;
	}

	return AFE_OK;
}

void AVIFILEEX::MoveHDRL(bool bEnabled)
{
	bMoveHDRLAround = bEnabled;
}


int AVIFILEEX::AddChunk(DWORD dwStreamNbr,void* lpData,DWORD dwSize,DWORD dwFlags)
{
	CHUNK*	NextChunk;
	INDEX*	NextIndex;
	char	Buffer[200];
	int		iSize;
	DWORD	dwKind = INDEXTYPE_UNCHANGED;
	void**	lplpData = (void**)lpData;
	bool	bContinueChunk = false;
	DWORD	dwOffset = 0;

	if (dwAccess!=FA_WRITE)	return AFE_INVALIDCALL;

	ZeroMemory(Buffer,sizeof(Buffer));
	if (!FirstChunk) {
		FirstChunk=new CHUNK;
		LastChunk = FirstChunk;
	}

	if (!FirstChunk->IsValid())	{
		// start up new chunk sequence
		NextChunk=FirstChunk;
	} else {
		// continue chunk seqeuence
		if ((iStreamOfLastChunk == (int)dwStreamNbr) && IsLowOverheadMode()) {
			LastChunk->IncreaseSizeBy(dwSize, &dwOffset);
			dwOffset += (dwOffset % 2);
			NextChunk = LastChunk;
			bContinueChunk = true;
		} else {
			NextChunk=new CHUNK;
			LastChunk->lpNext=NextChunk;
		}
	}
	if (!(Index->Valid()))
	{
		NextIndex=Index;
	}
	else
	{
		if (GetAVIType()==AT_STANDARD)
		{
			NextIndex=new INDEX;
			LastIndex->lpNext=NextIndex;
		}
		else
		{
			NextIndex=new STANDARDINDEX;
			LastIndex->lpNext=NextIndex;
		}
	}

	if (dwStreamNbr==SN_JUNK)
	{
		NextChunk->SetFourCC(MakeFourCC("JUNK"));
		NextChunk->SetSize(dwSize);
		NextChunk->SetData(lpData);
	}
	else
	{
		if (!bContinueChunk) {
			if (IsVideoStream(dwStreamNbr))	{
				NextChunk->SetFourCC(MKFOURCC(48+(dwStreamNbr/10),48+(dwStreamNbr%10),'d','c'));
				dwKind=AVI_VIDEOSTREAM;
				dwFramesInCurrIndex++;
			} else
			if (IsAudioStream(dwStreamNbr)) {
				NextChunk->SetFourCC(MKFOURCC(48+(dwStreamNbr/10),48+(dwStreamNbr%10),'w','b'));
				dwKind=AVI_AUDIOSTREAM;
			} else
			if (IsTextStream(dwStreamNbr)) {
				NextChunk->SetFourCC(MKFOURCC(48+(dwStreamNbr/10),48+(dwStreamNbr%10),'t','x'));
				dwKind=AVI_TEXTSTREAM;
			}

			NextChunk->SetSize(dwSize);
			NextChunk->SetData(*lplpData,0);
		} else {
			NextChunk->SetData(*lplpData, CHUNKSD_OVERWRITE, dwOffset);
		}

	
		if (GetAVIType()==AT_STANDARD) {
			NextIndex->SetData(dwStreamNbr,dwFlags,qwFilePos-dwMoviPos-8,dwSize,dwKind);
		} else {
			if (!bContinueChunk) {
				NextIndex->SetData(dwStreamNbr,dwFlags,qwFilePos,dwSize,dwKind);
			} else {
				NextIndex->SetData(dwStreamNbr,dwFlags,qwFilePos-8,dwSize,dwKind);
			}
		}
		siStreams[dwStreamNbr].qwStreamLength+=dwSize;
		siStreams[dwStreamNbr].dwChunkCount++;
		LastIndex=NextIndex;
	}

	if (!bContinueChunk) {
		qwFilePos+=dwSize+(dwSize%2)+8;
	} else {
		qwFilePos+=dwSize+(dwSize%2);
	}

	dwLargestChunks[dwStreamNbr]=max(dwLargestChunks[dwStreamNbr],dwSize);
	LastChunk=NextChunk;
	dwChunkCount++;

	if (GetPadding()>2) {
		if (qwFilePos%GetPadding())
		{
			iSize=(int)(GetPadding()-(qwFilePos%GetPadding())-8);
			iSize+=(iSize<0)?GetPadding():0;
			AddChunk(SN_JUNK,NULL,iSize,0);
		}
	}

	if ((FirstChunk->GetSize(LE_CHAIN)>=dwCacheSize) && (!bRECListOpen)) FlushWriteCache();

	if ((iStreamOfLastChunk = (int)dwStreamNbr) == SN_JUNK) {
		iStreamOfLastChunk = -3;
	}
	

	return AFE_OK;
}

void AVIFILEEX::SetPadding(DWORD _dwPadding)
{
	dwPadding=_dwPadding;
}

DWORD AVIFILEEX::GetPadding(void)
{
	return dwPadding;
}

int AVIFILEEX::BeginRECList(void)
{
	LIST*	NextList;
	INDEX*	NextIndex;
	char	Buffer[200];
	
	ZeroMemory(Buffer,sizeof(Buffer));
	if (FirstList->GetSize(LE_CHAIN)==12)
	{
		NextList=FirstList;
		DebugMsg("neue List-Kette begonnen");
	}
	else
	{
		NextList=new LIST;
		LastList->lpNext=NextList;
		wsprintf(Buffer,"  vorhandene List-Kette fortgesetzt bei %d Bytes",FirstList->GetSize(LE_CHAIN));
		DebugMsg(Buffer);
	}

	if (GetAVIType()==AT_STANDARD)
	{
		if (!(Index->Valid()))
		{
			NextIndex=Index;
		}
		else
		{
			NextIndex=new INDEX;
			LastIndex->lpNext=NextIndex;
		}
		RECIndex=NextIndex;
		RECIndex->SetData(0xffffffff,AVIIF_LIST,qwFilePos-dwMoviPos-8,0,0);
	}
	bRECListOpen=true;
	NextList->SetFourCC(MakeFourCC("rec "));
	if (!FirstChunk)
	{
		FirstChunk=new CHUNK;
	}
	LastChunk=FirstChunk;
	NextList->SetData(FirstChunk);
	
	if (GetAVIType()==AT_STANDARD) LastIndex=NextIndex;
	LastList=NextList;
	qwFilePos+=12;

	return AFE_OK;
} 

int AVIFILEEX::EndRECList(void)
{
	if (GetAVIType()==AT_STANDARD) {
		//RECIndex->SetData(0xffffffff,AVIIF_LIST,0,FirstList->GetSize(LE_CHAIN),0);
		RECIndex->SetData(0xffffffff,AVIIF_LIST,0,LastList->GetSize(LE_CHAIN)-8,0);
	}
	bRECListOpen=false;
	dwRecCount++;

	if (FirstList->GetSize(LE_CHAIN)>=dwCacheSize)
	{
		FlushWriteCache();
	}
	else
	{
		FirstChunk=NULL;
	}

	iStreamOfLastChunk = -2;

	return AFE_OK;
}

int AVIFILEEX::FinishStream(DWORD dwStreamNbr,__int64 qwMicroSec)
{
	WAVEFORMATEX*		lpwfe=(WAVEFORMATEX*)GetStreamFormat(dwStreamNbr);

	if (dwAccess!=FA_WRITE) return AFE_INVALIDCALL;
	if (IsVideoStream(dwStreamNbr)) return AFE_INVALIDPARAM;
	if (dwStreamNbr>=GetNbrOfStreams()) return AFE_INVALIDCALL;

	if (lpwfe && lpwfe->nAvgBytesPerSec) return AFE_INVALIDCALL;

	if (IsAudioStream(dwStreamNbr)) lpwfe->nAvgBytesPerSec=(DWORD)round((double)siStreams[dwStreamNbr].qwStreamLength*1000000/(double)qwMicroSec);
	if (IsTextStream(dwStreamNbr)) siStreams[dwStreamNbr].qwStreamLength=qwMicroSec;
	return AFE_OK;
}

int AVIFILEEX::FlushWriteCache(void)
{
//	void*	lpBuffer;
	DWORD	dwSize;
	char	Msg[200];

	if (dwAccess!=FA_WRITE)	return AFE_INVALIDCALL;

	if ((dwSize=FirstList->GetSize(LE_CHAIN))==12)
	{
		dwSize=FirstChunk->GetSize(LE_CHAIN);
		if (dwSize==8) return AFE_OK;

		FirstChunk->StoreToStream(dest,LE_CHAIN | LE_USELASTSIZE);
		ZeroMemory(Msg,sizeof(Msg));
		wsprintf(Msg,"Cache geschrieben: %d	kB",dwSize>>10);
		DebugMsg(Msg);

		FirstChunk->FreeData(LE_CHAIN);
		delete FirstChunk;
		FirstChunk=new CHUNK;
		LastChunk=FirstChunk;
	}
	else
	{
		FirstList->StoreToStream(dest,LE_CHAIN/* | LE_USELASTSIZE*/);
		ZeroMemory(Msg,sizeof(Msg));
		wsprintf(Msg,"Cache geschrieben: %d	kB",dwSize>>10);
		DebugMsg(Msg);

		FirstList->FreeData(LE_CHAIN);
		delete FirstList;
		FirstList=new LIST;
		LastList=FirstList;
		FirstChunk=new CHUNK;
		LastChunk=FirstChunk;
	}

	if (IsWriteODML())
	{
		if (dwFramesInCurrIndex>=dwFramesPerIndex)
		{
			WriteStandardIndex();
			dwFramesInCurrIndex-=dwFramesPerIndex;
		}

	}

	if (qwRIFFStart)
	{
		if ((qwFilePos-qwRIFFStart)>(dwMaxRIFFAVIXSize))
		{
			EndRIFFAVIX();
			BeginRIFFAVIX();
		}
	}
	else
	{
		if ((qwFilePos-qwRIFFStart)>(dwMaxRIFFAVISize))
		{
			EndRIFFAVI();
			BeginRIFFAVIX();
		}
	}

	return AFE_OK;
}

int AVIFILEEX::SetMaxRIFFAVISize(DWORD dwMaxSize)
{
	dwMaxRIFFAVISize=dwMaxSize;
	return AFE_OK;
}

int AVIFILEEX::SetMaxRIFFAVIXSize(DWORD dwMaxSize)
{
	dwMaxRIFFAVIXSize=dwMaxSize;
	return AFE_OK;
}

int AVIFILEEX::SetOutputResolution(int x, int y)
{
	vprp.dwFrameWidthInPixels = x;
	vprp.dwFrameHeightInLines = y;
	vprp.dwFrameAspectRatio = ((x & 0xFFFF) << 16) + (y & 0xFFFF);

	return 0;
}

int AVIFILEEX::CreateLegacyIndexForODML(bool bLegacyIndex)
{
	if (!IsWriteODML()) return AFE_INVALIDCALL;

	bCreateLegacyIndexForODML=bLegacyIndex;
	if (bLegacyIndex)
		EnableLowOverheadMode(false);
	
	return AFE_OK;
}

int AVIFILEEX::SetLegacyCallBack(STARTLEGACYCALLBACK stcb)
{
	cbfs.lpslcb=stcb;
	return AFE_OK;
}

DWORD AVIFILEEX::GetHeaderSpace(void)
{
	return dwHeaderSpace;
}

int AVIFILEEX::IsRecListOpen(void)
{
	return bRECListOpen; 
}

int AVIFILEEX::SetShallRebuildIndexCallback(SHALLREBUILDINDEXCALLBACK sri, void* lpUserData)
{
	cbfs.lpsri=sri;
	cbfs.lpsriud=lpUserData;
	return AFE_OK;
}
	
DWORD AVIFILEEX::EndRIFFAVI(void)
{
	DWORD	i;
	void*	lpBuffer;
	DWORD	dwSize;
	DWORD	dwRiffSize;
	LISTHEADER	lhdr;
	INDEX	*SourceIndex,*DestIndex;
	INDEX	*FirstDestIndex;
	CHUNKHEADER	ch;
	DWORD	dwIdx1Size;
	DWORD	dwPosCount=0;
	LEGACYSTATECALLBACK	lplscb;

	if (!IsWriteODML()) return AFE_INVALIDCALL;

	if (!SupIndex)
	{
		SupIndex=new SUPERINDEX;
		LastSupIndex=SupIndex;
	}

	for (i=0;i<GetNbrOfStreams();i++)
	{
		Index->SelectStream(i);
		dwSize=Index->GetSize();
		if (dwSize>32)
		{
			lpBuffer=malloc(dwSize);
			Index->Store(lpBuffer);
			dwStdIndex_RIFFAVIXOverhead+=32;
			LastSupIndex->SetData(i,0,qwFilePos,dwSize,GetKindOfStream(i));
			LastSupIndex->SetCurrentStreamSize(siStreams[i].qwStreamLength);
			LastSupIndex->lpNext=new SUPERINDEX;
			LastSupIndex=(SUPERINDEX*)LastSupIndex->lpNext;
			dest->Write(lpBuffer,dwSize);
			qwFilePos+=dwSize;
			free (lpBuffer);
		}
	}
	lpMainAVIHeader->dwTotalFrames=siStreams[0].dwChunkCount;
	dwMoviSize=(DWORD)(qwFilePos-dwMoviPos-8);
	dest->Seek(dwMoviPos);
	lhdr.dwListID=MakeFourCC("LIST");
	lhdr.dwFourCC=MakeFourCC("movi");
	lhdr.dwLength=dwMoviSize;
	dest->Write(&lhdr,12);
	dest->Seek(qwFilePos);

	if (bCreateLegacyIndexForODML)
	{
		SourceIndex=Index;
		DestIndex=new INDEX;
		FirstDestIndex=DestIndex;
		dwPosCount=0;
		if (cbfs.lpslcb)
		{
			lplscb=(LEGACYSTATECALLBACK)((*cbfs.lpslcb)(&cbfs.lplscbud));
		}
		while (SourceIndex)
		{
			DestIndex->SetData(SourceIndex->GetStream(),SourceIndex->GetFlags(),
				SourceIndex->GetOffset()-dwMoviPos-8,SourceIndex->GetChunkSize(),
				GetKindOfStream(SourceIndex->GetStream()));
			DestIndex->lpNext=new INDEX;
//			ZeroMemory(DestIndex->lpNext, sizeof(INDEX));
			DestIndex=DestIndex->lpNext;

			SourceIndex=SourceIndex->lpNext;
			dwPosCount++;
			if (((dwPosCount&0xFFF)==0)||(dwPosCount==dwChunkCount))
			{
				if (lplscb)
				{
					(*lplscb)(dwPosCount,dwChunkCount,cbfs.lplscbud);
				}
			}

		}

		dwIdx1Size=16*(dwChunkCount);
		lpBuffer=malloc(dwIdx1Size);
		FirstDestIndex->Store(lpBuffer);
		ch.dwFourCC=MakeFourCC("idx1");
		ch.dwLength=dwIdx1Size;
		dest->Write(&ch,sizeof(ch));
		dest->Write(lpBuffer,dwIdx1Size);
		qwFilePos+=8+dwIdx1Size;
		free (lpBuffer);
		FirstDestIndex->Delete();
		delete FirstDestIndex;
		bCreateLegacyIndexForODML=false;
	}

	dwRiffSize=(DWORD)(qwFilePos-8);
	dest->Seek(0);
	lhdr.dwListID=MakeFourCC("RIFF");
	lhdr.dwFourCC=MakeFourCC("AVI ");
	lhdr.dwLength=dwRiffSize;
	dest->Write(&lhdr,12);

	Index->Delete();
	delete Index;
	Index=new STANDARDINDEX;
	LastIndex=new STANDARDINDEX;

	dest->Seek(qwFilePos);

	return AFE_OK;
}

bool AVIFILEEX::IsLegacyEnabled()
{
	return bCreateLegacyIndexForODML;		
}


DWORD AVIFILEEX::BeginRIFFAVIX(void)
{
	char	Buffer[24];
	
	if (!IsWriteODML()) return AFE_INVALIDCALL;

	qwRIFFStart=qwFilePos;
	ZeroMemory(Buffer,sizeof(Buffer));
	dest->Write(Buffer,sizeof(Buffer));
	qwFilePos+=24;
	dwStdIndex_RIFFAVIXOverhead+=24;

	return AFE_OK;
}

int AVIFILEEX::SetFramesPerIndex(DWORD dwMax)
{
	dwFramesPerIndex=dwMax;
	return 1;
}

int AVIFILEEX::WriteStandardIndex(void)
{
	int i;
	DWORD	dwSize;
	void*	lpBuffer;

	if (!IsWriteODML()) return AFE_INVALIDCALL;

	if (!SupIndex)
	{
		SupIndex=new SUPERINDEX;
		LastSupIndex=SupIndex;
	}

	for (i=0;i<(int)GetNbrOfStreams();i++)
	{
		Index->SelectStream(i);
		dwSize=Index->GetSize();
		if (dwSize>32)
		{
			lpBuffer=malloc(dwSize);
			Index->Store(lpBuffer);
			LastSupIndex->SetData(i,0,qwFilePos,dwSize,GetKindOfStream(i));
			LastSupIndex->SetCurrentStreamSize(siStreams[i].qwStreamLength);
			LastSupIndex->lpNext=new SUPERINDEX;
			LastSupIndex=(SUPERINDEX*)LastSupIndex->lpNext;
			dest->Write(lpBuffer,dwSize);
			qwFilePos+=dwSize;
			free (lpBuffer);
			dwStdIndex_RIFFAVIXOverhead+=32;
		}
	}
	Index->SetRange(qwFilePos,(__int64)1<<60);

	return 1;
}

DWORD AVIFILEEX::EndRIFFAVIX(void)
{
	DWORD	i;
	void*	lpBuffer;
	DWORD	dwSize;
	DWORD	dwRiffSize;
	LISTHEADER	lhdr;

	if (!IsWriteODML()) return AFE_INVALIDCALL;

	for (i=0;i<GetNbrOfStreams();i++)
	{
		Index->SelectStream(i);
		dwSize=Index->GetSize();
		if (dwSize>32)
		{
			lpBuffer=malloc(dwSize);
			Index->Store(lpBuffer);
			LastSupIndex->SetData(i,0,qwFilePos,dwSize,GetKindOfStream(i));
			LastSupIndex->SetCurrentStreamSize(siStreams[i].qwStreamLength);
			LastSupIndex->lpNext=new SUPERINDEX;
			LastSupIndex=(SUPERINDEX*)LastSupIndex->lpNext;
			dest->Write(lpBuffer,dwSize);
			qwFilePos+=dwSize;
			free (lpBuffer);
			dwStdIndex_RIFFAVIXOverhead+=32;
		}
	}

	Index->Delete();
	delete Index;
	Index=new STANDARDINDEX;
	LastIndex=new STANDARDINDEX;

	dwRiffSize=(DWORD)(qwFilePos-qwRIFFStart-8);
	dwMoviSize=dwRiffSize-12;
	
	dest->Seek(qwRIFFStart);
	lhdr.dwListID=MakeFourCC("RIFF");
	lhdr.dwFourCC=MakeFourCC("AVIX");
	lhdr.dwLength=dwRiffSize;
	dest->Write(&lhdr,12);
	lhdr.dwListID=MakeFourCC("LIST");
	lhdr.dwFourCC=MakeFourCC("movi");
	lhdr.dwLength=dwMoviSize;
	dest->Write(&lhdr,12);
	dest->Seek(qwFilePos);

	return AFE_OK;
}

DWORD AVIFILEEX::GetStdIndexOverhead()
{
	DWORD dwRes;
	dwRes=dwStdIndex_RIFFAVIXOverhead;
	dwStdIndex_RIFFAVIXOverhead=0;
	return dwRes;
}

int AVIFILEEX::Enable(int iFlag, int iValue)
{
	switch (iFlag) {
		case AFE_LEGACYINDEX:
			CreateLegacyIndexForODML(!!iValue); break;

	}
	return 0;
}

				///////////////////////
				//  FRAME - Methoden //
				///////////////////////

FRAME::FRAME(void)
{
	dwWidth=dwHeight=dwBufferSize=0;
	dwBitDepth=24;
	lpBuffer=NULL;
	lpOrgBuffer=NULL;
	lpUserData=NULL;
	bExternalBuffer=false;
	csColorSpace=CS_RGB;
};

FRAME::~FRAME(void)
{
	if (lpOrgBuffer) free (lpOrgBuffer);
	if (lpUserData) free (lpUserData);
	lpOrgBuffer=NULL; lpBuffer=NULL, lpUserData=NULL;
};

DWORD FRAME::GetLineLength(void)
{
	DWORD  dwLL=0;

	dwLL = (dwBitDepth>>3)*dwWidth;

	if (dwLL%4) 
	{
		dwLL/=4; dwLL=(dwLL+1)*4;
	}

	return (dwLL);
}

bool FRAME::Realloc(void)
{
	if (bExternalBuffer) return true;

	if (lpOrgBuffer) free (lpOrgBuffer);
	lpOrgBuffer=NULL;
	lpBuffer=NULL;
	dwBufferSize=ImageSize();
	if (dwBufferSize)
	{
		lpOrgBuffer=(char*)new char[dwBufferSize+16];
		if ((DWORD)lpOrgBuffer%16)
		{
			lpBuffer=(char*)((((DWORD)lpOrgBuffer/16)+1)*16);
		}
		else
			lpBuffer=lpOrgBuffer;
	}
	return (lpBuffer!=NULL);
}

DWORD FRAME::ImageSize(void)
{
	if ((csColorSpace==CS_RGB)||(csColorSpace==CS_PACKEDYUV))
	{
		return (GetLineLength()*(dwHeight+2));
	}
	if (csColorSpace==CS_YV12)
	{
		return (dwWidth*dwHeight*3/2);
	}
	return (0);
}

bool FRAME::SetWidth(DWORD dwNewWidth)
{
	dwWidth=dwNewWidth;
	return true;
}

bool FRAME::SetHeight(DWORD dwNewHeight)
{
	dwHeight=dwNewHeight;
	return true;
}

bool FRAME::SetBitDepth(DWORD dwNewBitDepth)
{
	dwBitDepth=dwNewBitDepth;
	return true;
}

bool FRAME::SetUserData(void* lpNewData)
{
	lpUserData=lpNewData;
	return (lpUserData!=NULL);
}

DWORD FRAME::GetWidth(void)
{
	return dwWidth;
}

DWORD FRAME::GetHeight(void)
{
	return dwHeight;
}

DWORD FRAME::GetBitDepth(void)
{
	return dwBitDepth;
}

void* FRAME::GetUserData(DWORD dwSize=0)
{
	if (lpUserData)
	{
		return lpUserData;
	}
	else
	{
		lpUserData=new char[dwSize];
		return lpUserData;
	}
}

bool FRAME::UseInternalBuffer(void)
{
	bExternalBuffer=false;
	return Realloc();
}

bool FRAME::UseExternalBuffer(void* lpNewBuffer,DWORD dwLength)
{
	if (!lpNewBuffer) return false;
	if (lpOrgBuffer)
	{
		if (dwBufferSize) free(lpOrgBuffer);
		lpOrgBuffer=NULL;
		dwBufferSize=dwLength;
	}
	lpBuffer=lpOrgBuffer=(char*)lpNewBuffer;
	bExternalBuffer=true;
	return true;	
}

void* FRAME::GetBuffer(DWORD dwLine)
{
	return (  (void*)((DWORD)(lpBuffer)+dwLine*GetLineLength())  );
}

DWORD FRAME::GetBufferSize(void)
{
	return dwBufferSize;
}

bool FRAME::IsExternalBuffer(void)
{
	return bExternalBuffer;
}

bool FRAME::SetColorSpace(COLORSPACE csNewColorSpace)
{
	csColorSpace=csNewColorSpace;
	return true;
}