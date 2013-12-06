#ifndef I_STRINGS
#define I_STRINGS

#include "utf-8.h"

#include <vector>

char* ucase(char* s,char* d) ;
char* getword(char** s);
bool isint(const char* s);
bool isposint(char* s);

void splitpathname(char* p, char** f, char** e, char** cPath = NULL);
int split_string(char* in, char* separator, std::vector<char*>& dest);
void DeleteStringVector(std::vector<char*>& v);

class CASCIIString;
class CUTF8String;

class CBaseString
{
private:
	std::string m_data;
public:
	const std::string& Data() const
	{
		return m_data;
	}

	void SetData(char* source)
	{
		m_data = source;
	}
};

class CASCIIString : public CBaseString
{

public:
	CASCIIString();

	CASCIIString(char* source);
	CASCIIString(const CUTF8String& source);
};

class CUTF8String : public CBaseString
{
private:
	std::string m_data;
public:
	CUTF8String(const CASCIIString& source);
};

class CUTF16String : public CBaseString
{
};

#endif