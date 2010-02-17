#include "stdafx.h"
#include "Tracefile.h"
#include "stdlib.h"
#include "stdio.h"
#include "windows.h"
#include "winerror.h"
#include "tchar.h"
#include "../FormatTime.h"
#include "../FormatInt64.h"
#include "../Guard.h"

#include <vector>

#ifndef MAX_LONG_PATH
#define MAX_LONG_PATH 32767
#endif

static char* TRACE_LEVEL_NAMES[] = {
	"", "", "DEBUG ", "INFO  ", "NOTE  ", "WARN  ", "ERROR ", "ASSERT", "FATAL "
};

CUTF8 CTraceableParamHelper::s_inStr("[IN] ");
CUTF8 CTraceableParamHelper::s_outStr("[OUT]");

static CTraceFile* ApplicationTraceFile = NULL;
unsigned int* CTraceFile::Entry::m_pNextCounterValue = NULL;

CUTF8 CTraceFile::s_applicationTraceFileFolder;

void CTraceFile::CTraceFileEntry::Render(CUTF8& result) const
{
	char pnewLine[] = { 13, 10, 0 };
	size_t msgLen = strlen(GetMessage());

	char* msg = (char*)malloc(msgLen + 3);
	strcpy_s(msg, msgLen + 3, GetMessage());

	if (msgLen > 0) {
		strcat_s(msg, msgLen + 3, pnewLine);
	}

	std::ostringstream sstrComponent;

	if (GetComponent())
		sstrComponent << "Location: " << GetComponent() << "\x0D\x0A";
	std::string componentName = sstrComponent.str();

	char cThreadString[128];
	QW2Str(GetThreadId(), cThreadString, 0);

	SYSTEMTIME time;
	GetLocalTime(&time);
	char cHeader[32768]; memset(cHeader, 0, sizeof(cHeader));
	int size = sprintf_s(cHeader, "%02d:%02d:%02d.%03d %04d-%02d-%02d: #%d %s (ThreadId %s)%c%c%s%s%c%c", 
		time.wHour,
		time.wMinute,
		time.wSecond,
		time.wMilliseconds,
		time.wYear,
		time.wMonth,
		time.wDay,
		Counter(),
		TRACE_LEVEL_NAMES[GetLevel()],
		cThreadString, 13, 10,
		componentName.c_str(),
		GetTitle(), 13, 10);

	std::ostringstream sstrResult;
	sstrResult << cHeader;
	sstrResult << msg << "\x0D\x0A";
	
	result = CUTF8(sstrResult.str().c_str());
	free(msg);
}

void _stdcall SetApplicationTraceFile(CTraceFile* traceFile)
{
	if (ApplicationTraceFile)
		ApplicationTraceFile->Close();
	ApplicationTraceFile = traceFile;
	ApplicationTraceFile->IncRefCounter();
}

void CTraceFile::SetApplicationTraceFileFolder(const CUTF8& folder)
{
	s_applicationTraceFileFolder = folder;
}

const CUTF8& CTraceFile::GetApplicationTraceFileFolder()
{
	return s_applicationTraceFileFolder;
}

CTraceFile* GetApplicationTraceFile() 
{
	if (!ApplicationTraceFile) {
		ApplicationTraceFile = new CTraceFile();

		wchar_t moduleFileName[MAX_LONG_PATH];
		GetModuleFileNameW(NULL, moduleFileName, MAX_LONG_PATH);

		std::wstring strModuleFilePath = moduleFileName;
		size_t index_of_name = strModuleFilePath.find_last_of(L"\\");
		std::wstring strModulePath = strModuleFilePath.substr(0, index_of_name);
		std::wstring strModuleName = strModuleFilePath.substr(index_of_name+1);

		int historyLength = 4;
		int historyCurrent = historyLength;

		std::wstring logFileName;
		while (historyCurrent --> 1)
		{
			std::wostringstream osLogFileName;
			std::wostringstream osOlderLogFileName;

			std::wstring traceFileFolder;
			if (!CTraceFile::GetApplicationTraceFileFolder().Size())
				traceFileFolder = strModulePath;

			osLogFileName << traceFileFolder << L"\\" << "logfile." << strModuleName << "." << historyCurrent << ".txt"; 
			osOlderLogFileName << traceFileFolder << L"\\" << "logfile." << strModuleName << "." << historyCurrent + 1 << ".txt"; 

			std::wstring olderLogFileName = osOlderLogFileName.str();
			logFileName = osLogFileName.str();

			if (historyCurrent + 1 == historyLength)
			{
				DeleteFileW(logFileName.c_str());
			}
			else
			{
				MoveFileW(logFileName.c_str(), olderLogFileName.c_str());
			}
		}

		ApplicationTraceFile->Open(logFileName.c_str());
	}

	return ApplicationTraceFile;
}

