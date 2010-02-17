#include "stdafx.h"

#include "configscripts.h"
#include "IncResource.h"
#include "languages.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "windows.h"
#include "../strings.h"
#include "../basestreams.h"
#include "../../Common/TextFiles.h"
#include "AVIMux_GUI.h"
#include "global.h"
#include "../UnicodeCalls.h"
#include "../Filenames.h"
#include "../FileStream.h"
#include "../../Common/Path.h"

 HANDLE hGlobalMuxingStartedSemaphore;
 HANDLE hGlobalMuxSemaphore;

/* Instructions:

   Load
   Clear
   Select
     File
	 Audio
	 Subtitle
   Deselect
     File
	 Audio
	 Subtitle
   Add
     Videosource
   Set
     Option
	 Input Options
	 Output Options


  */

bool bWait = false;
 
bool	LoadScript(char* lpcName,HWND hwnd,UINT message)
{

	CFileStream* fs = new CFileStream;
	fs->Open(lpcName, StreamMode::Read);
	CTextFile* f = new CTextFile;
	f->Open(StreamMode::Read,fs);
	f->SetOutputEncoding(CharacterEncoding::UTF8);
		
	char*	buffer = NULL;
	char*   entire_line = NULL;
	int		i;
	bool	bError;
	char*	w = NULL, *v = NULL, *l = NULL;
	char*	with = NULL;
	char*	line = NULL,*line_old = NULL;
	char*	cText = NULL;
	bool	bFirstLine_OK=false;
	int		withpos[10];
	ZeroMemory(withpos,sizeof(withpos));
	int		withpos_ind = 0;
	char*   path;

	path = (char*)calloc(1,25600);
	//splitpathname(lpcName,&name,&extension,&path);

	std::string fileName;
	std::string fileExtension;
	std::string filePath;
	splitpathname<char>(lpcName, fileName, fileExtension, filePath);
	strcpy(path, filePath.c_str());

	buffer = (char*)calloc(1,25600);
	entire_line = (char*)calloc(1,25600);
	with = (char*)calloc(1,25600);
	line = (char*)calloc(1,25600);
	line_old = line;
	hGlobalMuxingStartedSemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, false, GlobalMuxingStartedSemaphoreName());
	hGlobalMuxSemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, false, GlobalMuxSemaphoreName());
	
	std::string textFileLine;
	while (f->ReadLine(textFileLine)>=0)
	{
		buffer[0]=0; strcpy(buffer, textFileLine.c_str());
/*	if (bWait) {
			WaitForSingleObject(hGlobalMuxingStartedSemaphore, INFINITE);
			ReleaseSemaphore(hGlobalMuxingStartedSemaphore, 1, NULL);
			WaitForSingleObject(hGlobalMuxSemaphore, INFINITE);
			ReleaseSemaphore(hGlobalMuxSemaphore, 1, NULL);
			bWait = false;
	} 
*/		bError=false;
		l = buffer;
		i=strlen(buffer)-1;
		if (buffer[i]==10) buffer[i]=0;
		strcpy(line,buffer);
		sprintf(entire_line, "%s%s", with, l);
		strcpy(l,entire_line);

		v=getword(&line);
		w=getword(&l);
		if (!strcmp(v,"END")) {
			v=getword(&line);
			if (!strcmp(v,"WITH")) {
				if (withpos_ind) {
					with[withpos[--withpos_ind]]=0;
				} else {
					w=v;bError=true;
				}
			} else bError=true;
		}
		else
		if (!strcmp(w,"REM"))
		{	}
		else
		if (!strcmp(w,"LOAD"))	{
			cText = (char*)calloc(2, 32768);

			std::string fullPathToLoad = CPath::Combine(path, l);
			CUTF8 utf8PathToLoad(fullPathToLoad.c_str());
			wcscpy((wchar_t*)cText, utf8PathToLoad.WStr());
/*			char curr_dir[65536];
			(*UGetCurrentDirectory())(32768, curr_dir);
			char* upath = NULL;
			fromUTF8(path, &upath);
			(*USetCurrentDirectory())(upath);
			free(upath);
*/
//			char* l2 = _strdup(l);
//			Filename2LongFilename(l2, cText, 32768);
//			free(l2);

//			(*USetCurrentDirectory())(curr_dir);		

			ProcessMsgQueue(hwnd);
			strcpy(cText, utf8PathToLoad.UTF8());
			PostMessage(hwnd,message,IDM_DOADDFILE,(LPARAM)cText);
		}
		else
		if (!strcmp(w,"LANGUAGE")) {
			cText = (char*)calloc(1,1+strlen(l));
			strcpy(cText,l);
			PostMessage(hwnd,message,IDM_SETLANGUAGE,(LPARAM)cText);
		}
		else
		if (!strcmp(v,"WITH")) {
			withpos[withpos_ind++] = strlen(with);
			v=line;
			strcat(with,line);
			strcat(with," ");
		}
		else
		if (!strcmp(w,"CLEAR"))	{
			PostMessage(hwnd,message,IDM_CLEARALL_NC,0);
		}
		else
		if (!strcmp(w,"SELECT")) { 
			w = getword(&l);
			if (!strcmp(w,"FILE")) { 
				PostMessage(hwnd,message,IDM_SELFILE,atoi(l)-1);
			} else
			if (!strcmp(w,"AUDIO")) {
				PostMessage(hwnd,message,IDM_SELAUDIO,atoi(l)-1);
			} else
			if (!strcmp(w,"SUBTITLE")) {
				PostMessage(hwnd,message,IDM_SELSUB,atoi(l)-1);
			} else
			if (!strcmp(w,"VIDEO")) {
				PostMessage(hwnd,message,IDM_SELVIDEO,atoi(l)-1);
			}
			else bError=true;
		}
		else
		if (!strcmp(w,"DESELECT")) {
			w = getword(&l);
			if (!strcmp(w,"FILE")) {
				PostMessage(hwnd,message,IDM_DESELFILE,atoi(l)-1);
			} else
			if (!strcmp(w,"SUBTITLE")) {
				PostMessage(hwnd,message,IDM_DESELSUB,atoi(l)-1);
			} else
			if (!strcmp(w,"AUDIO")) {
				PostMessage(hwnd,message,IDM_DESELAUDIO,atoi(l)-1);
			} else
			if (!strcmp(w, "VIDEO")) {
				PostMessage(hwnd, message, IDM_DESELVIDEO, atoi(l)-1);
			}
			else
			bError=true;
		}
		else
		if (!strcmp(w,"ADD")) {
			w=getword(&l);
			if (!strcmp(w,"VIDEOSOURCE")) {
				PostMessage(hwnd,message,IDM_ADDAVILIST,0);
			} else
			if (!strcmp(w,"MMSOURCE")) {
				PostMessage(hwnd,message,IDM_ADDAVILIST,0);
			}
		}
		else
		if (!strcmp(w,"SET")) { 
			w=getword(&l);
			if (!strcmp(w,"OPTION")) {
				cText = (char*)calloc(1,1+strlen(l));
				strcpy(cText,l);
				PostMessage(hwnd,message,IDM_SETOPTION,(LPARAM)cText);
			}
			else
			if (!strcmp(w,"OUTPUT")) { 
				w=getword(&l);
				if (!strcmp(w,"OPTIONS")) {
					PostMessage(hwnd,message,IDM_SETOUTPUTOPTIONS,0);
				}
				else bError=true;
			}
			else
			if (!strcmp(w,"INPUT")) { 
				w=getword(&l);
				if (!strcmp(w,"OPTIONS")) {
					PostMessage(hwnd,message,IDM_SETINPUTOPTIONS,0);
				}
				else bError=true;
			}
			else
			bError=true;
		}
		else
		if (!strcmp(w,"START"))
		{
			char*	lpcFile;
			lpcFile = (char*)malloc(1+strlen(l));
			strcpy(lpcFile,buffer+6);
			bWait = true;
			PostMessage(hwnd,message,IDM_STARTMUXING,(LPARAM)lpcFile);
		}
		else
		{
			bError=true;
		}
		
		if (bError)
		{
			if (bFirstLine_OK) {
				std::basic_string<TCHAR> strLoadUnknown = (TCHAR*)LoadString(STR_LOAD_UNKNOWN);
				std::basic_string<TCHAR> strError = (TCHAR*)LoadString(STR_GEN_ERROR);
				
				std::basic_string<TCHAR>::size_type pos = strLoadUnknown.find(_T("%s"), 0);
				strLoadUnknown.replace(pos, 2, entire_line);
				pos = strLoadUnknown.find(_T("%s"), 0);
				strLoadUnknown.replace(pos, 2, w);
				
				MessageBox (hwnd, strLoadUnknown.c_str(), strError.c_str(), MB_OK | MB_ICONERROR);
			}
			f->Close();
			fs->Close();
			delete f;
			delete fs;
			return bFirstLine_OK;
		}
		ZeroMemory(buffer,sizeof(buffer));
		line = line_old;
		bFirstLine_OK = true;
	}
	f->Close();
	fs->Close();
	delete f;
	delete fs;
	delete with;
	delete entire_line;
	delete buffer;
	delete line;
	CloseHandle(hGlobalMuxingStartedSemaphore);
	CloseHandle(hGlobalMuxSemaphore);
	return true;
}
