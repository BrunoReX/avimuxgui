#include "stdafx.h"
#include "Filenames.h"
#include "UnicodeCalls.h"


/* the input buffer must allow write access ! */
int Filename2LongFilename(char* in, char* out, int out_buf_len)
{
	bool bUNC = false;
	bool bDropFirstComponent = false;
	bool bBeginWithBackslash = false;
	char* org_in = in;
	char* org_out = out;
	out[0]=0;
	int in_buf_len = strlen(in) + 1;
	
	char new_in[65536]; 
	new_in[0]=0;


	if (!strncmp(in, "\\\\?\\", 4)) {
		in += 4;
		if (!strncmp(in, "UNC", 3)) {
			in += 2;
			*in++ = '\\';
			*in++ = '\\';
		}
/*		if (out_buf_len >= in_buf_len)
			strcpy(out, in);
		else
			strncpy(out, in, in_buf_len);

		return in_buf_len;
*/	}

	if (!strncmp(in, "\\\\", 2)) {
		strcpy(out, "\\\\?\\UNC\\");
		out += strlen(out);
		in += 2;
		strcpy(new_in, in);
		bUNC = true;
	} else {
		strcpy(out, "\\\\?\\");
		out += strlen(out);

		wchar_t c[32768];
		(*UGetCurrentDirectory())(32768, c);
           char* udir = NULL;
		toUTF8(c, &udir);
		if (in[0] == '\\') {
			// absolute path on current drive
			strncpy(out, udir, 2);
			out += 2;
			bBeginWithBackslash = true;
			in ++;
			strcpy(new_in, in);
		} else {
			if (in[1] == ':') {
				// absolute path with drive letter
				strcpy(new_in, in);
			} else {
	            strcat(new_in, udir);
				strcat(new_in, "\\");
				strcat(new_in, in);
			}
		}
		free(udir);
	}

	std::vector<char*> in_components;
	std::deque<char*> out_components;

	split_string(new_in, "\\", in_components);

	std::vector<char*>::iterator iter;
	for (iter = in_components.begin(); iter != in_components.end(); iter++) {
		char* component = *iter;
		// slash means nothing at all
		if (strcmp(component, ".")) {
			// path up
			if (!strcmp(component, "..")) {
				if (!out_components.empty())
					out_components.pop_back();
				else {
					// b0rked!!!
				}
			} else {
				out_components.push_back(component);
			}
		}
	}

	std::deque<char*>::iterator iter_out;
	for (iter_out = out_components.begin(); iter_out != out_components.end(); iter_out++) {
		if (bBeginWithBackslash || iter_out != out_components.begin())
			strcat(out, "\\");
		strcat(out, *iter_out);
	}

	DeleteStringVector(in_components);

	return 0;
}

void FilenameKeepName(char* in_out)
{
	char* cbegin = in_out;
	char* clast_dot = NULL;
	char* clast_backslash = NULL;

	while (*in_out) {
		if (*in_out == '.')
			clast_dot = in_out;
		if (*in_out == '\\')
			clast_backslash = in_out;
		in_out++;
	}

	if (clast_dot)
		*clast_dot = 0;

	if (clast_backslash)
		strcpy(cbegin, clast_backslash+1);
}

int UTF8CharLen(char in)
{
	unsigned char uin = (unsigned char)in;

	if (uin < 128)
		return 1;

	if (uin < 192)
		return -1;

	if (uin < 0xE0)
		return 2;

	if (uin < 0xF0)
		return 3;

	if (uin < 0xF8)
		return 4;

	if (uin < 0xFC)
		return 5;

	if (uin < 0xFE)
		return 6;

	if (uin < 0xFF)
		return 7;

	return 8;
}


void FilenameRemoveIllegalCharactersUTF8(char* in_out, char replacement)
{
	while (*in_out) {
		int cl = UTF8CharLen(*in_out);
		if (cl < 1)
			return;

		if (cl == 1) {
			if (*in_out == '\\' || *in_out == '"' || *in_out == '?' ||
				*in_out == '/' || *in_out == '<' || *in_out == '>' || 
				*in_out == ':' || *in_out == '*' || *in_out == '|')
				*in_out = replacement;
		}

		in_out += cl;
	}
}