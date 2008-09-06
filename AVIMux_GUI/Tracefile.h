#ifndef I_TRACEFILE
#define I_TRACEFILE

#include "windows.h"
#include <string>

#define TRACE_LEVEL_DEBUG 2
#define TRACE_LEVEL_INFO  3
#define TRACE_LEVEL_NOTE  4
#define TRACE_LEVEL_WARN  5
#define TRACE_LEVEL_ERROR 6
#define TRACE_LEVEL_FATAL 7
#define TRACE_LEVEL_NONE 8

class CTraceFile
{
private:
	char* pszFilename;
	HANDLE hFile;
	CRITICAL_SECTION csAddLogEntry;
	int m_Tracelevel;
protected:
	void DoOpenFile(char* fileName);
	void AddLogEntry(int minTraceLevel, char* title, char* message);
	int GetTraceLevelCount();
public:
	bool IsOpen() {
		return !!pszFilename;
	}
	CTraceFile();
	virtual ~CTraceFile();
	void Open(char* fileName);
	void SetTraceLevel(int trace_level) {
		if (trace_level < GetTraceLevelCount())
			m_Tracelevel = trace_level;
	}
	int GetTraceLevel() {
		return m_Tracelevel;
	}

	void Trace(int TraceLevel, char* title, char* message);
	void Trace(int TraceLevel, std::string title, std::string message);
	void Close();
};

CTraceFile* GetApplicationTraceFile();

#endif