CTraceFile::CTraceFile()
{
	pszFilename = NULL;
	hFile = NULL;
	InitializeCriticalSection(&csAddLogEntry);
	m_Tracelevel = TRACE_LEVEL_WARN;
	m_NextCheckpointID = 1;
	m_refCounter = 0;
}

void CTraceFile::IncRefCounter()
{
	// TODO: MultiCPU-sicher machen
	// InterlockedIncrement(&m_refCounter);
	m_refCounter++;
	InitializeCounter();
}

CTraceFile::~CTraceFile()
{
	m_refCounter = 1;
	Close();
	DeleteCriticalSection(&csAddLogEntry);
}

int CTraceFile::GetTraceLevelCount() {
	return (sizeof(TRACE_LEVEL_NAMES)/sizeof(TRACE_LEVEL_NAMES[0]));
}

void CTraceFile::DoOpenFile(const wchar_t* fileName)
{
	HANDLE h = CreateFileW(fileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
		CREATE_ALWAYS, NULL, NULL);

	if (h == INVALID_HANDLE_VALUE)
		return;

	pszFilename = _wcsdup(fileName);

	unsigned char utf8bom[] = { 0xEF, 0xBB, 0xBF };

	DWORD dwWritten;
	WriteFile(h, utf8bom+0, 3, &dwWritten, NULL);
	DWORD dwError = GetLastError();

	hFile = h;
}

void CTraceFile::Close()
{
	m_refCounter--;

	if (hFile && hFile != INVALID_HANDLE_VALUE) {
		if (m_refCounter == 0)
		{
			CloseHandle(hFile);
			hFile = NULL;

			free(pszFilename);
			pszFilename = NULL;
		}
		else
		{
			FlushFileBuffers(hFile);
		}
	}
}

void CTraceFile::Flush(bool bDiscard, unsigned __int64 ID)
{
	ENTRIES& entries = Entries();
	ENTRIES* lower_entries = LowerEntries();

	ENTRIES::iterator iter = entries.begin();
	ENTRIES::iterator iter_end = entries.end();
	unsigned int Counter = 0;

	for (; iter != iter_end; iter++)
	{
		unsigned __int64 _ID;
		if ((*iter)->IsCheckpoint(_ID)) {
			if (_ID == ID) {
				CTraceFileEntry* entry = *iter;
				delete entry;
				Counter++;
				break;
			} else {
				/* b0rked */
			}
		} else {
			CTraceFileEntry* entry = *iter;
			if (!bDiscard || entry->WriteAlways() /*|| entry->GetLevel() >= GetTraceLevel()*/) {
				if (lower_entries)
					lower_entries->push_back(entry);
				else {
					CUTF8 entryText;
					entry->Render(entryText);

					DWORD dwWritten;
					if (hFile)
						WriteFile(hFile, entryText.UTF8(), static_cast<DWORD>(strlen(entryText.UTF8())), &dwWritten, NULL);
					delete entry;
				}
			} else {
				delete entry;
			}

			Counter++;
		}
	}

	while (Counter--)
		Entries().pop_front();
}

void CTraceFile::AddLogEntry(unsigned int TraceLevel, unsigned int counter, const char* component, 
							 const char *title, const char *message, bool WriteAlways)
{
	if (TraceLevel >= TRACE_LEVEL_NONE)
		return;

	EnterCriticalSection(&csAddLogEntry);
	CHECK_POINT_VECTOR& cpv = CheckPoints();

	if (TraceLevel >= GetTraceLevel() || !cpv.empty())
	{
		SYSTEMTIME time;
		GetSystemTime(&time);

		CTraceFileEntry* entry = new CTraceFileEntry(TraceLevel, GetCurrentThreadId(), 
			WriteAlways, counter, component, title, message);

		if (!cpv.empty()) {
			Entries().push_back(entry);
			CHECK_POINT_VECTOR::reverse_iterator iter = cpv.rend();
			iter--;

			if (iter->m_Level < TraceLevel)
				Flush(false, iter->m_ID);
		} else {
			CUTF8 Entry;
			entry->Render(Entry);
			const char* cEntry = Entry.UTF8();

			DWORD dwWritten;
			if (hFile)
				WriteFile(hFile, cEntry, static_cast<DWORD>(strlen(cEntry)), &dwWritten, NULL);
			delete entry;
		}
	}

	LeaveCriticalSection(&csAddLogEntry);
}

