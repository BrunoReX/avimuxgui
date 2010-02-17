#ifndef I_TRACEFILE
#define I_TRACEFILE

#include "windows.h"
#include "..\utf-8.h"
#include "tchar.h"
#include <string>
#include <deque>
#include <vector>
#include <map>
#include <functional>
#include <iomanip>
#include <sstream>
#include "TraceLevel.h"
#include "..\HighPrecisionTimer.h"



/** \brief Klasse zur Nutzung von Ereignisaufzeichnung
 */
class CTraceFile
{
private:
	class CTraceFileEntry
	{
	private:
		//unsigned int m_Level;
		TraceLevel m_level;
		unsigned int m_counter;
		char* m_Component;
		char* m_Title;
		char* m_Message;
		bool m_bIsCheckpoint;
		bool m_bWriteAlways;

		unsigned __int64 m_CheckpointId;
		DWORD m_ThreadId;

		SYSTEMTIME m_SystemTime;
	public:
		CTraceFileEntry(unsigned int Level, DWORD ThreadId, bool WriteAlways, unsigned int counter,
			const char* Component, const char* Title, const char* Message)
		{
			::GetSystemTime(&m_SystemTime);
			m_level = Level;
			m_counter = counter;
			m_Component = _strdup(Component);
			m_Title = _strdup(Title);
			m_Message = _strdup(Message);
			m_ThreadId = ThreadId;
			m_bIsCheckpoint = false;
			m_bWriteAlways = WriteAlways;
		}

		CTraceFileEntry(unsigned int Level, DWORD ThreadId, unsigned __int64 CheckpointId)
		{
			m_Component = NULL;
			m_Message = NULL;
			m_Title = NULL;
			m_level = Level;
			m_bIsCheckpoint = true;
			m_ThreadId = ThreadId;
			m_CheckpointId = CheckpointId;
			m_bWriteAlways = false;
		}

		virtual ~CTraceFileEntry() {
			if (m_Component)
				free(m_Component);
			if (m_Title)
				free(m_Title);
			if (m_Message)
				free(m_Message);
			m_Component = NULL;
			m_Title = NULL;
			m_Message = NULL;
		}

		const TraceLevel& GetLevel() const {
			return m_level;
		}

		const char* GetComponent() const {
			return m_Component;
		}

		DWORD GetThreadId() const {
			return m_ThreadId;
		}

		const char* GetTitle() const {
			return m_Title;
		}

		const char* GetMessage() const {
			return m_Message;
		}

		bool WriteAlways() const {
			return m_bWriteAlways;
		}

		unsigned int Counter() const {
			return m_counter;
		}

		void GetSystemTime(SYSTEMTIME* lpSystemTime) const {
			memcpy(lpSystemTime, &m_SystemTime, sizeof(SYSTEMTIME));
		}

		bool IsCheckpoint(unsigned __int64& Id) const {
			if (m_bIsCheckpoint) {
				Id = m_CheckpointId;
				return true;
			}

			return false;
		}

		//void Render(char* pDest, unsigned int buf_len) const;
		void Render(CUTF8& result) const;
	};


	wchar_t* pszFilename;
	HANDLE hFile;
	CRITICAL_SECTION csAddLogEntry;
	int m_Tracelevel;
	int m_refCounter;

	unsigned __int64 m_NextCheckpointID;

	typedef struct
	{
		unsigned __int64 m_ID;
		unsigned int m_Level;
	} CHECK_POINT;

	typedef DWORD ThreadID_t;
	typedef std::vector<CHECK_POINT> CHECK_POINT_VECTOR;
	
	/** \brief Represents a queue of trace items
	 */
	typedef std::deque<CTraceFileEntry*> ENTRIES;

	/** \brief Represents a stack of queues of trace items
	 */
	typedef std::vector<ENTRIES> ENTRIES_VECTOR;
	
	/** \brief Represents a mapping assigning to each thread
	 * a stack of queues of trace items
	 */
	typedef std::map<ThreadID_t, ENTRIES_VECTOR> ENTRIES_VECTOR_MAP;

