#ifndef I_LANGUAGE_CODES
#define I_LANGUAGE_CODES

#include <vector>

class LANGUAGE_CODE_DESCRIPTOR
{
private:
	char	code[8];
	char	full[1024];
	float	usage_counter;
	int		priority;
	bool	bSeparator;
public:
	LANGUAGE_CODE_DESCRIPTOR();
	virtual ~LANGUAGE_CODE_DESCRIPTOR();
	bool operator<(const LANGUAGE_CODE_DESCRIPTOR& other); 
	void SetCode(char* c);
	void SetFull(char* c);
	char* GetCode();
	char* GetFull();
	void SetUsageCounter(float count);
	void AgeUsageCounter();
	void IncUsageCounter();
	void DecUsageCounter();
	void DecPriority();
	void IncPriority();
	void SetPriority(int priority);
	void Separator(int priority);
	void CreateString(char* buf, int buf_len);
	bool LoadFromString(char* buf);
};

class LANGUAGE_CODES
{
private:
	std::vector<LANGUAGE_CODE_DESCRIPTOR>	codes;
public:
	int CreateString(char* buf, int buf_len);
	int LoadFromString(char* buf);
	int GetCount();
	char* GetCode(int index);
	char* GetFullName(int index);
	LANGUAGE_CODES();
	virtual ~LANGUAGE_CODES();
};

LANGUAGE_CODES* GetLanguageCodesObject();

#endif