void CTraceFile::Trace(unsigned int  TraceLevel, const CUTF8& title, const CUTF8& message)
{
	AddLogEntry(TraceLevel, Entry::NextCounterValue(), NULL, title.UTF8(), message.UTF8(), false);
}

void CTraceFile::Trace(unsigned int TraceLevel, const CUTF8& component, const CUTF8& title, const CUTF8& message)
{
	AddLogEntry(TraceLevel, Entry::NextCounterValue(), component.UTF8(), title.UTF8(), message.UTF8(), false);
}

void CTraceFile::Trace(unsigned int TraceLevel, const CUTF8& component, const CUTF8& title, const CUTF8& message, char* duration)
{
	if (duration && *duration) {
		std::ostringstream sstrNewTitle;
		sstrNewTitle << title.UTF8() << " [" << duration << "]";
		std::string strNewTitle = sstrNewTitle.str();
		const char* newTitle = strNewTitle.c_str();
		AddLogEntry(TraceLevel, Entry::NextCounterValue(), component.UTF8(), newTitle, message.UTF8(), false);
	}
	else
	{
		AddLogEntry(TraceLevel, Entry::NextCounterValue(), component.UTF8(), title.UTF8(), message.UTF8(), false);
	}

}

void CTraceFile::Trace(CTraceFile::Entry TraceEntry)
{
	if (*(TraceEntry.m_Duration.UTF8())) {
		std::ostringstream sstrNewTitle;
		sstrNewTitle << TraceEntry.m_Title.UTF8() << " [" << TraceEntry.m_Duration.UTF8() << "]";
		std::string strNewTitle = sstrNewTitle.str();
		const char* newTitle = strNewTitle.c_str();
		AddLogEntry(TraceEntry.m_traceLevel, TraceEntry.m_counter, TraceEntry.m_Component.UTF8(), newTitle, TraceEntry.m_Message.UTF8(), false);
	}
	else
	{
		AddLogEntry(TraceEntry.m_traceLevel, TraceEntry.m_counter, TraceEntry.m_Component.UTF8(), TraceEntry.m_Title.UTF8(), TraceEntry.m_Message.UTF8(), false);
	}
}

void CTraceFile::TraceAlways(unsigned int  TraceLevel, char* component, char *title, char *message)
{
	AddLogEntry(TraceLevel, Entry::NextCounterValue(), component, title, message, true);
}

void CTraceFile::TraceAlways(unsigned int TraceLevel, char* component, char *title, char *message, char* duration)
{
	char newTitle[65536];
	newTitle[0]=0;
	sprintf_s(newTitle, "%s [%s]", title, duration);

	AddLogEntry(TraceLevel, Entry::NextCounterValue(), component, newTitle, message, true);
}

void CTraceFile::Open(const wchar_t* fileName)
{
	DoOpenFile(fileName);
}

unsigned __int64 CTraceFile::SetCheckpoint(unsigned int TraceLevel)
{
	CriticalSectionGuard csg1(&csAddLogEntry);

	CHECK_POINT cp;
	cp.m_ID = m_NextCheckpointID;
	cp.m_Level = TraceLevel;
	CheckPoints().push_back(cp);

	ENTRIES noEntries;
	ThreadEntries().push_back(noEntries);

	unsigned __int64 CPID = m_NextCheckpointID++;

	return CPID;
}

void CTraceFile::PassCheckpoint()
{
	CriticalSectionGuard csg1(&csAddLogEntry);

	CHECK_POINT_VECTOR& cpv = CheckPoints(); 
	CHECK_POINT& cp = cpv[cpv.size()-1];
	Entries().push_back(new CTraceFileEntry(cp.m_Level, GetCurrentThreadId(), cp.m_ID));

	if (!cpv.empty())
	{
		Flush(true, cpv[cpv.size()-1].m_ID);
	}
	else
	{
		Flush(true, 0);
	}

	cpv.pop_back();

	ThreadEntries().pop_back();
}