	/** \brief Represents a mapping assigning to each thread
	 * a stack of active check points
	 */
	typedef std::map<ThreadID_t, CHECK_POINT_VECTOR> CHECK_POINT_VECTOR_MAP;

	CHECK_POINT_VECTOR_MAP m_AllCheckPoints;
	ENTRIES_VECTOR_MAP m_AllThreadEntries;

	typedef struct
	{
		unsigned int EntryCounter;
	} GLOBAL_DATA;

	GLOBAL_DATA* pGlobalData;
	HANDLE hFileMapping;
public:
	static CUTF8 s_applicationTraceFileFolder;

	void InitializeCounter() {			
		TCHAR moduleFileName[32768]; moduleFileName[0]=0;
		GetModuleFileName(NULL, moduleFileName, 32768);
		_tcscat_s(moduleFileName, _T("TRACING"));
		for (int j=0; moduleFileName[j]; j++) {
			if (moduleFileName[j] == _T('\\') || moduleFileName[j] == _T(':'))
				moduleFileName[j] = _T('_');
		}
		hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(GLOBAL_DATA), moduleFileName);
		bool mustErase = false;
		DWORD d;
		if ((d = GetLastError()) == 0) {
			mustErase = true;
		}
		pGlobalData = (GLOBAL_DATA*)MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(GLOBAL_DATA));
		if (mustErase) {
			memset(pGlobalData, 0, sizeof(*pGlobalData));
		}
		Entry::m_pNextCounterValue = &pGlobalData->EntryCounter;
	}
private:
	/** \brief Returns the queue of trace entries that were created
	 * in the context of the check point on top of the check point vector
	 * of the executing thread
	 */
	ENTRIES& Entries() {
		ENTRIES_VECTOR& entries = ThreadEntries();
		return entries[entries.size()-1];
	}

	/** \brief Returns the queue of trace entries that were created
	 * in the context of the 2nd-last check point on the check point vector
	 * of the executing thread
	 */
	ENTRIES* LowerEntries() {
		ENTRIES_VECTOR& entries = ThreadEntries();
		if (entries.size() < 2)
			return NULL;
		else
			return &entries[entries.size()-2];
	}

	/** \brief Returns the vector of queues of trace entries representing
	 * the trace items created in the context of each check point by the
	 * executing thread
	 */
	ENTRIES_VECTOR& ThreadEntries() {
		ThreadID_t ThreadID = GetCurrentThreadId();
		ENTRIES_VECTOR_MAP::iterator iter = m_AllThreadEntries.find(ThreadID);
		if (iter == m_AllThreadEntries.end()) {
			ENTRIES_VECTOR newThreadEntryVector;
			m_AllThreadEntries[ThreadID] = newThreadEntryVector;
			return ThreadEntries();
		}
		return iter->second;
	}

	/** \brief Gets the stack of all check points that are active
	 *  for the executing thread
	 */
	CHECK_POINT_VECTOR&	CheckPoints() {
		ThreadID_t ThreadID = GetCurrentThreadId();
		CHECK_POINT_VECTOR_MAP::iterator iter = m_AllCheckPoints.find(ThreadID);
		if (iter == m_AllCheckPoints.end()) {
			CHECK_POINT_VECTOR newThreadEntryVector;
			m_AllCheckPoints[ThreadID] = newThreadEntryVector;
			return CheckPoints();
		}
		return iter->second;
	}
	
protected:
	void DoOpenFile(const wchar_t* fileName);
	void AddLogEntry(unsigned int minTraceLevel, unsigned int counter, const char* component, 
		const char* title, const char* message, bool ItemIsCheckpoint);
	void Flush(bool bDiscard, unsigned __int64 CheckpointID);
	int GetTraceLevelCount();
