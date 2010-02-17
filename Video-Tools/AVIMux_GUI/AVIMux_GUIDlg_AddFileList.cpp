#include "stdafx.h" 
#include "avimux_guidlg.h"
#include "formattext.h"
#include "avistream.h"
#include "..\UnicodeCalls.h"
#include "videosource_avi.h"
#include "videosource_matroska.h"
#include "UTF8Windows.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DWORD* DuplicateFileUsageList(DWORD* in)
{
	int size;
	DWORD* res = (DWORD*)malloc(size = sizeof(DWORD) * (in[0] + 1));
	memcpy(res, in, size);
	return res;
}

bool _is_equal(int v1, int v2)
{
	return v1 == v2;
}

bool _is_equal(char* c1, char* c2)
{
	return !strcmp(c1, c2);
}

bool _is_equal(const std::string& c1, const std::string& c2)
{
	return (c1 == c2);
}

template <class T>bool CheckForEquality(int msg_index, 
					  T v1, T v2, 
					  int n1, int n2,
					  const char* f1, const char* f2,
					  int final, char* final_error_message)
{
	char c[65536];
	sprintf(c, LoadString(msg_index, LOADSTRING_UTF8),
		n1, f1, v1,
		n2, f2, v2);

	char* hdl = NULL;
	char* u = NULL;
	bool bBad = false;
	fromUTF8(LoadString(STR_GEN_ERROR, LOADSTRING_UTF8), &hdl);

	if (!_is_equal(v1, v2)) {
		bBad = true;
		if (final_error_message) {
			strcat(final_error_message, c);
			strcat(final_error_message, "\x0D\x0D");
		}
	}

	if (final) {
		if (final_error_message[0]) {
			fromUTF8(final_error_message, &u);
			(*UMessageBox())(0, u, hdl, MB_OK | MB_ICONERROR);
			free(hdl);
			free(u);
			final_error_message[0]=0;
			return false;
		} else {
			if (bBad) {
				char* u = NULL;
				fromUTF8(c, &u);
				(*UMessageBox())(0, u, hdl, MB_OK | MB_ICONERROR);
				free(u);
			}
		}
	}

	free(hdl);
	return true;
}

bool CheckSampleRates(AUDIOSOURCE* a1, AUDIOSOURCE* a2, int n1, int n2,
					  const char* f1, const char* f2, int final, char* fem)
{
	return CheckForEquality(STR_ERR_SAMPLERATESBAD, a1->GetFrequency(),
		a2->GetFrequency(), n1, n2, f1, f2, final, fem);
}

bool CheckChannels(AUDIOSOURCE* a1, AUDIOSOURCE* a2, int n1, int n2,
				   const char* f1, const char* f2, int final, char* fem)
{
	return CheckForEquality(STR_ERR_CHANNELCOUNTBAD, a1->GetChannelCount(),
		a2->GetChannelCount(), n1, n2, f1, f2, final, fem);
}

bool CheckMPEGLayerVersion(MP3SOURCE* a1, MP3SOURCE* a2, int n1, int n2,
						   const char* f1, const char* f2, int final, char* fem)
{
	return CheckForEquality(STR_ERR_BADLAYERVERSIONS, a1->GetLayerVersion(),
		a2->GetLayerVersion(), n1, n2, f1, f2, final, fem);
}

bool CheckAACProfileVersion(AACSOURCE* a1, AACSOURCE* a2, int n1, int n2,
							const char* f1, const char* f2, int final, char* fem)
{
	std::string p1;
	std::string p2;
	a1->GetProfileString(p1);
	a2->GetProfileString(p2);
	return CheckForEquality(STR_ERR_BADAACPROFILES, p1.c_str(), p2.c_str(), n1, n2,
		f1, f2, final, fem);
}
/*bool CheckMPEGVersion(MP3SOURCE* a1, MP3SOURCE* a2, int n1, int n2,
					  char* f1, char* f2, int final, char* fem)
{
	return CheckForEquality(STR_ERR_BADMPEGVERSIONS, a1->GetMPEGVersion(),
		a2->GetMPEGVersion(), n1, n2, f1, f2, final, fem);
}
*/
template<class T>bool CheckMPEGVersion(T* a1, T* a2, int n1, int n2,
					  const char* f1, const char* f2, int final, char* fem)
{
	return CheckForEquality(STR_ERR_BADMPEGVERSIONS, a1->GetMPEGVersion(),
		a2->GetMPEGVersion(), n1, n2, f1, f2, final, fem);
}

void CAVIMux_GUIDlg::OnAddFileList() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
	if (bEditInProgess) return;
	int						dwNbrOfFiles,dwNbrOfSource;
	int						i,j;
	int						dwNbrOfSubs;
	int						iAudioStreamCount;
	DWORD*					dwSelectedFiles;

	VIDEO_STREAM_INFO*		lpvsi;
	VIDEOSOURCELIST*		lpvsl;

	AVISTREAM*				lpAS;

	AUDIOSOURCE**			lpAudiosources;
	AUDIOSOURCELIST**		lpASL;
	SUBTITLESOURCELIST**	lpSSL;

	DWORD*					lpdwSubtitleList;
	SUBTITLES**				subs;
	SUBTITLES*				temp_subs;
	FILE_INFO*				fi;
	CString					cStr,cStr2;
	__int64					qwBias;
	DWORD					dwSize;

	char					Buffer[65536];
	char					cMessage[65536];

	bool					bVFR = false;
	bool					bAVIOutputPossible = true;
	bool					bNoVideo = false;
	bool					bChaptersFromFiles = !!(ofOptions.dwFlags & SOFO_CH_FROMFILENAMES);
	bool					bImportChapters = !!(ofOptions.dwFlags & SOFO_CH_IMPORT);

	int bAVI = false; int bMKV = false; bool bCrap = false; int bMP3 = false;
	int bAAC = false; int bAC3 = false; bool bDTS = false; int bOVRB = false;
	
	AUDIO_STREAM_INFO**		asi;
	SUBTITLE_STREAM_INFO**	ssi;
	bool				bNoMP3CBR;

