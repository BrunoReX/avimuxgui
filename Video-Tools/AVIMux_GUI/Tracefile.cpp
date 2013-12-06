#include "stdafx.h"
#include "Tracefile.h"
#include "stdlib.h"
#include "stdio.h"
#include "windows.h"

static char* TRACE_LEVEL_NAMES[] = {
	"", "", "DEBUG", "INFO", "NOTE", "WARN", "ERROR", "FATAL"
};


static CTraceFile* ApplicationTraceFile = NULL;

CTraceFile* GetApplicationTraceFile() {
	if (!ApplicationTraceFile) 
	{
		ApplicationTraceFile = new CTraceFile();
	}

	if (!ApplicationTraceFile->IsOpen() && 
		(ApplicationTraceFile->GetTraceLevel() < TRACE_LEVEL_NONE))
	{
		ApplicationTraceFile->Open("trace.txt");
	}

	return ApplicationTraceFile;
}

CTraceFile::CTraceFile()
{
	pszFilename = NULL;
	hFile = NULL;
	InitializeCriticalSection(&csAddLogEntry);
	m_Tracelevel = TRACE_LEVEL_WARN;
}

CTraceFile::~CTraceFile()
{
	DeleteCriticalSection(&csAddLogEntry);
	if (pszFilename)
		delete pszFilename;
}

int CTraceFile::GetTraceLevelCount() {
	return (sizeof(TRACE_LEVEL_NAMES)/sizeof(TRACE_LEVEL_NAMES[0]));
}

void CTraceFile::DoOpenFile(char* fileName)
{
	HANDLE h = CreateFileA(fileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
		CREATE_ALWAYS, NULL, NULL);

	if (h == INVALID_HANDLE_VALUE)
		return;

	pszFilename = _strdup(fileName);

	hFile = h;
}

void CTraceFile::Close()
{
	if (hFile && hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
		hFile = NULL;

		free(pszFilename);
		pszFilename = NULL;
	}
}

void CTraceFile::AddLogEntry(int TraceLevel, char *title, char *message)
{
	if (TraceLevel >= TRACE_LEVEL_NONE)
		return;

	EnterCriticalSection(&csAddLogEntry);

	if (TraceLevel >= GetTraceLevel())
	{
		SYSTEMTIME time;
		GetSystemTime(&time);

//		char* msg = strdup(message);
//		if (strlen(msg) 

		size_t msgLen = strlen(message);

		char* msg = (char*)malloc(msgLen + 3);
		strcpy(msg, message);

		if (msgLen > 0) {
			strcat(msg, "\n");
		}

		char cTime[16384]; memset(cTime, 0, sizeof(cTime));
		int size = sprintf(cTime, "%02d:%02d:%02d.%03d %04d-%02d-%02d: %s\n%s\n%s\n", 
			time.wHour,
			time.wMinute,
			time.wSecond,
			time.wMilliseconds,
			time.wYear,
			time.wMonth,
			time.wDay,
			TRACE_LEVEL_NAMES[TraceLevel],
			title,
			msg);

		DWORD dwWritten;
		WriteFile(hFile, cTime, size, &dwWritten, NULL);

/*		if (TraceLevel >= TRACE_LEVEL_WARN)
			FlushFileBuffers(hFile);
*/
		free(msg);
	}

	LeaveCriticalSection(&csAddLogEntry);
}

void CTraceFile::Trace(int TraceLevel, char *title, char *message)
{
	AddLogEntry(TraceLevel, title, message);
}

void CTraceFile::Trace(int TraceLevel, std::string title, std::string message)
{
	const char* ttl = title.c_str();
	const char* msg = message.c_str();

	Trace(TraceLevel, (char*)ttl, (char*)msg);
}

void CTraceFile::Open(char* fileName)
{
	DoOpenFile(fileName);
}