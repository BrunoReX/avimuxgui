#pragma once

#include <stdexcept>
#include <string>
#include "UTF-8.h"

class io_exception : public std::runtime_error
{
public:
	ioexception(const CUTF8& message)
	{
		m_message = message;
	}

	virtual const char *__CLR_OR_THIS_CALL what()
	{	// return pointer to message string
		return (m_message);
	}

private:
	CUTF8 m_message;
};

class end_of_stream : public io_exception
{
public:
	end_of_stream(const CUTF8& message)
		: io_exception(message)
	{
	}

	virtual ~end_of_stream()
	{
	}
};