public:
	class Entry
	{
	public:
		static unsigned int* m_pNextCounterValue;
	public:
		//unsigned int m_TraceLevel;
		TraceLevel m_traceLevel;
		unsigned int m_counter;
		CUTF8 m_Component;
		CUTF8 m_Title;
		CUTF8 m_Message;
		CUTF8 m_Duration;
		Entry() {
		}

		Entry(const TraceLevel& traceLevel/*unsigned int TraceLevel*/, const CUTF8& component, const CUTF8& title, const CUTF8& message, char* duration)
		{
			m_counter = NextCounterValue();
			m_traceLevel = traceLevel;
			m_Component = (char*)component.UTF8();
			m_Title = (char*)title.UTF8();
			m_Message = (char*)message.UTF8();
			m_Duration = duration;
		}

		static unsigned int NextCounterValue() {
			if (m_pNextCounterValue)
				return (*m_pNextCounterValue)++;
			else 
				return 42;
		}
	};

	CTraceFile();
	virtual ~CTraceFile();
	void Open(const wchar_t* fileName);
	void SetTraceLevel(int trace_level) {
		if (trace_level < GetTraceLevelCount())
			m_Tracelevel = trace_level;
	}
	unsigned int GetTraceLevel() {
		return m_Tracelevel;
	}

	void Trace(unsigned int TraceLevel, const CUTF8& title, const CUTF8& message);
	void Trace(unsigned int TraceLevel, const CUTF8& component, const CUTF8& title, const CUTF8& message);
	void Trace(unsigned int TraceLevel, const CUTF8& component, const CUTF8& title, const CUTF8& message, char* duration);
//	void Trace(unsigned int TraceLevel, std::string component, std::string title, std::string message);
	void Trace(Entry TraceEntry);
	void TraceAlways(unsigned int TraceLevel, char* component, char* title, char* message);
	void TraceAlways(unsigned int TraceLevel, char* component, char* title, char* message, char* duration);
	void Close();

	unsigned __int64 SetCheckpoint(unsigned int TraceLevel);
	void PassCheckpoint();
	void IncRefCounter();
	static void SetApplicationTraceFileFolder(const CUTF8& folder);
	static const CUTF8& GetApplicationTraceFileFolder();
};

/** \brief Schnittstelle eines Parameters einer Funktion im Rahmen der Ereignisaufzeichnung
 *
 * Diese Schnittstelle definiert Funktionen, die zur Aufzeichnung von Parametern 
 * erforderlich sind.
 */
class ITraceableParam
{
public:
	virtual CUTF8 Format() const = NULL;
	virtual bool IsIn() const = NULL;
	virtual bool IsOut() const = NULL;
	virtual ITraceableParam* Clone() const = NULL;

	virtual ~ITraceableParam() {
	}
};

/** \brief Basisklasse aller formatierbarer Parameter.
 * \param TParam Typ des zu formatierenden Parameters
 *
 * Diese Klasse stellt die Basisimplementierung aller formatierbaren Parameter
 * mit einem gegebenen Typ dar. Sie definiert Funktoren, die zur Instanziierung
 * abgeleiteter Templates erforderlich sind. 
 */
template<class TParam>
class CTraceableParamBase : public ITraceableParam
{
public:
	struct param_formatter: std::unary_function<TParam, std::basic_string<TCHAR>>
	{
	public:
		virtual std::basic_string<TCHAR> operator() (TParam value) const = NULL;
	};

	struct display_value_provider_base : public std::unary_function<TParam, std::basic_string<TCHAR>>
	{
	protected:
		display_value_provider_base()
		{
		}
	public:
		virtual std::basic_string<TCHAR> operator() (TParam value) const = NULL;
	};

	/** \brief Standardformatierer für einen Parameter
	 *
	 * Dieser Funktor erzeugt eine String-Darstellung des Parameters in folgender
	 * Formatierung: [IN ]|[OUT]  <name>: <wert>
	 * Für die Ermittlung der Darstellung des Wertes des Objekts kann ein
	 * nutzerdefinierter Funktor angegeben werden.
	 */
	template<class TDisplayValueProvider>
	struct default_formatter : public param_formatter 
	{
	private:
		std::basic_string<TCHAR> m_name;
		std::basic_string<TCHAR> m_directionText;
		TDisplayValueProvider m_valueProvider;
		int m_width;
	public:
		default_formatter()
		{
		}
		
		default_formatter(
			std::basic_string<TCHAR> directionText, 
			std::basic_string<TCHAR> name, 
			TDisplayValueProvider dpv)
			: m_name(name), m_directionText(directionText), m_width(15), m_valueProvider(dpv)
		{
		}

