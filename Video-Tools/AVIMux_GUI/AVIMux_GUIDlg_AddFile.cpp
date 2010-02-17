#include "stdafx.h"
#include "..\..\Common\TraceFiles\TraceFile.h"
#include "AVIMux_GUIDlg.h"
#include "..\Basestreams.h"
#include "..\FileStream.h"
#include "..\Cache.h"
#include "Formattext.h"
#include "UTF8Windows.h"
#include "ConfigScripts.h"
#include "../strings.h"
#include "../Finalizer.h"
#include "MessageBoxHelper.h"
#include <sstream>

/** \brief Adds a file to the list of available files
 *
 * \param _lpcName UTF-8 encoded path of the file to add
 *
 */
void  CAVIMux_GUIDlg::doAddFile(char* _lpcName, int iFormat, int delete_file,
								HANDLE hSemaphore)
{
	CLocalTracer trace(GetApplicationTraceFile(), _T("CAVIMux_GUIDlg::doAddFile(...)"));

	char*	lpcExt;
	int		i;
	int				iIndex1;
	CFileStream*		filesource;
	STREAM*			source;
	CACHE*			cachesource = NULL;

	MODE2FORM2SOURCE* m2f2source;
	char			VideoFormat[100];
	union
	{
		MP3SOURCE*			mp3source;
		WAVSOURCE*			wavsource;
		AC3SOURCE*			ac3source;	
		AACSOURCE*			aacsource;	
		DTSSOURCE*			dtssource;
		VORBISFROMOGG*		vorbis;
	};
	OGGFILE*			oggfile;
	SUBTITLES*		subs;
	WAVEFILE*		wavfile;
	FILE_INFO*		fi;
	AUDIO_STREAM_INFO*	asi = NULL;
	AVIFILEEX*		AVIFile;
//	HANDLE			hSem;

	CString			cStr,cStr2;
	DWORD			dwUseCache;
	DWORD			dwFmtUpStr;
	DWORD			dwFmtUpStr3;
	SUBTITLE_STREAM_INFO*	lpssi;

	std::basic_string<TCHAR> messageBuffer;
	std::basic_string<TCHAR> formatName;
//	memset(cFmt, 0, sizeof(cFmt));
	DWORD			dwLength=strlen(_lpcName);
	CBuffer*		cb;

	if (_lpcName)
	{
		std::ostringstream sstrMsg;
		sstrMsg << _T("Opening file: ") << _lpcName;
		trace.Trace(TRACE_LEVEL_INFO, _T("Opening file"), sstrMsg.str());
	}

	char lpcName[65536];
	if (!strncmp(_lpcName, "\\\\?\\UNC\\", 8)) {
		strcpy(lpcName, "\\\\");
		strcat(lpcName, _lpcName + 8);
	} else {
		if (!strncmp(_lpcName, "\\\\?\\", 4))
			strcpy(lpcName, _lpcName + 4);
		else
			strcpy(lpcName, _lpcName);
	}

	if (!strcmp(_lpcName, "*stdin*")) {
		FILE* f = fopen("temp_std_file", "wb");
		char c;
		char d = 13;
		while ((c = getchar()) != EOF) {
			if (c == 0x0A) 
				fwrite(&d, sizeof(d), 1, f);
			fwrite(&c, sizeof(c), 1, f);
		}
		fclose(f);
		doAddFile("temp_std_file");
		DeleteFile(_T("temp_std_file"));
	}

	bool			bUnbuffered = !!settings->GetInt("input/general/unbuffered");
	bool			bOverlapped = !!settings->GetInt("input/general/overlapped");

	DWORD			dwCacheOpenMode = CACHE_OPEN_READ | CACHE_OPEN_ATTACH;
	StreamMode::StreamModes	dwFileOpenMode = StreamMode::Read;
	if (bUnbuffered)
		dwFileOpenMode |= StreamMode::Unbuffered;
	if (bOverlapped)
		dwFileOpenMode |= StreamMode::Overlapped;

	for (i=dwLength;(i>=0)&&(lpcName[i]!='.');i--);

	//lpcExt=new char[dwLength-i+2];
	lpcExt = new char[dwLength-i+2];
	Finalizer<char, void, delete_array> lpExtFinalizer(lpcExt);

	ZeroMemory(lpcExt,dwLength-i+2);
	strcpy(lpcExt,&(lpcName[i+1]));
	for (i=strlen(lpcExt)-1;i>=0;lpcExt[i--]|=((lpcExt[i]>=64)&&(lpcExt[i]<=90))?0x20:0);

//		ZeroMemory(Buffer,sizeof(Buffer));
		dwUseCache=(settings->GetInt("input/general/use cache"))?1:0;
		cb = new CBuffer;
		cb->SetSize(sizeof(FILE_INFO));
		cb->IncRefCount();

		fi = (FILE_INFO*)cb->GetData();
		ZeroMemory(fi,sizeof(FILE_INFO));
//		fi->cache=NULL;
		fi->dwType=0;
		fi->file=NULL;
		fi->AVIFile=NULL;
		fi->bM2F2CRC=!!(int)settings->GetInt("input/m2f2/crc check");//(ofOptions.dwFlags&SOFO_M2F2_DOM2F2CRCCHECK);
		fi->bInUse=false;
		fi->lpwav=NULL;
		fi->lpM2F2=NULL;
		fi->bAddedImmediately=1;
		fi->bMP3VBF_forced=!!(int)settings->GetInt("input/avi/force mp3 vbr");//(ofOptions.dwFlags&SOFO_AVI_FORCEMP3VBR);
		filesource=new CFileStream;
		if (filesource->Open(lpcName,StreamMode::Read)==STREAM_ERR) {
			DecBufferRefCount(&cb);
			return;
		}
		fi->file=filesource;
		fi->file_id = m_SourceFiles.GetCount();
		fi->current_pos = 2 * fi->file_id;

		m2f2source=new MODE2FORM2SOURCE;
		if (m2f2source->Open(filesource)==STREAM_OK) {
			/* don't open m2f2 files unbuffered */
			dwFileOpenMode &=~ (StreamMode::Overlapped | StreamMode::Unbuffered);
			source=(STREAM*)m2f2source;
			fi->dwType|=FILETYPE_M2F2;
			fi->lpM2F2=m2f2source;
		} else {
			source=filesource;
			m2f2source->Close();
			delete m2f2source;
			fi->lpM2F2=NULL;
		}
		
		XMLNODE* xmlTree;
		fi->dwType|=(iFormat!=FILETYPE_UNKNOWN)?iFormat:DetectFileType(source, 
			(void**)&xmlTree);

		source->Close();
		delete source;

		filesource = new CFileStream;
		if (filesource->Open(lpcName, dwFileOpenMode) <= 0) {
			if ((dwFileOpenMode & StreamMode::Unbuffered) ||
				(dwFileOpenMode & StreamMode::Overlapped)) {
				
				dwFileOpenMode &=~ (StreamMode::Unbuffered | StreamMode::Overlapped);
				MessageBox(_T("Failed to open file in unbuffered and/or overlapped mode!"),
					_T("Error"), MB_OK | MB_ICONERROR);
			}
			if (filesource->Open(lpcName, dwFileOpenMode) <= 0)
				MessageBox(_T("Weird: Could not open file again"), _T("Error"), 
					MB_OK | MB_ICONERROR);
		};
		fi->file = filesource;
		if (fi->dwType & FILETYPE_M2F2) {
			m2f2source=new MODE2FORM2SOURCE;
			m2f2source->Open(filesource);
			source=(STREAM*)m2f2source;
			fi->lpM2F2=m2f2source;
		} else
			source = filesource;

		source->Seek(0);

		if (fi->dwType & FILETYPE_AVI)
		{
			fi->bAddedImmediately=0;
			bool bCache;
			if (dwUseCache)	{
				cachesource=new CACHE(8,1<<18);
				cachesource->Open((STREAM*)source, dwCacheOpenMode);
				cachesource->Disable(CACHE_READ_AHEAD);
				bCache = true;
			} else {
				cachesource=(CACHE*)source;
				bCache = false;
			}
			AVIFile=new AVIFILEEX;
			AVIFile->SetDebugState(DS_DEACTIVATE);
			int ignore_large_chunks = (int)settings->GetInt("input/avi/large chunks/ignore");
			int repair_large_chunks = (int)settings->GetInt("input/avi/large chunks/repair");
			int repair_dx50 = (int)settings->GetInt("input/avi/repair DX50");
			AVIFile->SetMaxAllowedChunkSize(1024*(!!ignore_large_chunks)*ofOptions.dwIgnoreSize);
			AVIFile->TryToRepairLargeChunks(!!repair_large_chunks);
			
			if (AVIFile->Open(cachesource,FA_READ,AT_AUTODETECT)==AFE_OK)
			{
				if (bCache) {
					cachesource->InitCache(8 + 4 * AVIFile->GetNbrOfStreams(), -1, true);
					if (bOverlapped) {
						cachesource->Enable(CACHE_READ_AHEAD);
						cachesource->SetPrereadRange(2);
					}
				}
				if (repair_dx50) {
					if ((AVIFile->GetFormatTag(0)&0xffffdfdf)==MakeFourCC("DX50"))	{
						AVIFile->GetStreamHeader(0)->fccHandler=MakeFourCC("divx");
					}
				}
				dwFmtUpStr=AVIFile->GetFormatTag(0) & (0xdfdfdfdf);
				dwFmtUpStr3=AVIFile->GetFormatTag(0) & (0xffdfdfdf);
				if (dwFmtUpStr3==MakeFourCC("DIV3"))
				{
					sprintf(VideoFormat,"%s","divX 3.11 low motion");
				}
				else
				if (dwFmtUpStr3==MakeFourCC("DIV4"))
				{
					sprintf(VideoFormat,"%s","divX 3.11 fast motion");
				}
				else
				if (dwFmtUpStr==MakeFourCC("DIVX"))
				{
					if ( ((BITMAPINFOHEADER*)(AVIFile->GetStreamFormat(0)))->biCompression==MakeFourCC("DX50"))
					{
						sprintf(VideoFormat,"%s","divX 5");
					}
					else
					{
						sprintf(VideoFormat,"%s","divX 4");
					}
				}
				else
				if ((dwFmtUpStr3==MakeFourCC("DMB1"))||
					(dwFmtUpStr==MakeFourCC("MJPG"))||
					(dwFmtUpStr==MakeFourCC("MJPX")))
				{
					sprintf(VideoFormat,"%s","M-JPEG");
				}
				else
				if (dwFmtUpStr==MakeFourCC("HFYU")) {
					sprintf(VideoFormat,"%s","huff-YUV");
				} else
				if (dwFmtUpStr==MakeFourCC("XVID")) {
					sprintf(VideoFormat,"%s","XVID");
				} else
				if (AVIFile->GetFormatTag(0)==MakeFourCC("MJ2C")) {
					sprintf(VideoFormat,"%s","MorganMotion JPEG 2000");
				} else
					sprintf(VideoFormat,"%s","???");

				fi->AVIFile=AVIFile;
				AVIFile->SetProcessMode(SPM_SETALL,PM_DIRECTSTREAMCOPY);
				fi->dwType|=FILETYPE_AVI;
			// Infos anzeigen
				//sprintf(cFmt, "AVI");
				formatName = _T("AVI");
			}
			else
			{
				MessageBoxHelper::ShowByID(NULL, STR_GEN_ERROR, STR_ERR_OPENAVIERROR,
					MB_OK | MB_ICONERROR);
					
					//MessageBox (LoadString(STR_ERR_OPENAVIERROR));
				filesource->Close();
				fi->dwType=0;
			}
		}
		else
		if (fi->dwType&FILETYPE_MP3)
		{
			// use input cache?
			if (dwUseCache)
			{
				cachesource=new CACHE(4,1<<19);
				cachesource->Open(filesource, dwCacheOpenMode);
//				fi->cache=cachesource;
			}
			else
			{
				cachesource=(CACHE*)source;
			}
//			sprintf(Fmt, "%s", "MP");
			fi->dwType|=FILETYPE_MP3;
		}
		else
		if (fi->dwType&FILETYPE_AC3)
		{
			// use input cache?
			if (dwUseCache)	{
				cachesource=new CACHE(4,1<<19);
				cachesource->Open(source, dwCacheOpenMode);
//				fi->cache=cachesource;
			} else {
				cachesource=(CACHE*)source;
			}
		//	sprintf(Fmt,"%s","AC3");
			fi->dwType|=FILETYPE_AC3;
		}
		else
		if (fi->dwType&FILETYPE_DTS)
		{
			// use input cache?
			if (dwUseCache)	{
				cachesource=new CACHE(4,1<<19);
				cachesource->Open(source, dwCacheOpenMode);
//				fi->cache=cachesource;
			} else {
				cachesource=(CACHE*)source;
			}
		//	sprintf(Fmt,"%s","DTS");
			fi->dwType|=FILETYPE_DTS;
		}
		else
		if (fi->dwType&FILETYPE_AAC)
		{
			// use input cache?
			if (dwUseCache)
			{
				cachesource=new CACHE(4,1<<19);
				cachesource->Open(source, dwCacheOpenMode);
//				fi->cache=cachesource;
			}
			else
			{
				cachesource=(CACHE*)source;
			}
	//		wsprintf(Fmt,"%s","AAC");
			fi->dwType|=FILETYPE_AAC;
		}
		else
		if (fi->dwType&FILETYPE_OGGVORBIS)
		{
			// use input cache?
			if (dwUseCache)
			{
				cachesource=new CACHE(8,1<<16);
				cachesource->Open(source, dwCacheOpenMode);
//				fi->cache=cachesource;
			}
			else
			{
				cachesource=(CACHE*)source;
			}
	//		wsprintf(Fmt,"%s","OGG/Vorbis");
			fi->dwType|=FILETYPE_OGGVORBIS;
		}
		else
		if (fi->dwType&FILETYPE_WAV)
		{
			if (dwUseCache)
			{
				cachesource=new CACHE(4,1<<19);
				cachesource->Open(source, dwCacheOpenMode);
//				fi->cache=cachesource;
			}
			else
			{
				cachesource=(CACHE*)source;
			}
	//		wsprintf(Fmt,"%s","WAV");
			fi->dwType|=FILETYPE_WAV;
		}
		else
		if (fi->dwType&FILETYPE_SUBS)
		{
			if (dwUseCache)
			{
				cachesource=new CACHE(4,1<<19);
				cachesource->Open(source, dwCacheOpenMode);
//				fi->cache=cachesource;
			}
			else
			{
				cachesource=(CACHE*)source;
			}
	//		wsprintf(Fmt,"%s","SUB");
			fi->dwType|=FILETYPE_SUBS;
		}
		else
		if (fi->dwType&FILETYPE_MKV)
		{
			if (dwUseCache)
			{
				cachesource=new CACHE(16,1<<19);
				cachesource->Open(source, dwCacheOpenMode);
//				cachesource->Enable(CACHE_CREATE_LOG);
				cachesource->Enable(CACHE_CAN_GROW);
				cachesource->Disable(CACHE_READ_AHEAD);
				cachesource->SetPrereadRange(4);
//				fi->cache=cachesource;
			}
			else
			{
				cachesource=(CACHE*)source;
			}
		//	wsprintf(Fmt,"%s","MKV");
			fi->MKVFile = new MATROSKA;
			fi->bAddedImmediately=0;
			if (fi->MKVFile->Open(cachesource,MMODE_READ)==MOPEN_ERR) {
				delete fi->MKVFile;
				fi->MKVFile = NULL;
				fi->dwType&=~FILETYPE_MKV;
			} else {
				fi->dwType|=FILETYPE_MKV;
			}
			if (bOverlapped)
				cachesource->Enable(CACHE_READ_AHEAD);
		}
		
		if (fi->dwType!=FILETYPE_SCRIPT) {
			LVITEM	lvitem;
			int		iIndex_Enh;

			ZeroMemory(&lvitem,sizeof(lvitem));
			lvitem.mask=LVIF_TEXT | LVIF_NORECOMPUTE;
			lvitem.iItem=1;
			lvitem.iSubItem=0;
			lvitem.pszText=_T(""); //lpcName;
			iIndex_Enh=m_Enh_Filelist.InsertItem(&lvitem);
			m_Enh_Filelist.SetItemData(iIndex_Enh,(DWORD)fi);

			char cSize[100];
			__int64 dwSize = (cachesource)?cachesource->GetSize():fi->file->GetSize();
			FormatSize(cSize,dwSize);
		
//			::SendMessage(m_SourceFiles, LB_ADDSTRING, NULL, (L
			CUTF8 utf8Name(lpcName, CharacterEncoding::UTF8);
			// TODO: Do not pass utf8 here
			// iIndex1=m_SourceFiles.AddString(utf8Name.TStr());
			iIndex1=m_SourceFiles.AddString((LPCTSTR)lpcName);

//			char test[1024];
			//int res = m_SourceFiles.GetText(iIndex1, test);
			//m_SourceFiles.GetText(iIndex1, s);
//			::SendMessage(m_SourceFiles, LB_GETTEXT, iIndex1,
//				(LPARAM)test);

			fi->Name = CUTF8(lpcName, CharacterEncoding::UTF8);
//			fi->lpcName=new char[lstrlen(lpcName)+16];
			//lstrcpy(fi->lpcName,lpcName);
			fi->source=cachesource;
			m_SourceFiles.SetItemData(iIndex1,(LPARAM)cb);
			m_SourceFiles.InvalidateRect(NULL);
			m_SourceFiles.UpdateWindow();

			switch ((fi->dwType)&(FILETYPE_MASK))
			{
				case FILETYPE_AVI:
					{
						m_Enh_Filelist.SetItemText(iIndex_Enh,2, _T("AVI"));
					}
					formatName = _T("AVI");
					//strcpy(cFmt, "AVI");
					break;
				case FILETYPE_MKV:
					//sprintf(cFmt, "MKV");
					formatName = _T("MKV");
					break;
				case FILETYPE_MP3:
					{
						mp3source=new MP3SOURCE;
						if (!(mp3source->Open(cachesource)))
						{
							cStr=LoadString(IDS_CANTOPENMP3);
							MessageBox(cStr,cstrError,MB_OK | MB_ICONERROR);
							delete mp3source;
							m_SourceFiles.DeleteString(iIndex1);
							return;
						}
						fi->MP3File = mp3source;
						TCHAR cFmt[8]; cFmt[0]=0;
						_stprintf_s(cFmt, _T("MP%d"), mp3source->GetLayerVersion());
						formatName = cFmt;

						if (bAddAS_immed) {
							DWORD dwCheckVBR;
							asi = new AUDIO_STREAM_INFO;
							ZeroMemory(asi,sizeof(AUDIO_STREAM_INFO));
							if (((ofOptions.dwFlags&SOFO_MP3_MASK)==SOFO_MP3_CHECKCBRALWAYS)||((ofOptions.dwFlags&SOFO_MP3_MASK)==SOFO_MP3_CHECKCBRASK))
							{
								if (mp3source->ScanForCBR(1000)) {
									cStr=LoadString(IDS_COULDBECBR);
									if ((ofOptions.dwFlags&SOFO_MP3_MASK)==SOFO_MP3_CHECKCBRASK) {
										dwCheckVBR=MessageBox(cStr,cstrConfirmation,MB_YESNO | MB_ICONQUESTION);
									}
									else {
										dwCheckVBR=IDYES;
									}
									if (dwCheckVBR==IDYES)
									{
										if (mp3source->ScanForCBR(SCANFORCBR_ALL))	{
											if ((ofOptions.dwFlags&SOFO_MP3_RESULTDLG)==SOFO_MP3_RESULTDLG)	{
												cStr=LoadString(IDS_FILEISMP3CBR);
												MessageBox(cStr,cstrInformation,MB_OK | MB_ICONINFORMATION);
											}
											asi->dwType=AUDIOTYPE_MP3CBR;
										}
										else
										{
											if ((ofOptions.dwFlags&SOFO_MP3_RESULTDLG)==SOFO_MP3_RESULTDLG)	{
												cStr=LoadString(IDS_FILEISMP3VBR);
												MessageBox(cStr,cstrInformation,MB_OK | MB_ICONINFORMATION);
											}
										}
									}
								}
							}
							asi->lpdwFiles = new DWORD[2];
							asi->lpdwFiles[0]=1;
							asi->lpdwFiles[1]= fi->file_id;
							asi->dwFlags |= ASIF_ALLOCATED;
							fi->bInUse = 1;

							FillMP3_ASI(&asi,mp3source);
							fi->bAddedImmediately=1;
							asi->bNameFromFormatTag = true;
						} else {
							fi->bAddedImmediately=0;
						}

					/*	cStr2=LoadString(STR_KBYTE);
						cStr=LoadString(IDS_FILE);
						Buffer[0]=0;
						wsprintf(Buffer,"%s %d, MP3-%s: %d %s, Ch: %d, Fr.: %d, Begin: %d",cStr,
							iIndex1+1,mp3source->IsCBR()?"CBR":"VBR",
							(int)(asi->audiosource->GetSize()>>10),cStr2,
							(int)mp3source->GetChannelCount(),(int)mp3source->GetFrequency(),
							(int)mp3source->GetOffset());
						
						*/
						//m_Enh_Filelist.SetItemText(iIndex_Enh,2,"MP3");

					}
					break;
				case FILETYPE_OGGVORBIS:
					{
						oggfile = new OGGFILE();
						oggfile->Open(cachesource, OGG_OPEN_READ);

						if (oggfile->GetNumberOfStreams() != 1) {
							fi->dwType = FILETYPE_UNKNOWN;
							oggfile->Close();
							delete oggfile;
							oggfile = NULL;

							MessageBoxUTF8(0, LoadString(STR_ERR_OMG, LOADSTRING_UTF8),
								LoadString(IDS_ERROR, LOADSTRING_UTF8), MB_OK | MB_ICONERROR);

						} else {
							fi->OGGFile = oggfile;

							vorbis = new VORBISFROMOGG;
							vorbis->Open(oggfile);
						
							asi = NULL;
							if (bAddAS_immed) {
								asi = new AUDIO_STREAM_INFO;
								ZeroMemory(asi, sizeof(*asi));
								asi->audiosource = vorbis;
								asi->bNameFromFormatTag = true;
								asi->lpFormat = new byte[1<<16];
								asi->iFormatSize = vorbis->RenderSetupHeader(asi->lpFormat);
								asi->dwFlags = ASIF_ALLOCATED;
								asi->lpASH = new AVIStreamHeader;
								asi->dwType = AUDIOTYPE_VORBIS;
								asi->iSize = vorbis->GetSize();
								asi->lpdwFiles = new DWORD[2];
								asi->lpdwFiles[0]=1;
								asi->lpdwFiles[1]=fi->file_id;
								asi->dwFlags |= ASIF_ALLOCATED;
								fi->bInUse = 1;

								ZeroMemory(asi->lpASH, sizeof(AVIStreamHeader));
								fi->bAddedImmediately = 1;
							} else {
								fi->bAddedImmediately = 0;
								fi->VRBFile = vorbis;
							}
						}
						//sprintf(cFmt, "OGG/Vorbis");
						formatName = _T("OGG/Vorbis");
					}
					break;
				case FILETYPE_AC3:
					{
						ac3source=new AC3SOURCE;
						if (!(ac3source->Open(cachesource)))
						{
							cStr=LoadString(IDS_CANTOPENMP3);
							MessageBox(cStr,cstrError,MB_OK | MB_ICONERROR);
							delete mp3source;
							m_SourceFiles.DeleteString(iIndex1);
							return;
						} else {
							asi=NULL;
							if (bAddAS_immed) {
								FillAC3_ASI(&asi,ac3source);		
								cStr=LoadString(IDS_FILE);
								cStr2=LoadString(STR_KBYTE);
/*							wsprintf(Buffer,"%s %d, AC3: %d %s, Ch: %d, Br: %d, Fr: %d, Begin: %d",cStr.GetBuffer(255),iIndex1+1,
								(DWORD)(asi->audiosource->GetSize()>>10), cStr2.GetBuffer(255),
								ac3source->GetChannelCount(),
								ac3source->GetBitrate(),ac3source->GetFrequency(),ac3source->GetSource()->GetOffset());
							
							m_Enh_Filelist.SetItemText(iIndex_Enh,2,"AC3");*/
								asi->bNameFromFormatTag = true;
								asi->lpdwFiles = new DWORD[2];
								asi->lpdwFiles[0]=1;
								asi->lpdwFiles[1]=fi->file_id;
								asi->dwFlags |= ASIF_ALLOCATED;
								fi->bInUse = 1;

							} else {
								fi->bAddedImmediately = 0;
								fi->AC3File = ac3source;
							}
							
						}
						//sprintf(cFmt, "AC3");
						formatName = _T("AC3");
					}
					break;
				case FILETYPE_DTS:
					{
						dtssource=new DTSSOURCE;
						if (!(dtssource->Open(cachesource)))
						{
							cStr=LoadString(IDS_CANTOPENMP3);
							MessageBox(cStr,cstrError,MB_OK | MB_ICONERROR);
							delete mp3source;
							m_SourceFiles.DeleteString(iIndex1);
							return;
						}

						asi=NULL;
						if (bAddAS_immed) {
							FillDTS_ASI(&asi,dtssource);		
							cStr=LoadString(IDS_FILE);
							cStr2=LoadString(STR_KBYTE);
						/*	wsprintf(Buffer,"%s %d, DTS: %d %s, Ch: %d, Br: %d, Fr: %d, Begin: %d",cStr.GetBuffer(255),iIndex1+1,
								(DWORD)(asi->audiosource->GetSize()>>10),cStr2.GetBuffer(255),
								dtssource->GetChannelCount(),
								(DWORD)dtssource->GetBitrate(),dtssource->GetFrequency(),dtssource->GetSource()->GetOffset());
							
							m_Enh_Filelist.SetItemText(iIndex_Enh,2,"DTS");*/
							asi->bNameFromFormatTag = true;
							asi->lpdwFiles = new DWORD[2];
							asi->lpdwFiles[0]=1;
							asi->lpdwFiles[1]=fi->file_id;
							asi->dwFlags |= ASIF_ALLOCATED;
							fi->bInUse = 1;
						} else {
							fi->bAddedImmediately = 0;
							fi->DTSFile = dtssource;
						}
						formatName = _T("DTS");
					}
					break;
				case FILETYPE_AAC:
					{
						aacsource = new AACSOURCE;
						if (!(aacsource->Open(cachesource))) {
							cStr=LoadString(IDS_CANTOPENMP3);
							MessageBox(cStr,cstrError,MB_OK | MB_ICONERROR);
							delete aacsource;
							m_SourceFiles.DeleteString(iIndex1);
							return;
						}
						cStr=LoadString(IDS_FILE);
						cStr2=LoadString(STR_KBYTE);

						if (bAddAS_immed) {
							FillAAC_ASI(&asi, aacsource);
							TCHAR Buffer[65536]; Buffer[0]=0;
							_stprintf_s(Buffer, _T("%s %d, AAC: Fr: %d, MPEG: %d"),cStr.GetBuffer(255),iIndex1+1,
								(DWORD)(asi->audiosource->GetSize()>>10),cStr2.GetBuffer(255),
								aacsource->GetFrequency(), aacsource->FormatSpecific(MMSGFS_AAC_MPEGVERSION));
							messageBuffer = Buffer;
							m_Enh_Filelist.SetItemText(iIndex_Enh,2, _T("AAC"));
							asi->bNameFromFormatTag = true;

							DWORD dwStatus;
							HANDLE hCheckSemaphore;
							aacsource->PerformCFRCheck(&hCheckSemaphore, &dwStatus);

							SetDialogState_Muxing();
							m_Prg_Progress.SetRange32(0,1000);
							while (WaitForSingleObject(hCheckSemaphore, 100) == WAIT_TIMEOUT){
								m_Prg_Progress.SetPos(dwStatus);
								UpdateWindow();
							}

							CloseHandle(hCheckSemaphore);
	
							SetDialogState_Config();

							if (!aacsource->IsCFR()) {
								MessageBox(_T("It is not recommended to put this AAC file into an AVI file!"),
									(TCHAR*)LoadString(STR_GEN_WARNING), MB_OK | MB_ICONWARNING);
							}

							fi->bAddedImmediately = 1;

							asi->lpdwFiles = new DWORD[2];
							asi->lpdwFiles[0]=1;
							asi->lpdwFiles[1]=fi->file_id;
							asi->dwFlags |= ASIF_ALLOCATED;
							fi->bInUse = 1;
						} else {
							fi->bAddedImmediately = 0;
							fi->AACFile = aacsource;
						}
						formatName = _T("AAC");
					}
					break;
				case FILETYPE_WAV:
					{
						wavfile=new WAVEFILE;
						wavfile->Open(cachesource, WAVE_OPEN_READ);
						fi->lpwav=wavfile;
						asi=new AUDIO_STREAM_INFO;
						ZeroMemory(asi,sizeof(AUDIO_STREAM_INFO));
						wavsource = new WAVSOURCE();
						wavsource->Open(wavfile);
						asi->audiosource=wavsource;
						asi->dwFlags|=ASIF_ALLOCATED;
						asi->lpASH=new AVIStreamHeader;
						ZeroMemory(asi->lpASH,sizeof(AVIStreamHeader));
						asi->lpASH->dwScale=wavfile->GetGranularity();
						asi->lpASH->dwRate=wavfile->GetStreamFormat()->nAvgBytesPerSec;
						asi->lpASH->dwSampleSize=wavfile->GetGranularity();
						asi->lpASH->fccType=MakeFourCC("auds");
						asi->lpFormat=new byte[(wavfile->GetStreamFormat()->cbSize+sizeof(WAVEFORMATEX))];
						memcpy(asi->lpFormat,wavfile->GetStreamFormat(),wavfile->GetStreamFormat()->cbSize+sizeof(WAVEFORMATEX));
						cStr=LoadString(IDS_FILE);
						
						WAVEFORMATEX* lpwfe = (WAVEFORMATEX*)asi->lpFormat;
						if (lpwfe->wFormatTag==1) {
							asi->dwType = AUDIOTYPE_PCM;
						} else {
							asi->dwType=AUDIOTYPE_PLAINCBR;
						}

						cStr2=LoadString(STR_KBYTE);
						TCHAR Buffer[65536]; Buffer[0]=0;
						_stprintf(Buffer, _T("%s %d: %d %s"), cStr.GetBuffer(255), iIndex1+1,
							(DWORD)(asi->audiosource->GetSize()>>10),cStr2.GetBuffer(255));
						messageBuffer = Buffer;
						m_Enh_Filelist.SetItemText(iIndex_Enh,2, _T("WAV"));
						asi->bNameFromFormatTag = true;

						asi->lpdwFiles = new DWORD[2];
						asi->lpdwFiles[0]=1;
						asi->lpdwFiles[1]=fi->file_id;

						formatName = _T("WAV");
					}
					break;
				case FILETYPE_SUBS:
					{
						subs=new SUBTITLES;
						subs->Open(new CTextFile(StreamMode::Read, fi->source, 
							CharacterEncoding::UTF8));

						lpssi=new SUBTITLE_STREAM_INFO;
						lpssi->lpfi=fi;
						fi->bInUse = true;
						lpssi->lpsubs=subs;
						lpssi->lpash=new AVIStreamHeader;
						lpssi->lpash->fccType=MakeFourCC("txts");

						lpssi->lpdwFiles = new DWORD[2];
						lpssi->lpdwFiles[0]=1;
						lpssi->lpdwFiles[1]=fi->file_id;
						
						cStr=LoadString(IDS_FILE);
						cStr2=LoadString(STR_KBYTE);

						char f[65536]; f[0]=0;
						name_without_ext(lpcName, f); 			

						lpssi->lpsubs->SetName(f);


						if (subs->GetFormat() == SUBFORMAT_SRT)
							formatName = _T("SRT");
						if (subs->GetFormat() == SUBFORMAT_SSA)
							formatName = _T("SSA");

						AddSubtitleStream(lpssi);
					}
					break;
				case FILETYPE_XML:
					{
						int res = 0;

						if ((res = chapters->ImportFromXML(xmlTree)) == CHAP_IMPXML_OK) {
							m_SourceFiles.DeleteString(iIndex1);
						} else
						if (res == CHAP_IMPXML_NONUNIQUE_UID) {
							MessageBox(
								(TCHAR*)LoadString(STR_ERR_IMPCHAP_NONUNIQUEUID),
								(TCHAR*)LoadString(IDS_ERROR), MB_OK | MB_ICONERROR);
						} else if (res == CHAP_IMPXML_NO_CHAPTER_FILE) {
							settings->Import(xmlTree);
							Attribs(settings->GetAttr("gui/main_window"));
							ReinitPosition();
							m_SourceFiles.DeleteString(iIndex1);
						}

						filesource->Close();
						delete filesource;
						fi->file = NULL;
						xmlDeleteNode(&xmlTree);
						formatName = _T("XML");

					}
					break;
			}

			strcpy(fi->cFileformatString, CUTF8(formatName.c_str()).Str());
			RECT r;
			m_SourceFiles.GetItemRect(iIndex1, &r);
			m_SourceFiles.InvalidateRect(&r);
		}
		
		if (fi->dwType==FILETYPE_UNKNOWN) {
			m_SourceFiles.DeleteString(iIndex1);
			if (!LoadScript(_lpcName,m_hWnd,GetUserMessageID()))	{
				cStr=LoadString(IDS_FILETYPENOTSUPPORTED);
				TCHAR Buffer[65536]; Buffer[0]=0;
				_stprintf(Buffer,cStr);
				MessageBox(Buffer,cstrError,MB_OK | MB_ICONERROR);
				fi->file->Close();
				delete fi->file;
				fi->file = NULL;
			} else {
				fi->file->Close();
				delete fi->file;
				fi->file = NULL;
				DecBufferRefCount(&cb);
			}
		} else {
			std::basic_ostringstream<TCHAR> sstrMsg;
			sstrMsg << " loaded file: " << CUTF8(lpcName).TStr();
			std::basic_string<TCHAR> strMsg = sstrMsg.str();
			const TCHAR* cMsg = strMsg.c_str();
			m_StatusLine.SetWindowText(cMsg);
			m_StatusLine.InvalidateRect(NULL);
			m_StatusLine.UpdateWindow();
		}
		if (asi && asi->audiosource) {
			char f[65536]; f[0]=0;
			name_without_ext(lpcName, f);

			asi->audiosource->SetName(f);
			ucase(f, f);
			char* cDelayPos = strstr(f,"DELAY");
			if (cDelayPos) cDelayPos+=6;
			char* cMSPos = strstr(f,"MS");
			if (cMSPos) *cMSPos=0;
			if (cDelayPos && cMSPos) SetDelay(asi,atoi(cDelayPos));

			AddAudioStream(asi);
		}
		

	if (m_SourceFiles.GetCount()) {
		m_Add_Video_Source.EnableWindow(1);
	}
}