// wieviele gewählt?
	dwNbrOfFiles=SendDlgItemMessage(IDC_SOURCEFILELIST,LB_GETSELCOUNT,0,0);
	if (!dwNbrOfFiles)
	{
		cStr=LoadString(IDS_NOVIDEOSOURCECHOSEN);
		MessageBox(cStr,cstrError,MB_OK | MB_ICONERROR);
		return;
	}
// welche?
	dwSelectedFiles=new DWORD[1+dwNbrOfFiles];
	dwSelectedFiles[0]=dwNbrOfFiles;

	DWORD* dwSelectedFilesIndexes = new DWORD[1+dwNbrOfFiles];
	SendDlgItemMessage(IDC_SOURCEFILELIST,LB_GETSELITEMS,dwNbrOfFiles,
		(LPARAM)&(dwSelectedFilesIndexes[1]));
	for (i=0;i<dwNbrOfFiles;i++) {
		FILE_INFO* fi = (FILE_INFO*)((CBuffer*)m_SourceFiles.GetItemData(dwSelectedFilesIndexes[i+1]))->GetData();
		dwSelectedFiles[i+1] = fi->file_id;
	}
	delete dwSelectedFilesIndexes;

	if (dwNbrOfFiles==1) bChaptersFromFiles = false;
// join matroska, mp3 or avi files, but no mixture
	{
		for (i=1;i<=dwNbrOfFiles;i++)
		{
			fi = m_SourceFiles.GetFileInfoFromID(dwSelectedFiles[i]);
			if (fi->dwType & FILETYPE_AVI) {
				bAVI = 1;
			} else 
			if (fi->dwType & FILETYPE_MKV) {
				bMKV = 1;
			} else 
			if (fi->dwType & FILETYPE_MP3) {
				bMP3 = 1;
			} else 
			if (fi->dwType & FILETYPE_AAC) {
				bAAC = 1;
			} else
			if (fi->dwType & FILETYPE_AC3) {
				bAC3 = 1;
			} else 
			if (fi->dwType & FILETYPE_DTS) {
				bDTS = 1;
			} else
			if (fi->dwType & FILETYPE_OGGVORBIS) {
				bOVRB = 1;
			} else
				bCrap = 1;

			if (fi->bInUse) {
				char msg[1024]; msg[0] = 0;
				char* c;
				c = LoadString(STR_ERR_FILEINUSE, LOADSTRING_UTF8);
				sprintf(msg, c, fi->Name.UTF8());
				MessageBoxUTF8(m_hWnd, msg, LoadString(STR_GEN_ERROR),
					MB_OK | MB_ICONERROR);
				return;
			}
		}
		if (bCrap || (bAVI + bMKV + bMP3 + bAAC + bAC3 + bDTS + bOVRB> 1)) {
			MessageBox(LoadString(STR_ERR_LISTINCOMPATIBLE),
				LoadString(STR_GEN_ERROR), MB_OK | MB_ICONERROR);
			return;
		}
		if (bOVRB && dwNbrOfFiles>1) {
			MessageBox("Joining OGG/Vorbis files is not yet supported!", 
				LoadString(STR_GEN_ERROR), MB_OK | MB_ICONERROR);
			return;
		}
	}

	if (bMP3) {
		MP3SOURCE* m1, *m2;
		fi = m_SourceFiles.GetFileInfoFromID(dwSelectedFiles[1]);
		FILE_INFO* fi1 = fi;
		m1 = fi->MP3File;

		for (i=2;i<=dwNbrOfFiles;i++)
		{
			fi = m_SourceFiles.GetFileInfoFromID(dwSelectedFiles[i]);
			m2 = fi->MP3File;

			bool bSuccess = true;
			char errmsg[65536]; errmsg[0]=0;

			bSuccess &= CheckMPEGVersion(m1, m2, 1, i, fi1->Name.UTF8(), 
				fi->Name.UTF8(), 0, errmsg);
			bSuccess &= CheckSampleRates(m1, m2, 1, i, fi1->Name.UTF8(), 
				fi->Name.UTF8(), 0, errmsg);
			bSuccess &= CheckChannels(m1, m2, 1, i, fi1->Name.UTF8(), 
				fi->Name.UTF8(), 0, errmsg);
			bSuccess &= CheckMPEGLayerVersion(m1, m2, 1, i, fi1->Name.UTF8(), 
				fi->Name.UTF8(), 1, errmsg);

			if (!bSuccess)
					return;
		}
	}

	if (bAAC) {
		AACSOURCE* m1, *m2;
		fi = m_SourceFiles.GetFileInfoFromID(dwSelectedFiles[1]);
		FILE_INFO* fi1 = fi;
		m1 = fi->AACFile;

		for (i=2;i<=dwNbrOfFiles;i++) {
			fi = m_SourceFiles.GetFileInfoFromID(dwSelectedFiles[i]);
			m2 = fi->AACFile;

			bool bSuccess = true;
			char errmsg[65536]; errmsg[0]=0;

			bSuccess &= CheckMPEGVersion(m1, m2, 1, i, fi1->Name.UTF8(), 
				fi->Name.UTF8(), 0, errmsg);
			bSuccess &= CheckSampleRates(m1, m2, 1, i, fi1->Name.UTF8(), 
				fi->Name.UTF8(), 0, errmsg);
			bSuccess &= CheckChannels(m1, m2, 1, i, fi1->Name.UTF8(),
				fi->Name.UTF8(), 0, errmsg);
			bSuccess &= CheckAACProfileVersion(m1, m2, 1, i, fi1->Name.UTF8(), 
				fi->Name.UTF8(), 1, errmsg);

			if (!bSuccess)
					return;
		}
	}

	if (bAC3 || bDTS) {
		AUDIOSOURCE* m1, *m2;
		fi = m_SourceFiles.GetFileInfoFromID(dwSelectedFiles[1]);
		FILE_INFO* fi1 = fi;
		m1 = fi->AC3File;

		for (i=2;i<=dwNbrOfFiles;i++)
		{
			fi = m_SourceFiles.GetFileInfoFromID(dwSelectedFiles[i]);
			m2 = fi->AC3File;

			bool bSuccess = true;
			char errmsg[65536]; errmsg[0]=0;

			bSuccess &= CheckSampleRates(m1, m2, 1, i, fi1->Name.UTF8(), 
				fi->Name.UTF8(), 0, errmsg);
			bSuccess &= CheckChannels(m1, m2, 1, i, fi1->Name.UTF8(),
				fi->Name.UTF8(), 1, errmsg);

			if (!bSuccess)
					return;
		}
	}

	// check wether list of AVI files is valid
	if (bAVI)
	{
		AVIFILEEX* a1, *a2;

		fi = m_SourceFiles.GetFileInfoFromID(dwSelectedFiles[1]);
		a1 = fi->AVIFile;

		for (i=2;i<=dwNbrOfFiles;i++)
		{
			fi = m_SourceFiles.GetFileInfoFromID(dwSelectedFiles[i]);
			a2 = fi->AVIFile;

			// check if number of streams is the same
			if (a1->GetNbrOfStreams() != a2->GetNbrOfStreams()) {
				MessageBox(LoadString(STR_ERR_STREAMCOUNTDIFFERS),LoadString(STR_GEN_ERROR),MB_OK | MB_ICONERROR);
				return;
			}

			// check if video resolution is the same
			int dwX[2]; int dwY[2];
			a1->GetVideoResolution(&dwX[0],&dwY[0]);
			a2->GetVideoResolution(&dwX[1],&dwY[1]);
			if (dwX[0] != dwX[1] || dwY[0] != dwY[1]) {
				MessageBox(LoadString(STR_ERR_VIDEORESOLUTIONDIFFERS),
					LoadString(STR_GEN_ERROR),MB_OK | MB_ICONERROR);
				return;
			}
			
			// check if video compression formats are the same
			if (a1->GetFormatTag(0) != a2->GetFormatTag(0)) {
				__int64 q1 = a1->GetFormatTag(0); __int64 q2 = a2->GetFormatTag(0);
				wsprintf(cMessage,LoadString(STR_ERR_VIDEOCOMPRDIFFERS),&q1,&q2);
				MessageBox(cMessage,LoadString(STR_GEN_ERROR),MB_OK | MB_ICONERROR);
				return;
			}

			// check if framerate is the same. If it is not, only MKV output is possible 
			if (!bVFR && fabs((double)a1->GetNanoSecPerFrame() - (double)a2->GetNanoSecPerFrame()) > 1000) {
				bVFR = true;
				MessageBox(LoadString(STR_HINT_VFR),
					LoadString(STR_GEN_INFORMATION), MB_OK | MB_ICONINFORMATION);
			}	
		}
	}

	// check mkv list
	if (bMKV) {
		MATROSKA* m1, *m2;

		fi = m_SourceFiles.GetFileInfoFromID(dwSelectedFiles[1]);
		m1 = fi->MKVFile;

		for (i=2;i<=dwNbrOfFiles;i++) {
			fi = m_SourceFiles.GetFileInfoFromID(dwSelectedFiles[i]);
			m2 = fi->MKVFile;

			// check if number of streams is the same
			if (m1->GetTrackCount() != m2->GetTrackCount()) {
				MessageBox(LoadString(STR_ERR_STREAMCOUNTDIFFERS),LoadString(STR_GEN_ERROR),MB_OK | MB_ICONERROR);
				return;
			}

			// check if video compression formats are the same
			if (m1->SetActiveTrack(0, SAT_VIDEO) > -1 && m2->SetActiveTrack(0, SAT_VIDEO) > -1) {
				if (strcmp(m1->GetCodecID(), m2->GetCodecID())) {
					wsprintf(cMessage,LoadString(STR_ERR_CODECIDDIFFERS_S),m1->GetCodecID(),m2->GetCodecID());
					MessageBox(cMessage,LoadString(STR_GEN_ERROR),MB_OK | MB_ICONERROR);
					return;
				}

				if (!strcmp(m1->GetCodecID(),"V_MS/VFW/FOURCC")) {
					BITMAPINFOHEADER* h1 = (BITMAPINFOHEADER*)m1->GetCodecPrivate();
					BITMAPINFOHEADER* h2 = (BITMAPINFOHEADER*)m2->GetCodecPrivate();

					if (h1->biCompression != h2->biCompression) {
						__int64 q1 = h1->biCompression;
						__int64 q2 = h2->biCompression;

						wsprintf(cMessage,LoadString(STR_ERR_VIDEOCOMPRDIFFERS),&q1,&q2);
						MessageBox(cMessage,LoadString(STR_GEN_ERROR),MB_OK | MB_ICONERROR);
						return;
					}
				}

				// check resolution
				int dwX[2]; int dwY[2];
				m1->GetResolution(&dwX[0],&dwY[0], NULL, NULL, NULL);
				m2->GetResolution(&dwX[1],&dwY[1], NULL, NULL, NULL);
				if (dwX[0] != dwX[1] || dwY[0] != dwY[1]) {
					MessageBox(LoadString(STR_ERR_VIDEORESOLUTIONDIFFERS),LoadString(STR_GEN_ERROR),MB_OK | MB_ICONERROR);
					return;
				}
			}

		}
	}

	// create secondary video source class list
	lpvsl = new VIDEOSOURCELIST();

	// build AUDIO_STREAM_INFO structure from AVI sources
	if (fi->dwType & FILETYPE_AVI) {

		iAudioStreamCount=fi->AVIFile->GetAudioStreamCount();
		newz(AUDIOSOURCE*,iAudioStreamCount,lpAudiosources); 
		newz(AUDIOSOURCELIST*,iAudioStreamCount,lpASL);
		newz(AUDIO_STREAM_INFO*,iAudioStreamCount,asi);

		for (i=0;i<iAudioStreamCount;i++) {
			lpASL[i] = new AUDIOSOURCELIST();

			// allocate memory for stream headers and stream format
			asi[i]=new AUDIO_STREAM_INFO;
			asi[i]->dwType=0;
			asi[i]->dwFlags=ASIF_AVISOURCE | ASIF_ALLOCATED;
			asi[i]->iDelay=0;

			// get old headers from the AVI file
			asi[i]->lpASH=new AVIStreamHeader;
			memcpy(asi[i]->lpASH,fi->AVIFile->GetStreamHeader(i+1),sizeof(AVIStreamHeader));
			asi[i]->lpASH->dwSuggestedBufferSize = 0;
			asi[i]->lpFormat=new char[(dwSize=fi->AVIFile->strfSize(i+1,fi->AVIFile->GetStreamFormat(i+1)))];
			asi[i]->iFormatSize = dwSize;
			memcpy(asi[i]->lpFormat,fi->AVIFile->GetStreamFormat(i+1),dwSize);
		}
	}

	// build video source
	bNoMP3CBR=false;
	int edition_index = -1;
	if (bChaptersFromFiles && dwNbrOfFiles>1) {
		SINGLE_CHAPTER_DATA	scd; memset(&scd,0,sizeof(scd));
		scd.bIsEdition = 1;
		edition_index = chapters->AddChapter(&scd);
	}

	for (i=1;i<=dwNbrOfFiles;i++)
	{
		fi = m_SourceFiles.GetFileInfoFromID(dwSelectedFiles[i]);

		char* cNoExt = new char[512]; cNoExt[0]=0;

		name_without_ext(fi->Name.UTF8(), cNoExt);

		// if source is set of AVIs
		if (fi->dwType & FILETYPE_AVI) {
			VIDEOSOURCEFROMAVI*	 lpvsfa = new VIDEOSOURCEFROMAVI();
			lpvsfa->Open(fi->AVIFile);
			lpvsl->Append(lpvsfa);
			
			// create chapter if necessary
			
 			if (bChaptersFromFiles) chapters->GetSubChapters(edition_index)->AddChapter(
				lpvsfa->GetBias(BIAS_UNSCALED),-1,cNoExt);

		}
		if (fi->dwType & FILETYPE_MKV) {
			VIDEOSOURCEFROMMATROSKA* lpvsfm = new VIDEOSOURCEFROMMATROSKA(fi->MKVFile,-1);
			if (lpvsfm->IsOpen()) {
				lpvsl->Append(lpvsfm);

				// create chapter if necessary, and import the chapters in the file as subchapters
				if (bChaptersFromFiles) {

					chapters->GetSubChapters(edition_index)->AddChapter(lpvsfm->GetBias(BIAS_UNSCALED),-1,cNoExt);
					/*if (bImportChapters) {
						chapters->GetSubChapters(CHAP_LAST)->Import(new CChapters(fi->MKVFile->GetChapterInfo()),
							lpvsfm->GetBias(BIAS_UNSCALED), CHAP_IMPF_DELETE);

					}*/
					if (bImportChapters) {
						CChapters* c = new CChapters(fi->MKVFile->GetChapterInfo());
						for (int j=0;j<c->GetChapterCount();j++) {
							if (c->IsEdition(j)) {
								chapters->GetSubChapters(chapters->AddEmptyEdition())->
									Import(c->GetSubChapters(j), lpvsfm->GetBias(BIAS_UNSCALED));
							}
						}
					}
				} else
				if (bImportChapters) {
					CChapters* c = new CChapters(fi->MKVFile->GetChapterInfo());
					chapters->Import(c, lpvsfm->GetBias(BIAS_UNSCALED));
				}
				
			} else {
				bNoVideo = true;
				if (bImportChapters) {
					chapters->Import(new CChapters(fi->MKVFile->GetChapterInfo()),
						lpvsfm->GetBias(BIAS_UNSCALED), CHAP_IMPF_DELETE);
				}

			}
		}
		bNoMP3CBR|=fi->bMP3VBF_forced;
		delete cNoExt;
//		delete cPath;
	}

	if (fi->dwType & FILETYPE_MP3 || fi->dwType & FILETYPE_AAC
		|| fi->dwType & FILETYPE_AC3 || fi->dwType & FILETYPE_DTS 
		|| fi->dwType & FILETYPE_OGGVORBIS) {
		char* cNoExt = new char[512];
		int   time;
		char  cTime[20];
		void* p = malloc(1<<20);
		AUDIOSOURCE* a;

		lpAudiosources = new AUDIOSOURCE*[1];
		lpAudiosources[0] = new AUDIOSOURCE;
		lpASL = new AUDIOSOURCELIST*[1];
		lpASL[0] = new AUDIOSOURCELIST;
		asi= new AUDIO_STREAM_INFO*[1];
		newz(AUDIO_STREAM_INFO, 1, asi[0]);
	//	newz(DWORD, 1+dwNbrOfFiles, asi[0]->lpdwFiles);
	//	asi[0]->lpdwFiles[0] = dwNbrOfFiles;
		m_StatusLine.SetWindowText("checking duration of source files...");
		m_StatusLine.InvalidateRect(NULL);
		m_StatusLine.UpdateWindow();
		__int64 itc = 0;
		time = GetTickCount();

		CChapters* edition = NULL;
		if (dwNbrOfFiles > 1 && bChaptersFromFiles) {
//			chapters->AddEmptyEdition();
			edition = chapters->GetSubChapters(edition_index);
		}

		for (i=1;i<=dwNbrOfFiles;i++) {
		//	asi[0]->lpdwFiles[i] = dwSelectedFiles[i];
			fi = m_SourceFiles.GetFileInfoFromID(dwSelectedFiles[i]);
			if (bMP3) a = fi->MP3File;
			if (bAAC) a = fi->AACFile;
			if (bAC3) a = fi->AC3File;
			if (bDTS) a = fi->DTSFile;
			if (bOVRB) a = fi->VRBFile;
			lpASL[0]->Append(a);
			name_without_ext(fi->Name.UTF8(), cNoExt);

			if (dwNbrOfFiles > 1 && bChaptersFromFiles) {
				edition->AddChapter(itc, -1, cNoExt);
				if (1/*i!=dwNbrOfFiles*/) {
					__int64 r = 0;
					__int64 itc_begin = itc;
					while (!a->IsEndOfStream()) {
						a->Read(p, 1000, NULL, &r, NULL, NULL);
						itc += r;
						if (GetTickCount()-time>1000) {
							time+=1000;
							Millisec2Str(itc/1000000, cTime);
							sprintf(cMessage, "processing %s, total position: %s",
								fi->Name.UTF8(), cTime);
							m_StatusLine.SetWindowText(cMessage);
							m_StatusLine.InvalidateRect(NULL);
							m_StatusLine.UpdateWindow();
						}
					}

					Millisec2Str((itc-itc_begin)/1000000, cTime);
					sprintf(cMessage, "Duration found: %s", cTime);
					AddProtocolLine(cMessage, 4);
					a->Seek(0);
				}
			}
		}
		lpASL[0]->JoinSeamless(true);
		m_StatusLine.SetWindowText("audio source created successfully ...");
		m_StatusLine.InvalidateRect(NULL);
		m_StatusLine.UpdateWindow();
		
		if (bMP3) {
			FillMP3_ASI(&asi[0], fi->MP3File);
		} else 
		if (bAAC) {
			FillAAC_ASI(&asi[0], fi->AACFile);
		} else
		if (bAC3) {
			FillAC3_ASI(&asi[0], fi->AC3File);
			if (!lpASL[0]->IsCBR()) {
				// do not allow AC3-VBR-in-AVI atm
				lpASL[0]->AllowAVIOutput(false);
			}
		} else
		if (bDTS) {
			FillDTS_ASI(&asi[0], fi->DTSFile);
			if (!lpASL[0]->IsCBR()) {
				// do not allow DTS-VBR-in-AVI atm
				lpASL[0]->AllowAVIOutput(false);
			}
		}
		if (bOVRB) {
			lpASL[0]->AllowAVIOutput(false);
			VORBISSOURCE* vorbis = fi->VRBFile;
			asi[0] = new AUDIO_STREAM_INFO;
			ZeroMemory(asi[0], sizeof(*(asi[0])));
			asi[0]->audiosource = vorbis;
			asi[0]->bNameFromFormatTag = true;
			asi[0]->lpFormat = new byte[1<<16];
			asi[0]->iFormatSize = vorbis->RenderSetupHeader(asi[0]->lpFormat);
			asi[0]->dwFlags = ASIF_ALLOCATED;
			asi[0]->lpASH = new AVIStreamHeader;
			asi[0]->dwType = AUDIOTYPE_VORBIS;
			asi[0]->iSize = vorbis->GetSize();
			asi[0]->lpdwFiles = new DWORD[2];
			asi[0]->lpdwFiles[0]=1;
			asi[0]->lpdwFiles[1]=m_SourceFiles.GetCount()-1;
			asi[0]->dwFlags |= ASIF_ALLOCATED;
			ZeroMemory(asi[0]->lpASH, sizeof(AVIStreamHeader));
		}
		asi[0]->audiosource = lpASL[0];
		asi[0]->bNameFromFormatTag = 1;

		asi[0]->lpdwFiles = DuplicateFileUsageList(dwSelectedFiles);
		AddAudioStream(asi[0]);
		bNoVideo = 1;

		delete cNoExt; delete p;
	}

	if (!bNoVideo) {
		cStr=LoadString(IDS_SOURCE);
		cStr2=LoadString(STR_KBYTE);
		char cDuration[20];
		Millisec2Str(lpvsl->GetDuration() * lpvsl->GetTimecodeScale() / 1000000,cDuration);
		wsprintf(Buffer,"%s %d: %d %s, %s",cStr.GetBuffer(255),
			1+(dwNbrOfSource=SendDlgItemMessage(IDC_AVAILABLEVIDEOSTREAMS,LB_GETCOUNT)),
			(DWORD)(lpvsl->GetSize()>>10),cStr2.GetBuffer(255),cDuration);

		lpvsi=new VIDEO_STREAM_INFO;
		ZeroMemory(lpvsi,sizeof(VIDEO_STREAM_INFO));

		lpvsi->lpdwFiles=DuplicateFileUsageList(dwSelectedFiles);
		lpvsi->videosource = lpvsl;
		lpvsl->Enable(0);

		AddVideoStream(lpvsi);
	}

	if (fi->dwType & FILETYPE_AVI) {
		for (j=0;j<fi->AVIFile->GetAudioStreamCount();j++)	{
			for (i=1;i<=dwNbrOfFiles;i++) {
				fi = m_SourceFiles.GetFileInfoFromID(dwSelectedFiles[i]);

				// check for special formats: mp3, ac3, dts, divx, pcm
				switch (fi->AVIFile->GetFormatTag(j+1)) {
					case 0x0001:
						lpAudiosources[j] = new AUDIOSOURCEFROMAVI(fi->AVIFile,j+1);
						sprintf(Buffer,"%s", "PCM");
						asi[j]->dwType = AUDIOTYPE_PCM;
						break;
					case 0x0161:
						lpAudiosources[j] = new AUDIOSOURCEFROMAVI(fi->AVIFile,j+1);
						sprintf(Buffer,"%s", "divX audio");
						asi[j]->dwType = AUDIOTYPE_DIVX;
						break;
					case 0x0050:
					case 0x0055:
						MP3SOURCE* mp3;
						mp3 = new MP3SOURCE();
						mp3->SetResyncRange(1048576);
						lpAudiosources[j] = mp3;
						mp3->Open(new AVISTREAM(fi->AVIFile,j+1));
						if (!IsMP3SampleCount(fi->AVIFile->GetStreamHeader(j+1)->dwScale))
							if (!bNoMP3CBR) 
								mp3->AssumeCBR();
						
						FillMP3_ASI(&asi[j],(MP3SOURCE*)lpAudiosources[j]);
						asi[j]->lpASH->dwScale = ((WAVEFORMATEX*)asi[j]->lpFormat)->nBlockAlign;
						break;
					case 0x2000:
						AC3SOURCE* ac3;
						ac3 = (AC3SOURCE*)(lpAudiosources[j] = new AC3SOURCE());
						ac3->SetResyncRange(1<<20);
						ac3->Open(new AVISTREAM(fi->AVIFile,j+1));
						FillAC3_ASI(&asi[j],ac3);
						sprintf(Buffer,"%s", "AC3");
						break;
					case AAC_WFORMATTAG:
						lpAudiosources[j] = new AACFROMAVI(fi->AVIFile,j+1);
						FillAAC_ASI(&asi[j],(AACSOURCE*)lpAudiosources[j]);
						sprintf(Buffer,"%s", "AAC");
						fi->AVIFile->GetStreamName(j+1, Buffer);
						lpAudiosources[j]->SetName(Buffer);
						break;
					case 0x2001:
						lpAudiosources[j] = new DTSSOURCE(new AVISTREAM(fi->AVIFile,j+1));
						FillDTS_ASI(&asi[j],(DTSSOURCE*)lpAudiosources[j]);
						sprintf(Buffer,"%s", "DTS");
						break;
					case 0x566F: {
						VORBISPACKETSFROMAVI* a = new VORBISPACKETSFROMAVI(fi->AVIFile, j+1);
						VORBISFROMOGG*        v = new VORBISFROMOGG;
						v->Open(a);
						asi[j]->lpFormat = calloc(1<<16, 1);
						asi[j]->iFormatSize = v->RenderSetupHeader(asi[j]->lpFormatMKV);

						lpAudiosources[j] = v; }
						asi[j]->dwType = AUDIOTYPE_VORBIS;
						asi[j]->bNameFromFormatTag = 0;
						break;
					default:
						lpAudiosources[j] = new AUDIOSOURCEFROMAVI(fi->AVIFile,j+1);
						sprintf(Buffer,"%s", "unknown CBR");
						asi[j]->dwType = AUDIOTYPE_CBR;
						break;

				}
				lpAudiosources[j]->SetMaxLength(fi->AVIFile->GetNanoSecPerFrame() * fi->AVIFile->GetFrameCount(),TIMECODE_UNSCALED);
				lpAudiosources[j]->SetDefault(fi->AVIFile->IsDefault(j+1));
				if (i==1) {
					int k = lpAudiosources[j]->GetOffset();
					int avg = fi->AVIFile->GetAvgBytesPerSec(j+1);
					int cbr = fi->AVIFile->IsCBR(j+1);
					AVIStreamHeader* ash = fi->AVIFile->GetStreamHeader(j+1);
					if (k && avg && cbr) {
						asi[j]->iDelay += 1000 * k / avg;
					} else {
						if (!cbr && k) {
							asi[j]->iDelay += fi->AVIFile->VBR_FrameCountTillPos(j+1,k) * 
								1000 * ash->dwScale / ash->dwRate;
						}
					}
					if (ash->dwStart && avg) {
						if (!cbr) {
							asi[j]->iDelay += (int)((__int64)ash->dwStart * 1000 * ash->dwScale / ash->dwRate);
						} else {
							//asi[j]->iDelay += (int)((__int64)ash->dwStart * 1000 * ash->dwScale / ash->dwRate);
						}
					}
				}
				switch (lpASL[j]->IsCompatible(lpAudiosources[j])) {
					case MMS_COMPATIBLE:
						lpASL[j]->Append(lpAudiosources[j]);
						break;
					default:
						sprintf(Buffer,LoadString(STR_ERR_AUDIOINCOMPATIBLE),fi->Name.UTF8(),j+1);
						MessageBox(Buffer,LoadString(STR_GEN_ERROR),MB_OK | MB_ICONERROR);
						return;
						break;
				}
			}
			asi[j]->lpdwFiles = NULL;
			asi[j]->audiosource = lpASL[j];
			asi[j]->bNameFromFormatTag = true;

			/*
				Check if MP3 CBR audio sources of different bitrate have been joined. In this case,
				the joined stream is of course VBR, and only MP3 VBR can be put into AVI, but not
				MP1 VBR nor MP2 VBR.
			*/
			if (asi[j]->dwType == AUDIOTYPE_MP3CBR) {
				if (!lpASL[j]->IsCBR()) {
					asi[j]->dwType = AUDIOTYPE_MP3VBR;
					lpASL[j]->AssumeVBR();
					FillMP3_ASI(&asi[j], ((MP3SOURCE*)lpAudiosources[j]));
					asi[j]->audiosource = lpASL[j];
					MP3SOURCE* mp3 = (MP3SOURCE*)lpAudiosources[j];
					if (mp3->GetLayerVersion() != 3)
						lpASL[j]->AllowAVIOutput(false);
				}
			}
			asi[j]->lpdwFiles = DuplicateFileUsageList(dwSelectedFiles);
			AddAudioStream(asi[j]);
		}

	// Subtitles
		dwNbrOfSubs=0;

		AVIFILEEX* a = fi->AVIFile;
		for (i=0;i<(int)a->GetNbrOfStreams();i++) {
			if (a->IsTextStream(i)) dwNbrOfSubs++;
		}
		
		lpdwSubtitleList=NULL;
		if (dwNbrOfSubs)
		{
			lpdwSubtitleList=new DWORD[1+dwNbrOfSubs];
			lpdwSubtitleList[0]=dwNbrOfSubs;
			dwNbrOfSubs=0;

			for (i=0;i<(int)a->GetNbrOfStreams();i++) {
				if (a->IsTextStream(i)) lpdwSubtitleList[++dwNbrOfSubs] = i;
			}
			subs=new SUBTITLES*[lpdwSubtitleList[0]];
			for (i=1;i<=(int)lpdwSubtitleList[0];i++)
			{
				qwBias=0;
				fi = m_SourceFiles.GetFileInfoFromID(dwSelectedFiles[1]);
		
				lpAS=new AVISTREAM;
				lpAS->Open(fi->AVIFile,lpdwSubtitleList[i]);

				subs[i-1]=new SUBTITLES;
				subs[i-1]->Open(new CTextFile(StreamMode::Read,lpAS,CharacterEncoding::UTF8));

				lpAS->Close();
				delete lpAS;

				for (j=2;j<=(int)(dwSelectedFiles[0]);j++)
				{
					qwBias+=fi->AVIFile->GetNanoSecPerFrame()*fi->AVIFile->GetNbrOfChunks(0);
					fi = m_SourceFiles.GetFileInfoFromID(dwSelectedFiles[j]);

					lpAS=new AVISTREAM;
					lpAS->Open(fi->AVIFile,lpdwSubtitleList[i]);
					temp_subs=new SUBTITLES;
					temp_subs->Open(new CTextFile(StreamMode::Read, lpAS, CharacterEncoding::UTF8));
					if (!subs[i-1]->Merge(temp_subs,qwBias))
					{
						MessageBox(LoadString(IDS_COULDNTMERGESUBS),cstrError,MB_OK | MB_ICONERROR);
						return;
					}
					lpAS->Close();
					delete lpAS;
					delete temp_subs;
				}
			}

			ssi=new SUBTITLE_STREAM_INFO*[lpdwSubtitleList[0]];
			for (i=0;i<(int)(lpdwSubtitleList[0]);i++)
			{
				ssi[i]=new SUBTITLE_STREAM_INFO;
				ssi[i]->lpfi=NULL;
				ssi[i]->lpsubs=subs[i];
				ssi[i]->lpash=new AVIStreamHeader;
				ssi[i]->lpash->fccType=MakeFourCC("txts");
	
				cStr=LoadString(IDS_SOURCE);
				cStr2=LoadString(IDS_VI_STREAM);
				ssi[i]->lpdwFiles = DuplicateFileUsageList(dwSelectedFiles);
				AddSubtitleStream(ssi[i]);
			}
			delete ssi;
			delete subs;
			delete lpdwSubtitleList;
		}
	}

	if (fi->dwType & FILETYPE_MKV) {
		MATROSKA* m = fi->MKVFile;
		CDynIntArray* audio_streams = new CDynIntArray;
		CDynIntArray* subs = new CDynIntArray;
		int  k=0;

		// audio tracks
		for (j=0;j<m->GetTrackCount();j++) {
			if (m->GetTrackType(j) == MSTRT_AUDIO) audio_streams->Insert(j);
		}

		newz(AUDIOSOURCELIST*, audio_streams->GetCount(), lpASL);
		newz(AUDIO_STREAM_INFO*, audio_streams->GetCount(), asi);

		for (j=0;j<audio_streams->GetCount();j++) {
			lpASL[j] = new AUDIOSOURCELIST();
			newz(AUDIO_STREAM_INFO,1,asi[j]);

			for (i=1;i<=dwNbrOfFiles;i++)
			{
				fi = m_SourceFiles.GetFileInfoFromID(dwSelectedFiles[i]);
				m = fi->MKVFile;

				m->SetActiveTrack(audio_streams->At(j));
			
				AUDIOSOURCE* a = new AUDIOSOURCEFROMMATROSKA(m,audio_streams->At(j));
				if (a->FormatSpecific(MMSGFS_IS_VORBIS, 0)) {
					VORBISPACKETSFROMMATROSKA* p = new VORBISPACKETSFROMMATROSKA;
					p->Open((AUDIOSOURCEFROMMATROSKA*)a);
					VORBISFROMOGG* vorbis = new VORBISFROMOGG;
					vorbis->Open(p);
					vorbis->GetTitleSet()->Import(a->GetTitleSet());
					a = vorbis;
					asi[j]->lpFormat = new char[1<<16];
					asi[j]->iFormatSize = vorbis->RenderSetupHeader(asi[j]->lpFormat);
				}
				if (a->FormatSpecific(MMSGFS_IS_AAC)) {
					FillAAC_ASI(&asi[j], (AACSOURCE*)a);
				}


				switch (lpASL[j]->IsCompatible(a)) {
					case MMS_COMPATIBLE:
						lpASL[j]->Append(a);
						break;
					default:
						sprintf(Buffer,LoadString(STR_ERR_AUDIOINCOMPATIBLE),fi->Name.UTF8(),audio_streams->At(j));
						MessageBox(Buffer,LoadString(STR_GEN_ERROR),MB_OK | MB_ICONERROR);
						return;
						break;
				}

				if (i==1) {
					if (!asi[j]->lpFormat) {
						asi[j]->lpFormat = lpASL[j]->GetFormat();
						asi[j]->iFormatSize = m->GetCodecPrivateSize();
					}
				}

			}

			sprintf(Buffer,"%s",lpASL[j]->GetCodecID());

			asi[j]->audiosource = lpASL[j];
			asi[j]->bNameFromFormatTag = false;

			if (asi[j]->audiosource->FormatSpecific(MMSGFS_IS_AAC)) {
				asi[j]->dwType = AUDIOTYPE_AAC;
				asi[j]->bNameFromFormatTag = 1;
			}

			if (asi[j]->audiosource->FormatSpecific(MMSGFS_IS_VORBIS)) {
				asi[j]->dwType = AUDIOTYPE_VORBIS;
				asi[j]->bNameFromFormatTag = 1;
			}

			if (asi[j]->audiosource->FormatSpecific(MMSGFS_IS_AC3)) {
				asi[j]->dwType = AUDIOTYPE_AC3;
				asi[j]->bNameFromFormatTag = 1;
			}

			if (asi[j]->audiosource->FormatSpecific(MMSGFS_IS_DTS)) {
				asi[j]->dwType = AUDIOTYPE_DTS;
				asi[j]->bNameFromFormatTag = 1;
			}
	
			if (asi[j]->audiosource->FormatSpecific(MMSGFS_IS_MPEG)) {
				asi[j]->dwType = AUDIOTYPE_MP3VBR;
				asi[j]->bNameFromFormatTag = 1;
			}

			asi[j]->lpdwFiles = DuplicateFileUsageList(dwSelectedFiles);
			AddAudioStream(asi[j]);
		}

		delete asi;
		delete lpASL;

		// subtitles: currently only utf-8 plain text will work!
		for (j=0;j<m->GetTrackCount();j++) {
			if (m->GetTrackType(j) == MSTRT_SUBT) subs->Insert(j);
		}

		ssi = new SUBTITLE_STREAM_INFO*[subs->GetCount()];
		lpSSL = new SUBTITLESOURCELIST*[subs->GetCount()];
		ZeroMemory(lpSSL, j = subs->GetCount() * sizeof(*ssi));
		ZeroMemory(ssi, j);

		for (j=0;j<subs->GetCount();j++) {
			lpSSL[j] = new SUBTITLESOURCELIST();
			ssi[j] = new SUBTITLE_STREAM_INFO;
			ZeroMemory(ssi[j],sizeof(*ssi[j]));
			for (i=1;i<=dwNbrOfFiles;i++) {
				fi = m_SourceFiles.GetFileInfoFromID(dwSelectedFiles[i]);
				m = fi->MKVFile;

				m->SetActiveTrack(subs->At(j));
		
				SUBTITLESFROMMATROSKA* s = new SUBTITLESFROMMATROSKA(m,subs->At(j));

				switch (lpSSL[j]->IsCompatible(s)) {
					case MMS_COMPATIBLE:
						lpSSL[j]->Append(s);
						break;
					case MMSIC_COMPRESSION:
						sprintf(Buffer,LoadString(STR_ERR_SUBCOMPRESSIONINCOMPATIBLE),fi->Name.UTF8(),subs->At(j));
						MessageBox(Buffer,LoadString(STR_GEN_ERROR),MB_OK | MB_ICONERROR);
						return;
						break;
					default:
						sprintf(Buffer,LoadString(STR_ERR_SUBSINCOMPATIBLE),fi->Name.UTF8(),subs->At(j));
						MessageBox(Buffer,LoadString(STR_GEN_ERROR),MB_OK | MB_ICONERROR);
						return;
				}
			}
		
			sprintf(Buffer,"%s",lpSSL[j]->GetCodecID());
			
			ssi[j]->lpsubs = lpSSL[j];
			ssi[j]->lpfi = NULL;
			ssi[j]->lpash = NULL;
			lpSSL[j]->AllowAVIOutput(0);

			ssi[j]->lpdwFiles = DuplicateFileUsageList(dwSelectedFiles);
			AddSubtitleStream(ssi[j]);
		}
		

		subs->DeleteAll();
		delete subs;
		delete ssi;
		delete lpSSL;
		
		audio_streams->DeleteAll();
		delete audio_streams;
	}

	if (bVFR) {
		lpvsl->SetCFRFlag(false);
		lpvsl->AllowAVIOutput(false);
	}

// Dateien nur in Benutzung, wenn alles geklappt hat
	for (i=1;i<=dwNbrOfFiles;i++)
	{
		fi = m_SourceFiles.GetFileInfoFromID(dwSelectedFiles[i]);
		fi->bInUse=true;
	}


	m_SourceFiles.AllowMoving(false);
	m_SourceFiles.InvalidateRect(NULL);
	m_SourceFiles.UpdateWindow();
	
}