		virtual std::basic_string<TCHAR> operator() (TParam value) const {
			std::basic_ostringstream<TCHAR> sstrResult;
			sstrResult << std::setw(10) << std::left << m_directionText;
			sstrResult << std::setw(m_width) << std::left << m_name;
			sstrResult << ": ";
			sstrResult << m_valueProvider(value);

			std::basic_string<TCHAR> result = sstrResult.str();
			return result;
		}
	};

	/** \brief Standardformatierer für einen Wert.
	 *
	 * Dieser Funktor erzeugt eine String-Darstellung eines Objekts, indem es es
	 * in einen basic_ostringstream einfügt. Er kann für alle einfachen Datentypen
	 * verwendet werden.
	 */
	struct default_display_value_provider: display_value_provider_base
	{
	public:
		/** \brief Konstruktor
		 */
		default_display_value_provider()
		{
		}
	public:
		/** \brief Auswertungsfunktion des Funktors
		 */
		virtual std::basic_string<TCHAR> operator() (TParam value) const {
			std::basic_ostringstream<TCHAR> sstrResult;
			sstrResult << value;
			return sstrResult.str();
		};
	};

	struct default_ptr_display_value_provider: display_value_provider_base
	{
	public:
		/** \brief Konstruktor
		 */
		default_ptr_display_value_provider()
		{
		}
	public:
		/** \brief Auswertungsfunktion des Funktors
		 */
		virtual std::basic_string<TCHAR> operator() (TParam value) const {
			std::basic_ostringstream<TCHAR> sstrResult;
			sstrResult << _T("*0x") << std::setfill(_T('0')) << std::setw(8) << value;
			if (value)
				sstrResult << _T(" = ") << *value;
			return sstrResult.str();
		};
	};

};

/** \brief Definiert einen formatierbaren Parameter.
 * \param TParam Typ des Parameters
 * \param TFormatter Typ des Funktors, der den Parameter formatiert
 * \param TDisplayValueProvider Typ des Funktors, der aus dem zu formatierenden
 * Objekt eine String-Darstellung des Wertes generiert
 *
 * Diese Klasse repräsentiert formatiert darstellbare Parameter, die in 
 * ENTER- oder LEAVE-Einträgen im Log ausgegeben werden können. Die
 * Formatierung der Ausgabe kann durch Angabe geeigneter Funktoren konfiguriert
 * werden.
 */
template<class TParam, class TFormatter, class TDisplayValueProvider>
class CTraceableParam : public virtual CTraceableParamBase<TParam>
{
private:
	std::basic_string<TCHAR> m_name;
	std::basic_string<TCHAR> m_direction;
	bool m_isInParam;
	bool m_isOutParam;
	TParam m_value;
	TFormatter m_formatter;
	TDisplayValueProvider p;
public:
	CTraceableParam(bool isInParam, bool isOutParam, const std::basic_string<TCHAR>& directionText, const std::basic_string<TCHAR>& name, TParam value)
		: m_name(name), m_value(value), m_isInParam(isInParam), m_isOutParam(isOutParam)
	{
		p = TDisplayValueProvider();
		m_formatter = TFormatter(directionText, name, p);
		m_direction = directionText;
	}

	CTraceableParam(
			bool isInParam, 
			bool isOutParam, 
			const std::basic_string<TCHAR>& directionText, 
			const std::basic_string<TCHAR>& name, 
			TParam value, 
			TDisplayValueProvider dpv
		) : m_name(name), m_value(value), m_isInParam(isInParam), m_isOutParam(isOutParam),
		    p(dpv)
	{
		m_formatter = TFormatter(directionText, name, p);
		m_direction = directionText;
	}

/*	CTraceableParam(bool isInParam, const std::basic_string<TCHAR>& directionText,const std::basic_string<TCHAR>& name, TParam value, std::unary_function<TParam, std::basic_string<TCHAR>> formatter)
		: m_name(name), m_value(value), m_formatter(&formatter), m_isInParam(isInParam)
	{
		p = new default_display_value_provider();
		deletep = true;
		m_direction = directionText;
		doDeleteFormatter = false;
	}
*/
	CTraceableParam(const CTraceableParam<TParam, TFormatter, TDisplayValueProvider>& other) 
		: m_name(other.m_name),
		  m_value(other.m_value),
		  m_formatter(other.m_formatter),
		  m_direction(other.m_direction),
		  m_isInParam(other.m_isInParam),
		  m_isOutParam(other.m_isOutParam),
		  p(other.p)
	{
	}

