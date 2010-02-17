#include "stdafx.h"
#include "strings.h"

// upper case
char* ucase(char* s,char* d) 
{
	char* t = d;
	while (*s) {
		if (*s >= 'a' && *s <= 'z') *d++ = *s -32; 
		  else if (*s == (int)'ü') *d++ = 'Ü';
		  else if (*s == (int)'ö') *d++ = 'Ö';
		  else if (*s == (int)'a') *d++ = 'Ä';
		  else if (*s == (int)'ß') { *d++ = 'S'; *d++ = 'S'; } else *d++ = *s;
		s++;
	}
	*d = 0;
	return t;
}

// find next word in string, set *s to beginning of the next word and return the old *s
char* getword(char** s)
{
	while (**s == ' ') (*s)++;
	char* r = *s;

	while (**s && **s != ' ') (*s)++;
	*(*s)++ = '\0';
	while (**s && **s == ' ') (*s)++;
	return r;
}

// check if string is a number
bool isint(const char* s)
{
	if (!s || !*s) return false;
	if (*s != '-' && (*s<'0' || *s>'9')) return false;
	s++;
	while (*s) {
		if (*s < '0' || *s > '9') return false;
		s++;
	}
	return true;
}

// check if string is a positive number
bool isposint(char* s)
{
	if (!s || !*s) return false;
	while (*s) {
		if (*s < '0' || *s > '9') return false;
		s++;
	}
	return true;
}

template<>
void splitpathname<char>(const std::string& fullPath,
						  std::string& fileName,
						  std::string& fileNameExtension,
						  std::string& filePath)
{
	char pathSeperator = '\\';
	char extensionSeperator = '.';


	size_t last_backslash_pos = fullPath.find_last_of(pathSeperator);
	if (last_backslash_pos != std::string::npos)
	{
		filePath = fullPath.substr(0, last_backslash_pos);
		fileName = fullPath.substr(last_backslash_pos + 1);
		size_t last_dot_pos = fileName.find_last_of(extensionSeperator);
		if (last_dot_pos != std::string::npos)
		{
			fileNameExtension = fileName.substr(last_dot_pos + 1);
		}
		else
		{
			fileNameExtension = "";
		}
	}
}

template<>
void splitpathname<wchar_t>(const std::wstring& fullPath,
						  std::wstring& fileName,
						  std::wstring& fileNameExtension,
						  std::wstring& filePath)
{
	wchar_t pathSeperator = L'\\';
	wchar_t extensionSeperator = L'.';


	size_t last_backslash_pos = fullPath.find_last_of(pathSeperator);
	if (last_backslash_pos != std::wstring::npos)
	{
		filePath = fullPath.substr(0, last_backslash_pos);
		fileName = fullPath.substr(last_backslash_pos + 1);
		size_t last_dot_pos = fileName.find_last_of(extensionSeperator);
		if (last_dot_pos != std::wstring::npos)
		{
			fileNameExtension = fileName.substr(last_dot_pos + 1);
		}
		else
		{
			fileNameExtension = L"";
		}
	}
}


/** \brief Splits a full path into components.
 */
int split_string(char* in, char* separator, std::vector<char*>& dest)
{
	char* c = (char*)calloc(sizeof(char), 1+strlen(in));
	char* d = c;
	char* e;
	int sep_len = (int)strlen(separator);
	strcpy(c, in);

	int i = 0;

	do  {
		e = strstr(c, separator);
		if (e) for (int j=0;j<sep_len;j++)
			*e++ = 0;

		dest.push_back(_strdup(c));
		c = e;
//		in = c;
		i++;
	} while (c);

	free(d);

	return i;
}

void DeleteStringVector(std::vector<char*>& v)
{
	std::vector<char*>::iterator iter = v.begin();
	for (; iter != v.end(); delete (*iter++));
}

