#pragma once

#include <string>

#define TRACE_LEVEL_NEVER 0

#define TRACE_LEVEL_DEBUG 2
#define TRACE_LEVEL_INFO  3
#define TRACE_LEVEL_NOTE  4
#define TRACE_LEVEL_WARN  5
#define TRACE_LEVEL_ERROR 6
#define TRACE_LEVEL_ASSERT 7
#define TRACE_LEVEL_FATAL 8
#define TRACE_LEVEL_NONE 9

class TraceLevel
{
public:
	enum TraceLevels {
		Debug = TRACE_LEVEL_DEBUG,
		Info = TRACE_LEVEL_INFO,
		Note = TRACE_LEVEL_NOTE,
		Warn = TRACE_LEVEL_WARN,
		Error = TRACE_LEVEL_ERROR,
		Assert = TRACE_LEVEL_ASSERT,
		Fatal = TRACE_LEVEL_FATAL,
		None = TRACE_LEVEL_NONE
	};

private:
	TraceLevels m_value;

public:
	TraceLevel()
		: m_value(static_cast<TraceLevels>(-1))
	{
	}

	TraceLevel(unsigned int value)
		: m_value(static_cast<TraceLevels>(value))
	{
	}

	virtual ~TraceLevel()
	{
	}

	operator unsigned int() const
	{
		return m_value;
	}

	const char* GetText() const;
};