	virtual ITraceableParam* Clone() const
	{
		return new CTraceableParam<TParam, TFormatter, TDisplayValueProvider>(*this);
	}

	virtual ~CTraceableParam()
	{
	}

public:
	virtual CUTF8 Format() const
	{
		return (m_formatter)(m_value);
	}

	bool IsIn() const {
		return m_isInParam;
	}

	bool IsOut() const {
		return m_isOutParam;
	}
};

class CTraceableParamHelper
{
private:
	static CUTF8 s_inStr;
	static CUTF8 s_outStr;

public:
	template<class T>
	static  CTraceableParam<T, typename CTraceableParamBase<T>::default_formatter<typename CTraceableParamBase<T>::default_display_value_provider>, typename CTraceableParamBase<T>::default_display_value_provider> GetIn(const std::basic_string<TCHAR>& name, T value) {
		return CTraceableParam<T, typename CTraceableParamBase<T>::default_formatter<typename CTraceableParamBase<T>::default_display_value_provider>, typename CTraceableParamBase<T>::default_display_value_provider>(true, false, s_inStr.TStr(), name, value);
	}

	template<class T, class U>
	static  CTraceableParam<T, typename CTraceableParamBase<T>::default_formatter<typename U>, typename U> GetIn(const std::basic_string<TCHAR>& name, T value, U displayValueGetter) {
		return CTraceableParam<T, typename CTraceableParamBase<T>::default_formatter<typename U>, typename U>(true, false, s_inStr.TStr(), name, value, displayValueGetter);
	}

	template<class T>
	static CTraceableParam<T*, typename CTraceableParamBase<T*>::default_formatter<typename CTraceableParamBase<T*>::default_display_value_provider>, typename CTraceableParamBase<T*>::default_display_value_provider> GetOut(const std::basic_string<TCHAR>& name, T* value) {
		return CTraceableParam<T*, typename CTraceableParamBase<T*>::default_formatter<typename CTraceableParamBase<T*>::default_display_value_provider>, typename CTraceableParamBase<T*>::default_display_value_provider>(false, true, s_outStr.TStr(), name, value);
	}

	template<class T>
	static CTraceableParam<T*, typename CTraceableParamBase<T*>::default_formatter<typename CTraceableParamBase<T*>::default_ptr_display_value_provider>, typename CTraceableParamBase<T*>::default_ptr_display_value_provider> GetOutPtr(const std::basic_string<TCHAR>& name, T* value) {
		return CTraceableParam<T*, typename CTraceableParamBase<T*>::default_formatter<typename CTraceableParamBase<T*>::default_ptr_display_value_provider>, typename CTraceableParamBase<T*>::default_ptr_display_value_provider>(false, true, s_outStr.TStr(), name, value);
	}

	template<class T, class U>
	static CTraceableParam<T*, typename CTraceableParamBase<T*>::default_formatter<typename U>, typename U> GetOut(const std::basic_string<TCHAR>& name, T* value, U displayValueGetter) {
		return CTraceableParam<T*, typename CTraceableParamBase<T*>::default_formatter<typename U>, typename U>(false, true, _T("[OUT]"), name, value, displayValueGetter);
	}

	template<class T, class U>
	static CTraceableParam<T*, typename CTraceableParamBase<T*>::default_formatter<typename U>, typename U> GetInOut(const std::basic_string<TCHAR>& name, T* value, U displayValueGetter) {
		return CTraceableParam<T*, typename CTraceableParamBase<T*>::default_formatter<typename U>, typename U>(true, true, _T("[IN/OUT]"), name, value, displayValueGetter);
	}
};

