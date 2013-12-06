#include "stdafx.h"
#include "LanguageCodes.h"
#include "../Strings.h"

LANGUAGE_CODE_DESCRIPTOR::LANGUAGE_CODE_DESCRIPTOR()
{
	memset(code, 0, sizeof(code));
	memset(full, 0, sizeof(full));
	priority = 0;
	usage_counter = 0.;
	bSeparator = false;
}

LANGUAGE_CODE_DESCRIPTOR::~LANGUAGE_CODE_DESCRIPTOR()
{

}

void LANGUAGE_CODE_DESCRIPTOR::IncPriority()
{
	priority++;
}

void LANGUAGE_CODE_DESCRIPTOR::DecPriority()
{
	if (priority > 0)
		priority--;
}

void LANGUAGE_CODE_DESCRIPTOR::IncUsageCounter()
{
	usage_counter += 1;
}

void LANGUAGE_CODE_DESCRIPTOR::DecUsageCounter()
{
	if (usage_counter > 0)
		usage_counter -= 1;
}

void LANGUAGE_CODE_DESCRIPTOR::AgeUsageCounter()
{
	usage_counter /= 1.1f;
}

void LANGUAGE_CODE_DESCRIPTOR::SetUsageCounter(float count)
{
	usage_counter = count;
}

void LANGUAGE_CODE_DESCRIPTOR::SetCode(char* c)
{
	memset(code, 0, sizeof(code));
	strncpy(code, c, sizeof(code)-1);
}

void LANGUAGE_CODE_DESCRIPTOR::SetFull(char* c)
{
	memset(full, 0, sizeof(full));
	strncpy(full, c, sizeof(full)-1);
}

char* LANGUAGE_CODE_DESCRIPTOR::GetCode()
{
	return code;
}

char* LANGUAGE_CODE_DESCRIPTOR::GetFull()
{
	return full;
}

void LANGUAGE_CODE_DESCRIPTOR::Separator(int _priority)
{
	bSeparator = true;
	SetPriority(_priority);
	usage_counter = -1.;
}

void LANGUAGE_CODE_DESCRIPTOR::SetPriority(int _priority)
{
	priority = _priority;
}

bool LANGUAGE_CODE_DESCRIPTOR::operator <(const LANGUAGE_CODE_DESCRIPTOR& other)
{
	if (priority < other.priority || priority == other.priority && usage_counter < other.usage_counter)
		return true;

	return false;
}

void LANGUAGE_CODE_DESCRIPTOR::CreateString(char* buf, int buf_len)
{
	_snprintf(buf, buf_len, "%s:%s:%1.4f:%d", code, full, usage_counter, priority);
	buf[buf_len-1] = 0;
}

bool LANGUAGE_CODE_DESCRIPTOR::LoadFromString(char* buf)
{
	
	std::vector<char*> in;

	split_string(buf, ":", in);
	if (in.size() != 4) {
		DeleteStringVector(in);
		return false;
	}

	char* _code = in[0];
	int code_len = strlen(_code);
	char* _full = in[1];
	char* _usage_counter = in[2];
	char* _priority = in[3];

	if (!isint(_priority) || code_len > 4) {
		DeleteStringVector(in);
		return false;
	}

	priority = atoi(_priority);
	strcpy(code, _code);
	strcpy(full, _full);
	usage_counter = (float)atof(_usage_counter);

	DeleteStringVector(in);

	return true;
}

LANGUAGE_CODES::LANGUAGE_CODES()
{
}

LANGUAGE_CODES::~LANGUAGE_CODES()
{
}

int LANGUAGE_CODES::CreateString(char* buf, int buf_len)
{
	std::vector<LANGUAGE_CODE_DESCRIPTOR>::iterator iter = codes.begin();
	int pos = 0;

	for (; iter != codes.end(); iter++) {
		iter->CreateString(buf + pos, buf_len - pos);
        pos += strlen(buf + pos);
		buf[pos++] = 13;
		buf[pos++] = 10;
	}

	return pos;
}

int LANGUAGE_CODES::LoadFromString(char* buf)
{
	std::vector<char*> lines;
	std::vector<char*>::iterator line;
	split_string(buf, "\x0A", lines);
	int j = 1;

	for (line = lines.begin(); line != lines.end(); line++) {
		if (strlen(*line) > 3) {
			LANGUAGE_CODE_DESCRIPTOR lcd;
			if ((*line)[strlen(*line)-1] == 13)
				(*line)[strlen(*line)-1] = 0;
			if (!lcd.LoadFromString(*line)) {
				DeleteStringVector(lines);
				return -j;
			}
			codes.push_back(lcd);
		}
		j++;
	}

	DeleteStringVector(lines);

	return 1;
}

int LANGUAGE_CODES::GetCount()
{
	return codes.size();
}

char* LANGUAGE_CODES::GetCode(int index)
{
	if (index >= 0 && (size_t)index < codes.size()) {
		return codes[index].GetCode();
	}
	return NULL;
}

char* LANGUAGE_CODES::GetFullName(int index)
{
	if (index >= 0 && (size_t)index < codes.size()) {
		return codes[index].GetFull();
	}
	return NULL;
}

LANGUAGE_CODES* language_codes = NULL;

LANGUAGE_CODES* GetLanguageCodesObject()
{
	if (!language_codes)
		language_codes = new LANGUAGE_CODES;

	return language_codes;
}