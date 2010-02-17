#include "stdafx.h"
#include "TraceLevel.h"

static char* TRACE_LEVEL_NAMES[] = {
	"", "", "DEBUG ", "INFO  ", "NOTE  ", "WARN  ", "ERROR ", "ASSERT", "FATAL "
};

static unsigned int MaxTraceLevel = sizeof(TRACE_LEVEL_NAMES) / sizeof(TRACE_LEVEL_NAMES[0]) - 1;

const char* TraceLevel::GetText() const
{
	if ((unsigned int)m_value >= MaxTraceLevel)
		return "<undefined level>";

	return TRACE_LEVEL_NAMES[m_value];
}