class CLocalTracer
{
private:
	int m_traceLevel;
	CTraceFile* m_trace;
	CTraceFile::Entry m_enterEntry;
	CTraceFile::Entry m_leaveEntry;
	CUTF8 m_where;
	CHighPrecisionTimer timer1;

public:
	const CUTF8& Where() const {
		return m_where;
	}

	void SetWhere(const CUTF8& other) {
		m_where = other;
	}
private:
	std::vector<const ITraceableParam*> m_allParams;
	std::vector<const ITraceableParam*> m_inParams;
	std::vector<const ITraceableParam*> m_outParams;
private:
	void SetEnterLeaveEntryTraceLevel(int traceLevel)
	{
		this->m_traceLevel = traceLevel;
	}


	void Dispatch(const ITraceableParam* param) {
		ITraceableParam* copy = param->Clone();
		if (copy->IsIn())
			m_inParams.push_back(copy);
		if (copy->IsOut())
			m_outParams.push_back(copy);

		m_allParams.push_back(copy);
	}

	void GenerateEnterEntry()
	{
		std::basic_ostringstream<TCHAR> sstrMsg;

		if (!m_inParams.empty()) {
			sstrMsg << "Parameter: ";
			for each(const ITraceableParam* p in m_inParams) {
				sstrMsg << "\x0D\x0A" << "  " << p->Format().TStr();
			}
		}

		timer1.Start();
		m_enterEntry = CTraceFile::Entry(TRACE_LEVEL_DEBUG, m_where, L"ENTER", CUTF8(sstrMsg.str()), "");
	}
	
	void GenerateLeaveEntry()
	{
		timer1.Stop();
		std::basic_ostringstream<TCHAR> sstrMsg;

		if (!m_outParams.empty()) {
			sstrMsg << "Parameter: ";
			for each(const ITraceableParam* p in m_outParams) {
				sstrMsg  << "\x0D\x0A" << "  " << p->Format().TStr();
			}
		}

		std::wostringstream sstrTitle;
		sstrTitle << L"LEAVE (#" << m_enterEntry.m_counter << L")";

		m_leaveEntry = CTraceFile::Entry(TRACE_LEVEL_DEBUG, m_where, sstrTitle.str(), sstrMsg.str(), timer1);
	}
public:
	CLocalTracer(CTraceFile* trace, const CUTF8& _where)
	{ //CTraceFile::Entry enter, CTraceFile::Entry leave) {
		SetEnterLeaveEntryTraceLevel(TRACE_LEVEL_DEBUG);
		m_trace = trace;

		if (!trace)
			return;

		m_where = _where;

		m_enterEntry = CTraceFile::Entry(TRACE_LEVEL_DEBUG, m_where, L"ENTER", "", "");
		m_trace->Trace(m_enterEntry);
		timer1.Start();
	}

	CLocalTracer(CTraceFile* trace, const CUTF8& _where, int traceLevel)
	{ //CTraceFile::Entry enter, CTraceFile::Entry leave) {
		m_trace = trace;
		SetEnterLeaveEntryTraceLevel(traceLevel);

		if (!trace)
			return;

		m_where = _where;

		if (m_traceLevel > TRACE_LEVEL_NEVER)
		{
			m_enterEntry = CTraceFile::Entry(traceLevel, m_where, L"ENTER", "", "");
			m_trace->Trace(m_enterEntry);
			timer1.Start();
		}
	}

	CLocalTracer(CTraceFile* trace, const CUTF8& _where, 
			const ITraceableParam& param) 
	{
		SetEnterLeaveEntryTraceLevel(TRACE_LEVEL_DEBUG);
		m_trace = trace;
		if (!trace)
			return;
		m_where = _where;

		Dispatch(&param);
		
		GenerateEnterEntry();
		m_trace->Trace(m_enterEntry);
	}

	CLocalTracer(CTraceFile* trace, const CUTF8& _where, 
			const ITraceableParam& param1, 
			const ITraceableParam& param2) 
	{
		SetEnterLeaveEntryTraceLevel(TRACE_LEVEL_DEBUG);
		m_trace = trace;
		if (!trace)
			return;
		m_where = _where;

		Dispatch(&param1);
		Dispatch(&param2);
		
		GenerateEnterEntry();
		m_trace->Trace(m_enterEntry);
	}

