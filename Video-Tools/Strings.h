#ifndef I_STRINGS
#define I_STRINGS

#include "utf-8.h"
#include <string>
#include <algorithm>
#include <vector>

char* ucase(char* s,char* d) ;
char* getword(char** s);
bool isint(const char* s);
bool isposint(char* s);

//void splitpathname(const char* p, char** f, char** e, char** cPath = NULL);

template<class T>
void splitpathname(const std::basic_string<T>& fullPath,
				   std::basic_string<T>& fileName,
				   std::basic_string<T>& fileNameExtension,
				   std::basic_string<T>& filePath);



int split_string(char* in, char* separator, std::vector<char*>& dest);
void DeleteStringVector(std::vector<char*>& v);

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

template<class T>
std::basic_string<T> string_replace_first(
	const std::basic_string<T>& input, const std::basic_string<T>& toReplace,
	const std::basic_string<T>& replaceWith)
{
	std::basic_string<T>::size_type posOfSubstr = input.find(toReplace);
	if (posOfSubstr != std::basic_string<T>::npos)
	{
		std::basic_string<T> result = input;
		result.replace(posOfSubstr, toReplace.size(), replaceWith);
		return result;
	}

	return input;
}

#endif