	CLocalTracer(CTraceFile* trace, const CUTF8& _where, 
			const ITraceableParam& param1, 
			const ITraceableParam& param2,
			const ITraceableParam& param3) 
	{
		SetEnterLeaveEntryTraceLevel(TRACE_LEVEL_DEBUG);
		m_trace = trace;
		if (!trace)
			return;
		m_where = _where;

		Dispatch(&param1);
		Dispatch(&param2);
		Dispatch(&param3);
		
		GenerateEnterEntry();
		m_trace->Trace(m_enterEntry);
	}

	CLocalTracer(CTraceFile* trace, const CUTF8& _where, 
			const ITraceableParam& param1, 
			const ITraceableParam& param2,
			const ITraceableParam& param3,
			const ITraceableParam& param4
			) 
	{
		SetEnterLeaveEntryTraceLevel(TRACE_LEVEL_DEBUG);
		m_trace = trace;
		if (!trace)
			return;
		m_where = _where;

		Dispatch(&param1);
		Dispatch(&param2);
		Dispatch(&param3);
		Dispatch(&param4);
		
		GenerateEnterEntry();
		m_trace->Trace(m_enterEntry);
	}

	CLocalTracer(CTraceFile* trace, const CUTF8& _where, 
		const ITraceableParam& param1, 
		const ITraceableParam& param2,
		const ITraceableParam& param3,
		const ITraceableParam& param4,
		const ITraceableParam& param5) 
	{
		SetEnterLeaveEntryTraceLevel(TRACE_LEVEL_DEBUG);
		m_trace = trace;
		if (!trace)
			return;
		m_where = _where;

		Dispatch(&param1);
		Dispatch(&param2);
		Dispatch(&param3);
		Dispatch(&param4);
		Dispatch(&param5);
		
		GenerateEnterEntry();
		m_trace->Trace(m_enterEntry);
	}

	CLocalTracer(CTraceFile* trace, const CUTF8& _where, 
		const ITraceableParam& param1, 
		const ITraceableParam& param2,
		const ITraceableParam& param3,
		const ITraceableParam& param4,
		const ITraceableParam& param5,
		const ITraceableParam& param6) 
	{
		SetEnterLeaveEntryTraceLevel(TRACE_LEVEL_DEBUG);
		m_trace = trace;
		if (!trace)
			return;
		m_where = _where;

		Dispatch(&param1);
		Dispatch(&param2);
		Dispatch(&param3);
		Dispatch(&param4);
		Dispatch(&param5);
		Dispatch(&param6);
		
		GenerateEnterEntry();
		m_trace->Trace(m_enterEntry);
	}

	void Trace(unsigned int TraceLevel, const CUTF8& title, const CUTF8& message) {
		if (!m_trace)
			return;
		m_trace->Trace(CTraceFile::Entry(TraceLevel, m_where, title, message, NULL)); 
	}

	virtual ~CLocalTracer() {
		if (m_traceLevel > TRACE_LEVEL_NEVER)
		{
			GenerateLeaveEntry();
			if (m_trace)
				m_trace->Trace(m_leaveEntry);
		}

		for each(const ITraceableParam* p in m_allParams) {
			delete p;
		}
	}
};

class CFormatHelper
{
private:
public:
	template<class TData>
	static std::wstring FormatSimple(
		const std::wstring format,
		const TData& data1)
	{
		std::wstring result;
		std::wstring::size_type size = format.size();

		for (std::wstring::size_type j=0; j<size; j++)
		{
			wchar_t c = format[j];
			if (c != L'%')
			{
				result.append(1, c);
			}
			else
			{
				if (j+1 < size)
				{
					if (format[j+1] == L'%')
					{
						result.append(1, L'%');
					}
					else
					{
						if (format[j+1] == L'1')
						{
							std::wostringstream sstr;
							sstr << data1;
							result.append(sstr.str());
						}
					}
				}
				j++;
			}
		}
		return result;
	}
};

CTraceFile* GetApplicationTraceFile();
void _stdcall SetApplicationTraceFile(CTraceFile* traceFile);

#endif